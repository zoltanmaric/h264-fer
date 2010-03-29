#include "rawreader.h"
#include "expgolomb.h"
#include "nal.h"
#include "h264_globals.h"
#include "headers_and_parameter_sets.h"

struct SPS_data sps;
struct PPS_data pps;
struct SH_data shd;

// (7.3.3.1) Reference picture list modification syntax

void ref_pic_list_modification_write()
{	
	shd.ref_pic_list_modification_flag_l0 = 0;	// no changes in reference picture lists yet

	if((shd.slice_type%5)!=I_SLICE && (shd.slice_type%5)!=SI_SLICE)
	{
		writeFlag(shd.ref_pic_list_modification_flag_l0);

		if (shd.ref_pic_list_modification_flag_l0==1)
		{
			int i = 0, j = 0, k = 0;
			do
			{
				expGolomb_UC(shd.modification_of_pic_nums_idc[i]);

				if (shd.modification_of_pic_nums_idc[i] == 0 || shd.modification_of_pic_nums_idc[i] == 1)
				{
					expGolomb_UC(shd.abs_diff_pic_num_minus1[j++]);
				}
				else if (shd.modification_of_pic_nums_idc[i] == 2)
				{
					expGolomb_UC(shd.long_term_pic_num[k++]);
				}
			} while (shd.modification_of_pic_nums_idc[i++] != 3);
		}
	}

	// Norm: slice_type%5 cannot be 1 in baseline (no B frames)

}

void ref_pic_list_modification()
{
	if((shd.slice_type%5)!=I_SLICE && (shd.slice_type%5)!=SI_SLICE)
	{
		shd.ref_pic_list_modification_flag_l0		=getRawBits(1);

		if (shd.ref_pic_list_modification_flag_l0)
		{
			int i = 0, j = 0, k = 0;
			do
			{
				shd.modification_of_pic_nums_idc[i]	=expGolomb_UD();
				if (shd.modification_of_pic_nums_idc[i] == 0 ||
					shd.modification_of_pic_nums_idc[i] == 1)
				{
					shd.abs_diff_pic_num_minus1[j++]	=expGolomb_UD();
				}
				else if (shd.modification_of_pic_nums_idc[i] == 2)
				{
					shd.long_term_pic_num[k++]			=expGolomb_UD();
				}
			} while (shd.modification_of_pic_nums_idc[i++] != 3);
		}
	}

	// Norm: slice_type%5 cannot be 1 in baseline (no B frames)
}

// (7.3.3.3) Decoded reference picture marking syntax
void dec_ref_pic_marking_write(bool IdrPicFlag)
{
	// inferred values:
	shd.no_output_of_prior_pics_flag = 0;	// pictures are output in decoding order
	shd.long_term_reference_flag = 0;		// no long term reference pictures yet
	shd.adaptive_ref_pic_marking_mode_flag = 0;	// no adaptive ref pic marking yet

	if (IdrPicFlag)
	{
		writeFlag(shd.no_output_of_prior_pics_flag);
		writeFlag(shd.long_term_reference_flag);
	}
	else
	{
		writeFlag(shd.adaptive_ref_pic_marking_mode_flag);

		if (shd.adaptive_ref_pic_marking_mode_flag)
		{
			int i = 0, j = 0, k = 0, l = 0, m = 0;

			// Because the last "operation" is number 0
			//number_of_mmc_operations=-1;
			do
			{
				expGolomb_UC(shd.memory_management_control_operation[i]);

				//number_of_mmc_operations++;

				if (shd.memory_management_control_operation[i] == 1 || shd.memory_management_control_operation[i] == 3)
				{
					expGolomb_UC(shd.difference_of_pic_nums_minus1[j++]);
				}
				if (shd.memory_management_control_operation[i] == 2)
				{
					expGolomb_UC(shd.long_term_pic_num[k++]);
				}
				if (shd.memory_management_control_operation[i] == 3 || shd.memory_management_control_operation[i] == 6)
				{
					expGolomb_UC(shd.long_term_frame_idx[l++]);
				}
				if (shd.memory_management_control_operation[i] == 4)
				{
					expGolomb_UC(shd.max_long_term_frame_idx_plus1[m++]);
				}
			} while (shd.memory_management_control_operation[i++] != 0);
		}
	}
}

