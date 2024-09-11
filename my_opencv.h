# ifndef __MY_OPENCV__
# define __MY_OPENCV__

# include <math.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include "show.h"


// 获取图片信息
unsigned char* read_jpeg(const char *filename, int* pic_format);                              // 读取JPEG图像的像素内容
unsigned char* RGB_B(int x0, int y0, const unsigned char* pic, int* pic_format, int show);    // 获取B通道   
unsigned char* RGB_G(int x0, int y0, const unsigned char* pic, int* pic_format, int show);    // 获取G通道   
unsigned char* RGB_R(int x0, int y0, const unsigned char* pic, int* pic_format, int show);    // 获取R通道                                               
unsigned char* RGB2GRAY(int x0, int y0, const unsigned char* pic, int* pic_format, int show); // RGB2GRAY

// 卷积
unsigned char* Conv2D(const unsigned char* input, int input_width, int input_height,
                        const float* kernel, int kernel_width, int kernel_height,
                        int x0, int y0, int show);                                             // Conv2D                 

// 形态学处理
unsigned char* OTSU(const unsigned char* input, int input_width, int input_height, 
                        int x0, int y0, int show);                                             // OTSU                                                  

unsigned char* canny(const unsigned char* input, int input_width, int input_height, 
                        int x0, int y0, int show);                                             // canny

unsigned char* erosion_dliation(int operation, const unsigned char* input, int input_width, int input_height, 
                                    const int* kernel, int kernel_width, int kernel_height,
                                    int x0, int y0, int show);                                 // 腐蚀&膨胀


// 直线&圆检测
unsigned char* hough_transform(unsigned int* input, int width, int height, int R_MAX);         // 直线检测

#endif