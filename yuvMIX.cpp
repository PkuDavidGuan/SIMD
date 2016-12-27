#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include <immintrin.h>
#include <emmintrin.h>
using namespace std;
const int WIDTH = 1920;
const int HEIGHT = 1080;

double YuvToArgbMat[3][3] = {1.164383,	0,		1.596027,
							1.164383,	-0.391762, -0.812968,
							1.164383,	2.017232,  0};
double ArgbToYuvMat[4][4] = {0.256788,	0.504129,	0.097906,	16,
							-0.148223,	-0.290993,	0.439216,	128,
							0.439216,	-0.367788,	-0.071427,	128};

class YuvToArgbToYuv {
public:
	//transmit yuv to rgb and back to a new yuv
	void transmit(unsigned char* y1, unsigned char* u1, unsigned char* v1, 
	        unsigned char* y2, unsigned char* u2, unsigned char* v2, 
	        unsigned char* y3, unsigned char* u3, unsigned char* v3, int A) 
	{
		unsigned char r1, r2, g1, g2, b1, b2, r, g, b;
		int temp = 0;
		for(int y = 0; y < HEIGHT; y++) 
		{
			for(int x=0; x < WIDTH; x++) 
			{
				//r分量
				temp = (y1[y * WIDTH + x] - 16) * YuvToArgbMat[0][0] + (v1[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[0][2];
				r1 = temp < 0 ? 0 : (temp>255 ? 255 : temp);
				
				temp = (y2[y * WIDTH + x] - 16) * YuvToArgbMat[0][0] + (v2[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[0][2];
				r2 = temp < 0 ? 0 : (temp>255 ? 255 : temp);
				
				r = (float)(A * r1 + (256 - A) * r2) / 256;

				//g分量
				temp = (y1[y * WIDTH + x] - 16) * YuvToArgbMat[1][0] + (u1[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[1][1] + (v1[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[1][2];
				g1 = temp < 0 ? 0 : (temp>255 ? 255 : temp);

				temp = (y2[y * WIDTH + x] - 16) * YuvToArgbMat[1][0] + (u2[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[1][1] + (v2[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[1][2];
				g2 = temp < 0 ? 0 : (temp>255 ? 255 : temp);

				g = (float)(A * g1 + (256 - A) * g2) / 256;

				//b分量
				temp = (y1[y * WIDTH + x] - 16) * YuvToArgbMat[2][0] + (u1[( y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[2][1];
				b1 = temp < 0 ? 0 : (temp>255 ? 255 : temp);

				temp = (y2[y * WIDTH + x] - 16) * YuvToArgbMat[2][0] + (u2[( y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[2][1];
				b2 = temp < 0 ? 0 : (temp>255 ? 255 : temp);

				b = (float)(A * b1 + (256 - A) * b2) / 256;

				y3[y * WIDTH + x] = r * ArgbToYuvMat[0][0] + g * ArgbToYuvMat[0][1] + b * ArgbToYuvMat[0][2] + ArgbToYuvMat[0][3];
				u3[(y / 2) * (WIDTH / 2) + x / 2] = r * ArgbToYuvMat[1][0] + g * ArgbToYuvMat[1][1] + b * ArgbToYuvMat[1][2] + ArgbToYuvMat[1][3];
				v3[(y / 2) * (WIDTH / 2) + x / 2] = r * ArgbToYuvMat[2][0] + g * ArgbToYuvMat[2][1] + b * ArgbToYuvMat[2][2] + ArgbToYuvMat[2][3];
			}
		}
	}

	void Convert(char *file1, char *file2, char *outputfile,int type) {
		int fReadSize = 0;
		int ImgSize = WIDTH * HEIGHT;
		FILE *fp = NULL;
		unsigned char* cTemp[9];

		int FrameSize = ImgSize + (ImgSize >> 1);
		unsigned char* yuv = new unsigned char[FrameSize];
		unsigned char* yuv2 = new unsigned char[FrameSize];
		unsigned char* newyuv = new unsigned char[FrameSize * 85];

		//get the content by byte from the two .yuv file
		if((fp = fopen(file1, "rb")) == NULL)
	        return;
		fReadSize = fread(yuv, 1, FrameSize, fp);
		fclose(fp);
		if(fReadSize < FrameSize)
			return;
		if((fp = fopen(file2, "rb")) == NULL)
	        return;
		fReadSize = fread(yuv2, 1, FrameSize, fp);
		fclose(fp);
		if(fReadSize < FrameSize)
			return;

		cTemp[0] = yuv;                        //y分量地址
		cTemp[1] = cTemp[0] + ImgSize;            //u分量地址
		cTemp[2] = cTemp[1] + (ImgSize >> 2);    //v分量地址

		cTemp[3] = yuv2;                        //y分量地址
		cTemp[4] = cTemp[3] + ImgSize;            //u分量地址
		cTemp[5] = cTemp[4] + (ImgSize >> 2);    //v分量地址

		cTemp[6] = newyuv;                        //y分量地址
		cTemp[7] = cTemp[6] + ImgSize;            //u分量地址
		cTemp[8] = cTemp[7] + (ImgSize >> 2);        //v分量地址


		if((fp = fopen(outputfile, "wb")) == NULL)
			return;

		struct timeval start, end;
		cout << "Convert, wait..." << endl;
		gettimeofday(&start, NULL);
		for (int A = 1; A <= 255; A += 3) 
		{
			switch(type)
			{
				case 0: transmit(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], cTemp[6], cTemp[7], cTemp[8], A);
						break;
				case 1: transmitSSE(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], cTemp[6], cTemp[7], cTemp[8], A);
						break;
				case 2: transmitSSE2(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], cTemp[6], cTemp[7], cTemp[8], A);
						break;
				case 3: transmitAVX(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], cTemp[6], cTemp[7], cTemp[8], A);
						break;
				default:
						cout << "Mode is incorrect, halt.\n" << endl;
						return;
			}

			cTemp[6] += FrameSize;
			cTemp[7] += FrameSize;
			cTemp[8] += FrameSize;
		}
		gettimeofday(&end, NULL);
		fwrite(newyuv, 1, FrameSize * 85, fp);
		fclose(fp);
		cout << "Succeed, time is " << (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec) << "us.";
		return;
	}

	void transmitSSE(unsigned char* y1, unsigned char* u1, unsigned char* v1,
	        unsigned char* y2, unsigned char* u2, unsigned char* v2,
	        unsigned char* y3, unsigned char* u3, unsigned char* v3, int A) 
	{
		float tempY1[4];
		float tempV1[4];
		float tempU1[4];
		float tempY2[4];
		float tempV2[4];
		float tempU2[4];

		float retR1[4];
		float retG1[4];
		float retB1[4];
		float retR2[4];
		float retG2[4];
		float retB2[4];

		float numA[4];
		float num256[4];
		for(int i = 0; i < 4; ++i)
			numA[i] = A;
		for(int i = 0; i < 4; ++i)
			num256[i] = 256;

		float var1[4];
		float var2[4];
		float var3[4];
		float var4[4];

		for(int y = 0; y < HEIGHT; y++) 
		{
			for(int x=0; x < WIDTH; x+=4) 
			{
				for(int i = 0; i < 4; ++i)
					tempY1[i] = y1[y*WIDTH+x+i]-16;
				for(int i = 0; i < 4; ++i)
					tempV1[i] = v1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
				for(int i = 0; i < 4; ++i)
					tempU1[i] = u1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
				for(int i = 0; i < 4; ++i)
					tempY2[i] = y2[y*WIDTH+x+i]-16;
				for(int i = 0; i < 4; ++i)
					tempV2[i] = v2[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
				for(int i = 0; i < 4; ++i)
					tempU2[i] = u2[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
				//r
				for(int i = 0; i < 4; ++i)
					var1[i] = 1.164383;	
				for(int i = 0; i < 4; ++i)
					var2[i] = 1.596027;
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm2;"
					"mulps %%xmm2, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%4);"
					:
					:"p"(tempY1),"p"(var1),"p"(tempV1),"p"(var2),"p"(retR1)
					:
				);
				for(int i = 0; i < 4; ++i)
				{
					if(retR1[i] < 0)
						retR1[i] = 0;
					else if(retR1[i] > 255)
						retR1[i] = 255;
				}

				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm2;"
					"mulps %%xmm2, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%4);"
					:
					:"p"(tempY2),"p"(var1),"p"(tempV2),"p"(var2),"p"(retR2)
					:
				);
				for(int i = 0; i < 4; ++i)
				{
					if(retR2[i] < 0)
						retR2[i] = 0;
					else if(retR2[i] > 255)
						retR2[i] = 255;
				}	

				//g
				for(int i = 0; i < 4; ++i)
					var1[i] = YuvToArgbMat[1][0];	
				for(int i = 0; i < 4; ++i)
					var2[i] = YuvToArgbMat[1][1];
				for(int i = 0; i < 4; ++i)
					var3[i] = YuvToArgbMat[1][2];
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm2;"
					"mulps %%xmm2, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%4), %%xmm1;"
					"movaps (%5), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%6);"
					:
					:"p"(tempY1),"p"(var1),"p"(tempU1),"p"(var2),"p"(tempV1),"p"(var3),"p"(retG1)
					:
				);
				for(int i = 0; i < 4; ++i)
				{
					if(retG1[i] < 0)
						retG1[i] = 0;
					else if(retG1[i] > 255)
						retG1[i] = 255;
				}

				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm2;"
					"mulps %%xmm2, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%4), %%xmm1;"
					"movaps (%5), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%6);"
					:
					:"p"(tempY2),"p"(var1),"p"(tempU2),"p"(var2),"p"(tempV2),"p"(var3),"p"(retG2)
					:
				);
				for(int i = 0; i < 4; ++i)
				{
					if(retG2[i] < 0)
						retG2[i] = 0;
					else if(retG2[i] > 255)
						retG2[i] = 255;
				}
				//b
				for(int i = 0; i < 4; ++i)
					var1[i] = YuvToArgbMat[2][0];	
				for(int i = 0; i < 4; ++i)
					var2[i] = YuvToArgbMat[2][1];
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm2;"
					"mulps %%xmm2, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%4);"
					:
					:"p"(tempY1),"p"(var1),"p"(tempU1),"p"(var2),"p"(retB1)
					:
				);
				for(int i = 0; i < 4; ++i)
				{
					if(retB1[i] < 0)
						retB1[i] = 0;
					else if(retB1[i] > 255)
						retB1[i] = 255;
				}

				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm2;"
					"mulps %%xmm2, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm3;"
					"mulps %%xmm3, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%4);"
					:
					:"p"(tempY2),"p"(var1),"p"(tempU2),"p"(var2),"p"(retB2)
					:
				);
				for(int i = 0; i < 4; ++i)
				{
					if(retB2[i] < 0)
						retB2[i] = 0;
					else if(retB2[i] > 255)
						retB2[i] = 255;
				}

				//mix
				asm
				(
					"movaps (%6), %%xmm0;"//A
					"movaps (%7), %%xmm1;"//256
					"movaps (%7), %%xmm2;"
					"subps %%xmm0, %%xmm2;"//256-A

					"movaps (%0), %%xmm3;"
					"movaps (%1), %%xmm4;"
					"mulps %%xmm0, %%xmm3;"
					"mulps %%xmm2, %%xmm4;"
					"addps %%xmm4, %%xmm3;"
					"divps %%xmm1, %%xmm3;"
					"movaps %%xmm3, (%0);" 

					"movaps (%2), %%xmm3;"
					"movaps (%3), %%xmm4;"
					"mulps %%xmm0, %%xmm3;"
					"mulps %%xmm2, %%xmm4;"
					"addps %%xmm4, %%xmm3;"
					"divps %%xmm1, %%xmm3;"
					"movaps %%xmm3, (%2);" 

					"movaps (%4), %%xmm3;"
					"movaps (%5), %%xmm4;"
					"mulps %%xmm0, %%xmm3;"
					"mulps %%xmm2, %%xmm4;"
					"addps %%xmm4, %%xmm3;"
					"divps %%xmm1, %%xmm3;"
					"movaps %%xmm3, (%4);" 
					:
					:"p"(retR1),"p"(retR2),"p"(retG1),"p"(retG2),"p"(retB1),"p"(retB2),"p"(numA),"p"(num256)
					:
				);

				for(int i = 0; i < 4; ++i)
					var1[i] = ArgbToYuvMat[0][0];	
				for(int i = 0; i < 4; ++i)
					var2[i] = ArgbToYuvMat[0][1];
				for(int i = 0; i < 4; ++i)
					var3[i] = ArgbToYuvMat[0][2];
				for(int i = 0; i < 4; ++i)
					var4[i] = ArgbToYuvMat[0][3];
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm1;"
					"mulps %%xmm1, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm2;"
					"mulps %%xmm2, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%4), %%xmm1;"
					"movaps (%5), %%xmm2;"
					"mulps %%xmm2, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%6), %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%7);"
					:
					:"p"(retR1),"p"(var1),"p"(retG1),"p"(var2),"p"(retB1),"p"(var3),"p"(var4),"p"(tempY1)
					:
				);
				for(int i = 0; i < 4; ++i)
					y3[y * WIDTH + x+i] = tempY1[i];

				for(int i = 0; i < 4; ++i)
					var1[i] = ArgbToYuvMat[1][0];	
				for(int i = 0; i < 4; ++i)
					var2[i] = ArgbToYuvMat[1][1];
				for(int i = 0; i < 4; ++i)
					var3[i] = ArgbToYuvMat[1][2];
				for(int i = 0; i < 4; ++i)
					var4[i] = ArgbToYuvMat[1][3];
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm1;"
					"mulps %%xmm1, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm2;"
					"mulps %%xmm2, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%4), %%xmm1;"
					"movaps (%5), %%xmm2;"
					"mulps %%xmm2, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%6), %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%7);"
					:
					:"p"(retR1),"p"(var1),"p"(retG1),"p"(var2),"p"(retB1),"p"(var3),"p"(var4),"p"(tempU1)
					:
				);
				for(int i = 0; i < 4; ++i)
					u3[(y / 2) * (WIDTH / 2) + (x+i)/2] = tempU1[i];

				for(int i = 0; i < 4; ++i)
					var1[i] = ArgbToYuvMat[2][0];	
				for(int i = 0; i < 4; ++i)
					var2[i] = ArgbToYuvMat[2][1];
				for(int i = 0; i < 4; ++i)
					var3[i] = ArgbToYuvMat[2][2];
				for(int i = 0; i < 4; ++i)
					var4[i] = ArgbToYuvMat[2][3];
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm1;"
					"mulps %%xmm1, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"movaps (%3), %%xmm2;"
					"mulps %%xmm2, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%4), %%xmm1;"
					"movaps (%5), %%xmm2;"
					"mulps %%xmm2, %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps (%6), %%xmm1;"
					"addps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%7);"
					:
					:"p"(retR1),"p"(var1),"p"(retG1),"p"(var2),"p"(retB1),"p"(var3),"p"(var4),"p"(tempV1)
					:
				);
				for(int i = 0; i < 4; ++i)
					v3[(y / 2) * (WIDTH / 2) + (x+i)/2] = tempV1[i];
			}
		}
	}
	void transmitSSE2(unsigned char* y1, unsigned char* u1, unsigned char* v1,
	        unsigned char* y2, unsigned char* u2, unsigned char* v2, unsigned char* y3, 
	        unsigned char* u3, unsigned char* v3, int A) 
	{
		__m128i YUV2RGB1_1 = _mm_set1_epi16((1 << 16) * 0.164383);		//1
		__m128i YUV2RGB1_2 = _mm_set1_epi16(0);						//0
		__m128i YUV2RGB1_3 = _mm_set1_epi16((1 << 16) * 0.096027);		//1.5
		__m128i YUV2RGB2_1 = _mm_set1_epi16((1 << 16) * 0.164383);		//1
		__m128i YUV2RGB2_2 = _mm_set1_epi16((1 << 16) * 0.391762);		//0
		__m128i YUV2RGB2_3 = _mm_set1_epi16((1 << 16) * 0.312968);		//0.5
		__m128i YUV2RGB3_1 = _mm_set1_epi16((1 << 16) * 0.164383);		//1
		__m128i YUV2RGB3_2 = _mm_set1_epi16((1 << 16) * 0.017232);		//2
		__m128i YUV2RGB3_3 = _mm_set1_epi16(0);						//0

		__m128i MAX255 = _mm_set1_epi16(255);
		__m128i MIN0 = _mm_set1_epi16(0);
		__m128i Ymask = _mm_set1_epi16(16);
		__m128i Umask = _mm_set1_epi16(128);
		__m128i Vmask = _mm_set1_epi16(128);

		__m128i RGB2YUV1_1 = _mm_set1_epi16((1 << 16) * 0.256788);		//0
		__m128i RGB2YUV1_2 = _mm_set1_epi16((1 << 16) * 0.004129);		//0.5
		__m128i RGB2YUV1_3 = _mm_set1_epi16((1 << 16) * 0.097906);		//0
		__m128i RGB2YUV1_4 = _mm_set1_epi16(16);
		__m128i RGB2YUV2_1 = _mm_set1_epi16((1 << 16) * 0.148223);		//0
		__m128i RGB2YUV2_2 = _mm_set1_epi16((1 << 16) * 0.290993);		//0
		__m128i RGB2YUV2_3 = _mm_set1_epi16((1 << 16) * 0.439216);		//0
		__m128i RGB2YUV2_4 = _mm_set1_epi16(128);
		__m128i RGB2YUV3_1 = _mm_set1_epi16((1 << 16) * 0.439216);		//0
		__m128i RGB2YUV3_2 = _mm_set1_epi16((1 << 16) * 0.367788);		//0
		__m128i RGB2YUV3_3 = _mm_set1_epi16((1 << 16) * 0.071427);		//0
		__m128i RGB2YUV3_4 = _mm_set1_epi16(128);
		__m128i mA = _mm_set1_epi16(A);
		__m128i mA1 = _mm_set1_epi16(256 - A);
		for (int y = 0; y < HEIGHT; y++) 
		{
			for (int x = 0; x < WIDTH; x += 8) 
			{
				__m128i temp1, temp2, temp3, temp4;
				__m128i tempR, tempG, tempB;
				__m128i R, G, B;
				__m128i R1, G1, B1, R2, G2, B2;
				__m128i Y, U, V;
				__m128i compareResult, mediaNum;
				
				__m128i my = _mm_set_epi16(y1[y * WIDTH + x], y1[y * WIDTH + x + 1], y1[y * WIDTH + x + 2], y1[y * WIDTH + x + 3], y1[y * WIDTH + x + 4], y1[y * WIDTH + x + 5], y1[y * WIDTH + x + 6], y1[y * WIDTH + x + 7]);
				__m128i mu = _mm_set_epi16(u1[(y / 2) * (WIDTH / 2) + x / 2], u1[(y / 2) * (WIDTH / 2) + (x + 1) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 2) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 3) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 4) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 5) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 6) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 7) / 2]);
				__m128i mv = _mm_set_epi16(v1[(y / 2) * (WIDTH / 2) + x / 2], v1[(y / 2) * (WIDTH / 2) + (x + 1) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 2) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 3) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 4) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 5) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 6) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 7) / 2]);

				my = _mm_sub_epi16(my, Ymask);
				mu = _mm_sub_epi16(mu, Umask);
				mv = _mm_sub_epi16(mv, Vmask);
				
				temp1 = _mm_mulhi_epi16(my, YUV2RGB1_1);
				temp1 = _mm_add_epi16(temp1, my);
				temp2 = _mm_mulhi_epi16(mu, YUV2RGB1_2);
				temp3 = _mm_mulhi_epi16(mv, YUV2RGB1_3);
				temp4 = _mm_srai_epi16(mv, 1);
				temp3 = _mm_add_epi16(temp3, mv);
				temp3 = _mm_add_epi16(temp3, temp4);

				tempR = _mm_add_epi16(temp1, temp2);
				tempR = _mm_add_epi16(tempR, temp3);

				compareResult = _mm_cmpgt_epi16(tempR, MAX255);
				mediaNum = _mm_sub_epi16(MAX255, tempR);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempR = _mm_add_epi16(mediaNum, tempR);
				
				compareResult = _mm_cmplt_epi16(tempR, MIN0);
				mediaNum = mediaNum = _mm_sub_epi16(MIN0, tempR);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempR = _mm_add_epi16(mediaNum, tempR);
				R1 = _mm_mullo_epi16(tempR, mA);

				//get G element
				temp1 = _mm_mulhi_epi16(my, YUV2RGB2_1);
				temp1 = _mm_add_epi16(temp1, my);
				temp2 = _mm_mulhi_epi16(mu, YUV2RGB2_2);
				temp3 = _mm_mulhi_epi16(mv, YUV2RGB2_3);
				temp4 = _mm_srai_epi16(mv, 1);
				temp3 = _mm_add_epi16(temp3, temp4);

				tempG = _mm_sub_epi16(temp1, temp2);
				tempG = _mm_sub_epi16(tempG, temp3);	

				compareResult = _mm_cmpgt_epi16(tempG, MAX255);
				mediaNum = _mm_sub_epi16(MAX255, tempG);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempG = _mm_add_epi16(mediaNum, tempG);
				
				compareResult = _mm_cmplt_epi16(tempG, MIN0);
				mediaNum = mediaNum = _mm_sub_epi16(MIN0, tempG);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempG = _mm_add_epi16(mediaNum, tempG);
				G1 = _mm_mullo_epi16(tempG, mA);

				//get B element
				temp1 = _mm_mulhi_epi16(my, YUV2RGB3_1);
				temp1 = _mm_add_epi16(temp1, my);
				temp2 = _mm_mulhi_epi16(mu, YUV2RGB3_2);
				temp4 = _mm_slli_epi16(mu, 1);
				temp2 = _mm_add_epi16(temp4, temp2);
				temp3 = _mm_mulhi_epi16(mv, YUV2RGB3_3);
				
				tempB = _mm_add_epi16(temp1, temp2);
				tempB = _mm_add_epi16(tempB, temp3);

				compareResult = _mm_cmpgt_epi16(tempB, MAX255);
				mediaNum = _mm_sub_epi16(MAX255, tempB);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempB = _mm_add_epi16(mediaNum, tempB);
				
				compareResult = _mm_cmplt_epi16(tempB, MIN0);
				mediaNum = mediaNum = _mm_sub_epi16(MIN0, tempB);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempB = _mm_add_epi16(mediaNum, tempB);
				B1 = _mm_mullo_epi16(tempB, mA);


				my = _mm_set_epi16(y2[y * WIDTH + x], y2[y * WIDTH + x + 1], y2[y * WIDTH + x + 2], y2[y * WIDTH + x + 3], y2[y * WIDTH + x + 4], y2[y * WIDTH + x + 5], y2[y * WIDTH + x + 6], y2[y * WIDTH + x + 7]);
				mu = _mm_set_epi16(u2[(y / 2) * (WIDTH / 2) + x / 2], u2[(y / 2) * (WIDTH / 2) + (x + 1) / 2], u2[(y / 2) * (WIDTH / 2) + (x + 2) / 2], u2[(y / 2) * (WIDTH / 2) + (x + 3) / 2], u2[(y / 2) * (WIDTH / 2) + (x + 4) / 2], u2[(y / 2) * (WIDTH / 2) + (x + 5) / 2], u2[(y / 2) * (WIDTH / 2) + (x + 6) / 2], u2[(y / 2) * (WIDTH / 2) + (x + 7) / 2]);
				mv = _mm_set_epi16(v2[(y / 2) * (WIDTH / 2) + x / 2], v2[(y / 2) * (WIDTH / 2) + (x + 1) / 2], v2[(y / 2) * (WIDTH / 2) + (x + 2) / 2], v2[(y / 2) * (WIDTH / 2) + (x + 3) / 2], v2[(y / 2) * (WIDTH / 2) + (x + 4) / 2], v2[(y / 2) * (WIDTH / 2) + (x + 5) / 2], v2[(y / 2) * (WIDTH / 2) + (x + 6) / 2], v2[(y / 2) * (WIDTH / 2) + (x + 7) / 2]);

				my = _mm_sub_epi16(my, Ymask);
				mu = _mm_sub_epi16(mu, Umask);
				mv = _mm_sub_epi16(mv, Vmask);
				
				temp1 = _mm_mulhi_epi16(my, YUV2RGB1_1);
				temp1 = _mm_add_epi16(temp1, my);
				temp2 = _mm_mulhi_epi16(mu, YUV2RGB1_2);
				temp3 = _mm_mulhi_epi16(mv, YUV2RGB1_3);
				temp4 = _mm_srai_epi16(mv, 1);
				temp3 = _mm_add_epi16(temp3, mv);
				temp3 = _mm_add_epi16(temp3, temp4);

				tempR = _mm_add_epi16(temp1, temp2);
				tempR = _mm_add_epi16(tempR, temp3);

				compareResult = _mm_cmpgt_epi16(tempR, MAX255);
				mediaNum = _mm_sub_epi16(MAX255, tempR);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempR = _mm_add_epi16(mediaNum, tempR);
				
				compareResult = _mm_cmplt_epi16(tempR, MIN0);
				mediaNum = mediaNum = _mm_sub_epi16(MIN0, tempR);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempR = _mm_add_epi16(mediaNum, tempR);
				R2 = _mm_mullo_epi16(tempR, mA1);

				//get G element
				temp1 = _mm_mulhi_epi16(my, YUV2RGB2_1);
				temp1 = _mm_add_epi16(temp1, my);
				temp2 = _mm_mulhi_epi16(mu, YUV2RGB2_2);
				temp3 = _mm_mulhi_epi16(mv, YUV2RGB2_3);
				temp4 = _mm_srai_epi16(mv, 1);
				temp3 = _mm_add_epi16(temp3, temp4);

				tempG = _mm_sub_epi16(temp1, temp2);
				tempG = _mm_sub_epi16(tempG, temp3);	

				compareResult = _mm_cmpgt_epi16(tempG, MAX255);
				mediaNum = _mm_sub_epi16(MAX255, tempG);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempG = _mm_add_epi16(mediaNum, tempG);
				
				compareResult = _mm_cmplt_epi16(tempG, MIN0);
				mediaNum = mediaNum = _mm_sub_epi16(MIN0, tempG);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempG = _mm_add_epi16(mediaNum, tempG);
				G2 = _mm_mullo_epi16(tempG, mA1);

				//get B element
				temp1 = _mm_mulhi_epi16(my, YUV2RGB3_1);
				temp1 = _mm_add_epi16(temp1, my);
				temp2 = _mm_mulhi_epi16(mu, YUV2RGB3_2);
				temp4 = _mm_slli_epi16(mu, 1);
				temp2 = _mm_add_epi16(temp4, temp2);
				temp3 = _mm_mulhi_epi16(mv, YUV2RGB3_3);
				
				tempB = _mm_add_epi16(temp1, temp2);
				tempB = _mm_add_epi16(tempB, temp3);

				compareResult = _mm_cmpgt_epi16(tempB, MAX255);
				mediaNum = _mm_sub_epi16(MAX255, tempB);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempB = _mm_add_epi16(mediaNum, tempB);
				
				compareResult = _mm_cmplt_epi16(tempB, MIN0);
				mediaNum = mediaNum = _mm_sub_epi16(MIN0, tempB);
				mediaNum = _mm_and_si128(mediaNum, compareResult);
				tempB = _mm_add_epi16(mediaNum, tempB);
				B2 = _mm_mullo_epi16(tempB, mA1);


				//alpha mix
				R = _mm_add_epi16(R1, R2);
				R = _mm_srai_epi16(R, 8);
				G = _mm_add_epi16(G1, G2);
				G = _mm_srai_epi16(G, 8);
				B = _mm_add_epi16(B1, B2);
				B = _mm_srai_epi16(B, 8);

				temp1 = _mm_mulhi_epi16(R, RGB2YUV1_1);
				temp2 = _mm_mulhi_epi16(G, RGB2YUV1_2);
				temp4 = _mm_srai_epi16(G, 1);
				temp2 = _mm_add_epi16(temp4, temp2);
				temp3 = _mm_mulhi_epi16(B, RGB2YUV1_3);

				Y = _mm_add_epi16(temp1, temp2); 
				Y = _mm_add_epi16(Y, temp3);
				Y = _mm_add_epi16(Y, RGB2YUV1_4);


				temp1 = _mm_mulhi_epi16(R, RGB2YUV2_1);
				temp2 = _mm_mulhi_epi16(G, RGB2YUV2_2);
				temp3 = _mm_mulhi_epi16(B, RGB2YUV2_3);

				U = _mm_sub_epi16(temp3, temp1); 
				U = _mm_sub_epi16(U, temp2);
				U = _mm_add_epi16(U, RGB2YUV2_4);

				temp1 = _mm_mulhi_epi16(R, RGB2YUV3_1);
				temp2 = _mm_mulhi_epi16(G, RGB2YUV3_2);
				temp3 = _mm_mulhi_epi16(B, RGB2YUV3_3);

				V = _mm_sub_epi16(temp1, temp2); 
				V = _mm_sub_epi16(V, temp3);
				V = _mm_add_epi16(V, RGB2YUV3_4);

				for (int i = 0; i < 8; i++) 
				{
					y3[y * WIDTH + x + i] = ((short *)&Y)[8 - i];
					u3[(y / 2) * (WIDTH / 2) + (x + i) / 2] = ((short *)&U)[8 - i];
					v3[(y / 2) * (WIDTH / 2) + (x + i) / 2] = ((short *)&V)[8 - i];
				}

			}
		}
	}
	void transmitAVX(unsigned char* y1, unsigned char* u1, unsigned char* v1,
	        unsigned char* y2, unsigned char* u2, unsigned char* v2, 
	        unsigned char* y3, unsigned char* u3, unsigned char* v3, int A) 
	{
		__m256 YUV2RGB1_1 = _mm256_set1_ps(1.164383);
		__m256 YUV2RGB1_2 = _mm256_set1_ps(0.0);
		__m256 YUV2RGB1_3 = _mm256_set1_ps(1.596027);
		__m256 YUV2RGB2_1 = _mm256_set1_ps(1.164383);
		__m256 YUV2RGB2_2 = _mm256_set1_ps(-0.391762);
		__m256 YUV2RGB2_3 = _mm256_set1_ps(-0.812968);
		__m256 YUV2RGB3_1 = _mm256_set1_ps(1.164383);
		__m256 YUV2RGB3_2 = _mm256_set1_ps(2.017232);
		__m256 YUV2RGB3_3 = _mm256_set1_ps(0);

		__m256 divMask = _mm256_set1_ps(256);

		__m256 RGB2YUV1_1 = _mm256_set1_ps(0.256788);
		__m256 RGB2YUV1_2 = _mm256_set1_ps(0.504129);
		__m256 RGB2YUV1_3 = _mm256_set1_ps(0.097906);
		__m256 RGB2YUV1_4 = _mm256_set1_ps(16);
		__m256 RGB2YUV2_1 = _mm256_set1_ps(-0.148223);
		__m256 RGB2YUV2_2 = _mm256_set1_ps(-0.290993);
		__m256 RGB2YUV2_3 = _mm256_set1_ps(0.439216);
		__m256 RGB2YUV2_4 = _mm256_set1_ps(128);
		__m256 RGB2YUV3_1 = _mm256_set1_ps(0.439216);
		__m256 RGB2YUV3_2 = _mm256_set1_ps(-0.367788);
		__m256 RGB2YUV3_3 = _mm256_set1_ps(-0.071427);
		__m256 RGB2YUV3_4 = _mm256_set1_ps(128);
		for (int y = 0; y < HEIGHT; y++) 
		{
			for (int x = 0; x < WIDTH; x += 8) 
			{
				float tempY[8];
				float tempU[8];
				float tempV[8];
				__m256 R, G, B;
				__m256 R1, G1, B1, R2, G2, B2;
				__m256 Y, U, V;
				__m256 temp1, temp2, temp3;

				__m256 mask = _mm256_set1_ps((float)A);

				for (int i = 0; i < 8; i++) {
					tempY[i] = (int)y1[y * WIDTH + x + i] - 16;
					tempU[i] = (int)u1[(y / 2) * (WIDTH / 2) + (x + i) / 2] - 128;
					tempV[i] = (int)v1[(y / 2) * (WIDTH / 2) + (x + i) / 2] - 128;
				}

				__m256 my =  _mm256_loadu_ps(tempY);
				__m256 mu =  _mm256_loadu_ps(tempU);
				__m256 mv =  _mm256_loadu_ps(tempV);
				
				temp1 = _mm256_mul_ps(my, YUV2RGB1_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB1_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB1_3);

				R1 = _mm256_add_ps(temp1, temp2);
				R1 = _mm256_add_ps(R1, temp3);

				temp1 = _mm256_mul_ps(my, YUV2RGB2_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB2_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB2_3);

				G1 = _mm256_add_ps(temp1, temp2);
				G1 = _mm256_add_ps(G1, temp3);


				temp1 = _mm256_mul_ps(my, YUV2RGB3_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB3_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB3_3);

				B1 = _mm256_add_ps(temp1, temp2);
				B1 = _mm256_add_ps(B1, temp3);

				__m256 mask1 = _mm256_set1_ps((float)(256 - A));

				for (int i = 0; i < 8; i++) 
				{
					tempY[i] = (int)y2[y * WIDTH + x + i] - 16;
					tempU[i] = (int)u2[(y / 2) * (WIDTH / 2) + (x + i) / 2] - 128;
					tempV[i] = (int)v2[(y / 2) * (WIDTH / 2) + (x + i) / 2] - 128;
				}

				my =  _mm256_loadu_ps(tempY);
				mu =  _mm256_loadu_ps(tempU);
				mv =  _mm256_loadu_ps(tempV);
				
				temp1 = _mm256_mul_ps(my, YUV2RGB1_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB1_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB1_3);

				R2 = _mm256_add_ps(temp1, temp2);
				R2 = _mm256_add_ps(R2, temp3);

				temp1 = _mm256_mul_ps(my, YUV2RGB2_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB2_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB2_3);

				G2 = _mm256_add_ps(temp1, temp2);
				G2 = _mm256_add_ps(G2, temp3);


				temp1 = _mm256_mul_ps(my, YUV2RGB3_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB3_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB3_3);

				B2 = _mm256_add_ps(temp1, temp2);
				B2 = _mm256_add_ps(B2, temp3);


				R1 = _mm256_mul_ps(R1, mask);
				R2 = _mm256_mul_ps(R2, mask1);
				R = _mm256_add_ps(R1, R2);
				R = _mm256_div_ps(R, divMask);

				G1 = _mm256_mul_ps(G1, mask);
				G2 = _mm256_mul_ps(G2, mask1);
				G = _mm256_add_ps(G1, G2);
				G = _mm256_div_ps(G, divMask);

				B1 = _mm256_mul_ps(B1, mask);
				B2 = _mm256_mul_ps(B2, mask1);
				B = _mm256_add_ps(B1, B2);
				B = _mm256_div_ps(B, divMask);


				temp1 = _mm256_mul_ps(R, RGB2YUV1_1);
				temp2 = _mm256_mul_ps(G, RGB2YUV1_2);
				temp3 = _mm256_mul_ps(B, RGB2YUV1_3);
				Y = _mm256_add_ps(temp1, temp2);
				Y = _mm256_add_ps(Y, temp3);
				Y = _mm256_add_ps(Y, RGB2YUV1_4);

				temp1 = _mm256_mul_ps(R, RGB2YUV2_1);
				temp2 = _mm256_mul_ps(G, RGB2YUV2_2);
				temp3 = _mm256_mul_ps(B, RGB2YUV2_3);
				U = _mm256_add_ps(temp1, temp2);
				U = _mm256_add_ps(U, temp3);
				U = _mm256_add_ps(U, RGB2YUV2_4);

				temp1 = _mm256_mul_ps(R, RGB2YUV3_1);
				temp2 = _mm256_mul_ps(G, RGB2YUV3_2);
				temp3 = _mm256_mul_ps(B, RGB2YUV3_3);
				V = _mm256_add_ps(temp1, temp2);
				V = _mm256_add_ps(V, temp3);
				V = _mm256_add_ps(V, RGB2YUV3_4);

				_mm256_storeu_ps(tempY, Y);
				_mm256_storeu_ps(tempU, U);
				_mm256_storeu_ps(tempV, V);

				for (int i = 0; i < 8; i++) 
				{
					y3[y * WIDTH + x + i] = (int)tempY[i];
					u3[(y / 2) * (WIDTH / 2) + (x + i) / 2] = (int)tempU[i];
					v3[(y / 2) * (WIDTH / 2) + (x + i) / 2] = (int)tempV[i];
				}
			}
		}
	}
};


int main() {
	int type;
	cout << "-------------MODE-------------" << endl;
	cout << "0:normal, 1:sse, 2:sse2, 3:avx" << endl;
	cout << "##############################" << endl;
	cout <<  "Choose a mode:";
	cin >> type;
	cout << endl;
	char name1[30] = "../image/dem1.yuv";
	char name2[30] = "../image/dem2.yuv";
	char name3[30] = "../image/mix_noassem.yuv";
	YuvToArgbToYuv example;
	example.Convert(name1, name2, name3, type);
	return 0;
}