#include <stdlib.h>
#include <stdio.h>

#include "h264_globals.h"
	
unsigned char *r;
unsigned char *g;
unsigned char *b;

void toRGB()
{
	r = new unsigned char[frame.Lwidth * frame.Lheight];
	g = new unsigned char[frame.Lwidth * frame.Lheight];
	b = new unsigned char[frame.Lwidth * frame.Lheight];

	for (int i = 0; i < frame.Lheight; i++)
	{
		for (int j = 0; j < frame.Lwidth; j++)
		{
			r[i*frame.Lwidth + j] = 1.164*(frame.L[i*frame.Lwidth + j] - 16) + 1.596*(frame.C[1][(i/2)*frame.Cwidth + (j/2)] - 128);
			g[i*frame.Lwidth + j] = 1.164*(frame.L[i*frame.Lwidth + j] - 16) - 0.813*(frame.C[1][(i/2)*frame.Cwidth + (j/2)] - 128) - 0.391*(frame.C[0][(i/2)*frame.Cwidth + (j/2)] - 128);
			b[i*frame.Lwidth + j] = 1.164*(frame.L[i*frame.Lwidth + j] - 16) + 2.018*(frame.C[0][(i/2)*frame.Cwidth + (j/2)] - 128);
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
		slika[k++] = r[i];
		slika[k++] = g[i];
		slika[k++] = b[i];
	}

	fwrite(slika, 1, frame.Lwidth * frame.Lheight, f);
}