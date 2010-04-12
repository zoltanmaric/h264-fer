#include "residual_tables.h"

int levelcode_to_outputstream[MAX_LEVELCODE_VALUE][7][4];
int inputstream_to_levelcode[16][7][MAX_SUFFIX_VALUE];

void generate_residual_level_tables()
{
	for (int i=0;i<MAX_LEVELCODE_VALUE;i++)
	{
		//There are 7 VLC tables
		for (int j=0;j<7;j++)
		{
			//Maximal prefix value is 15, maximal suffix value is 12
			levelcode_to_outputstream[i][j][0]=30;
		}
	}

	//Simulation of the level decoding (by the norm)
	int level_prefix;
	int suffixLength;
	int levelCode,level_suffix;
	int levelSuffixSize;
	
	for (int i=0;i<16;i++)
	{
		for (int j=0;j<7;j++)
		{
			level_prefix=i;
			suffixLength=j;
			if (level_prefix==14 && suffixLength==0)
			{
				levelSuffixSize=4;
			}
			else if (level_prefix>=15)
			{
				levelSuffixSize=(level_prefix-3);
			}
			else
			{
				levelSuffixSize=suffixLength;
			}

			for (int k=0;k<(1<<levelSuffixSize);k++)
			{
				levelCode=(((level_prefix<15)?level_prefix:15)<<suffixLength);

				if (levelSuffixSize>0 || level_prefix>=14)
				{
					level_suffix=k;
					levelCode+=level_suffix;
				}
				else
				{
						level_suffix=0;
				}

				if (level_prefix>=15 && suffixLength==0)
				{
					levelCode += 15;
				}

				inputstream_to_levelcode[i][j][k]=levelCode;
				if (levelcode_to_outputstream[levelCode][j][0]>=i+levelSuffixSize)
				{
					levelcode_to_outputstream[levelCode][j][0]=i+levelSuffixSize;
					levelcode_to_outputstream[levelCode][j][1]=i;
					levelcode_to_outputstream[levelCode][j][2]=levelSuffixSize;
					levelcode_to_outputstream[levelCode][j][3]=k;
				}
			}
		}
	}
}