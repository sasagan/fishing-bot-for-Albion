#include <Windows.h>
#include <windowsx.h>
#include <iostream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdlib.h>
using namespace std;
#include <gdiplus.h>

#include <fstream>
#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>

#pragma comment(lib, "GdiPlus.lib")
using namespace Gdiplus;
static const GUID png =
{ 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e } };
using namespace cv;

void screenshot()
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HDC scrdc, memdc;
    HBITMAP membit;

    scrdc = GetDC(0);

    int Height, Width;
    Height = GetSystemMetrics(SM_CYSCREEN);
    Width = GetSystemMetrics(SM_CXSCREEN);

    memdc = CreateCompatibleDC(scrdc);
    membit = CreateCompatibleBitmap(scrdc, Width, Height);
    SelectObject(memdc, membit);

    BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);
    HBITMAP hBitmap;
    hBitmap = (HBITMAP)SelectObject(memdc, membit);
    Gdiplus::Bitmap bitmap(hBitmap, NULL);
    bitmap.Save(L"F:/Games/programming/c++/Albion bot/x64/Debug/screen.png", &png);

    DeleteObject(hBitmap);
    DeleteObject(&gdiplusToken);
    DeleteObject(&gdiplusStartupInput);
    DeleteObject(scrdc);
    DeleteObject(memdc);
    DeleteObject(membit);
    DeleteObject(&bitmap);
}

IplImage* image = 0;
IplImage* templ = 0;


#define RECT_COLORS_SIZE 10
#define CV_PIXEL(type,img,x,y) (((type*)((img)->imageData+(y)*(img)->widthStep))+(x)*(img)->nChannels)

enum { cBLACK = 0, cWHITE, cGREY, cRED, cORANGE, cYELLOW, cGREEN, cAQUA, cBLUE, cPURPLE, NUM_COLOR_TYPES };
const char* sCTypes[NUM_COLOR_TYPES] = { "Black", "White","Grey","Red","Orange","Yellow","Green","Aqua","Blue","Purple" };
uchar cCTHue[NUM_COLOR_TYPES] = { 0,       0,      0,     0,     20,      30,      60,    85,   120,    138 };
uchar cCTSat[NUM_COLOR_TYPES] = { 0,       0,      0,    255,   255,     255,     255,   255,   255,    255 };
uchar cCTVal[NUM_COLOR_TYPES] = { 0,      255,    120,   255,   255,     255,     255,   255,   255,    255 };

typedef unsigned int uint;


int colorCount[NUM_COLOR_TYPES] = { 0,		0,		0,		0,		0,		0,		0,		0,		0,		0 };


int getPixelColorType(int H, int S, int V)
{
    int color = cBLACK;

#if 1
    if (V < 75)
        color = cBLACK;
    else if (V > 190 && S < 27)
        color = cWHITE;
    else if (S < 53 && V < 185)
        color = cGREY;
    else
#endif
    {
        if (H < 7)
            color = cRED;
        else if (H < 25)
            color = cORANGE;
        else if (H < 34)
            color = cYELLOW;
        else if (H < 73)
            color = cGREEN;
        else if (H < 102)
            color = cAQUA;
        else if (H < 140)
            color = cBLUE;
        else if (H < 170)
            color = cPURPLE;
        else
            color = cRED;
    }
    return color;
}


