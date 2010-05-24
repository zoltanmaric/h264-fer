#include "nal.h"
#include "headers_and_parameter_sets.h"
#include "residual.h"
#include "expgolomb.h"
#include "h264_globals.h"
#include "rbsp_IO.h"
#include "intra.h"
#include "inttransform.h"
#include "fileIO.h"
#include "ref_frames.h"
#include "mocomp.h"
#include "mode_pred.h"
#include "moestimation.h"
#include "quantizationTransform.h"
#include "rbsp_decoding.h"
#include "openCL_functions.h"

// ENCODING:

void setCodedBlockPattern()
{
	CodedBlockPatternLuma = 0;
	CodedBlockPatternChroma = 0;

	// Set CodedBlockPatternLuma:
	for (int i8x8 = 0; i8x8 < 4; i8x8++)
	{
		bool allZero = true;
		for (int i4x4 = 0; i4x4 < 4; i4x4++)
		{
			if (MbPartPredMode(mb_type,0) == Intra_16x16)
			{
				for (int i = 0; i < 15; i++)
				{
					if (Intra16x16ACLevel[(i8x8<<2) + i4x4][i] != 0)
					{
						allZero = false;
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < 16; i++)
				{
					if (LumaLevel[(i8x8<<2) + i4x4][i] != 0)
					{
						allZero = false;
						break;
					}
				}
			}

			if (allZero == false)
			{
				// Set the corresponding bit of CodedBlockPattern to 1
				CodedBlockPatternLuma |= 1 << i8x8;
				break;
			}
		}
	}

	// Set CodedBlockPatternChroma:
	for (int i = 0; i < 4; i++)
	{
		if ((ChromaDCLevel[0][i] != 0) || (ChromaDCLevel[1][i] != 0))
		{
			CodedBlockPatternChroma |= 1;
			break;
		}
	}

	bool allZero = true;
	for (int i4x4 = 0; i4x4 < 4; i4x4++)
	{
		for (int i = 0; i < 15; i++)
		{
			if ((ChromaACLevel[0][i4x4][i] != 0) || (ChromaACLevel[1][i4x4][i] != 0))
			{
				allZero = false;
				break;
			}
		}
		if (allZero == false)
		{
			CodedBlockPatternChroma |= 2;
			break;
		}
	}

	if ((MbPartPredMode(mb_type,0) == Intra_16x16) && (CodedBlockPatternLuma != 0))
	{
		CodedBlockPatternLuma = 15;
	}

	if (CodedBlockPatternChroma == 3)
	{
		// CodedBlockPatternChroma == 3 is not defined
		CodedBlockPatternChroma = 2;
	}

	CodedBlockPatternLumaArray[CurrMbAddr] = CodedBlockPatternLuma;
	CodedBlockPatternChromaArray[CurrMbAddr] = CodedBlockPatternChroma;
}

// 7.3.2.11
void RBSP_trailing_bits()
{
	writeOnes(1);
	flushWriteBuffer();
	if (RBSP_write_current_bit != 0)
	{
		writeZeros(8 - RBSP_write_current_bit);
	}
	flushWriteBuffer();
}

void RBSP_encode(NALunit &nal_unit)
{
	initRawWriter(nal_unit.rbsp_byte, 500000);

	if (nal_unit.nal_unit_type == NAL_UNIT_TYPE_SPS)
	{
		sps_write();
		init_h264_structures_encoder();
		InitializeInterpolatedRefFrame();	
		AllocateMemory();
		AllocateFrameBuffers();
		RBSP_trailing_bits();
		nal_unit.NumBytesInRBSP = RBSP_write_current_byte;
	}
	else if (nal_unit.nal_unit_type == NAL_UNIT_TYPE_PPS)
	{
		pps_write();
		RBSP_trailing_bits();
		nal_unit.NumBytesInRBSP = RBSP_write_current_byte;
	}
	else if ((nal_unit.nal_unit_type == NAL_UNIT_TYPE_IDR) || (nal_unit.nal_unit_type == NAL_UNIT_TYPE_NOT_IDR))
	{
		if (nal_unit.nal_unit_type == NAL_UNIT_TYPE_IDR)
		{
			static bool firstFrame = true;
			shd.slice_type = I_SLICE;
			if (firstFrame == true)
			{
				firstFrame = false;
				shd.idr_pic_id = 0;	
			}
			else if (shd.frame_num == 0)	// if previous frame was IDR
			{
				shd.idr_pic_id++;
			}
			else
			{
				shd.idr_pic_id = 0;	
			}

			shd.frame_num = 0;
			IntraCL();
		}
		else
		{
			shd.slice_type = P_SLICE;
			shd.frame_num++;
		}

		shd_write(nal_unit);

		int predL[16][16], predCb[8][8], predCr[8][8];
		int mb_skip_run = 0;


		for (CurrMbAddr = 0; CurrMbAddr < shd.PicSizeInMbs; CurrMbAddr++)
		{
			// TODO: Try avoiding this.
			clear_residual_structures();
			if ((shd.slice_type != I_SLICE) && (shd.slice_type != SI_SLICE))
			{
				interEncoding(predL, predCr, predCb);
				mb_type_array[CurrMbAddr] = mb_type;
				if (mb_type == P_Skip)
				{
					mb_skip_run++;
					transformDecodingP_Skip(predL, predCb, predCr, QPy);
					continue;
				}
				expGolomb_UC(mb_skip_run);

				mb_skip_run = 0;

				quantizationTransform(predL, predCb, predCr, true);
				setCodedBlockPattern();
			}
			else
			{
				int intra16x16PredMode = intraPredictionEncoding(predL, predCr, predCb);

				// intra4x4 prediction
				if (intra16x16PredMode == -1)
				{
					mb_type = I_4x4;
					quantizationTransform(predL, predCb, predCr, true);
					setCodedBlockPattern();
				}
				// intra16x16 prediction
				else
				{
					mb_type = intra16x16PredMode + 1;
					
					quantizationTransform(predL, predCb, predCr, true);
					setCodedBlockPattern();

					mb_type += (CodedBlockPatternChroma << 2);
					if (CodedBlockPatternLuma == 15)
					{
						mb_type += 12;
					}
				}
				mb_type_array[CurrMbAddr] = mb_type;
			}

			expGolomb_UC(mb_type);

			if ((mb_type != I_4x4) && (MbPartPredMode(mb_type,0) != Intra_16x16) && (NumMbPart(mb_type) == 4))
			{
				// Norm: start sub_mb_pred()
				int mbPartIdx;
				for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
				{
					expGolomb_UC(sub_mb_type[mbPartIdx]);
				}
				// Norm: currently there is no support for reference picture list
				// modifications in the encoder

				for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
				{
					for (int subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(sub_mb_type[mbPartIdx]); subMbPartIdx++)
					{
						expGolomb_SC(mvd_l0[mbPartIdx][subMbPartIdx][0]);
						expGolomb_SC(mvd_l0[mbPartIdx][subMbPartIdx][1]);
					}
				}
				// Norm: end sub_mb_pred()
			}
			else
			{

				// Norm: start mb_pred()
				if ((MbPartPredMode(mb_type,0) == Intra_4x4) || (MbPartPredMode(mb_type,0) == Intra_16x16))
				{
					if (MbPartPredMode(mb_type, 0) == Intra_4x4)
					{
						for(int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
						{
							writeFlag(prev_intra4x4_pred_mode_flag[luma4x4BlkIdx]);
							if (prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] == false)
							{
								writeRawBits(3, rem_intra4x4_pred_mode[luma4x4BlkIdx]);
							}
						}
						
					}

					expGolomb_UC(intra_chroma_pred_mode);
				}
				else
				{
					// Norm: currently there is no support for reference picture list
					// modifications in the encoder
					for (int mbPartIdx = 0; mbPartIdx < NumMbPart(mb_type); mbPartIdx++)
					{
						expGolomb_SC(mvd_l0[mbPartIdx][0][0]);
						expGolomb_SC(mvd_l0[mbPartIdx][0][1]);
					}
				}
				// Norm: end mb_pred()
			}

			if (MbPartPredMode(mb_type, 0) != Intra_16x16)
			{
				int coded_block_pattern = (CodedBlockPatternChroma << 4) | CodedBlockPatternLuma;
				if (MbPartPredMode(mb_type,0) == Intra_4x4)
				{								
					expGolomb_UC(coded_block_pattern_to_codeNum_intra[coded_block_pattern]);
				}
				else
				{
					expGolomb_UC(coded_block_pattern_to_codeNum_inter[coded_block_pattern]);
				}
			}

			if ((CodedBlockPatternLuma > 0) || (CodedBlockPatternChroma > 0) ||
				(MbPartPredMode(mb_type, 0) == Intra_16x16))
			{
				// mb_qp_delta = 0;
				expGolomb_SC(0);

				residual_write();
			}
			// Norm: end macroblock_layer()
			else
			{
				clear_residual_structures();
			}
		}	

		if (mb_skip_run > 0)
		{
			expGolomb_UC(mb_skip_run);
		}

		RBSP_trailing_bits();
		nal_unit.NumBytesInRBSP = RBSP_write_current_byte;

		if (shd.slice_type % 5 == I_SLICE)
		{
			initialisationProcess();
		}
		modificationProcess();		
		FillInterpolatedRefFrame();
	}

	flushWriteBuffer();
}

//MB size testing function used in prediction process

unsigned int coded_mb_size(int intra16x16PredMode, int predL[16][16], int predCb[8][8], int predCr[8][8])
{
	unsigned int totalSize=0;

	if ((shd.slice_type != I_SLICE) && (shd.slice_type != SI_SLICE))
	{
		quantizationTransform(predL, predCb, predCr, false);
		setCodedBlockPattern();
	}
	else
	{
		// intra4x4 prediction
		if (intra16x16PredMode == -1)
		{
			mb_type = I_4x4;
			quantizationTransform(predL, predCb, predCr, false);
			setCodedBlockPattern();
		}
		// intra16x16 prediction
		else
		{
			mb_type = intra16x16PredMode + 1;
			
			quantizationTransform(predL, predCb, predCr, false);
			setCodedBlockPattern();

			mb_type += (CodedBlockPatternChroma << 2);
			if (CodedBlockPatternLuma == 15)
			{
				mb_type += 12;
			}
		}
	}

	totalSize+=expgolomb_UC_codes[mb_type][0]*2+1;

	if ((mb_type != I_4x4) && (MbPartPredMode(mb_type,0) != Intra_16x16) && (NumMbPart(mb_type) == 4))
	{
		int mbPartIdx;
		for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
		{
			totalSize+=expgolomb_UC_codes[sub_mb_type[mbPartIdx]][0]*2+1;
		}
		for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
		{
			for (int subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(sub_mb_type[mbPartIdx]); subMbPartIdx++)
			{
				totalSize+=expgolomb_UC_codes[SC_to_UC(mvd_l0[mbPartIdx][subMbPartIdx][0])][0]*2+1;
				totalSize+=expgolomb_UC_codes[SC_to_UC(mvd_l0[mbPartIdx][subMbPartIdx][1])][0]*2+1;
			}
		}
	}
	if ((MbPartPredMode(mb_type,0) == Intra_4x4) || (MbPartPredMode(mb_type,0) == Intra_16x16))
	{
		if (MbPartPredMode(mb_type, 0) == Intra_4x4)
		{
			for(int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
			{
				totalSize++;
				if (prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] == false)
				{
					totalSize+=3;
				}
			}
			
		}

		totalSize+=expgolomb_UC_codes[intra_chroma_pred_mode][0]*2+1;
	}
	else
	{
		for (int mbPartIdx = 0; mbPartIdx < NumMbPart(mb_type); mbPartIdx++)
		{
			totalSize+=expgolomb_UC_codes[SC_to_UC(mvd_l0[mbPartIdx][0][0])][0]*2+1;
			totalSize+=expgolomb_UC_codes[SC_to_UC(mvd_l0[mbPartIdx][0][1])][0]*2+1;
		}
	}

	if (MbPartPredMode(mb_type, 0) != Intra_16x16)
	{
		int coded_block_pattern = (CodedBlockPatternChroma << 4) | CodedBlockPatternLuma;
		if (MbPartPredMode(mb_type,0) == Intra_4x4)
		{				
			totalSize+=expgolomb_UC_codes[coded_block_pattern_to_codeNum_intra[coded_block_pattern]][0]*2+1;
		}
		else
		{
			totalSize+=expgolomb_UC_codes[coded_block_pattern_to_codeNum_inter[coded_block_pattern]][0]*2+1;
		}
	}

	if ((CodedBlockPatternLuma > 0) || (CodedBlockPatternChroma > 0) ||
		(MbPartPredMode(mb_type, 0) == Intra_16x16))
	{
		totalSize+=1;

		int startIdx=0;
		int endIdx=15;

		if (startIdx == 0 && MbPartPredMode(mb_type, 0) == Intra_16x16)
		{
			invoked_for_Intra16x16DCLevel=1;
			totalSize+=residual_block_cavlc_size(Intra16x16DCLevel, 0, 15, 16);
			invoked_for_Intra16x16DCLevel=0;
		}

		
		for (i8x8=0;i8x8<4;i8x8++)
		{
			for (i4x4=0;i4x4<4;i4x4++)
			{
				if (CodedBlockPatternLuma & (1<<i8x8))
				{
					if (endIdx>0 && MbPartPredMode(mb_type, 0) == Intra_16x16)
					{
						invoked_for_Intra16x16ACLevel=1;
						totalSize+=residual_block_cavlc_size(Intra16x16ACLevel[i8x8*4+i4x4],(((startIdx-1)>0)?(startIdx-1):0),endIdx-1,15);
						invoked_for_Intra16x16ACLevel=0;
					}
					else
					{
						invoked_for_LumaLevel=1;
						totalSize+=residual_block_cavlc_size(LumaLevel[i8x8*4 + i4x4],startIdx,endIdx,16);
						invoked_for_LumaLevel=0;
					}
				}
			}
		}

		NumC8x8 = 4 / (SubWidthC * SubHeightC );
		for (iCbCr=0; iCbCr<2; iCbCr++)
		{
			if ((CodedBlockPatternChroma & 3) && startIdx==0)
			{
				invoked_for_ChromaDCLevel=1;
				totalSize+=residual_block_cavlc_size(ChromaDCLevel[iCbCr],0,4*NumC8x8-1, 4*NumC8x8);
				invoked_for_ChromaDCLevel=0;
			}
		}

		for (iCbCr=0;iCbCr<2;iCbCr++)
		{
			for (i8x8=0;i8x8<NumC8x8;i8x8++)
			{
				for (cb4x4BlkIdx=0; cb4x4BlkIdx<4; cb4x4BlkIdx++)
				{
					if ((CodedBlockPatternChroma & 2) && endIdx>0)
					{
						invoked_for_ChromaACLevel=1;
						totalSize+=residual_block_cavlc_size(ChromaACLevel[iCbCr][i8x8*4+cb4x4BlkIdx],((startIdx-1)>0?(startIdx-1):0),endIdx-1,15);
						invoked_for_ChromaACLevel=0;
					}
				}
			}
		}
	}
		
return totalSize;
}