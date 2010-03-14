#include <stdlib.h>
#include <stdio.h>

#include "h264_globals.h"

int R[816][1920],G[816][1920],B[816][1920];

int *red;
int *green;
int *blue;

void toRGB()
{
	red = new int[frame.Lwidth * frame.Lheight];
	green = new int[frame.Lwidth * frame.Lheight];
	blue = new int[frame.Lwidth * frame.Lheight];

	int lumaIndex, chromaIndex;
	int r_shift, g_shift, b_shift;
	int y_shift, cb_shift, cr_shift;
	for (int i = 0; i < frame.Lheight; i++)
	{
		for (int j = 0; j < frame.Lwidth; j++)
		{
			lumaIndex = i*frame.Lwidth + j;
			chromaIndex = (i/2)*frame.Cwidth + (j/2);
			y_shift = (frame.L[lumaIndex] - 16) << 10;
			cb_shift = (frame.C[0][chromaIndex] - 128) << 10;
			cr_shift = (frame.C[1][chromaIndex] - 128) << 10;

			// 1.164 << 10 == 1192
			// 1.596 << 10 == 1634
			// 0.391 << 10 == 401
			// 0.813 << 10 == 832
			// 2.018 << 10 == 2066
			
			r_shift = 1192 * y_shift + 1634 * cr_shift;
			g_shift = 1192 * y_shift + 401 * cb_shift - 832 * cr_shift;
			b_shift = 1192 * y_shift + 2066 * cb_shift;

			//red[lumaIndex] = 1.164*(frame.L[lumaIndex] - 16) + 1.596*(frame.C[1][chromaIndex] - 128);
			//green[lumaIndex] = 1.164*(frame.L[lumaIndex] - 16) - 0.391*(frame.C[0][chromaIndex] - 128) - 0.813*(frame.C[1][chromaIndex] - 128);
			//blue[lumaIndex] = 1.164*(frame.L[lumaIndex] - 16) + 2.018*(frame.C[0][chromaIndex] - 128);

			red[lumaIndex] = r_shift >> 20;
			green[lumaIndex] = g_shift >> 20;
			blue[lumaIndex] = b_shift >> 20;

			if (red[lumaIndex] > 255) red[lumaIndex] = 255;
			else if (red[lumaIndex] < 0) red[lumaIndex] = 0;

			if (green[lumaIndex] > 255)	green[lumaIndex] = 255;
			else if (green[lumaIndex] < 0) green[lumaIndex] = 0;

			if (blue[lumaIndex] > 255) blue[lumaIndex] = 255;
			else if (blue[lumaIndex] < 0) blue[lumaIndex] = 0;
		}
	}
}

void writeToPPM_bringer(int frameCount)
{
	FILE *out;

	char filename[50];
	sprintf(filename, "frame%d.ppm", frameCount);
    out=fopen(filename,"w");
    fprintf(out,"P3\n1920 816\n255\n");
    int i,j;

    for (i=0;i<816;i++)
	{
    for (j=0;j<1920;j++)
    {
		B[i][j] = 1.164*(frame.L[i*frame.Lwidth+j]- 16)                   + 2.018*(frame.C[0][(i/2)*frame.Cwidth+(j/2)] - 128);
		G[i][j] = 1.164*(frame.L[i*frame.Lwidth+j] - 16) - 0.813*(frame.C[1][(i/2)*frame.Cwidth+(j/2)] - 128) - 0.391*(frame.C[0][(i/2)*frame.Cwidth+(j/2)] - 128);
		R[i][j] = 1.164*(frame.L[i*frame.Lwidth+j] - 16) + 1.596*(frame.C[1][(i/2)*frame.Cwidth+(j/2)] - 128);
		if (R[i][j]>255) R[i][j]=255;
		if (G[i][j]>255) G[i][j]=255;
		if (B[i][j]>255) B[i][j]=255;
		if (R[i][j]<0) R[i][j]=0;
		if (G[i][j]<0) G[i][j]=0;
		if (B[i][j]<0) B[i][j]=0;

		fprintf(out,"%d %d %d ",R[i][j],G[i][j],B[i][j]);
    }
		fprintf(out,"\n");
	}
    
    fclose(out);
}

void writeToPPM(int frameCount)
{
	toRGB();

	char *pic;
	pic = new char[frame.Lwidth * frame.Lheight * 12 + frame.Lheight];
	
	int pic_size = frame.Lheight * frame.Lwidth;
	int k = 0;
	for (int i = 0; i < pic_size; i++)
	{
		k += sprintf(pic + k, "%d %d %d ", red[i], green[i], blue[i]);
		
		if (i % frame.Lwidth == frame.Lwidth - 1)
		{
			pic[k++] = '\n';
		}
	}

	FILE *f;
	char filename[50];
	sprintf(filename, "frame%d.ppm", frameCount);
	f = fopen(filename,"wt");
	fprintf(f, "P3\n%d %d\n255\n", frame.Lwidth, frame.Lheight);
	fwrite(pic, 1, k, f);
	fclose(f);
}