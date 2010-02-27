#include "rawreader.h"
#include "expgolomb.h"
#include "nal.h"
#include "h264_globals.h"
#include "headers_and_parameter_sets.h"

struct SPS_data sps;
struct PPS_data pps;
struct SH_data shd;

//////////////////////////////////////////////////////////////////
//Reading slice header
//////////////////////////////////////////////////////////////////

void fill_shd(NALunit *nal_unit)
{
  shd.first_mb_in_slice			=expGolomb_UD();
  shd.slice_type				=expGolomb_UD()%5;
  shd.pic_parameter_set_id		=expGolomb_UD();
  shd.frame_num					=getRawBits(sps.log2_max_frame_num);
  
  shd.PicHeightInMbs=sps.FrameHeightInMbs;
  shd.PicHeightInSamples=(shd.PicHeightInMbs)<<4;
  shd.PicSizeInMbs=sps.PicWidthInMbs*shd.PicHeightInMbs;

  if(nal_unit->nal_unit_type==5)
    shd.idr_pic_id				=expGolomb_UD();

  shd.pic_order_cnt_lsb			=getRawBits(sps.log2_max_pic_order_cnt_lsb);

  if((shd.slice_type%5)==P_SLICE || (shd.slice_type%5)==B_SLICE || (shd.slice_type%5)==SP_SLICE)
  {
    shd.num_ref_idx_active_override_flag=getRawBits(1);
  }

  if((shd.slice_type%5)!=I_SLICE && (shd.slice_type%5)!=SI_SLICE)
  {
    shd.ref_pic_list_reordering_flag_l0		=getRawBits(1);
  }

  if(nal_unit->nal_ref_idc!=0)
  {
    if(nal_unit->nal_unit_type==5)
	{
      shd.no_output_of_prior_pics_flag		=getRawBits(1);
      shd.long_term_reference_flag			=getRawBits(1);
    }
	else
	{
	  //Adaptive ref pic marking mode is TODO
      shd.adaptive_ref_pic_marking_mode_flag=getRawBits(1);
    }
  }

  shd.slice_qp_delta						=expGolomb_SD();
  shd.SliceQPy=pps.pic_init_qp+shd.slice_qp_delta;

  if((shd.slice_type%5)==SP_SLICE || (shd.slice_type%5)==SI_SLICE)
  {
    if((shd.slice_type%5)==SP_SLICE)
	{
      shd.sp_for_switch_flag				=getRawBits(1);
	}
    shd.slice_qs_delta						=expGolomb_SD();
  }
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

	//Derived from maximal frame number
	sps.MaxFrameNum=1<<sps.log2_max_frame_num;

	//Unsigned expGolomb
	sps.bottom_field_pic_order_in_frame=expGolomb_UD();

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