void dec_ref_pic_marking(bool IdrPicFlag)
{
	if (IdrPicFlag)
	{
		shd.no_output_of_prior_pics_flag		=getRawBits(1);
		shd.long_term_reference_flag			=getRawBits(1);
	}
	else
	{
		shd.adaptive_ref_pic_marking_mode_flag	=getRawBits(1);

		if (shd.adaptive_ref_pic_marking_mode_flag)
		{
			int i = 0, j = 0, k = 0, l = 0, m = 0;

			// Because the last "operation" is number 0
			//number_of_mmc_operations=-1;
			do
			{
				shd.memory_management_control_operation[i] =expGolomb_UD();

				//number_of_mmc_operations++;

				if (shd.memory_management_control_operation[i] == 1 ||
					shd.memory_management_control_operation[i] == 3)
				{
					shd.difference_of_pic_nums_minus1[j++]	=expGolomb_UD();
				}
				if (shd.memory_management_control_operation[i] == 2)
				{
					shd.long_term_pic_num[k++]				=expGolomb_UD();
				}
				if (shd.memory_management_control_operation[i] == 3 ||
					shd.memory_management_control_operation[i] == 6)
				{
					shd.long_term_frame_idx[l++]			=expGolomb_UD();
				}
				if (shd.memory_management_control_operation[i] == 4)
				{
					shd.max_long_term_frame_idx_plus1[m++]	=expGolomb_UD();
				}
			} while (shd.memory_management_control_operation[i++] != 0);
		}
	}
}

//////////////////////////////////////////////////////////////////
//Writing slice header
//////////////////////////////////////////////////////////////////

