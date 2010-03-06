#include <stdlib.h>
#include <stdio.h>

#include "h264_globals.h"
	
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