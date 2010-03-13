#include "nal.h"
#include "headers_and_parameter_sets.h"
#include "residual.h"
#include "expgolomb.h"
#include "h264_globals.h"
#include "rawreader.h"
#include "intra.h"
#include "inttransform.h"
#include "writeToPPM.h"
#include <stdio.h>


void RBSP_decode(NALunit nal_unit)
{

	static int nalBrojac=0;

	static int idr_frame_number=0;
	printf("Ulaz u RBPS_decode broj %d\n",nalBrojac++);

	initRawReader(nal_unit.rbsp_byte, nal_unit.NumBytesInRBSP);

	//TYPE 7 = Sequence parameter set TODO: Provjera je li vec postoji SPS
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
	//TYPE 8 = Picture parameter set TODO: Provjera je li vec postoji PPS i SPS
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
		//Read slice header
		fill_shd(&nal_unit);

		printf("Working on frame number %d...\n", shd.frame_num);

		int MbCount=shd.PicSizeInMbs;

		//Norm: firstMbAddr=first_mb_in_slice * ( 1 + MbaffFrameFlag );
		int firstMbAddr = 0;

		//Norm: CurrMbAddr = firstMbAddr
		//Already globaly defined
		CurrMbAddr = firstMbAddr;

		//Norm: moreDataFlag = 1
		bool moreDataFlag = true;

		//Norm: prevMbSkipped = 0
		int prevMbSkipped = 0;

		//Used later on
		int mb_skip_run;

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
					//Norm: CurrMbAddr = NextMbAddress( CurrMbAddr )
					CurrMbAddr++;

					//Current macroblock coordinates
					mb_pos_x=CurrMbAddr%sps.PicWidthInMbs;
					mb_pos_y=CurrMbAddr/sps.PicWidthInMbs;

					//This macroblock has a "P_Skip" value of "mb_type"
					//PITCH=Line size, currently equal to "width"

					mb_type_array[CurrMbAddr]=P_Skip;

					//Transform macroblock-level coordinates to pixel-level coordinates
					int pixel_pos_x=mb_pos_x*16;
					int pixel_pos_y=mb_pos_y*16;

					// BEGIN INTER
					/*
					Derive_P_Skip_MVs(mpi,mb_pos_x,mb_pos_y);
					MotionCompensateMB(this,ref,mpi,mb_pos_x,mb_pos_y);
					*/
					//END INTER
				}
				
				if ((CurrMbAddr != firstMbAddr) || (mb_skip_run > 0))
				{
					moreDataFlag = more_rbsp_data();
				}
			}

			//
			/*if(CurrMbAddr>=MbCount)
			{
			return;
			}
			*/

			if(moreDataFlag)
			{ 
				// Norm: start macroblock_layer()
				mb_pos_array[CurrMbAddr]=(RBSP_current_bit+1)%8;
				mb_type = expGolomb_UD();

				//Current macroblock coordinates
				mb_pos_x=CurrMbAddr%sps.PicWidthInMbs;
				mb_pos_y=CurrMbAddr/sps.PicWidthInMbs;

				//Store the mb_type at appropriate location in the array
				//PITCH=Line size, currently equal to "width"

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

				// Prediction samples formed by either intra or inter prediction.
				int predL[16][16], predCr[8][8], predCb[8][8];
				//Specific inter prediction?
				if(mb_type != I_4x4 /*&& mb_type != I_8x8*/ && MbPartPredMode( mb_type, 0 )!=Intra_16x16 && NumMbPart( mb_type )==4 )
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
							ref_idx_l0[mbPartIdx] = expGolomb_TD();
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
								ref_idx_l0[mbPartIdx] = expGolomb_TD();
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
					CodedBlockPatternChroma=I_Macroblock_Modes[mb_type][5];
					CodedBlockPatternLuma=I_Macroblock_Modes[mb_type][6];
				}				

				CodedBlockPatternLumaArray[CurrMbAddr] = CodedBlockPatternLuma;
				CodedBlockPatternChromaArray[CurrMbAddr] = CodedBlockPatternChroma;

				if(CodedBlockPatternLuma>0 || CodedBlockPatternChroma>0 || MbPartPredMode(mb_type,0)==Intra_16x16)
				{

					mb_qp_delta=expGolomb_SD();

					//BEGIN Kvantizacijski paramteri
					/*
					int iCbCr,QPi;

					QPy=(QPy+mb_qp_delta+52)%52;
					QPi=QPy+pps.chroma_qp_index_offset;
					QPi=CustomClip(QPi,0,51);
					if(QPi<30)
					{
					QPc=QPi;
					}
					else
					{
					QPc=QPcTable[QPi-30];
					}
					*/
					//END Kvantizacijski parametri


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

				intraPrediction(predL, predCr, predCb);

				if (MbPartPredMode(mb_type, 0) != Intra_4x4)
				{
					transformDecodingIntra_16x16Luma(Intra16x16DCLevel, Intra16x16ACLevel, predL, QPy);
				}
				transformDecodingChroma(ChromaDCLevel[0], ChromaACLevel[0], predCb, QPy, true);
				transformDecodingChroma(ChromaDCLevel[1], ChromaACLevel[1], predCr, QPy, false);

				moreDataFlag=more_rbsp_data();
				++CurrMbAddr;
			}
		}

		static int intraFrameCounter = 1;
		
		writeToPPM(intraFrameCounter++);

		idr_frame_number++;
		
		
		//if (intraFrameCounter > 3)
		//	exit(0);
		
		int stop = 0;
	}
}