void shd_write(NALunit &nal_unit)
{
	// inferred values:
	shd.first_mb_in_slice = 0;
	shd.pic_parameter_set_id = 0;	// always zero because there's only one pps
	shd.pic_order_cnt_lsb = 0;
	shd.num_ref_idx_active_override_flag = 0;	// no changes in the reference picture list order yet
	shd.slice_qp_delta = -14;		// inferred quantization parameter
	shd.PicSizeInMbs = frame.Lwidth * frame.Lheight >> 8;

	unsigned char buffer[4];

	expGolomb_UC(shd.first_mb_in_slice);
	expGolomb_UC(shd.slice_type);
	expGolomb_UC(shd.pic_parameter_set_id);

	UINT_to_RBSP_size_known(shd.frame_num,sps.log2_max_frame_num,buffer);
	writeRawBits(sps.log2_max_frame_num,buffer); 

	if(nal_unit.nal_unit_type==5)
	{
		expGolomb_UC(shd.idr_pic_id);
	}

	UINT_to_RBSP_size_known(shd.pic_order_cnt_lsb,sps.log2_max_pic_order_cnt_lsb,buffer);
	writeRawBits(sps.log2_max_pic_order_cnt_lsb,buffer);

	if((shd.slice_type%5)==P_SLICE || (shd.slice_type%5)==B_SLICE || (shd.slice_type%5)==SP_SLICE)
	{
		if (shd.num_ref_idx_active_override_flag==1)
		{
			writeOnes(1);
		}
		else
		{
			writeZeros(1);
		}
		
		if (shd.num_ref_idx_active_override_flag==1)
		{
			expGolomb_UC(shd.num_ref_idx_l0_active_minus1);
		}

	}

	//TODO: check upper if-clause and the function below - double reading/writing?
	ref_pic_list_modification_write();

	if(nal_unit.nal_ref_idc!=0)
	{
		bool IdrPicFlag = (nal_unit.nal_unit_type == 5);
		dec_ref_pic_marking_write(IdrPicFlag);
	}

	expGolomb_SC(shd.slice_qp_delta);

	//Deblocking filter flag is inferred as NOT PRESENT.
	//This is copy-paste from shd reading functions
	/*
	if(pps.deblocking_filter_control_present_flag==1)
	{
		shd.disable_deblocking_filter_idc	= expGolomb_UD();
		if (shd.disable_deblocking_filter_idc != 1)
		{
			shd.slice_alpha_c0_offset_div2	= expGolomb_SD();
			shd.slice_beta_offset_div2		= expGolomb_SD();
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////
//Reading slice header
//////////////////////////////////////////////////////////////////

void fill_shd(NALunit *nal_unit)
{
	shd.first_mb_in_slice			=expGolomb_UD();
	shd.slice_type				=expGolomb_UD();
	shd.pic_parameter_set_id		=expGolomb_UD();

	//From the norm:
	/*
	frame_num is used as an identifier for pictures and shall be represented by log2_max_frame_num_minus4 + 4 bits in
	the bitstream.
	*/

	shd.frame_num					=getRawBits(sps.log2_max_frame_num);

	shd.PicHeightInMbs=sps.FrameHeightInMbs;
	shd.PicHeightInSamples=(shd.PicHeightInMbs)<<4;
	shd.PicSizeInMbs=sps.PicWidthInMbs*shd.PicHeightInMbs;

	if(nal_unit->nal_unit_type==5)
	{
		shd.idr_pic_id				=expGolomb_UD();
	}

	shd.pic_order_cnt_lsb			=getRawBits(sps.log2_max_pic_order_cnt_lsb);

	if((shd.slice_type%5)==P_SLICE || (shd.slice_type%5)==B_SLICE || (shd.slice_type%5)==SP_SLICE)
	{
		shd.num_ref_idx_active_override_flag=getRawBits(1);
		if (shd.num_ref_idx_active_override_flag==1)
		{
			shd.num_ref_idx_l0_active_minus1	=expGolomb_UD();
		}
	}

	// These variables are stored for the reference picture
	// list modification process described in 8.2.4.3
	shd.modification_of_pic_nums_idc = new int[sps.MaxFrameNum];
	shd.abs_diff_pic_num_minus1 = new int[sps.MaxFrameNum];
	shd.long_term_pic_num = new int[sps.MaxFrameNum];

	shd.memory_management_control_operation = new int[sps.MaxFrameNum];
	shd.difference_of_pic_nums_minus1 = new int[sps.MaxFrameNum];
	shd.long_term_frame_idx = new int[sps.MaxFrameNum];
	shd.max_long_term_frame_idx_plus1 = new int[sps.MaxFrameNum];

	ref_pic_list_modification();

	if(nal_unit->nal_ref_idc!=0)
	{
		bool IdrPicFlag = (nal_unit->nal_unit_type == 5);
		dec_ref_pic_marking(IdrPicFlag);
	}

	shd.slice_qp_delta						=expGolomb_SD();
	shd.SliceQPy=pps.pic_init_qp+shd.slice_qp_delta;

	if(pps.deblocking_filter_control_present_flag==1)
	{
		shd.disable_deblocking_filter_idc	= expGolomb_UD();
		if (shd.disable_deblocking_filter_idc != 1)
		{
			shd.slice_alpha_c0_offset_div2	= expGolomb_SD();
			shd.slice_beta_offset_div2		= expGolomb_SD();
		}
	}
}

/////////////////////////////////////////////////
/// Writing sequence parameter set
/////////////////////////////////////////////////

