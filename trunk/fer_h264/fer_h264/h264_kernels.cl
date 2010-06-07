kernel void AbsDiff(global uchar16 *a, global uchar16 *b, global uchar16 *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}