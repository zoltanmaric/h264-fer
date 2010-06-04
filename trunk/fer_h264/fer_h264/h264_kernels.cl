kernel void 
AbsDiff(global uchar16 *a, global uchar16 *b, global uchar16 *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}

// Global work size: size of input buffer / 4
kernel void CharToInt(global unsigned int *input, global int *output)
{
	int gid = get_global_id(0);
	for (int i = 0; i < 4; i++)
	{
		int oid = (gid * 4) + i;
		output[oid] = (input[gid] >> (8 * i)) & 0xff;
	}
}