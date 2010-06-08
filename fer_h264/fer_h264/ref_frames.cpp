#include <cstdlib>
#include <cstring>
#include <string>
#include <cassert>

#include <CL/cl.h>

#include "h264_globals.h"
#include "headers_and_parameter_sets.h"
#include "ref_frames.h"
#include "h264_math.h"
#include "openCL_functions.h"
ref_pic_type RefPicList0[50000];
frame_type dpb;
int iskoristeno;

void frameDeepCopy()
{
	for (int i = 0; i < frame.Lheight; i++)
	{
		for (int j = 0; j < frame.Lwidth; j++)
		{
			dpb.L[i*frame.Lwidth+j]=frame.L[i*frame.Lwidth+j];
		}
	}

	for (int i = 0; i < frame.Cheight; i++)
	{
		for (int j = 0; j < frame.Cwidth; j++)
		{
			dpb.C[0][i*frame.Cwidth+j]=frame.C[0][i*frame.Cwidth+j];
			dpb.C[1][i*frame.Cwidth+j]=frame.C[1][i*frame.Cwidth+j];
		}
	}
}

void decodePictureNumbers()
{
	for (int i = 0; i < 16; i)
	{
		if (!RefPicList0[i].RefPicPresent)
			break;
		if (!RefPicList0[i].IsLongTerm)
		{
			if (RefPicList0[i].FrameNum > shd.frame_num)
				RefPicList0[i].FrameNumWrap = RefPicList0[i].FrameNum - sps.MaxFrameNum;			else 
				RefPicList0[i].FrameNumWrap = RefPicList0[i].FrameNum;
			RefPicList0[i].PicNum = RefPicList0[i].FrameNum;
		} else {
			RefPicList0[i].LongTermPicNum = RefPicList0[i].LongTermFrameIdx;
		}
	}
}

void initialisationProcess()
{
	static bool firstTime = true;
	if (firstTime)
	{
		firstTime = false;
		dpb.Lheight=frame.Lheight;
		dpb.Lwidth=frame.Lwidth;
		dpb.Cheight=frame.Cheight;
		dpb.Cwidth=frame.Cwidth;

		dpb.L = new unsigned char[frame.Lheight*frame.Lwidth];

		dpb.C[0] = new unsigned char[frame.Cheight*frame.Cwidth];
		dpb.C[1] = new unsigned char[frame.Cheight*frame.Cwidth];
	}

	for (int refPicIdx = 0; refPicIdx < shd.num_ref_idx_l0_active_minus1+1; refPicIdx++)
	{
		RefPicList0[refPicIdx].RefPicPresent = false;
		RefPicList0[refPicIdx].frame = &dpb;
	}
}

int picNumF(int cIdx)
{
	if (!RefPicList0[cIdx].IsLongTerm)
		return RefPicList0[cIdx].PicNum;
	return sps.MaxFrameNum;
}

int longTermPicNumF(int cIdx)
{
	if (RefPicList0[cIdx].IsLongTerm)
		return RefPicList0[cIdx].LongTermPicNum;
	return 2 * shd.max_long_term_frame_idx_plus1[cIdx];
}

