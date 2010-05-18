
__kernel void 
absDiff(__global int *a,
	__global int *b,
	__global int *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}