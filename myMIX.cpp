void transmitFIFO(unsigned char* y1, unsigned char* u1, unsigned char* v1, 
        unsigned char* y2, unsigned char* u2, unsigned char* v2, int A) 
{
    unsigned short tempY1[8];
    unsigned short tempV1[8];
    unsigned short tempU1[8];
    unsigned short tempY2[8];
    unsigned short tempV2[8];
    unsigned short tempU2[8];

    unsigned short retR1[8];
    unsigned short retG1[8];
    unsigned short retB1[8];
    unsigned short retR2[8];
    unsigned short retG2[8];
    unsigned short retB2[8];

    unsigned short numA[8];
    unsigned short num256[8];
    unsigned short num255[8];
    unsigned short num128[8];
    unsigned short num16[8];

    for(int i = 0; i < 8; ++i)
        numA[i] = A;
    for(int i = 0; i < 8; ++i)
        num256[i] = 256;
    for(int i = 0; i < 8; ++i)
        num255[i] = 255;
    for(int i = 0; i < 8; ++i)
        num128[i] = 128;
    for(int i = 0; i < 8; ++i)
        num16[i] = 16;

    unsigned short var1[4];
    unsigned short var2[4];
    unsigned short var3[4];
    unsigned short var4[4];
    int shift = 8;

    for(int y = 0; y < HEIGHT; y++) 
    {
        for(int x=0; x < WIDTH; x+=8) 
        {
            for(int i = 0; i < 8; ++i)
                tempY1[i] = y1[y*WIDTH+x+i]-16;
            for(int i = 0; i < 8; ++i)
                tempV1[i] = v1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
            for(int i = 0; i < 8; ++i)
                tempU1[i] = u1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
            for(int i = 0; i < 8; ++i)
                tempY2[i] = y1[y*WIDTH+x+i]-16;
            for(int i = 0; i < 8; ++i)
                tempV2[i] = v1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
            for(int i = 0; i < 8; ++i)
                tempU2[i] = u1[(y / 2) * (WIDTH / 2) + (x+i) / 2]-128;
            //r
            for(int i = 0; i < 8; ++i)
                var1[i] = 298;	
            for(int i = 0; i < 8; ++i)
                var2[i] = 409;
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PADDW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %5, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %6;"
                "STIII %%pmm2, %4, 0;"
                :
                :"p"(tempY1),"p"(var1),"p"(tempV1),"p"(var2),"p"(retR1),"p"(num128),"p"(shift)
                :
            );
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PADDW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %5, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %6;"
                "STIII %%pmm2, %4, 0;"
                :
                :"p"(tempY2),"p"(var1),"p"(tempV2),"p"(var2),"p"(retR2),"p"(num128),"p"(shift)
                :
            );
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm3, %2, 0;"
                "PSUBW %%pmm3, %%pmm3, %%pmm1;"
                "LDIII %%pmm4, %3, 0;"
                "PMULW %%pmm4, %%pmm4, %%pmm3;"
                "PADDW %%pmm2, %%pmm2, %%pmm4;"
                "STIII %%pmm2, %0, 0;"
                :
                :"p"(retR1),"p"(numA),"p"(num256),"p"(retR2)
            )

            //g
            for(int i = 0; i < 8; ++i)
                var1[i] = 298;	
            for(int i = 0; i < 8; ++i)
                var2[i] = 100;
            for(int i = 0; i < 8; ++i)
                var3[i] = 208;
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm0, %4, 0;"
                "LDIII %%pmm1, %5, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %7, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %8;"
                "LDIII %%pmm1, %9, 0;"
                "PMULW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %8;"
                "STIII %%pmm2, %4, 0;"
                :
                :"p"(tempY1),"p"(var1),"p"(tempU1),"p"(var2),"p"(tempV1),"p"(var3),"p"(retG1),"p"(num128),"p"(shift),"p"(numA)
                :
            );
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm0, %4, 0;"
                "LDIII %%pmm1, %5, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %7, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %8;"
                "LDIII %%pmm1, %9, 0;"
                "PMULW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %8;"
                "STIII %%pmm2, %4, 0;"
                :
                :"p"(tempY2),"p"(var1),"p"(tempU2),"p"(var2),"p"(tempV2),"p"(var3),"p"(retG2),"p"(num128),"p"(shift),"p"(numA)
                :
            );

            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm3, %2, 0;"
                "PSUBW %%pmm3, %%pmm3, %%pmm1;"
                "LDIII %%pmm4, %3, 0;"
                "PMULW %%pmm4, %%pmm4, %%pmm3;"
                "PADDW %%pmm2, %%pmm2, %%pmm4;"
                "STIII %%pmm2, %0, 0;"
                :
                :"p"(retG1),"p"(numA),"p"(num256),"p"(retG2)
            )
            //b
            for(int i = 0; i < 8; ++i)
                var1[i] = 298;	
            for(int i = 0; i < 8; ++i)
                var2[i] = 516;
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PADDW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %5, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %6;"
                "LDIII %%pmm1, %7, 0;"
                "PMULW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %6;"
                "STIII %%pmm2, %4, 0;"
                :
                :"p"(tempY1),"p"(var1),"p"(tempU1),"p"(var2),"p"(retB1),"p"(num128),"p"(shift),"p"(numA)
                :
            );
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PADDW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %5, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %6;"
                "LDIII %%pmm1, %7, 0;"
                "PMULW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %6;"
                "STIII %%pmm2, %4, 0;"
                :
                :"p"(tempY2),"p"(var1),"p"(tempU2),"p"(var2),"p"(retB2),"p"(num128),"p"(shift),"p"(numA)
                :
            );
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm3, %2, 0;"
                "PSUBW %%pmm3, %%pmm3, %%pmm1;"
                "LDIII %%pmm4, %3, 0;"
                "PMULW %%pmm4, %%pmm4, %%pmm3;"
                "PADDW %%pmm2, %%pmm2, %%pmm4;"
                "STIII %%pmm2, %0, 0;"
                :
                :"p"(retB1),"p"(numA),"p"(num256),"p"(retB2)
            )

            for(int i = 0; i < 8; ++i)
                var1[i] = 66;	
            for(int i = 0; i < 8; ++i)
                var2[i] = 129;
            for(int i = 0; i < 8; ++i)
                var3[i] = 25;
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PADDW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm0, %4, 0;"
                "LDIII %%pmm1, %5, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PADDW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %6, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %7;"
                "LDIII %%pmm1, %8, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "STIII %%pmm2, %9, 0;"
                :
                :"p"(retR1),"p"(var1),"p"(retG1),"p"(var2),"p"(retB1),"p"(var3),"p"(num128),"p"(shift),"p"(num16),"p"(tempY1)
                :
            );
            for(int i = 0; i < 8; ++i)
                y3[y * WIDTH + x+i] = tempY1[i];

            for(int i = 0; i < 8; ++i)
                var1[i] = 38;	
            for(int i = 0; i < 8; ++i)
                var2[i] = 74;
            for(int i = 0; i < 8; ++i)
                var3[i] = 112;
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm0, %4, 0;"
                "LDIII %%pmm1, %5, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %6, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %7;"
                "LDIII %%pmm1, %8, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "STIII %%pmm2, %9, 0;"
                :
                :"p"(retR1),"p"(var1),"p"(retG1),"p"(var2),"p"(retB1),"p"(var3),"p"(num128),"p"(shift),"p"(num128),"p"(tempU1)
                :
            );
            for(int i = 0; i < 8; ++i)
                u3[(y / 2) * (WIDTH / 2) + (x+i)/2] = tempU1[i];

            for(int i = 0; i < 8; ++i)
                var1[i] = 112;	
            for(int i = 0; i < 8; ++i)
                var2[i] = 94;
            for(int i = 0; i < 8; ++i)
                var3[i] = 18;
            asm
            (
                "LDIII %%pmm0, %0, 0;"
                "LDIII %%pmm1, %1, 0;"
                "PMULW %%pmm2, %%pmm0, %%pmm1;"
                "LDIII %%pmm0, %2, 0;"
                "LDIII %%pmm1, %3, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm0, %4, 0;"
                "LDIII %%pmm1, %5, 0;"
                "PMULW %%pmm3, %%pmm0, %%pmm1;"
                "PSUBW %%pmm2, %%pmm2, %%pmm3;"
                "LDIII %%pmm1, %6, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "PSRLW %%pmm2, %%pmm2, %7;"
                "LDIII %%pmm1, %8, 0;"
                "PADDW %%pmm2, %%pmm1, %%pmm2;"
                "STIII %%pmm2, %9, 0;"
                :
                :"p"(retR1),"p"(var1),"p"(retG1),"p"(var2),"p"(retB1),"p"(var3),"p"(num128),"p"(shift),"p"(num128),"p"(tempV1)
                :
            );
            for(int i = 0; i < 8; ++i)
                v3[(y / 2) * (WIDTH / 2) + (x+i)/2] = tempV1[i];
        }
    }
}