/*
*  basicOCR.c
*  
*
*  Created by damiles on 18/11/08.
*  Copyright 2008 Damiles. GPL License
*
*/
#ifdef _CH_
#pragma package <opencv>
#endif

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#include "ml.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif

#include "preprocessing.h"
#include "basicOCR.h"

basicOCR::basicOCR()//构造函数
{

	//initial
	sprintf(file_path , "OCR/");
	train_samples = 50; //训练样本
	classes = 95;// ASCII

	size = 128;//


	trainData = cvCreateMat(train_samples*classes, size*size, CV_32FC1);//训练数据的矩阵
	trainClasses = cvCreateMat(train_samples*classes, 1, CV_32FC1);

	//Get data (get images and process it)
	getData();

	//train	
	train();
	//Test	
	//test();
}



void basicOCR::getData()
{
	IplImage* src_image;
	IplImage prs_image;
	CvMat row,data;
	char file[255];
	int i,j;
	//for(i =0; i<classes; i++)
	for (i = 32; i < 32 + classes; i++)
	{
		for ( j = 0; j < train_samples; j++)
		{
			//加载pbm格式图像，作为训练
			/*if(j < 10)
			sprintf(file,"%s%d/%d0%d.pbm",file_path, i - 48, i - 48 , j);
			else
			sprintf(file,"%s%d/%d%d.pbm",file_path, i - 48, i - 48 , j);*/
			if (i >= 48 && i <= 57)
				sprintf(file,"%s%d/%d.pbm",file_path, i, j);
			else
				sprintf(file,"%s%d/%d.bmp",file_path, i, j);
			src_image = cvLoadImage(file,0);
			if(!src_image)
			{
				//printf("Error: Cant load image %s\n", file);
				continue;
				//exit(-1);
			}
			//process file
			prs_image = preprocessing(src_image, size, size);

			//Set class label
			cvGetRow(trainClasses, &row, (i - 32)*train_samples + j);
			cvSet(&row, cvRealScalar(i));
			//Set data 
			cvGetRow(trainData, &row, (i - 32)*train_samples + j);

			IplImage* img = cvCreateImage( cvSize( size, size ), IPL_DEPTH_32F, 1 );
			//convert 8 bits image to 32 float image
			cvConvertScale(&prs_image, img, 0.0039215, 0);

			cvGetSubRect(img, &data, cvRect(0,0, size,size));

			CvMat row_header, *row1;
			//convert data matrix sizexsize to vecor
			row1 = cvReshape( &data, &row_header, 0, 1 );
			cvCopy(row1, &row, NULL);
		}
	}
}

void basicOCR::train()
{
	knn=new CvKNearest( trainData, trainClasses, 0, false, K );
}

float basicOCR::classify(IplImage* img, int showResult)//if showresult == 1, print result
{
	IplImage prs_image;
	CvMat data;
	CvMat* nearest=cvCreateMat(1,K,CV_32FC1);
	float result;
	//处理输入的图像
	prs_image = preprocessing(img, size, size);

	//Set data 
	IplImage* img32 = cvCreateImage( cvSize( size, size ), IPL_DEPTH_32F, 1 );
	cvConvertScale(&prs_image, img32, 0.0039215, 0);
	cvGetSubRect(img32, &data, cvRect(0,0, size,size));
	CvMat row_header, *row1;
	row1 = cvReshape( &data, &row_header, 0, 1 );

	result=knn->find_nearest(row1,K,0,0,nearest,0);

	int accuracy=0;
	for(int i=0;i<K;i++)
	{
		if( (nearest->data.fl[i]) == result)
			accuracy++;
	}
	float pre=100*((float)accuracy/(float)K);
	if(showResult==1)
	{
		//printf("|\t    %.0f    \t| \t    %.2f%%  \t| \t %d of %d \t| \n",result,pre,accuracy,K);
		printf("|\t    %c    \t| \t    %.2f%%  \t| \t %d of %d \t| \n", (char)result,pre,accuracy,K);
		printf(" ------------------------------------------------------------------------\n");
	}

	return result;

}

CvRect basicOCR::findFirstChar(CvSeq* seq, int column)
{
	CvRect rcFirst = {0};
	int y = 0;	// find first row
	int x = 0;

	for (CvSeq* c = seq; c != NULL; c = c->h_next)
	{
		CvRect rc = cvBoundingRect(c,0);
		if (rc.y > column)
		{
			if (y == 0)
				y = rc.y;
			else if (rc.y < y)
			{
				y = rc.y;
				x = rc.x;
			}
		}
	}

	for (CvSeq* c = seq; c != NULL; c = c->h_next)
	{
		CvRect rc = cvBoundingRect(c,0);
		if ((rc.y >= (y - rc.height / 2)) && (rc.y <= (y + rc.height / 2)))	// in the same row
		{
			if (rc.x < x)
			{
				x = rc.x;	// find first column
				rcFirst = cvBoundingRect(c, 0);
			}
		}
	}

	return rcFirst;	// if cannot find return 0
}

CvRect basicOCR::findPrintRect(CvSeq* seq, int x, int y, CvRect rcFirst)
{
	CvRect rcPrint = {0};

	//printf("\n>>>Testing in basicOCR::findPrintRect<<<\n");
	//printf("x = %d, y = %d\n\n", x, y);
	for (CvSeq* c = seq; c != NULL; c = c->h_next)
	{
		CvRect rc = cvBoundingRect(c,0);
		if ((rc.y >= (y - rcFirst.height)) && (rc.y <= (y + rcFirst.height)))	// in the same row
		{
			//printf("rc.x = %d, rc.y = %d\n", rc.x, rc.y);
			if (rc.x >= x)
				if (rcPrint.x == 0)
					rcPrint = rc;
				else if (rc.x <= rcPrint.x)
					rcPrint = rc;				
		}
	}

	return rcPrint;
}

