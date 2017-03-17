#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "..\SDL\include\SDL_opengl.h"
#include "ysglfontdata.h"


static void ysGlMakeFontBitmapDisplayList(int listBase,unsigned char *fontPtr[],int wid,int hei)
{
	int i;
	glRasterPos2i(0,0);
	for(i=0; i<256; i++)
	{
		glNewList(listBase+i,GL_COMPILE);
		glBitmap(wid,hei,0,0,wid,0,fontPtr[i]);
		glEndList();
	}
}


void YsGlUseFontBitmap6x7(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont6x7,6,7);
}

void YsGlUseFontBitmap6x8(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont6x8,6,8);
}

void YsGlUseFontBitmap6x10(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont6x10,6,10);
}

void YsGlUseFontBitmap7x10(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont7x10,7,10);
}

void YsGlUseFontBitmap8x8(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont8x8,8,8);
}

void YsGlUseFontBitmap8x12(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont8x12,8,12);
}

void YsGlUseFontBitmap12x16(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont12x16,12,16);
}

void YsGlUseFontBitmap16x20(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont16x20,16,20);
}

void YsGlUseFontBitmap16x24(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont16x24,16,24);
}

void YsGlUseFontBitmap20x28(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont20x28,20,28);
}

void YsGlUseFontBitmap20x32(int listBase)
{
	ysGlMakeFontBitmapDisplayList(listBase,YsFont20x32,20,32);
}


void YsGlSelectAndUseFontBitmapByHeight(int *selectedWidth,int *selectedHeight,int listBase,int fontHeight)
{
	if(7>=fontHeight)
	{
		YsGlUseFontBitmap6x7(listBase);
		*selectedWidth=6;
		*selectedHeight=7;
	}
	else if(8>=fontHeight)
	{
		YsGlUseFontBitmap6x8(listBase);
		*selectedWidth=6;
		*selectedHeight=8;
	}
	else if(10>=fontHeight)
	{
		YsGlUseFontBitmap7x10(listBase);
		*selectedWidth=7;
		*selectedHeight=10;
	}
	else if(12>=fontHeight)
	{
		YsGlUseFontBitmap8x12(listBase);
		*selectedWidth=8;
		*selectedHeight=12;
	}
	else if(16>=fontHeight)
	{
		YsGlUseFontBitmap12x16(listBase);
		*selectedWidth=12;
		*selectedHeight=16;
	}
	else if(20>=fontHeight)
	{
		YsGlUseFontBitmap16x20(listBase);
		*selectedWidth=16;
		*selectedHeight=20;
	}
	else if(24>=fontHeight)
	{
		YsGlUseFontBitmap16x24(listBase);
		*selectedWidth=16;
		*selectedHeight=24;
	}
	else if(28>=fontHeight)
	{
		YsGlUseFontBitmap20x28(listBase);
		*selectedWidth=20;
		*selectedHeight=28;
	}
	else // if(32>=fontHeight)
	{
		YsGlUseFontBitmap20x32(listBase);
		*selectedWidth=20;
		*selectedHeight=32;
	}
}
