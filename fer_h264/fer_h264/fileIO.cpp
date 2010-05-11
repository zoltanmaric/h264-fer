#include <string.h>

#include "fileIO.h"
#include "h264_globals.h"

int *red;
int *green;
int *blue;

FILE *yuvoutput;
FILE *yuvinput;

const unsigned int BUFFER_SIZE = 100000000; //(100MB)

unsigned int streamBufferSize;
char *streamBuffer;
unsigned long streamBufferPos;

char *input;	// the array holding one frame read from the stream

// The dimensions of the input frames (may not be multiples of 16)
int inputWidth;
int inputHeight;

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
			y_shift = (frame.L[i][j] - 16) << 10;
			cb_shift = (frame.C[0][i/2][j/2] - 128) << 10;
			cr_shift = (frame.C[1][i/2][j/2] - 128) << 10;

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

void writeToPPM(char *namePrefix)
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
	char filename[50];
	sprintf(filename, "%s%d%d%d%d.ppm", namePrefix, (frameCount%10000)/1000, (frameCount%1000)/100, (frameCount%100)/10, frameCount%10);
	f = fopen(filename,"wb");
	fwrite(pic, 1, pos, f);

	fclose(f);
	free(red);
	free(green);
	free(blue);
	free(pic);
}

// This (headerless) format is required by H264visa
void writeToYUV()
{
	int i, j;

	char *output;
	output = new char[5000000];

	unsigned int pos = 0;
	for (i = 0; i < frame.Lheight; i++)
	{
		for (j = 0; j < frame.Lwidth; j++)
		{
			output[pos++] = frame.L[i][j];
		}
	}
	for (i = 0; i < frame.Cheight; i++)
	{
		for (j = 0; j < frame.Cwidth; j++)
		{
			output[pos++] = frame.C[0][i][j];
		}
	}
	for (i = 0; i < frame.Cheight; i++)
	{
		for (j = 0; j < frame.Cwidth; j++)
		{
			output[pos++] = frame.C[1][i][j];
		}
	}

	fwrite(output, 1, pos, yuvoutput);
	free(output);
}

void writeToY4M()
{
	//static unsigned long frameCount = 0;
	static bool firstFrame = true;
	int i, j;

	char *output;
	output = new char[5000000];

	unsigned int pos = 0;
	if (firstFrame)
	{
		// Write file header
		pos = sprintf(output, "YUV4MPEG2 C420jpeg W%d H%d F24:1 Ip A1:1%c", frame.Lwidth, frame.Lheight, 0x0a);
		firstFrame = false;
	}

	pos += sprintf(&(output[pos]), "FRAME%c", 0x0a);
	for (i = 0; i < frame.Lheight; i++)
	{
		for (j = 0; j < frame.Lwidth; j++)
		{
			output[pos++] = frame.L[i][j];
		}
	}
	for (i = 0; i < frame.Cheight; i++)
	{
		for (j = 0; j < frame.Cwidth; j++)
		{
			output[pos++] = frame.C[0][i][j];
		}
	}
	for (i = 0; i < frame.Cheight; i++)
	{
		for (j = 0; j < frame.Cwidth; j++)
		{
			output[pos++] = frame.C[1][i][j];
		}
	}

	fwrite(output, 1, pos, yuvoutput);
	free(output);
}


// ENCODING:
void initStreamBuffer()
{
	streamBufferSize = BUFFER_SIZE;
	streamBufferPos = 0;
	streamBuffer = new char[streamBufferSize];
	while (streamBuffer == NULL)
	{
		streamBufferSize >>= 1;
		streamBuffer = new char[streamBufferSize];
	}

	int test = fread(streamBuffer, 1, streamBufferSize, yuvinput);
}

