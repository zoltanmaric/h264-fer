#pragma once

#include "nal.h"

///////////////////////////////////////////////////////////
/*	Slice header structure.
	Data is arranged in the order of appearance in the RBSP.
*/
///////////////////////////////////////////////////////////

struct SH_data
{

/*	Address of the first macroblock in the current slice. 
	Since this implementation currently produces and reads "one frame per slice" streams,
	this will always be equal to zero. TODO. ?
*/

  int first_mb_in_slice;

/*	Defines the slice type according to the table below:

	0 P (P slice)
	1 B (B slice)
	2 I (I slice)
	3 SP (SP slice)
	4 SI (SI slice)
	5 P (P slice)
	6 B (B slice)
	7 I (I slice)
	8 SP (SP slice)
	9 SI (SI slice)

	NOTE: THESE VALUES ARE NOT EQUAL TO NAL_UNIT_TYPE (where types 1 and 5 are supported)

	Currently supported - "P" and "I" slices.
*/

  int slice_type;

/* ID of the currently used picture parameter set.
	Useful when receiving/creating multiple video streams, useless in this implementation.
	It's basically a number between 0 and 255.
*/

  int pic_parameter_set_id;

/*	Defined as: ( PrevRefFrameNum + 1 ) % MaxFrameNum
	where PrevRefFrameNum is the frame_num of the previously decoded frame.
*/

  int frame_num;

/*	Denotes slices coded as fields instead of frames. Used when interlaced coding is enabled.
	Not implemented, stuck at zero.
*/

  int field_pic_flag;

/*	Denotes frames coded using macroblock adaptive frame-field coding.
	Not implementeded, stuck at zero.
*/

  int MbaffFrameFlag;

/*	Picture height in macroblocks.
*/

  int PicHeightInMbs;

/*	Defined as: PicHeightInSamples = PicHeightInMbs * 16
*/

  int PicHeightInSamples;

/*	Defined as: PicSizeInMbs = PicWidthInMbs * PicHeightInMbs
*/

  int PicSizeInMbs;

/*	Denotes slice that contains bottom field data.
	Used only with interlacing, not implemented, stuck at zero.
*/

  int bottom_field_flag;

/*	All slices describing the same IDR frame must contain same idr_pic_id
	Contains a number from 0 to 65535.
*/

  int idr_pic_id;

/*	Used only with interlaced coding.
	Not implemented.
*/

  int pic_order_cnt_lsb;
  int delta_pic_order_cnt_bottom;
  int delta_pic_order_cnt[2];

/*	Definition from the ITU:
	A coded representation of a picture or a part of a picture. The content of a
	redundant coded picture shall not be used by the decoding process for a bitstream conforming to this
	Recommendation | International Standard. A redundant coded picture is not required to contain all macroblocks in the
	primary coded picture. Redundant coded pictures have no normative effect on the decoding process. See also primary
	coded picture.

	Redundant codings are not implemented.
*/

  int redundant_pic_cnt;

/*	Denotes the method of calculating motion vectors and reference indices for inter prediction.
	0 = Temporal direct mode prediction
	1 = Spatial direct mode prediction
*/

  int direct_spatial_mv_pred_flag;

/*	Used to control the usage of 2 queues with previous frames which are used as a reference.
	TODO:
*/

  int num_ref_idx_active_override_flag;
  int num_ref_idx_l0_active;
  int num_ref_idx_l1_active;

/*	Not implemented.
	TODO:
*/

  int ref_pic_list_modification_flag_l0;
  int no_output_of_prior_pics_flag;
  int long_term_reference_flag;
  int adaptive_ref_pic_marking_mode_flag;
  
  int *modification_of_pic_nums_idc;
  int *abs_diff_pic_num_minus1;
  int *long_term_pic_num;

  int *memory_management_control_operation;
  int *difference_of_pic_nums_minus1;
  int *long_term_frame_idx;
  int *max_long_term_frame_idx_plus1;

/*	Specifies CABAC initialization table, values from 0 to 2.
	Not implemented since only CAVLC is used.
*/

  int cabac_init_idc;

/*	Various quantization flags.
	Specifies initial value for quantization parameters for all macroblocks in the slice (until further modification).
	The initial QPY quantisation parameter for the slice is computed as SliceQPY = 26 + pic_init_qp_minus26 + slice_qp_delta
*/

  int slice_qp_delta;
  int SliceQPy;
  int slice_qs_delta;

/*	Describes decoding of P macroblocks in SP slices.
	Not implemented.
*/

  int sp_for_switch_flag;



/*	Not implemented.
*/
  int slice_group_change_cycle;


  //TODO:
  int num_ref_idx_l0_active_minus1;

  /*	Determines the operation of deblocking filter.
	Not implemented.
*/

  int disable_deblocking_filter_idc;
  int slice_alpha_c0_offset_div2;
  int slice_beta_offset_div2;

};