int colorRed(int argc, char* argv[], IplImage* image)
{
    colorCount[cRED] = 0;

    POINT cp;
    GetCursorPos(&cp);
    IplImage* hsv = 0, * dst = 0, * dst2 = 0, * color_indexes = 0;

    CvMat ref = imread("screen.png");

    cvSetImageROI(image, cvRect(argc >= 3 ? atoi(argv[2]) : cp.x - 11, argc >= 4 ? atoi(argv[3]) : cp.y - 16, argc >= 5 ? atoi(argv[4]) : 22, argc >= 6 ? atoi(argv[5]) : 32));

    hsv = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
    cvCvtColor(image, hsv, CV_BGR2HSV);

    dst = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
    dst2 = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
    color_indexes = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1); //для хранения индексов цвета

    CvScalar rgb_colors[NUM_COLOR_TYPES];

    for (int j = 0; j < NUM_COLOR_TYPES; j++) {
        rgb_colors[j] = cvScalarAll(0);
    }

    for (int y = 0; y < hsv->height; y++) {
        for (int x = 0; x < hsv->width; x++) {

            uchar H = CV_PIXEL(uchar, hsv, x, y)[0];
            uchar S = CV_PIXEL(uchar, hsv, x, y)[1];
            uchar V = CV_PIXEL(uchar, hsv, x, y)[2];

            int ctype = getPixelColorType(H, S, V);

            CV_PIXEL(uchar, dst, x, y)[0] = cCTHue[ctype];
            CV_PIXEL(uchar, dst, x, y)[1] = cCTSat[ctype];
            CV_PIXEL(uchar, dst, x, y)[2] = cCTVal[ctype];

            rgb_colors[ctype].val[0] += CV_PIXEL(uchar, image, x, y)[0];
            rgb_colors[ctype].val[1] += CV_PIXEL(uchar, image, x, y)[1];
            rgb_colors[ctype].val[2] += CV_PIXEL(uchar, image, x, y)[2];

            CV_PIXEL(uchar, color_indexes, x, y)[0] = ctype;

            colorCount[ctype]++;
        }
    } 
    cvReleaseImage(&hsv);
    cvReleaseImage(&dst);
    cvReleaseImage(&dst2);
    cvReleaseImage(&color_indexes);
    return colorCount[cRED];
   
}

int main(int argc, char* argv[])
{
    const char* filename2 = argc >= 3 ? argv[2] : "pattern.png";
    printf("[i] template: %s\n", filename2);
    templ = cvLoadImage(filename2, 1);
    assert(templ != 0);

    int width = templ->width;
    int height = templ->height;

    Sleep(8000);
    cout << "start bot" << endl;

    int coof = 75;
    int colored;

    while (true)
    {
        screenshot();

        const char* filename = argc >= 2 ? argv[1] : "screen.png";
        printf("[i] image: %s\n", filename);
        image = cvLoadImage(filename, 1);
        assert(image != 0);
        

        colored = colorRed(argc, argv, image);

        cout << "number of pixels: " << colored << endl;


        if (colored > 10 and colored < coof and colored != 0)
        {
            cout << "pattern found: yes" << endl;
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            int i = 0;
            Sleep(510);
            IplImage* res;
            
            while (true)
            {
                screenshot();
                
                filename = argc >= 2 ? argv[1] : "screen.png";
                image = cvLoadImage(filename, 1);
                printf("[i] image: %s\n", filename);
                assert(image != 0);

                res = cvCreateImage(cvSize((image->width - templ->width + 1), (image->height - templ->height + 1)), IPL_DEPTH_32F, 1);
                cvMatchTemplate(image, templ, res, CV_TM_SQDIFF);
               
                double    minval, maxval;
                CvPoint    minloc, maxloc, minloclimit, maxloclimit;

                minloclimit.x = 420;
                minloclimit.y = 382;
                maxloclimit.x = 598;
                maxloclimit.y = 490;

                cvMinMaxLoc(res, &minval, &maxval, &minloc, &maxloc, 0);
                
                cvNormalize(res, res, 1, 0, CV_MINMAX);
                
                cout << "minval: " << minval << endl;
                i++;
                if (minval <= 16500000)
                {

                    if (minloc.x < 494)
                    {
                        i = 0;
                        cout << minloc.x << endl;
                        cout << minloc.y << endl;
                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                        Sleep(1800);
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        cvReleaseImage(&res);
                        cvReleaseImage(&image);
                    }
                    else if (minloc.x >= 495)
                    {
                        continue;
                    }

                }
                else if (minval > 16500000)
                {
                    cout << "work yes" << endl;
                    
                    cvReleaseImage(&res);
                    cvReleaseImage(&image);

                    Sleep(1900);
                    break;
                }

                if (i >= 6)
                {
                    cout << "i59fwe04fa" << endl;
                    cvReleaseImage(&res);
                    cvReleaseImage(&image);
                    break;
                }
                
                cvReleaseImage(&res);
                cvReleaseImage(&image);
            }
        }
        if (colored == 0)
        {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            Sleep(1800);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            Sleep(1000);
            cvReleaseImage(&image);
        }
        colored = 0;
        cvReleaseImage(&image);
    }
    cvWaitKey(0);
   
    return 0;
}