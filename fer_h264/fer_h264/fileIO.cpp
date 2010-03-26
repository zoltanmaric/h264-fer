#include <string.h>

#include "fileIO.h"
#include "h264_globals.h"

int *red;
int *green;
int *blue;

FILE *yuvoutput;
FILE *yuvinput;

// DECODING
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

void writeToPPM()
{
	toRGB();

	char *pic;
	pic = new  char[frame.Lwidth * frame.Lheight * 3 + 17];
	int pic_size = frame.Lheight * frame.Lwidth;
	int pos = sprintf(pic, "P6\n%d %d\n255\n", frame.Lwidth, frame.Lheight);
	for (int i = 0; i < pic_size; i++)
	{
		pic[pos++] = red[i];
		pic[pos++] = green[i];
		pic[pos++] = blue[i];
	}

	FILE *f;
	char filename[15];
	sprintf(filename, "frame%d%d%d%d.ppm", (frameCount%10000)/1000, (frameCount%1000)/100, (frameCount%100)/10, frameCount%10);
	f = fopen(filename,"wb");
	fwrite(pic, 1, pos, f);

	fclose(f);
	free(red);
	free(green);
	free(blue);
	free(pic);
}

void writeToY4M()
{
	//static unsigned long frameCount = 0;
	static bool firstFrame = true;
	int i;

	char *output;
	output = new char[5000000];

	unsigned int pos = 0;
	if (firstFrame)
	{
		// Write file header
		pos = sprintf(output, "YUV4MPEG2 W%d H%d F24000:1001 Ip A1:1 C420 %c", frame.Lwidth, frame.Lheight, 0x0a);
		firstFrame = false;
	}

	pos += sprintf(&(output[pos]), "FRAME%c", 0x0a);
	for (i = 0; i < frame.Lwidth * frame.Lheight; i++)
	{
		output[pos++] = frame.L[i];
	}
	for (i = 0; i < frame.Cwidth * frame.Cheight; i++)
	{
		output[pos++] = frame.C[0][i];
	}
	for (i = 0; i < frame.Cwidth * frame.Cheight; i++)
	{
		output[pos++] = frame.C[1][i];
	}

	fwrite(output, 1, pos, yuvoutput);
	free(output);
	if (frameCount == 600) exit(0);
}


// ENCODING:

// Returns the offset of the start of the
// next frame in the input buffer
unsigned int findStartOfFrame(char *input)
{
	unsigned int pos, bufSize;
	char frameString[6];
	sprintf(frameString, "FRAME");

	bufSize = frame.Lwidth * frame.Lheight + (frame.Cwidth*frame.Cheight << 1) + 1000;

	pos = (unsigned int)(strstr(input, frameString) - input) + 5;

	//pos = (unsigned int)(memchr(&input[pos], 0x0a, 1000) - input) + 1;
	//memchr(
	pos = ((unsigned int)memchr(&input[pos], 0x0a, 1000) - (unsigned int)input) + 1;

	// frame parameters are not allowed

	// skip frame parameters
	//while(input[pos] == 0x20)
	//{
	//	temp = ((unsigned int)memchr(&input[pos], 0x0a, 1000) - (unsigned int)input) + 1;
	//	if (temp > bufSize)
	//	{
	//		break;
	//	}
	//	else
	//	{
	//		pos = temp;
	//	}
	//}

	return pos;
}

int readFromY4M()
{
	static bool firstFrame = true;
	char *input;

	unsigned int pos;
	if (firstFrame)
	{
		input = new char[1000];
		fread(input, 1, 1000, yuvinput);
		
		// strstr returns a pointer, not the offset inside the
		// string, so I am converting it
		pos = (unsigned int)(strstr(input, " W") - input) + 2;
		sscanf(&input[pos], "%d", &frame.Lwidth);

		pos = (unsigned int)(strstr(input, " H") - input) + 2;
		sscanf(&input[pos], "%d", &frame.Lheight);

		// TODO: handle chroma for non-4:2:0 subsampling
		frame.Cwidth = frame.Lwidth >> 1;
		frame.Cheight = frame.Lheight >> 1;

		frame.L = new unsigned char[frame.Lwidth*frame.Lheight];
		frame.C[0] = new unsigned char[frame.Cwidth*frame.Cheight];
		frame.C[1] = new unsigned char[frame.Cwidth*frame.Cheight];

		// TODO: handle interlaced frames
		pos = findStartOfFrame(input);
		fseek(yuvinput, pos, SEEK_SET);
		
		free(input);
		firstFrame = false;
	}

	int lumaSize = frame.Lwidth*frame.Lheight;
	int chromaSize = frame.Cwidth*frame.Cheight;
	int bufSize = lumaSize + (chromaSize << 1) + 1000;	// == lumaSize + 2*chromaSize + 1000
	
	input = new char[bufSize]; 
	
	int test = ftell(yuvinput);
	
	if (fread(input, 1, bufSize, yuvinput) < bufSize)
	{
		return -1;
	}

	pos = 0;
	memcpy(frame.L, input, lumaSize);
	pos += lumaSize;

	memcpy(frame.C[0], &input[pos], chromaSize);
	pos += chromaSize;

	memcpy(frame.C[1], &input[pos], chromaSize);
	pos = findStartOfFrame(input);

	fseek(yuvinput, pos - bufSize, SEEK_CUR);
	free(input);
	return 0;
}