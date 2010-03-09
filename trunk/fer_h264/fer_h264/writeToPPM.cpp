#include <stdlib.h>
#include <stdio.h>

#include "h264_globals.h"

int R[816][1920],G[816][1920],B[816][1920];

unsigned char *red;
unsigned char *green;
unsigned char *blue;

void toRGB()
{
	red = new unsigned char[frame.Lwidth * frame.Lheight];
	green = new unsigned char[frame.Lwidth * frame.Lheight];
	blue = new unsigned char[frame.Lwidth * frame.Lheight];

	for (int i = 0; i < frame.Lheight; i++)
	{
		for (int j = 0; j < frame.Lwidth; j++)
		{
			red[i*frame.Lwidth + j] = 1.164*(frame.L[i*frame.Lwidth + j] - 16) + 1.596*(frame.C[1][(i/2)*frame.Cwidth + (j/2)] - 128);
			green[i*frame.Lwidth + j] = 1.164*(frame.L[i*frame.Lwidth + j] - 16) - 0.813*(frame.C[1][(i/2)*frame.Cwidth + (j/2)] - 128) - 0.391*(frame.C[0][(i/2)*frame.Cwidth + (j/2)] - 128);
			blue[i*frame.Lwidth + j] = 1.164*(frame.L[i*frame.Lwidth + j] - 16) + 2.018*(frame.C[0][(i/2)*frame.Cwidth + (j/2)] - 128);
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
    char znak;
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

void writeToPPM()
{
	toRGB();

	FILE *f;
	f = fopen("slika.ppm", "wt");
	fprintf(f, "P6\n%d\n%d\n255\n", frame.Lwidth, frame.Lheight);
	unsigned char *slika;
	slika = new unsigned char[frame.Lwidth * frame.Lheight * 3];
	int k = 0;
	for (int i = 0; i < frame.Lwidth * frame.Lheight; i++)
	{
		slika[k++] = red[i];
		slika[k++] = green[i];
		slika[k++] = blue[i];
	}

	fwrite(slika, 1, frame.Lwidth * frame.Lheight * 3, f);
	fclose(f);
}