// Returns the offset of the start of the
// next frame in the input buffer
unsigned int findStartOfFrame(char *input)
{
	unsigned int pos;
	char frameString[6];
	sprintf(frameString, "FRAME");
	
	pos = (unsigned int)(strstr(input, frameString) - input) + 5;
	pos = ((unsigned int)memchr(&input[pos], 0x0a, 1000) - (unsigned int)input) + 1;

	// frame parameters are not allowed

	// skip frame parameters
	//bufSize = frame.Lwidth * frame.Lheight + (frame.Cwidth*frame.Cheight << 1) + 1000;
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

void loadY4MHeader()
{
	char input[1000];
	fread(input, 1, 1000, yuvinput);
	
	// strstr returns a pointer, not the offset inside the
	// string, so I am converting it
	int pos = (unsigned int)(strstr(input, " W") - input) + 2;
	sscanf(&input[pos], "%d", &inputWidth);

	pos = (unsigned int)(strstr(input, " H") - input) + 2;
	sscanf(&input[pos], "%d", &inputHeight);

	// Crop frame dimensions to multiples of 16:
	frame.Lwidth = inputWidth & 0xfffffff0;
	frame.Lheight = inputHeight & 0xfffffff0;

	// TODO: handle chroma for non-4:2:0 subsampling
	frame.Cwidth = frame.Lwidth >> 1;
	frame.Cheight = frame.Lheight >> 1;

	// TODO: handle interlaced frames
	pos = findStartOfFrame(input);
	fseek(yuvinput, pos, SEEK_SET);
}

// This function expects the file pointer to be set
// at the begining of the frame data when invoked.
// This implies invoking loadY4MHeader() for reading
// the first frame.
int readFromY4M()
{
	static bool firstFrame = true;
	char *input;
	int i, j, k, l;

	int lumaSize = inputWidth*inputHeight;
	int chromaSize = lumaSize >> 2;
	int bufSize = lumaSize + (chromaSize << 1) + 1000;	// == lumaSize + 2*chromaSize + 1000
	
	input = new char[bufSize];

	int test = ftell(yuvinput);
	
	if (fread(input, 1, bufSize, yuvinput) < (lumaSize + (chromaSize << 1)))
	{
		free(input);
		return -1;
	}
	else
	{
		int cropTop = (inputHeight - frame.Lheight) >> 1;
		int cropBottom = cropTop + frame.Lheight;
		int cropLeft = (inputWidth - frame.Lwidth) >> 1;
		int cropRight = cropLeft + frame.Lwidth;

		k = 0;
		for (i = cropTop; i < cropBottom; i++)
		{
			l = 0;
			for (j = cropLeft; j < cropRight; j++)
			{
				frame.L[k][l] = input[i*inputWidth + j];
				l++;
			}
			k++;
		}
		unsigned int  pos = lumaSize;

		cropTop >>= 1;
		cropBottom >>= 1;
		cropLeft >>= 1;
		cropRight >>= 1;

		int inputHeightC = inputHeight >> 1;
		int inputWidthC = inputWidth >> 1;

		k = 0;
		for (i = cropTop; i < cropBottom; i++)
		{
			l = 0;
			for (j = cropLeft; j < cropRight; j++)
			{
				frame.C[0][k][l] = input[pos + i*inputWidthC + j];
				l++;
			}
			k++;
		}

		pos += chromaSize;
		
		k = 0;
		for (i = cropTop; i < cropBottom; i++)
		{
			l = 0;
			for (j = cropLeft; j < cropRight; j++)
			{
				frame.C[1][k][l] = input[pos + i*inputWidthC + j];
				l++;
			}
			k++;
		}

		pos = findStartOfFrame(input);
		fseek(yuvinput, pos - bufSize, SEEK_CUR);
		free(input);
		return 0;
	}
}

// This function expects the file pointer to be set
// at the begining of the frame data when invoked.
// This implies invoking loadY4MHeader() for reading
// the first frame.
int bufferedReadFromY4M()
{
	static bool firstFrame = true;
	int i, j, k, l;
	static int lumaSize, chromaSize, bufSize;

	if (firstFrame == true)
	{
		firstFrame = false;
		lumaSize = inputWidth*inputHeight;
		chromaSize = lumaSize >> 2;
		bufSize = lumaSize + (chromaSize << 1) + 1000;	// == lumaSize + 2*chromaSize + 1000
		input = new char[bufSize];

		initStreamBuffer();
	}

	if ((streamBufferSize - streamBufferPos) < bufSize)
	{
		// fetch more data from the input stream
		fseek(yuvinput, streamBufferPos - streamBufferSize, SEEK_CUR);
		int test = fread(streamBuffer, 1, streamBufferSize, yuvinput);
		streamBufferPos = 0;
		// TODO: handle end of file
	}
	else
	{
		memcpy(input, &streamBuffer[streamBufferPos], bufSize);

		int cropTop = (inputHeight - frame.Lheight) >> 1;
		int cropBottom = cropTop + frame.Lheight;
		int cropLeft = (inputWidth - frame.Lwidth) >> 1;
		int cropRight = cropLeft + frame.Lwidth;

		k = 0;
		for (i = cropTop; i < cropBottom; i++)
		{
			l = 0;
			for (j = cropLeft; j < cropRight; j++)
			{
				frame.L[k][l] = input[i*inputWidth + j];
				l++;
			}
			k++;
		}
		unsigned int  pos = lumaSize;

		cropTop >>= 1;
		cropBottom >>= 1;
		cropLeft >>= 1;
		cropRight >>= 1;

		int inputHeightC = inputHeight >> 1;
		int inputWidthC = inputWidth >> 1;

		k = 0;
		for (i = cropTop; i < cropBottom; i++)
		{
			l = 0;
			for (j = cropLeft; j < cropRight; j++)
			{
				frame.C[0][k][l] = input[pos + i*inputWidthC + j];
				l++;
			}
			k++;
		}

		pos += chromaSize;
		
		k = 0;
		for (i = cropTop; i < cropBottom; i++)
		{
			l = 0;
			for (j = cropLeft; j < cropRight; j++)
			{
				frame.C[1][k][l] = input[pos + i*inputWidthC + j];
				l++;
			}
			k++;
		}

		pos = findStartOfFrame(input);
		streamBufferPos += pos;
		return 0;
	}
}

void FileIOCleanup()
{
	delete [] input;
	fclose(yuvinput);
	fclose(yuvoutput);
}