///////////////////////////////////////////////////////////////////
//Sequence parameter set structure
///////////////////////////////////////////////////////////////////

struct SPS_data {

/*	Parameters profile_idc an level_idc determine the H.264 profile of the current stream.
*/

  int profile_idc;

/*	These flags determine the level of conformance to the norm of the current stream. 
*/

  int constraint_set0_flag;
  int constraint_set1_flag;
  int constraint_set2_flag;

/*	Value of this parameter has to be "00000".
*/

  int reserved_zero_5bits;

/*	Parameters profile_idc an level_idc determine the H.264 profile of the current stream.
*/

  int level_idc;

/*	Determines the identification number of the current SPS element. This data is needed
	to determine on which SPS does the PPS refer to.
*/
  
  int seq_parameter_set_id;

  /*
  TODO: Quick fix for missing data
  */
  int offset_for_non_ref_pic;
  int offset_for_top_to_bottom_field;
  int num_ref_frames_in_pic_order_cnt_cycle;
  int mb_adaptive_frame_field_flag;
  int *offset_for_ref_frame;


/*	The maximal frame number is used in various operation with frame number.
	It is written in the (log2 minus 4) form.
*/

  int log2_max_frame_num;
  int MaxFrameNum;

/*	Determines the method in which the variable "picture_order_cnt" is to be coded.
	Only the natural order of picture coding is supported.
*/

  int pic_order_cnt_type;

/*	Variable maximal picture order count is used in operations with the variable
	pic_order_cnt. It is written in the (log2 lsb minus 4)
*/

  int log2_max_pic_order_cnt_lsb;
  int MaxPicOrderCntLsb;

/*	Value of "1" determines that variables delta_pic_order_cntX will not be present in the slice
	header.
*/

  int delta_pic_order_always_zero_flag;

/*	Maximal allowed number of referenced frames when performing inter prediction
*/

  int max_num_ref_frames;

/*	Allowance of gaps in frame_num variable incremental progression.
	Stuck at "0", gaps are not allowed.
*/

  int gaps_in_frame_num_value_allowed_flag;

/*	Various picture size parameters.
*/

  int PicWidthInMbs;
  int PicWidthInSamples;
  int PicHeightInMapUnits;
  int PicSizeInMapUnits;
  int FrameHeightInMbs;
  int FrameHeightInSamples;

/*	Value of "1" determines that the video stream contains only coded pictures.
	It is stuck at that value in this implementation.
*/

  int frame_mbs_only_flag;

/*	Value is irrelevant since it applies to motion vector derivation in bidirectional slice data,
	which is not supported. Stuck at "0".
*/

  int direct_8x8_inference_flag;

/*	Value of "0" determines that frame cropping is disabled.
	It is stuck at "0" in this implementation.
*/

  int frame_cropping_flag;

/*	Value of "0" determines that video usability information structure is not present.
	It is stuck at "0" in this implementation.
*/

  int vui_parameters_present_flag;

  /*	Used in interlaced mode, value stuck at "0".
*/

  int bottom_field_pic_order_in_frame;
};


/////////////////////////////////////////////////////////////
// Picture parameter set structure
/////////////////////////////////////////////////////////////

struct PPS_data {

/*	Identification number found in all frames that are using this picture parameter set.
*/

  int pic_parameter_set_id;

/*	Identification number of the SPS to which this PPS refers to.	
*/

  int seq_parameter_set_id;

/*	Value of 0 means CAVLC, value of 1 means CABAC.
	Stuck at 0 in this implementation.
*/
 
  int entropy_coding_mode_flag;

/*	Determines the number of slice groups in the current frame. 
	Stuck at "1" in this implementation. 
	Stored in "num_slice_groups_minus_1" form in the bitstream (so default value is stuck at "0" in bitstream).
*/

  int num_slice_groups;

  /*
  TODO: Quick fix for missing data
  */
    int bottom_field_pic_order_in_frame;

/*	Long-term prediction data, not implemented.
	TODO
*/

  int num_ref_idx_l0_active;
  int num_ref_idx_l1_active;

/*	Weighted prediction and biprediction is not supported, both flags stuck at "0".
*/

  int weighted_pred_flag;
  int weighted_bipred_idc;

/*	Various quantization parameters.
*/

  int pic_init_qp;
  int pic_init_qs;
  int chroma_qp_index_offset;

/*	Deblocking filter not implemented, value stuck at "0".
*/

  int deblocking_filter_control_present_flag;

/*	Constrainted intra prediction is currently forced, value stuck at "1".
*/

  int constrained_intra_pred_flag;

/*	Redundant frames are not supported, value stuck at "0".
*/

  int redundant_pic_cnt_present_flag;
};

extern struct SPS_data sps;
extern struct PPS_data pps;
extern struct SH_data shd;

void fill_sps(NALunit *);
void fill_pps(NALunit *);
void fill_shd(NALunit *);