#ifdef _CH_
#pragma package <opencv>
#endif


#pragma   comment(lib,   "vfw32.lib ")
#pragma comment (lib , "comctl32.lib")

#ifndef _EiC
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <ctype.h>
#include "basicOCR.h"
#endif

IplImage* imagen;
int red,green,blue;
IplImage* screenBuffer;
int drawing;
int r,last_x, last_y;

void draw(int x,int y)
{
	//Draw a circle when the mouse click or drag
	cvCircle(imagen, cvPoint(x,y), r, CV_RGB(red,green,blue), -1, 4, 0);
	//Get clean copy of image
	screenBuffer = cvCloneImage(imagen);
	//cvShowImage("pad", screenBuffer);
}

void drawCursor(int x, int y)
{
	//Get clean copy of image
	screenBuffer=cvCloneImage(imagen);
	//Draw a circle where is the mouse
	cvCircle(screenBuffer, cvPoint(x,y), r, CV_RGB(0,0,0), 1, 4, 0);
}


/*************************
* Mouse CallBack 在opencv的highgui.h头文件中有以下宏定义
* event: 
*	#define CV_EVENT_MOUSEMOVE      0
*	#define CV_EVENT_LBUTTONDOWN    1
*	#define CV_EVENT_RBUTTONDOWN    2
*	#define CV_EVENT_MBUTTONDOWN    3
*	#define CV_EVENT_LBUTTONUP      4
*	#define CV_EVENT_RBUTTONUP      5
*	#define CV_EVENT_MBUTTONUP      6
*	#define CV_EVENT_LBUTTONDBLCLK  7
*	#define CV_EVENT_RBUTTONDBLCLK  8
*	#define CV_EVENT_MBUTTONDBLCLK  9
*
* x, y: mouse position
*
* flag:
*	#define CV_EVENT_FLAG_LBUTTON   1
*	#define CV_EVENT_FLAG_RBUTTON   2
*	#define CV_EVENT_FLAG_MBUTTON   4
*	#define CV_EVENT_FLAG_CTRLKEY   8
*	#define CV_EVENT_FLAG_SHIFTKEY  16
*	#define CV_EVENT_FLAG_ALTKEY    32
*
**************************/

void on_mouse( int event, int x, int y, int flags, void* param )
{
	last_x=x;
	last_y=y;
	drawCursor(x,y);
	//Select mouse Event
	if(event==CV_EVENT_LBUTTONDOWN)
	{
		drawing=1;
		draw(x,y);
	}
	else if(event==CV_EVENT_LBUTTONUP)
	{
		//drawing=!drawing;
		drawing=0;
	}
	else if(event == CV_EVENT_MOUSEMOVE  &&  flags & CV_EVENT_FLAG_LBUTTON)
	{
		if(drawing)
			draw(x,y);
	}
}



int main( int argc, char** argv )
{
	printf( "                                 OCR\n"
		"HotKey: \n"
		"\tt - Test Mode\n"
		"\t\t+ - Size ++\n"
		"\t\t- - Size --\n"
		"\t\tr - reset the pad\n"
		"\t\tc - Recognise\n"
		"\tf - Read a text pic file\n"
		"\to - Open a pic\n"
		//		"\ts - 保存输入为 out.pbm\n"	//输入可以作为样本再次部署进去
		"\tESC - 退出程序\n");

	//////////////////
	//生成基础OCR类
	//////////////////
	basicOCR ocr;

	//Main Loop
	int c;
	c = getchar();

	while (true)
	{
		if((char)c == 't')
		{
			drawing=0;
			r=10;
			red=green=blue=0;
			last_x=last_y=red=green=blue=0;

			//Create image
			imagen = cvCreateImage(cvSize(128,128),IPL_DEPTH_8U,1); //白板图像保存为128*128
			//Set data of image to white
			cvSet(imagen, CV_RGB(255,255,255),NULL);
			//Image we show user with cursor and other artefacts we need
			screenBuffer=cvCloneImage(imagen);

			//Create window
			cvNamedWindow( "Pad", 0 );

			cvResizeWindow("Pad", 512, 512);
			//Create mouse CallBack
			cvSetMouseCallback("Pad", &on_mouse, 0);

			printf(" ------------------------------------------------------------------------\n");
			printf("|\t result \t|\t Accuracy\t|\t  Accuracy \t|\n");
			printf(" ------------------------------------------------------------------------\n");

			while(true)
			{
				cvShowImage( "Pad", screenBuffer );
				c = cvWaitKey(10);
				if(c == 27)
					break;
				else if((char) c == '+')
				{
					r++;
					drawCursor(last_x,last_y);
				}
				else if(((char)c == '-') && (r > 1))
				{
					r--;
					drawCursor(last_x,last_y);
				}
				else if((char)c == 'r')
				{
					cvSet(imagen, cvRealScalar(255),NULL);
					drawCursor(last_x,last_y);
				}
				/*else if((char)c == 's')
				{
				cvSaveImage("out.pbm", imagen);
				}*/
				else if((char)c == 'c')
				{
					ocr.classify(imagen,1);
				}
			}

			cvDestroyWindow("Pad");
		}
		else if((char)c == 'f')
		{
			char path[255];
			printf("Please input the path of the ImageFile:\n");
			scanf("%s", path);
			ocr.splitImage(path);
		}
		else if ((char)c == 'o')
		{
			char path[255];
			printf("Please input the path of the ImageFile:\n");
			scanf("%s", path);
			IplImage* src_image;
			/*if(!src_image)
			{
				printf("Error: Cant load image %s\n", path);
				continue;
			}*/
			src_image = cvLoadImage(path,0);
			ocr.classify(src_image, 1);
		}
		c = getchar();
	}
	return 0;
}

#ifdef _EiC
main(1,"mouseEvent.c");
#endif