void modificationProcess()
{
	if (shd.ref_pic_list_modification_flag_l0)
	{
		int refIdxL0 = 0, curr_id = 0;
		int picNumL0Pred = shd.frame_num, picNumL0NoWrap, picNumL0;
		do
		{
			if (shd.modification_of_pic_nums_idc[curr_id] == 0 || shd.modification_of_pic_nums_idc[curr_id] == 1)
			{
				if (shd.modification_of_pic_nums_idc[curr_id] == 0)
				{
					if (picNumL0Pred - (shd.abs_diff_pic_num_minus1[curr_id]+1) < 0)
						picNumL0NoWrap = picNumL0Pred - (shd.abs_diff_pic_num_minus1[curr_id]+1) + sps.MaxFrameNum;
					else
						picNumL0NoWrap = picNumL0Pred - (shd.abs_diff_pic_num_minus1[curr_id]+1);
				} else {
					if (picNumL0Pred + (shd.abs_diff_pic_num_minus1[curr_id]+1) >= sps.MaxFrameNum)
						picNumL0NoWrap = picNumL0Pred + (shd.abs_diff_pic_num_minus1[curr_id]+1) - sps.MaxFrameNum;
					else
						picNumL0NoWrap = picNumL0Pred + (shd.abs_diff_pic_num_minus1[curr_id]+1);
				}
				picNumL0Pred = picNumL0NoWrap;
				if (picNumL0NoWrap > shd.frame_num)
					picNumL0 = picNumL0NoWrap - sps.MaxFrameNum;
				else 
					picNumL0 = picNumL0NoWrap;

				for (int cIdx = shd.num_ref_idx_l0_active_minus1+1; cIdx>refIdxL0; cIdx--)
					RefPicList0[cIdx] = RefPicList0[cIdx-1];
				// Postavljanje u listu
				RefPicList0[refIdxL0].PicNum = picNumL0;
				RefPicList0[refIdxL0].LongTermPicNum = 0;
				RefPicList0[refIdxL0].FrameNum = shd.frame_num;
				RefPicList0[refIdxL0].IsLongTerm = false;
				RefPicList0[refIdxL0].RefPicPresent = true;
				frameDeepCopy();

				int nIdx = refIdxL0;
				for (int cIdx = refIdxL0; cIdx <= shd.num_ref_idx_l0_active_minus1+1; cIdx++)
					if (picNumF(cIdx) != picNumL0)
						RefPicList0[nIdx++] = RefPicList0[cIdx];
			} else if (shd.modification_of_pic_nums_idc[curr_id] == 2) {
				for (int cIdx = shd.num_ref_idx_l0_active_minus1+1; cIdx>refIdxL0; cIdx--)
					RefPicList0[cIdx] = RefPicList0[cIdx-1];
				// Postavljanje u listu
				RefPicList0[refIdxL0].LongTermPicNum = shd.long_term_pic_num[curr_id];
				RefPicList0[refIdxL0].PicNum = 0;
				RefPicList0[refIdxL0].FrameNum = shd.frame_num;
				RefPicList0[refIdxL0].IsLongTerm = true;
				RefPicList0[refIdxL0].RefPicPresent = true;
				//frameDeepCopy(RefPicList0 + (refIdxL0++));
				frameDeepCopy();

				int nIdx = refIdxL0;
				for (int cIdx = refIdxL0; cIdx <= shd.num_ref_idx_l0_active_minus1+1; cIdx++)
					if (longTermPicNumF(cIdx) != shd.long_term_pic_num[curr_id])
						RefPicList0[nIdx++] = RefPicList0[cIdx];
			}
		} while (shd.modification_of_pic_nums_idc[curr_id++] != 3);
	} else {
				int refIdxL0 = 0, curr_id = 0;
		//int picNumL0Pred = shd.frame_num, picNumL0NoWrap, picNumL0;
		//		if (picNumL0Pred - (shd.abs_diff_pic_num_minus1[curr_id]+1) < 0)
		//			picNumL0NoWrap = picNumL0Pred - (shd.abs_diff_pic_num_minus1[curr_id]+1) + sps.MaxFrameNum;
		//		else
		//			picNumL0NoWrap = picNumL0Pred - (shd.abs_diff_pic_num_minus1[curr_id]+1);
		//		picNumL0Pred = picNumL0NoWrap;
		//		if (picNumL0NoWrap > shd.frame_num)
		//			picNumL0 = picNumL0NoWrap - sps.MaxFrameNum;
		//		else 
		//			picNumL0 = picNumL0NoWrap;

		//		for (int cIdx = shd.num_ref_idx_l0_active_minus1+1; cIdx>refIdxL0; cIdx--)
		//			RefPicList0[cIdx] = RefPicList0[cIdx-1];
				// Postavljanje u listu
				RefPicList0[refIdxL0].PicNum = shd.frame_num;
				RefPicList0[refIdxL0].LongTermPicNum = 0;
				RefPicList0[refIdxL0].FrameNum = shd.frame_num;
				RefPicList0[refIdxL0].IsLongTerm = false;
				RefPicList0[refIdxL0].RefPicPresent = true;
				//frameDeepCopy(RefPicList0 + (refIdxL0++));
				frameDeepCopy();

				//int nIdx = refIdxL0;
				//for (int cIdx = refIdxL0; cIdx <= shd.num_ref_idx_l0_active_minus1+1; cIdx++)
				//	if (picNumF(cIdx) != picNumL0)
				//		RefPicList0[nIdx++] = RefPicList0[cIdx];	
	}
	RefPicList0[shd.num_ref_idx_l0_active_minus1+1].RefPicPresent = false;
}

int selectNALUnitType()
{
	unsigned long sad = 0;
	unsigned long picSizeInMBs = PicWidthInMbs * PicHeightInMbs;
	static unsigned int pFrameRun = 0;

	if (dpb.L == NULL)
	{
		return NAL_UNIT_TYPE_IDR;
	}

	if (OpenCLEnabled == true)
	{
		int frameSize = frame.Lwidth * frame.Lheight;

		unsigned char *result = new unsigned char[frameSize];
		subtractFramesCL(dpb.L, result);

		for (int i = 0; i < frameSize; i++)
		{
			sad += result[i];
		}
		delete [] result;
	}

	else
	{
		for (int i = 0; i < frame.Lheight; i++)
		{
			for (int j = 0; j < frame.Lwidth; j++)
			{
				sad += ABS(frame.L[i*frame.Lwidth+j] - dpb.L[i*frame.Lwidth+j]);
			}
		}
	}

	// The chosen average threshold difference per macroblock
	// is 4096, which corresponds to 16 per pixel. Therefore
	// the treshold SAD is picSizeInMBs * 4096 or picSizeInMBs << 12
	if ((sad > (picSizeInMBs << 12)))
	{
		pFrameRun = 0;
		return NAL_UNIT_TYPE_IDR;
	}
	else
	{
		pFrameRun++;
		return NAL_UNIT_TYPE_NOT_IDR;
	}
}
