#include "ysglfontdata.h"
#include <stdio.h>

int YsGlWriteStringToSingleBitBitmap(
	char str[],
    unsigned char bmpPtr[],unsigned int bmpWid,unsigned int bmpHei,
    int bottomLeftX,int bottomLeftY,
    unsigned char *fontPtr[],int fontWid,int fontHei)
{
	int bmpY0,bmpX,fontY0;
	int strPtr;

	const unsigned int bytePerBmpLine=((bmpWid+31)/32)*4;   /* Assuming 32bit alignment */
	const unsigned int bytePerFontLine=((fontWid+31)/32)*4;
	const unsigned int byteWritePerFontLine=(fontWid+7)/8;

	unsigned char *bmpLineTop0;

	if(bottomLeftY<0)
	{
		fontY0=-bottomLeftY;
		bmpY0=0;
	}
	else
	{
		fontY0=0;
		bmpY0=bottomLeftY;
	}

	bmpLineTop0=bmpPtr+bytePerBmpLine*bmpY0;


	bmpX=bottomLeftX;
	for(strPtr=0; 0!=str[strPtr]; strPtr++,bmpX+=fontWid)
	{
		const unsigned char c=str[strPtr];
		unsigned char *bmpLineTop;
		unsigned char *fontLineTop;

		int fontY=fontY0;
		int bmpY=bmpY0;

		if(bmpX<=-fontWid)
		{
			continue;
		}

		bmpLineTop=bmpLineTop0;
		fontLineTop=fontPtr[c]+bytePerFontLine*fontY0;

		if(bmpX<0)
		{
			/* Partially visible
			   Optimization impact is low in this block >> */
			unsigned int hiddenWidth=-bmpX;

			unsigned int fontByteLoc=(hiddenWidth>>3);
			unsigned int fontBitShift=(hiddenWidth&7);
			unsigned char rightMask=(255>>(8-fontBitShift));

			while(fontY<fontHei && bmpY<bmpHei)
			{
				int i,bmpByteLoc;
				bmpByteLoc=0;
				for(i=fontByteLoc; i<byteWritePerFontLine && bmpByteLoc<bytePerBmpLine; i++)
				{
					unsigned char orByte=fontLineTop[i]<<fontBitShift;

					if(0!=fontBitShift && i+1<byteWritePerFontLine)
					{
						orByte|=(fontLineTop[i+1]>>(8-fontBitShift))&rightMask;
					}

					bmpLineTop[bmpByteLoc]|=orByte;

					bmpByteLoc++;
				}

				bmpLineTop+=bytePerBmpLine;
				fontLineTop+=bytePerFontLine;

				fontY++;
				bmpY++;
			}
			/* Optimization impact is low in this block << */
		}
		else /* if(0<bmpX) */
		{
			while(fontY<fontHei && bmpY<bmpHei)
			{
				unsigned int bitShift=(bmpX&7);
				unsigned int byteLoc=(bmpX>>3);

				if(0!=bitShift)
				{
					const unsigned char leftMask=(255<<bitShift);
					const unsigned char rightMask=(255>>(8-bitShift));
					int i;
					for(i=0; i<byteWritePerFontLine && byteLoc+i<bytePerBmpLine; i++)
					{
						unsigned char byteToCopy=fontLineTop[i];
						unsigned char left;

						left=(byteToCopy&leftMask)>>bitShift;
						bmpLineTop[byteLoc+i  ]|=left;

						if(byteLoc+i+1<bytePerBmpLine)
						{
							unsigned char right;
							right=(byteToCopy&rightMask)<<(8-bitShift);
							bmpLineTop[byteLoc+i+1]|=right;
						}
					}
				}
				else
				{
					int i;
					for(i=0; i<byteWritePerFontLine && byteLoc+i<bytePerBmpLine; i++)
					{
						bmpLineTop[byteLoc+i]|=fontLineTop[i];
					}
				}

				bmpLineTop+=bytePerBmpLine;
				fontLineTop+=bytePerFontLine;

				fontY++;
				bmpY++;
			}
		}
	}

	return 0;
}



int YsGlWriteStringToRGBA8Bitmap(
    char str[],unsigned int c0,unsigned int c1,unsigned int c2,unsigned int c3,
    unsigned char bmpPtr[],unsigned int bmpWid,unsigned int bmpHei,
    int bottomLeftX,int bottomLeftY,
    unsigned char *fontPtr[],int fontWid,int fontHei)
/* In OpenGL:   c0=r  c1=g  c2=b  c3=a
   In Direct3D: c0=a  c1=r  c2=g  c3=b */
{
	int bmpY0,bmpX,fontY0;
	int strPtr;

	const unsigned int bytePerBmpLine=bmpWid*4;
	const unsigned int bytePerFontLine=((fontWid+31)/32)*4;

	unsigned char *bmpLineTop0;

	if(bottomLeftY<0)
	{
		fontY0=-bottomLeftY;
		bmpY0=0;
	}
	else
	{
		fontY0=0;
		bmpY0=bottomLeftY;
	}

	bmpLineTop0=bmpPtr+bytePerBmpLine*bmpY0;


	bmpX=bottomLeftX;
	for(strPtr=0; 0!=str[strPtr]; strPtr++,bmpX+=fontWid)
	{
		const unsigned char c=str[strPtr];
		unsigned char *bmpLineTop;
		unsigned char *fontLineTop;

		int fontY=fontY0;
		int bmpY=bmpY0;

		int x;
		unsigned int fontX;
		unsigned int bit;

		if(bmpX<=-fontWid)
		{
			continue;
		}

		bmpLineTop=bmpLineTop0;
		fontLineTop=fontPtr[c]+bytePerFontLine*fontY0;


		while(fontY<fontHei && bmpY<bmpHei)
		{
			fontX=0;
			bit=fontLineTop[fontX];

			x=0;

			while(x+bmpX<0)
			{
				bit<<=1;
				if(7==(x&7))
				{
					bit=fontLineTop[++fontX];
				}
				x++;
			}

			for(x=x; x<fontWid && x+bmpX<bmpWid; x++)
			{
				if(bit&0x80)
				{
					unsigned int bmpXTimes4=((x+bmpX)<<2);
					bmpLineTop[bmpXTimes4  ]=c0;
					bmpLineTop[bmpXTimes4+1]=c1;
					bmpLineTop[bmpXTimes4+2]=c2;
					bmpLineTop[bmpXTimes4+3]=c3;
				}

				bit<<=1;
				if(7==(x&7))
				{
					bit=fontLineTop[++fontX];
				}
			}

			bmpLineTop+=bytePerBmpLine;
			fontLineTop+=bytePerFontLine;

			fontY++;
			bmpY++;
		}
	}

	return 0;
}



