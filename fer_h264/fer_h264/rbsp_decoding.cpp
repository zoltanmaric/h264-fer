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

	printf("Ulaz u RBPS_decode broj %d",nalBrojac++);

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
	else if (nal_unit.nal_unit_type==NAL_UNIT_TYPE_IDR || nal_unit.nal_unit_type==NAL_UNIT_TYPE_NOT_IDR)
	{

		//Read slice header
		fill_shd(&nal_unit);

		printf("Working on frame number %d...\n", shd.frame_num);

		int MbCount=shd.PicSizeInMbs;

		//Norm: firstMbAddr=first_mb_in_slice * ( 1 + MbaffFrameFlag );
		int firstMbAddr = 0;

		//Norm: CurrMbAddr = firstMbAddr
		//Already globaly defined
		//int CurrMbAddr = 0;

		//Norm: moreDataFlag = 1
		bool moreDataFlag = true;

		//Norm: prevMbSkipped = 0
		int prevMbSkipped = 0;

		//Used later on
		int mb_skip_run;

		int QPy;
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
				//Norm: if( CurrMbAddr != firstMbAddr | | mb_skip_run > 0 ) ...
				if (mb_skip_run>0)
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
if (CurrMbAddr>80)
break;

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

					//Norm:

					/*
					for( mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++ )
					sub_mb_type[ mbPartIdx ]
					*/

					for(int mbPartIdx=0; mbPartIdx<4; mbPartIdx++)
					{
						sub_mb_type_array[mbPartIdx]=expGolomb_UD();
					}

					//Let's read the list of referenced frames for inter prediction.
					//Norm: 

					/*
					for( mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++ )
					if (( num_ref_idx_l0_active_minus1 > 0 || mb_field_decoding_flag != field_pic_flag ) && mb_type != P_8x8ref0 &&
					sub_mb_type[ mbPartIdx ] != B_Direct_8x8 && SubMbPredMode( sub_mb_type[ mbPartIdx ] ) != Pred_L1 )

					ref_idx_l0[ mbPartIdx ]
					*/

					//This is not implemented, only 1-frame prediction is currently supported.


					//Let's read the motion vector information for the motion vector derivation process
					//Norm:

					/*
					for( mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++ )
					if( sub_mb_type[ mbPartIdx ] != B_Direct_8x8 && SubMbPredMode( sub_mb_type[ mbPartIdx ] ) != Pred_L1 )
					for( subMbPartIdx = 0; subMbPartIdx < NumSubMbPart( sub_mb_type[ mbPartIdx ] ); subMbPartIdx++)
					for( compIdx = 0; compIdx < 2; compIdx++ )
					mvd_l0[ mbPartIdx ][ subMbPartIdx ][ compIdx ]
					*/

					for(int mbPartIdx=0; mbPartIdx<4; mbPartIdx++)
					{
						if(sub_mb_type_array[mbPartIdx] != B_Direct_8x8 && SubMbPredMode(sub_mb_type_array[mbPartIdx])!=Pred_L1)
						{ 
							for(int subMbPartIdx=0; subMbPartIdx<NumSubMbPart(sub_mb_type_array[mbPartIdx]); subMbPartIdx++)
							{
								int mvdx=expGolomb_SD();
								int mvdy=expGolomb_SD();
								//BEGIN INTER
								/*
								DeriveMVs(mpi, mb_pos_x+Intra4x4ScanOrder[mbPartIdx*4+subMbPartIdx*SOF][0], 
								mb_pos_y+Intra4x4ScanOrder[mbPartIdx*4+subMbPartIdx*SOF][1],
								sub[mbPartIdx].SubMbPartWidth,
								sub[mbPartIdx].SubMbPartHeight,
								mvdx, mvdy);
								*/
								//END INTER
					  }
						}
					}
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
								prev_intra4x4_pred_mode_flag[luma4x4BlkIdx]=getRawBits(1);
								if(prev_intra4x4_pred_mode_flag[luma4x4BlkIdx]==0)
								{
									rem_intra4x4_pred_mode[luma4x4BlkIdx]=getRawBits(3);
								}
							}
						}

						//Norm:
						//if( MbPartPredMode( mb_type, 0 ) = = Intra_8x8 )
						//This if clause has been skipped, because "intra_8x8" is not supported.


						//Norm:
						//if( ChromaArrayType == 1 || ChromaArrayType == 2 )
						//We only support "ChromaArrayType==1", so the if clause is skipped

						intra_chroma_pred_mode=expGolomb_UD();

					intraPrediction(CurrMbAddr, predL, predCr, predCb);
				}
				else
					{
						for(int mbPartIdx=0; mbPartIdx<NumMbPart( mb_type ) ; ++mbPartIdx)
						{
							if(MbPartPredMode( mb_type, mbPartIdx )!=Pred_L1)
							{
								int mvdx=expGolomb_SD();
								int mvdy=expGolomb_SD();
								/*
								DeriveMVs(mpi, mb_pos_x+Intra4x4ScanOrder[mbPartIdx*SOF][0],
								mb_pos_y+Intra4x4ScanOrder[mbPartIdx*SOF][1],
								mb.MbPartWidth, mb.MbPartHeight, mvdx, mvdy);
								*/
							}
						}
					}
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
						coded_block_pattern=codeNum_to_coded_block_pattern_intra[coded_block_pattern];
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

				

				if(CodedBlockPatternLuma>0 || CodedBlockPatternChroma>0 || MbPartPredMode(mb_type,0)==Intra_16x16)
				{

					//Norm: mb_qp_delta

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
					//Additional parameter "nC" has been added.

//					residual(0, 15);

					//TEST INSERT

		if(MbPartPredMode(mb_type,0)==Intra_16x16)
            residual_block(&LumaDCLevel[0],16,LumaDC_nC);
          for(int i8x8=0; i8x8<4; ++i8x8)
            for(int i4x4=0; i4x4<4; ++i4x4)
              if(CodedBlockPatternLuma&(1<<i8x8)) {
                if(MbPartPredMode(mb_type,0)==Intra_16x16)
                  LumaAdjust residual_block(&LumaACLevel[i8x8*4+i4x4][1],15,LumaAC_nC);
                else
                  LumaAdjust residual_block(&LumaACLevel[i8x8*4+i4x4][0],16,LumaAC_nC);
              };
          for(int iCbCr=0; iCbCr<2; iCbCr++)
            if(CodedBlockPatternChroma&3)
              residual_block(&ChromaDCLevel[iCbCr][0],4,ChromaDC_nC);
          for(int iCbCr=0; iCbCr<2; iCbCr++)
            for(int i4x4=0; i4x4<4; ++i4x4)
              if(CodedBlockPatternChroma&2)
                ChromaAdjust residual_block(&ChromaACLevel[iCbCr][i4x4][1],15,ChromaAC_nC);



					//Old prediction code is currently commented out:
					/*
					if(MbPartPredMode(mb_type,0)==Intra_16x16)
					{
					residual_block(&coeffLevel_luma_DC[0],16,get_nC(mb_pos_x,mb_pos_y,LUMA));
					}

					for(int i8x8=0; i8x8<4; i8x8++)
					{
					for(int i4x4=0; i4x4<4; ++i4x4)
					{
					if(CodedBlockPatternLuma&(1<<i8x8))
					{
					if(MbPartPredMode(mb_type,0)==Intra_16x16)
					{
					TotalCoeff_luma_array[(mb_pos_x+Intra4x4ScanOrder[i8x8*4+i4x4][0])/4][(mb_pos_y+Intra4x4ScanOrder[i8x8*4+i4x4][1])/4]=residual_block(&coeffLevel_luma_AC[i8x8*4+i4x4][1],15,get_nC(mb_pos_x,mb_pos_y,LUMA));
					}
					else
					{
					TotalCoeff_luma_array[(mb_pos_x+Intra4x4ScanOrder[i8x8*4+i4x4][0])/4][(mb_pos_y+Intra4x4ScanOrder[i8x8*4+i4x4][1])/4]=residual_block(&coeffLevel_luma_AC[i8x8*4+i4x4][0],16,get_nC(mb_pos_x,mb_pos_y,LUMA));
					}
					}
					}
					}

					for(int iCbCr=0; iCbCr<2; iCbCr++)
					{
					if(CodedBlockPatternChroma&3)
					{
					residual_block(&coeffLevel_chroma_DC[iCbCr][0],4,get_nC(mb_pos_x,mb_pos_y,CHROMA));
					}
					}

					for(int iCbCr=0; iCbCr<2; iCbCr++)
					{
					for(int i4x4=0; i4x4<4; ++i4x4)
					{
					if(CodedBlockPatternChroma&2)
					{
					TotalCoeff_chroma_array[iCbCr][((mb_pos_y+(i4x4>>1)*8)>>3)][((mb_pos_x+(i4x4&1)*8)>>3)]=residual_block(&coeffLevel_chroma_AC[iCbCr][i4x4][1],15,get_nC(mb_pos_x,mb_pos_y,CHROMA)); 
					}
					}
					}
					*/


				}


				//Data ready for rendering

				QPy = (QPy + mb_qp_delta + 52)%52;

				if (MbPartPredMode(mb_type, 0) == Intra_4x4)
				{
					transformDecoding4x4LumaResidual(LumaLevel, predL, QPy, CurrMbAddr);
				}
				else
				{
					transformDecodingIntra_16x16Luma(Intra16x16DCLevel, Intra16x16ACLevel, predL, QPy, CurrMbAddr);
				}
				transformDecodingChroma(ChromaDCLevel[0], ChromaACLevel[0], predCb, QPy, true);
				transformDecodingChroma(ChromaDCLevel[1], ChromaACLevel[1], predCr, QPy, false);



				if(MbPartPredMode(mb_type,0)==Intra_4x4)
				{
					for(int i=0; i<16; ++i)
					{

						//BEGIN INTRA
						/*
						int x=mb_pos_x+Intra4x4ScanOrder[i][0];
						int y=mb_pos_y+Intra4x4ScanOrder[i][1];
						Intra_4x4_Dispatch(this,mpi,x,y,i);
						*/
						//END INTRA

						//BEGIN T&Q
						/*
						enter_luma_block(&LumaACLevel[i][0],this,x,y,QPy,0);
						*/
						//END T&Q
					}

					//BEGIN INTRA
					/*
					Intra_Chroma_Dispatch(this,mpi,intra_chroma_pred_mode,mb_pos_x>>1,mb_pos_y>>1,pps->constrained_intra_pred_flag);
					*/
					//END INTRA

				}
				else if(MbPartPredMode(mb_type,0)==Intra_16x16)
				{
					//BEGIN INTRA
					/*
					Intra_16x16_Dispatch(this,mpi,mb.Intra16x16PredMode,mb_pos_x,mb_pos_y,pps->constrained_intra_pred_flag);
					*/
					//END INTRA

					//BEGIN T&Q
					/*
					transform_luma_dc(&LumaDCLevel[0],&LumaACLevel[0][0],QPy);
					*/
					//END T&Q



					for(int i=0; i<16; i++)
					{
						int x=mb_pos_x+Intra4x4ScanOrder[i][0];
						int y=mb_pos_y+Intra4x4ScanOrder[i][1];
						//BEGIN T&Q
						/*
						enter_luma_block(&LumaACLevel[i][0],this,x,y,QPy,1);
						*/
						//END T&Q
					}

					//BEGIN INTRA
					/*
					Intra_Chroma_Dispatch(this,mpi,intra_chroma_pred_mode,mb_pos_x>>1,mb_pos_y>>1,pps->constrained_intra_pred_flag);

					for(i=0; i<4; ++i) for(j=0; j<4; ++j)
					{
					ModePredInfo_Intra4x4PredMode(mpi,(mb_pos_x>>2)+j,(mb_pos_y>>2)+i)=2;
					}
					*/
					//END INTRA
				}
				else
				{
					//BEGIN INTER
					/*
					MotionCompensateMB(this,ref,mpi,mb_pos_x,mb_pos_y);
					*/
					//END INTER

					for(int i=0; i<16; ++i)
					{
						int x=mb_pos_x+Intra4x4ScanOrder[i][0];
						int y=mb_pos_y+Intra4x4ScanOrder[i][1];
						//BEGIN T&Q
						/*
						enter_luma_block(&LumaACLevel[i][0],this,x,y,QPy,0);
						*/
						//END T&Q
					}
				}

				if(CodedBlockPatternChroma!=0)
				{
					for(int iCbCr=0; iCbCr<2; ++iCbCr)
					{
						//BEGIN T&Q
						/*
						transform_chroma_dc(&ChromaDCLevel[iCbCr][0],QPc);
						*/
						//END T&Q

						for(int i=0; i<4; ++i)
						{
							ChromaACLevel[iCbCr][i][0]=ChromaDCLevel[iCbCr][i];
						}

						for(int i=0; i<4; ++i)
						{
							//BEGIN T&Q
							/*
							enter_chroma_block(&ChromaACLevel[iCbCr][i][0],this,iCbCr,
							(mb_pos_x>>1)+Intra4x4ScanOrder[i][0],
							(mb_pos_y>>1)+Intra4x4ScanOrder[i][1],
							QPc,1);
							*/
							//END T&Q
						}
					}
				}

				moreDataFlag=more_rbsp_data();
				++CurrMbAddr;

				
			}
			
		}

		writeToPPM();
		exit(0);
		int stop = 0;

	}
}


