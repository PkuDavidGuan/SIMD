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
	void transmit(unsigned char* y1, unsigned char* u1, unsigned char* v1, 
	        unsigned char* y2, unsigned char* u2, unsigned char* v2, int A) 
	{
		int temp = 0;
		unsigned char r, g, b;
		for(int y = 0; y < HEIGHT; y++) 
		{
			for(int x=0; x < WIDTH; x++) 
			{
				//r分量
				temp = (y1[y * WIDTH + x] - 16) * YuvToArgbMat[0][0] + (v1[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[0][2];
				r = temp < 0 ? 0 : (temp>255 ? 255 : temp);
				r = (float)A * r / 256;

				//g分量
				temp = (y1[y * WIDTH + x] - 16) * YuvToArgbMat[1][0] + (u1[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[1][1] + (v1[(y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[1][2];
				g = temp < 0 ? 0 : (temp>255 ? 255 : temp);
				g = (float)A * g / 256;
				//b分量
				temp = (y1[y * WIDTH + x] - 16) * YuvToArgbMat[2][0] + (u1[( y / 2) * (WIDTH / 2) + x / 2] - 128) * YuvToArgbMat[2][1];
				b = temp < 0 ? 0 : (temp>255 ? 255 : temp);
				b = (float)A * b / 256;

				y2[y * WIDTH + x] = r * ArgbToYuvMat[0][0] + g * ArgbToYuvMat[0][1] + b * ArgbToYuvMat[0][2] + ArgbToYuvMat[0][3];
				u2[(y / 2) * (WIDTH / 2) + x / 2] = r * ArgbToYuvMat[1][0] + g * ArgbToYuvMat[1][1] + b * ArgbToYuvMat[1][2] + ArgbToYuvMat[1][3];
				v2[(y / 2) * (WIDTH / 2) + x / 2] = r * ArgbToYuvMat[2][0] + g * ArgbToYuvMat[2][1] + b * ArgbToYuvMat[2][2] + ArgbToYuvMat[2][3];
			}
		}
	}

	void Convert(char *file, char *outputfile, int type) {
		int fReadSize = 0;
		int ImgSize = WIDTH * HEIGHT;
		FILE *fp = NULL;
		unsigned char* cTemp[6];

		int FrameSize = ImgSize + (ImgSize >> 1);
		unsigned char* yuv = new unsigned char[FrameSize];
		unsigned char* newyuv = new unsigned char[FrameSize * 85];
		unsigned char* rgb = new unsigned char[ImgSize * 3];
		if((fp = fopen(file, "rb")) == NULL)
	        return;
		fReadSize = fread(yuv, 1, FrameSize, fp);
		fclose(fp);
		if(fReadSize < FrameSize)
			return;

		cTemp[0] = yuv;                        //y分量地址
		cTemp[1] = cTemp[0] + ImgSize;            //u分量地址
		cTemp[2] = cTemp[1] + (ImgSize >> 2);    //v分量地址
		cTemp[3] = newyuv;                        
		cTemp[4] = cTemp[3] + ImgSize;            
		cTemp[5] = cTemp[4] + (ImgSize >> 2);        
		

		if((fp = fopen(outputfile, "wb")) == NULL)
			return;
		struct timeval start, end;
		cout << "Convert, wait..." << endl;
		gettimeofday(&start, NULL);
		for (int A = 1; A <= 255; A += 3) 
		{
			switch(type)
			{
				case 0: transmit(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], A);
						break;
				case 1: transmitSSE(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], A);
						break;
				case 2: transmitSSE2(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], A);
						break;
				case 3: transmitAVX(cTemp[0], cTemp[1], cTemp[2], cTemp[3], cTemp[4], cTemp[5], A);
						break;
				default:
						cout << "Mode is incorrect, halt.\n" << endl;
						return;
			}

			cTemp[3] += FrameSize;
			cTemp[4] += FrameSize;
			cTemp[5] += FrameSize;
		}
		gettimeofday(&end, NULL);
		fwrite(newyuv, 1, FrameSize * 85, fp);
		fclose(fp);
		cout << "Succeed, time is " << (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec) << "us.";
		return;
	}
	void transmitSSE(unsigned char* y1, unsigned char* u1, unsigned char* v1, 
	        unsigned char* y2, unsigned char* u2, unsigned char* v2, int A) 
	{
		float tempY[4];
		float tempV[4];
		float tempU[4];

		float retR[4];
		float retG[4];
		float retB[4];

		float numA[4];
		float num256[4];
		float num255[4];
		float num0[4];
		for(int i = 0; i < 4; ++i)
			numA[i] = A;
		for(int i = 0; i < 4; ++i)
			num256[i] = 256;
		for(int i = 0; i < 4; ++i)
			num255[i] = 255;
		for(int i = 0; i < 4; ++i)
			num0[i] = 0;

		float var1[4];
		float var2[4];
		float var3[4];
		float var4[4];

		for(int y = 0; y < HEIGHT; y++) 
		{
			for(int x=0; x < WIDTH; x+=4) 
			{
				for(int i = 0; i < 4; ++i)
					tempY[i] = y1[y*WIDTH+x+i]-16;
				for(int i = 0; i < 4; ++i)
					tempV[i] = v1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
				for(int i = 0; i < 4; ++i)
					tempU[i] = u1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
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
					"movaps (%5), %%xmm1;"
					"movaps (%6), %%xmm2;"
					"maxps %%xmm1, %%xmm0;"
					"minps %%xmm2, %%xmm0;"
					"movaps %%xmm0, (%4);"
					:
					:"p"(tempY),"p"(var1),"p"(tempV),"p"(var2),"p"(retR),"p"(num0),"p"(num255)
					:
				);
				// for(int i = 0; i < 4; ++i)
				// {
				// 	if(retR[i] < 0)
				// 		retR[i] = 0;
				// 	else if(retR[i] > 255)
				// 		retR[i] = 255;
				// }
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm1;"
					"mulps %%xmm1, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"divps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%0);"
					:
					:"p"(retR), "p"(numA), "p"(num256)
					:
				);

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
					"movaps (%7), %%xmm1;"
					"movaps (%8), %%xmm2;"
					"maxps %%xmm1, %%xmm0;"
					"minps %%xmm2, %%xmm0;"
					"movaps %%xmm0, (%6);"
					:
					:"p"(tempY),"p"(var1),"p"(tempU),"p"(var2),"p"(tempV),"p"(var3),"p"(retG),"p"(num0),"p"(num255)
					:
				);
				// for(int i = 0; i < 4; ++i)
				// {
				// 	if(retG[i] < 0)
				// 		retG[i] = 0;
				// 	else if(retG[i] > 255)
				// 		retG[i] = 255;
				// }
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm1;"
					"mulps %%xmm1, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"divps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%0);"
					:
					:"p"(retG), "p"(numA), "p"(num256)
					:
				);

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
					"movaps (%5), %%xmm1;"
					"movaps (%6), %%xmm2;"
					"maxps %%xmm1, %%xmm0;"
					"minps %%xmm2, %%xmm0;"
					"movaps %%xmm0, (%4);"
					:
					:"p"(tempY),"p"(var1),"p"(tempU),"p"(var2),"p"(retB),"p"(num0),"p"(num255)
					:
				);
				// for(int i = 0; i < 4; ++i)
				// {
				// 	if(retB[i] < 0)
				// 		retB[i] = 0;
				// 	else if(retB[i] > 255)
				// 		retB[i] = 255;
				// }
				asm
				(
					"movaps (%0), %%xmm0;"
					"movaps (%1), %%xmm1;"
					"mulps %%xmm1, %%xmm0;"
					"movaps (%2), %%xmm1;"
					"divps %%xmm1, %%xmm0;"
					"movaps %%xmm0, (%0);"
					:
					:"p"(retB), "p"(numA), "p"(num256)
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
					:"p"(retR),"p"(var1),"p"(retG),"p"(var2),"p"(retB),"p"(var3),"p"(var4),"p"(tempY)
					:
				);
				for(int i = 0; i < 4; ++i)
					y2[y * WIDTH + x+i] = tempY[i];

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
					:"p"(retR),"p"(var1),"p"(retG),"p"(var2),"p"(retB),"p"(var3),"p"(var4),"p"(tempU)
					:
				);
				for(int i = 0; i < 4; ++i)
					u2[(y / 2) * (WIDTH / 2) + (x+i)/2] = tempU[i];

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
					:"p"(retR),"p"(var1),"p"(retG),"p"(var2),"p"(retB),"p"(var3),"p"(var4),"p"(tempV)
					:
				);
				for(int i = 0; i < 4; ++i)
					v2[(y / 2) * (WIDTH / 2) + (x+i)/2] = tempV[i];
			}
		}
	}
	void transmitSSE2(unsigned char* y1, unsigned char* u1, unsigned char* v1, 
	        unsigned char* y2, unsigned char* u2, unsigned char* v2, short A) 
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
		for (int y = 0; y < HEIGHT; y++) 
		{
			for (int x = 0; x < WIDTH; x += 8) 
			{
				__m128i temp1, temp2, temp3, temp4;
				__m128i tempR, tempG, tempB;
				__m128i R, G, B;
				__m128i Y, U, V;
				__m128i compareResult, mediaNum;
				
				__m128i my1 = _mm_set_epi16(y1[y * WIDTH + x], y1[y * WIDTH + x + 1], y1[y * WIDTH + x + 2], y1[y * WIDTH + x + 3], y1[y * WIDTH + x + 4], y1[y * WIDTH + x + 5], y1[y * WIDTH + x + 6], y1[y * WIDTH + x + 7]);
				__m128i mu = _mm_set_epi16(u1[(y / 2) * (WIDTH / 2) + x / 2], u1[(y / 2) * (WIDTH / 2) + (x + 1) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 2) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 3) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 4) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 5) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 6) / 2], u1[(y / 2) * (WIDTH / 2) + (x + 7) / 2]);
				__m128i mv = _mm_set_epi16(v1[(y / 2) * (WIDTH / 2) + x / 2], v1[(y / 2) * (WIDTH / 2) + (x + 1) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 2) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 3) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 4) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 5) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 6) / 2], v1[(y / 2) * (WIDTH / 2) + (x + 7) / 2]);

				my1 = _mm_sub_epi16(my1, Ymask);
				mu = _mm_sub_epi16(mu, Umask);
				mv = _mm_sub_epi16(mv, Vmask);
				
				temp1 = _mm_mulhi_epi16(my1, YUV2RGB1_1);
				temp1 = _mm_add_epi16(temp1, my1);
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
				R = _mm_mullo_epi16(tempR, mA);

				//get G element
				temp1 = _mm_mulhi_epi16(my1, YUV2RGB2_1);
				temp1 = _mm_add_epi16(temp1, my1);
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
				G = _mm_mullo_epi16(tempG, mA);

				//get B element
				temp1 = _mm_mulhi_epi16(my1, YUV2RGB3_1);
				temp1 = _mm_add_epi16(temp1, my1);
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
				B = _mm_mullo_epi16(tempB, mA);

				//alpha mix
				R = _mm_srai_epi16(R, 8);
				G = _mm_srai_epi16(G, 8);
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

				for (int i = 0; i < 8; i++) {
					y2[y * WIDTH + x + i] = ((short *)&Y)[8 - i];
					u2[(y / 2) * (WIDTH / 2) + (x + i) / 2] = ((short *)&U)[8 - i];
					v2[(y / 2) * (WIDTH / 2) + (x + i) / 2] = ((short *)&V)[8 - i];
				}

			}
		}
	}
	void transmitAVX(unsigned char* y1, unsigned char* u1, unsigned char* v1, 
	unsigned char* y2, unsigned char* u2, unsigned char* v2, short A) 
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
				float yyy[8];
				float uuu[8];
				float vvv[8];
				for (int i = 0; i < 8; i++) {
					yyy[i] = (int)y1[y * WIDTH + x + i] - 16;
					uuu[i] = (int)u1[(y / 2) * (WIDTH / 2) + (x + i) / 2] - 128;
					vvv[i] = (int)v1[(y / 2) * (WIDTH / 2) + (x + i) / 2] - 128;
				}

				__m256 R, G, B;
				__m256 Y, U, V;
				__m256 temp1, temp2, temp3;
				__m256 mask = _mm256_set1_ps((float)A);

				__m256 my =  _mm256_loadu_ps(yyy);
				__m256 mu =  _mm256_loadu_ps(uuu);
				__m256 mv =  _mm256_loadu_ps(vvv);
				
				temp1 = _mm256_mul_ps(my, YUV2RGB1_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB1_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB1_3);

				R = _mm256_add_ps(temp1, temp2);
				R = _mm256_add_ps(R, temp3);

				for (int i = 0; i < 8; i++) {
					if ( ((float *)&R)[i] > 255 )
						((float *)&R)[i] = 255;
					else if ( ((float *)&R)[i] < 0 )
						((float *)&R)[i] = 0;
				}

				temp1 = _mm256_mul_ps(my, YUV2RGB2_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB2_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB2_3);

				G = _mm256_add_ps(temp1, temp2);
				G = _mm256_add_ps(G, temp3);

				for (int i = 0; i < 8; i++) {
					if ( ((float *)&G)[i] > 255 )
						((float *)&G)[i] = 255;
					else if ( ((float *)&G)[i] < 0 )
						((float *)&G)[i] = 0;
				}
				
				temp1 = _mm256_mul_ps(my, YUV2RGB3_1);
				temp2 = _mm256_mul_ps(mu, YUV2RGB3_2);
				temp3 = _mm256_mul_ps(mv, YUV2RGB3_3);

				B = _mm256_add_ps(temp1, temp2);
				B = _mm256_add_ps(B, temp3);

				for (int i = 0; i < 8; i++) {
					if ( ((float *)&B)[i] > 255 )
						((float *)&B)[i] = 255;
					else if ( ((float *)&B)[i] < 0 )
						((float *)&B)[i] = 0;
				}

				R = _mm256_mul_ps(R, mask);
				R = _mm256_div_ps(R, divMask);
				G = _mm256_mul_ps(G, mask);
				G = _mm256_div_ps(G, divMask);
				B = _mm256_mul_ps(B, mask);
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

				_mm256_storeu_ps(yyy, Y);
				_mm256_storeu_ps(uuu, U);
				_mm256_storeu_ps(vvv, V);

				for (int i = 0; i < 8; i++) {
					y2[y * WIDTH + x + i] = (int)yyy[i];
					u2[(y / 2) * (WIDTH / 2) + (x + i) / 2] = (int)uuu[i];
					v2[(y / 2) * (WIDTH / 2) + (x + i) / 2] = (int)vvv[i];
				}
			}
		}
	}
};


int main() {
	cout << "Fade in fade out lab" << endl;
	cout << "choose a yuv image:../image/";
	char name1[30] = "../image/";
	cin >> (name1 + 9);
	int type;
	cout << "-------------MODE-------------" << endl;
	cout << "0:normal, 1:sse, 2:sse2, 3:avx" << endl;
	cout << "##############################" << endl;
	cout <<  "Choose a mode:";
	cin >> type;
	char name2[35] = "../image/fadeinout_noassem.yuv";
	YuvToArgbToYuv example;
	example.Convert(name1, name2, type);
	return 0;
}