void sps_write()
{
	sps.profile_idc = 66;
	sps.constraint_set0_flag = 1;
	sps.constraint_set1_flag = 1;
	sps.constraint_set2_flag = 0;
	sps.reserved_zero_5bits = 0;
	sps.level_idc = 41;
	sps.seq_parameter_set_id = 0;
	sps.offset_for_non_ref_pic = 0;
	sps.offset_for_top_to_bottom_field = 0;
	sps.num_ref_frames_in_pic_order_cnt_cycle = 0;
	sps.mb_adaptive_frame_field_flag = 0;
	//sps.offset_for_ref_frame
	sps.log2_max_frame_num = 9;
	sps.pic_order_cnt_type = 0;
	sps.log2_max_pic_order_cnt_lsb = 10;
	sps.delta_pic_order_always_zero_flag = 0;
	sps.max_num_ref_frames = 1;
	sps.gaps_in_frame_num_value_allowed_flag = 0;
	sps.frame_mbs_only_flag = 1;
	sps.direct_8x8_inference_flag = 1;
	sps.frame_cropping_flag = 0;
	sps.vui_parameters_present_flag = 0;

	// TODO: handle non-multiple-of-16 frame dimensions
	sps.PicWidthInMbs = frame.Lwidth >> 4;	// == /16
	PicWidthInMbs = sps.PicWidthInMbs;
	sps.PicHeightInMapUnits = frame.Lheight >> 4;
	sps.FrameHeightInMbs = sps.PicHeightInMapUnits;

	unsigned char buffer[4];
	
	UINT_to_RBSP_size_known(sps.profile_idc, 8, buffer);
	writeRawBits(8, buffer);

	writeFlag(sps.constraint_set0_flag);
	writeFlag(sps.constraint_set1_flag);
	writeFlag(sps.constraint_set2_flag);
	writeZeros(5);

	UINT_to_RBSP_size_known(sps.level_idc, 8, buffer);
	writeRawBits(8, buffer);

	expGolomb_UC(sps.seq_parameter_set_id);
	expGolomb_UC(sps.log2_max_frame_num-4);
	expGolomb_UC(sps.pic_order_cnt_type);

	if (sps.pic_order_cnt_type==0)
	{
		expGolomb_UC(sps.log2_max_pic_order_cnt_lsb-4);
	}
	else if (sps.pic_order_cnt_type==1)
	{
		writeFlag(sps.delta_pic_order_always_zero_flag);
		expGolomb_SC(sps.offset_for_non_ref_pic);
		expGolomb_SC(sps.offset_for_top_to_bottom_field);
		expGolomb_UC(sps.num_ref_frames_in_pic_order_cnt_cycle);
		for(int i=0; i<sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
		{
			expGolomb_SC(sps.offset_for_ref_frame[i]);
		}
	}

	expGolomb_UC(sps.max_num_ref_frames);
	writeFlag(sps.gaps_in_frame_num_value_allowed_flag);
	expGolomb_UC(sps.PicWidthInMbs-1);
	expGolomb_UC(sps.PicHeightInMapUnits-1);
	writeFlag(sps.frame_mbs_only_flag);

	if (sps.frame_mbs_only_flag)
	{
		writeFlag(sps.mb_adaptive_frame_field_flag);
	}

	writeFlag(sps.direct_8x8_inference_flag);
	writeFlag(sps.frame_cropping_flag);
	writeFlag(sps.vui_parameters_present_flag);
	
}

/////////////////////////////////////////////////
/// Reading sequence parameter set
/////////////////////////////////////////////////


void fill_sps(NALunit *nal_unit)
{
	//One byte
	sps.profile_idc=getRawBits(8);

	//NORMA JE PROMIJENJENA, SADA JE 5+3
	//3+5 bits
	sps.constraint_set0_flag=getRawBits(1);
	sps.constraint_set1_flag=getRawBits(1);
	sps.constraint_set2_flag=getRawBits(1);
	sps.reserved_zero_5bits=getRawBits(5);

	//One byte
	sps.level_idc=getRawBits(8);

	//TODO: provjeri profil

	//Unsigned expGolomb
	sps.seq_parameter_set_id=expGolomb_UD();

	//Unsigned expGolomb
	sps.log2_max_frame_num=expGolomb_UD()+4;

	//Derived from maximum frame number
	sps.MaxFrameNum=1<<sps.log2_max_frame_num;
	// TODO: invoke RefPicList0 initialization
	// according to MaxFrameNum

	//Unsigned expGolomb
	sps.pic_order_cnt_type=expGolomb_UD();

	//Unsigned expGolomb+derived from maximal picture order count
	if(sps.pic_order_cnt_type==0)
	{
		sps.log2_max_pic_order_cnt_lsb=expGolomb_UD()+4;
		sps.MaxPicOrderCntLsb=1<<sps.log2_max_pic_order_cnt_lsb;
	}
	//One bit + SEG + SEG + UEG + n*SED
	else if(sps.pic_order_cnt_type==1)
	{
		sps.delta_pic_order_always_zero_flag=getRawBits(1);
		sps.offset_for_non_ref_pic=expGolomb_SD();
		sps.offset_for_top_to_bottom_field=expGolomb_SD();
		sps.num_ref_frames_in_pic_order_cnt_cycle=expGolomb_UD();

		sps.offset_for_ref_frame=new int[sps.num_ref_frames_in_pic_order_cnt_cycle];

		for(int i=0; i<sps.num_ref_frames_in_pic_order_cnt_cycle; ++i)
		  sps.offset_for_ref_frame[i]=expGolomb_SD();
	}

	sps.max_num_ref_frames=expGolomb_UD();
	sps.gaps_in_frame_num_value_allowed_flag=getRawBits(1);
	sps.PicWidthInMbs=expGolomb_UD()+1;
	sps.PicWidthInSamples=sps.PicWidthInMbs*16;

	sps.PicHeightInMapUnits=expGolomb_UD()+1;

	sps.PicSizeInMapUnits=sps.PicWidthInMbs*sps.PicHeightInMapUnits;
	
	sps.frame_mbs_only_flag=getRawBits(1);
	
	sps.FrameHeightInMbs=(2-sps.frame_mbs_only_flag)*sps.PicHeightInMapUnits;
	sps.FrameHeightInSamples=16*sps.FrameHeightInMbs;
	if(!sps.frame_mbs_only_flag)
		sps.mb_adaptive_frame_field_flag=getRawBits(1);
	sps.direct_8x8_inference_flag=getRawBits(1);
	sps.frame_cropping_flag=getRawBits(1);
	sps.vui_parameters_present_flag=getRawBits(1);

	if (sps.vui_parameters_present_flag==1)
	{
		printf("SPS decoding -> VUI is present but ignored.\n");
	}
}

/////////////////////////////////////////////////
/// Writing picture parameter set
/////////////////////////////////////////////////

void pps_write()
{
	unsigned char buffer[4];

	pps.pic_parameter_set_id = 0;
	pps.seq_parameter_set_id = 0;
	pps.entropy_coding_mode_flag = 0;
	pps.num_slice_groups = 1;
	pps.bottom_field_pic_order_in_frame = 0;
	pps.num_ref_idx_l0_active = 1;
	pps.num_ref_idx_l1_active = 1;
	pps.weighted_pred_flag = 0;
	pps.weighted_bipred_idc = 0;
	pps.pic_init_qp = 26;
	pps.pic_init_qs = 26;
	pps.chroma_qp_index_offset = 0;
	pps.deblocking_filter_control_present_flag = 0;
	pps.constrained_intra_pred_flag = 0;
	pps.redundant_pic_cnt_present_flag = 0;

	expGolomb_UC(pps.pic_parameter_set_id);
	expGolomb_UC(pps.seq_parameter_set_id);
	writeFlag(pps.entropy_coding_mode_flag);
	writeFlag(pps.bottom_field_pic_order_in_frame);
	expGolomb_UC(pps.num_slice_groups-1);
	expGolomb_UC(pps.num_ref_idx_l0_active-1);
	expGolomb_UC(pps.num_ref_idx_l1_active-1);
	writeFlag(pps.weighted_pred_flag);

	UINT_to_RBSP_size_known(pps.num_ref_idx_l1_active,2,buffer);
	writeRawBits(2,buffer);
	expGolomb_SC(pps.pic_init_qp-26);
	expGolomb_SC(pps.pic_init_qs-26);
	expGolomb_SC(pps.chroma_qp_index_offset);
	writeFlag(pps.deblocking_filter_control_present_flag);
	writeFlag(pps.constrained_intra_pred_flag);
	writeFlag(pps.redundant_pic_cnt_present_flag);

}

/////////////////////////////////////////////////
/// Reading picture parameter set
/////////////////////////////////////////////////

void fill_pps(NALunit *nal_unit)
{
	pps.pic_parameter_set_id=expGolomb_UD();
	pps.seq_parameter_set_id=expGolomb_UD();
	pps.entropy_coding_mode_flag=getRawBits(1);
	pps.bottom_field_pic_order_in_frame=getRawBits(1);
	pps.num_slice_groups=expGolomb_UD()+1;
	pps.num_ref_idx_l0_active=expGolomb_UD()+1;
	pps.num_ref_idx_l1_active=expGolomb_UD()+1;
	pps.weighted_pred_flag=getRawBits(1);
	pps.weighted_bipred_idc=getRawBits(2);
	pps.pic_init_qp=expGolomb_SD()+26;
	pps.pic_init_qs=expGolomb_SD()+26;
	pps.chroma_qp_index_offset=expGolomb_SD();
	pps.deblocking_filter_control_present_flag=getRawBits(1);
	pps.constrained_intra_pred_flag=getRawBits(1);
	pps.redundant_pic_cnt_present_flag=getRawBits(1);
}

