#pragma once

typedef struct {
	int FrameNum, FrameNumWrap;
	int PicNum, LongTermFrameIdx, LongTermPicNum;
	bool RefPicPresent, IsLongTerm;
	frame_type * frame;
} ref_pic_type;

extern ref_pic_type RefPicList0[50000];

void frameDeepCopy(frame_type * OldFrame, frame_type * NewFrame);

void decodePictureNumbers();

void initialisationProcess();

void modificationProcess();

int selectNALUnitType();