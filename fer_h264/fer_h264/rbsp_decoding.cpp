#include "nal.h"
#include "headers_and_parameter_sets.h"
#include "residual.h"
#include "expgolomb.h"
#include "h264_globals.h"
#include "rawreader.h"
#include "intra.h"
#include "inttransform.h"
#include "fileIO.h"
#include "ref_frames.h"
#include "mocomp.h"
#include "mode_pred.h"
#include "quantizationTransform.h"

void RBSP_decode(NALunit nal_unit)
{
	static int nalBrojac=0;
	static int idr_frame_number=0;

	printf("Entering RBPS_decode #%d\n",nalBrojac++);

	initRawReader(nal_unit.rbsp_byte, nal_unit.NumBytesInRBSP);

	//TYPE 7 = Sequence parameter set TODO: Provjera postoji li veæ SPS
	//READ SPS

	if (nal_unit.nal_unit_type==NAL_UNIT_TYPE_SEI)
	{
		printf("RBSP_decode -> Not supported NAL unit type: SEI (type 6)\n");
	}
	else if (nal_unit.nal_unit_type==NAL_UNIT_TYPE_SPS)
	{
		fill_sps(&nal_unit);
		init_h264_structures();
	}
	//TYPE 8 = Picture parameter set TODO: Provjera postoji li veæ PPS i SPS
	else if (nal_unit.nal_unit_type==NAL_UNIT_TYPE_PPS)
	{
		fill_pps(&nal_unit);
	}

	//IDR or NOT IDR slice data
	////////////////////////////////////////////////////////////////////
	//Actual picture decoding takes place here
	//The main loop works macroblock by macroblock until the end of the slice
	//Macroblock skipping is implemented
	else if ((nal_unit.nal_unit_type==NAL_UNIT_TYPE_IDR) || (nal_unit.nal_unit_type==NAL_UNIT_TYPE_NOT_IDR))
	{
		// TEST: tu postavi frejm od kojeg želiš poèet ispisivanje
		// u ppm. Indeksi su isti ko u h264visi.
		frameCount++;
		if (frameCount < 109)
		{	
			//frameCount++;
			return;
		}

		//Read slice header
		fill_shd(&nal_unit);

		printf("Working on frame #%d...\n", frameCount);

		int MbCount=shd.PicSizeInMbs;

		//Norm: firstMbAddr=first_mb_in_slice * ( 1 + MbaffFrameFlag );
		int firstMbAddr = 0;

		CurrMbAddr = firstMbAddr;

		//Norm: moreDataFlag = 1
		bool moreDataFlag = true;

		//Norm: prevMbSkipped = 0
		int prevMbSkipped = 0;

		//Used later on
		int mb_skip_run;

		// Prediction samples formed by either intra or inter prediction.
		int predL[16][16], predCb[8][8], predCr[8][8];

		QPy = shd.SliceQPy;
		while (moreDataFlag && CurrMbAddr<MbCount)
		{
			if ((shd.slice_type%5)!=I_SLICE && (shd.slice_type%5)!=SI_SLICE)
			{

				//First decode various data at the beggining of each slice/frame

				//Norm: if( !entropy_coding_mode_flag ) ... this "if clause" is skipped.
				mb_skip_run=expGolomb_UD();
				prevMbSkipped = (mb_skip_run > 0);
				for(int i=0; i<mb_skip_run; i++ )
				{
					if (CurrMbAddr >= MbCount)
					{
						break;
					}
					
					mb_type = P_Skip;
					mb_type_array[CurrMbAddr]=P_Skip;

					// Inter prediction:
					DeriveMVs();
					Decode(predL, predCr, predCb);

					// Norm: QpBdOffsetY == 0 in baseline
					QPy = (QPy + mb_qp_delta + 52) % 52;

					// Inverse transformation and decoded sample construction:
					transformDecodingP_Skip(predL, predCb, predCr, QPy);

					//Norm: CurrMbAddr = NextMbAddress( CurrMbAddr )
					CurrMbAddr++;
				}
				
				if ((CurrMbAddr != firstMbAddr) || (mb_skip_run > 0))
				{
					moreDataFlag = more_rbsp_data();
				}
			}

			if(moreDataFlag)
			{ 
				// Norm: start macroblock_layer()
				mb_pos_array[CurrMbAddr]=(RBSP_current_bit+1)%8;

				mb_type = expGolomb_UD();
				mb_type_array[CurrMbAddr]=mb_type;
				

				//Transform macroblock-level coordinates to pixel-level coordinates
				int pixel_pos_x=mb_pos_x*16;
				int pixel_pos_y=mb_pos_y*16;

				//Contains sub_mb_type for each 8x8 submacroblock for inter prediction.
				int sub_mb_type_array[4];

				//Norm: if( mb_type != I_NxN && MbPartPredMode( mb_type, 0 ) != Intra_16x16 && NumMbPart( mb_type ) == 4 )
				// I_NxN is an alias for Intra_4x4 and Intra_8x8 MbPartPredMode (mb_type in both cases equal to 0)
				// mb_type (positive integer value) is equal to "Name of mb_type" (i.e. I_NxN). These are often interchanged in the norm
				// Everything as described in norm page 119. table 7-11.

				//Specific inter prediction?
				// Norm: if (mb_type != I_NxN && MbPartPredMode(mb_type,0) != Intra_16x16 && NumMbPart(mb_type) == 4)
				if(MbPartPredMode(mb_type,0) != Intra_4x4 && MbPartPredMode( mb_type, 0 )!=Intra_16x16 && NumMbPart( mb_type )==4 )
				{
					// Norm: start sub_mb_pred(mb_type)
					int mbPartIdx;
					for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
					{
						sub_mb_type_array[mbPartIdx]=expGolomb_UD();
					}

					for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
					{
						if ((shd.num_ref_idx_active_override_flag > 0) &&
							(mb_type != P_8x8ref0) &&
							(SubMbPredMode(sub_mb_type_array[mbPartIdx]) != Pred_L1))
						{
							ref_idx_l0_array[CurrMbAddr][mbPartIdx] = expGolomb_TD();
						}
					}

					// Norm: there are no B-frames in baseline, so the stream
					// does not contain ref_idx_l1 or mvd_l1

					for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
					{
						for (int subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(sub_mb_type_array[mbPartIdx]); subMbPartIdx++)
						{
							mvd_l0[mbPartIdx][subMbPartIdx][0] = expGolomb_SD();
							mvd_l0[mbPartIdx][subMbPartIdx][1] = expGolomb_SD();
						}
					}
					// Norm: end sub_mb_pred(mb_type)
				}
				else
				{
					//Norm:	This is section "mb_pred( mb_type )"
					if(MbPartPredMode(mb_type, 0) == Intra_4x4 /*|| MbPartPredMode(mb_type, 0) == Intra_8x8*/ || MbPartPredMode(mb_type, 0) == Intra_16x16 )
					{
						if(MbPartPredMode(mb_type, 0) == Intra_4x4)
						{
							for(int luma4x4BlkIdx=0; luma4x4BlkIdx<16; luma4x4BlkIdx++)
							{
								prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = (bool)getRawBits(1);
								if(prev_intra4x4_pred_mode_flag[luma4x4BlkIdx]==false)
								{
									rem_intra4x4_pred_mode[luma4x4BlkIdx]=getRawBits(3);
								}
							}
						}

						//Norm:
						//if( MbPartPredMode( mb_type, 0 ) = = Intra_8x8 )
						//This if clause has been skipped, because "intra_8x8" is not supported in baseline.

						//Norm:
						//if( ChromaArrayType == 1 || ChromaArrayType == 2 )
						//baseline defines "ChromaArrayType==1", so the if clause is skipped

						intra_chroma_pred_mode=expGolomb_UD();
					}
					else
					{
						int mbPartIdx;
						for (mbPartIdx = 0; mbPartIdx < NumMbPart(mb_type); mbPartIdx++)
						{
							if ((shd.num_ref_idx_l0_active_minus1 > 0) &&
								(MbPartPredMode(mb_type, mbPartIdx) != Pred_L1))
							{
								ref_idx_l0_array[CurrMbAddr][mbPartIdx] = expGolomb_TD();
							}
						}

						// Norm: there are no B-frames in baseline, so the stream
						// does not contain ref_idx_l1 or mvd_l1

						for (mbPartIdx = 0; mbPartIdx < NumMbPart(mb_type); ++mbPartIdx)
						{
							if (MbPartPredMode(mb_type, mbPartIdx) != Pred_L1)
							{
								mvd_l0[mbPartIdx][0][0] = expGolomb_SD();	
								mvd_l0[mbPartIdx][0][1] = expGolomb_SD();
							}
						}
					}
					// Norm: end mb_pred(mb_type)
				}

				//If the next if clause does not execute, this is the final value of coded block patterns for this macroblock
				CodedBlockPatternLuma=-1;
				CodedBlockPatternChroma=-1;

				if(MbPartPredMode(mb_type,0)!=Intra_16x16)
				{
					int coded_block_pattern=expGolomb_UD();

					//This is not real coded_block_pattern, it's the coded "codeNum" value which is now being decoded:

					if(MbPartPredMode(mb_type,0)==Intra_4x4 /*|| MbPartPredMode(mb_type,0)==Intra_8x8*/ )
					{
						coded_block_pattern=codeNum_to_coded_block_pattern_intra[coded_block_pattern];
					}
					//Inter prediction
					else
					{
						coded_block_pattern=codeNum_to_coded_block_pattern_inter[coded_block_pattern];
					}

					CodedBlockPatternLuma=coded_block_pattern%16;
					CodedBlockPatternChroma=coded_block_pattern/16;

					//Norm:
					/*
					if( CodedBlockPatternLuma > 0 && transform_8x8_mode_flag && mb_type != I_NxN && noSubMbPartSizeLessThan8x8Flag &&
					( mb_type != B_Direct_16x16 || direct_8x8_inference_flag))
					*/
					//This if clause is not implemented since transform_8x8_mode_flag is not supported
				}

				//DOES NOT EXIST IN THE NORM!
				else
				{
					if (shd.slice_type % 5 == I_SLICE)
					{
						CodedBlockPatternChroma=I_Macroblock_Modes[mb_type][5];
						CodedBlockPatternLuma=I_Macroblock_Modes[mb_type][6];
					}
					else
					{
						CodedBlockPatternChroma=P_and_SP_macroblock_modes[mb_type][5];
						CodedBlockPatternLuma=P_and_SP_macroblock_modes[mb_type][6];
					}
				}				

				CodedBlockPatternLumaArray[CurrMbAddr] = CodedBlockPatternLuma;
				CodedBlockPatternChromaArray[CurrMbAddr] = CodedBlockPatternChroma;

				if(CodedBlockPatternLuma>0 || CodedBlockPatternChroma>0 || MbPartPredMode(mb_type,0)==Intra_16x16)
				{

					mb_qp_delta=expGolomb_SD();

					//Norm: decode residual data.
					//residual_block_cavlc( coeffLevel, startIdx, endIdx, maxNumCoeff )

					residual(0, 15);
				}
				else
				{
					clear_residual_structures();
				}
				// Norm: end macroblock_layer()

				//Data ready for rendering			
			
				// Norm: QpBdOffsetY == 0 in baseline
				QPy = (QPy + mb_qp_delta + 52) % 52;

				if ((MbPartPredMode(mb_type , 0) == Intra_4x4) || (MbPartPredMode(mb_type , 0) == Intra_16x16))
				{
					intraPrediction(predL, predCr, predCb);
				}
				else
				{
					DeriveMVs();
					Decode(predL, predCr, predCb);
				}

				if (MbPartPredMode(mb_type, 0) == Intra_16x16)
				{
					transformDecodingIntra_16x16Luma(Intra16x16DCLevel, Intra16x16ACLevel, predL, QPy);
				}
				else if (MbPartPredMode(mb_type, 0) != Intra_4x4)	// Intra
				{
					for(int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
					{
						transformDecoding4x4LumaResidual(LumaLevel, predL, luma4x4BlkIdx, QPy);
					}
				}
				transformDecodingChroma(ChromaDCLevel[0], ChromaACLevel[0], predCb, QPy, true);
				transformDecodingChroma(ChromaDCLevel[1], ChromaACLevel[1], predCr, QPy, false);

				moreDataFlag=more_rbsp_data();
				++CurrMbAddr;
			}
		}

		if (shd.slice_type%5 == I_SLICE)
		{
			// Reference frame list initialisation
			initialisationProcess();
			idr_frame_number++;
		}

		// Reference frame list modification
		modificationProcess();
		
		//writeToPPM();
		writeToY4M();
	}
}




// ENCODING:

// 7.3.2.11
void RBSP_trailing_bits()
{
	writeOnes(1);
	if (RBSP_current_bit != 0)
	{
		writeZeros(8 - RBSP_current_bit);
	}
}

void RBSP_encode(NALunit &nal_unit)
{
	initRawWriter(nal_unit.rbsp_byte, 500000);

	if (nal_unit.nal_unit_type == NAL_UNIT_TYPE_SPS)
	{
		sps_write();
		init_h264_structures_encoder();
		RBSP_trailing_bits();
		nal_unit.NumBytesInRBSP = RBSP_current_byte;
	}
	else if (nal_unit.nal_unit_type == NAL_UNIT_TYPE_PPS)
	{
		pps_write();
		RBSP_trailing_bits();
		nal_unit.NumBytesInRBSP = RBSP_current_byte;
		// TEST: ovo stoji samo dok imamo samo intra frejmove
		shd.idr_pic_id = 0;
	}
	else if ((nal_unit.nal_unit_type == NAL_UNIT_TYPE_IDR) || (nal_unit.nal_unit_type == NAL_UNIT_TYPE_NOT_IDR))
	{
		if (nal_unit.nal_unit_type == NAL_UNIT_TYPE_IDR)
		{
			shd.slice_type = I_SLICE;
			shd.frame_num = 0;
			// TEST: ovo stoji samo dok imamo samo intra frejmove
			shd.idr_pic_id++;
		}
		else
		{
			shd.slice_type = P_SLICE;
			shd.frame_num++;
		}

		shd_write(nal_unit);

		int predL[16][16], predCb[8][8], predCr[8][8];
		int intra16x16PredMode;
		for (CurrMbAddr = 0; CurrMbAddr < shd.PicSizeInMbs; CurrMbAddr++)
		{
			intra16x16PredMode = intraPredictionEncoding(predL, predCr, predCb);

			// intra4x4 prediction
			if (intra16x16PredMode == -1)
			{
				mb_type = I_4x4;
				mb_type_array[CurrMbAddr] = I_4x4;
				expGolomb_UC(mb_type);

				for(int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
				{
					writeFlag(prev_intra4x4_pred_mode_flag[luma4x4BlkIdx]);
					if (prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] == false)
					{
						unsigned char buffer[4];
						RBSPtoUINT(buffer, rem_intra4x4_pred_mode[luma4x4BlkIdx]);
						writeRawBits(3, buffer);
					}
				}
			}
			// intra16x16 prediction
			else
			{
				// TODO: choose mb_type according to intra16x16PredMode
				// and Luma and Chroma array types.
			}
			quantizationTransform(predL, predCb, predCr);
			expGolomb_UC(intra_chroma_pred_mode);
		}
	}
}