void basicOCR::printCvSeq(CvSeq* seq, IplImage* imgSrc, IplImage* img_gray, CvMemStorage* storage)
{
	CvSeq* si = seq;
	CvRect rcFirst = findFirstChar(seq, 0);
	if (rcFirst.x == 0)
	{
		printf("No words found...\n");
		return;
	}
	else
		printf("\nOCR of text:\n");
	CvRect rcNewFirst = rcFirst;
	cvDrawRect(imgSrc, cvPoint(rcFirst.x, rcFirst.y), cvPoint(rcFirst.x + rcFirst.width, rcFirst.y + rcFirst.height), CV_RGB(0, 0, 0));
	int printX = rcFirst.x - 1;
	int printY = rcFirst.y - 1;

	int idx = 0;
	char szName[56] = {0};
	int tempCount=0;

	while (true)
	{
		CvRect rc = findPrintRect(seq, printX, printY, rcFirst);
		cvDrawRect(imgSrc, cvPoint(rc.x, rc.y), cvPoint(rc.x + rc.width, rc.y + rc.height), CV_RGB(0, 0, 0));
		// dealing with useless Part
		/*if (rc.width <= 1 && rc.height <= 1)
		{
		continue;
		}*/

		if (printX < rc.x)
		{
			if ((rc.x - printX) >= (rcFirst.width / 2))
				printf(" ");
			printX = rc.x;
			//cvDrawRect(imgSrc, cvPoint(rc.x, rc.y), cvPoint(rc.x + rc.width, rc.y + rc.height), CV_RGB(255, 0, 0));
			IplImage* imgNo = cvCreateImage(cvSize(rc.width, rc.height), IPL_DEPTH_8U, 3);
			cvSetImageROI(imgSrc, rc);
			cvCopyImage(imgSrc, imgNo);
			cvResetImageROI(imgSrc);
			sprintf(szName, "wnd_%d", idx++);
			// show splited picture or not
			cvNamedWindow(szName);
			cvShowImage(szName, imgNo);
			IplImage* imgDst = cvCreateImage(cvSize(rc.width, rc.height),IPL_DEPTH_8U,1);
			cvCvtColor(imgNo, imgDst, CV_RGB2GRAY);
			printf("%c", (char)classify(imgDst, 0));
			cvReleaseImage(&imgNo);
		}
		else if (printX == rc.x && printX < imgSrc->width)
		{
			printX += rc.width;
		}
		else
		{
			printf("\n");
			printY = rcNewFirst.y + rcNewFirst.height;
			rcNewFirst = findFirstChar(seq, printY);
			if (rcNewFirst.x == 0)
				break;
			cvDrawRect(imgSrc, cvPoint(rcNewFirst.x, rcNewFirst.y), cvPoint(rcNewFirst.x + rcNewFirst.width, rcNewFirst.y + rcNewFirst.height), CV_RGB(0, 0, 0));
			printX = rcNewFirst.x - 1;
			printY = rcNewFirst.y - 1;
		}
	}
	cvNamedWindow("src");
	cvShowImage("src", imgSrc);
	cvWaitKey(0);
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&imgSrc);
	cvReleaseImage(&img_gray);
	cvDestroyAllWindows();

}

void basicOCR::splitImage(char *path)
{
	IplImage* imgSrc = cvLoadImage(path, CV_LOAD_IMAGE_COLOR);
	if(!imgSrc)
	{
		printf("Error: Cant load image %s\n", path);
		return ;
	}
	IplImage* img_gray = cvCreateImage(cvGetSize(imgSrc), IPL_DEPTH_8U, 1);
	cvCvtColor(imgSrc, img_gray, CV_BGR2GRAY);
	cvThreshold(img_gray, img_gray,100, 255,CV_THRESH_BINARY_INV);// CV_THRESH_BINARY_INV使得背景为黑色，字符为白色，这样找到的最外层才是字符的最外层
	//cvShowImage("ThresholdImg",img_gray);
	CvSeq* contours = NULL;
	CvMemStorage* storage = cvCreateMemStorage(0);
	int count = cvFindContours(img_gray, storage, &contours,sizeof(CvContour),CV_RETR_EXTERNAL);
	printf("count number：%d\n",count);
	printCvSeq(contours, imgSrc, img_gray, storage);
}

void basicOCR::test()
{
	IplImage* src_image;
	IplImage prs_image;
	CvMat row,data;
	char file[255];
	int i,j;
	int error=0;
	int testCount=0;
	for(i =0; i<classes; i++)
	{
		for( j = 50; j< 50+train_samples; j++)//五十个测试样本，计算一下错误率
		{

			sprintf(file,"%s%d/%d%d.pbm",file_path, i, i , j);
			src_image = cvLoadImage(file,0);
			if(!src_image)
			{
				printf("Error: Cant load image %s\n", file);
				exit(-1);
			}
			//process file
			prs_image = preprocessing(src_image, size, size);
			float r=classify(&prs_image,0);
			if((int)r!=i)
				error++;

			testCount++;
		}
	}
	float totalerror=100*(float)error/(float)testCount;
	printf("测试系统误识率: %.2f%%\n", totalerror);

}
