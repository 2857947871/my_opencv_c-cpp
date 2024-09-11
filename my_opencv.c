# include <stdio.h>
# include <fcntl.h>
# include <stdlib.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <linux/fb.h>
# include <jpeglib.h>
# include "show.h"
# include "my_opencv.h"


// 获取图片信息
unsigned char* read_jpeg(const char *filename, int* pic_format) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;

    // 打开 JPEG 文件
    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        return NULL;
    }

    // 设置 JPEG 错误处理
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    // 指定输入源
    jpeg_stdio_src(&cinfo, infile);
    
    // 读取 JPEG 文件头
    jpeg_read_header(&cinfo, TRUE);
    
    // 启动解压缩
    jpeg_start_decompress(&cinfo);

    // 获取图像的宽度、高度和每行字节数
    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int row_stride = width * cinfo.output_components;
    printf("width: %d\n", width);
    printf("height: %d\n", height);
    
    pic_format[0] = width;
    pic_format[1] = height;

    unsigned char* pic = (unsigned char* )malloc(width * height * 3 * sizeof(unsigned char));

    // 分配内存以存储扫描行
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    // 处理每行的像素数据
    int i = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        
        // 处理当前扫描行的像素数据
        int x = 0;
        for (x = 0; x < width; x++) {
            // 每个像素由 cinfo.output_components 个分量组成（通常为 3 或 4）
            unsigned char *pixel = buffer[0] + x * cinfo.output_components;
            
            // 获取 RGB 值（假设图像为 RGB）
            unsigned char red = pixel[0];   
            pic[i] = red;   ++i;
            unsigned char green = pixel[1];
            pic[i] = green; ++i;
            unsigned char blue  = pixel[2];
            pic[i] = blue;  ++i;
            // 输出当前像素的 RGB 值
            // printf("Pixel(%d, %d): R=%u, G=%u, B=%u\n", x, cinfo.output_scanline - 1, red, green, blue);
        }
    }

    // 完成解压缩
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);


    return pic;
}

// 提取三通道
unsigned char* RGB_B(int x0, int y0, const unsigned char* pic, int* pic_format, int show) {

    int i, j, idx;

    int width = pic_format[0];
    int height = pic_format[1];
    unsigned char* pic_b = malloc(sizeof(int) * width * height);

    idx = 0;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            col c;
            unsigned int b;
            b = *pic;
            pic_b[idx++] = *pic;
            pic += 3;
            c.l = (b >> 3) << 0;
            if (show) {
                draw_point(x0 + i, y0 + j, c);
            } else {
                continue;
            }
        }
    }

    return pic_b;
}

unsigned char* RGB_G(int x0, int y0, const unsigned char* pic, int* pic_format, int show) {

    int i, j, idx;

    int width = pic_format[0];
    int height = pic_format[1];
    unsigned char* pic_g = malloc(sizeof(int) * width * height);

    idx = 0;
    pic = pic + 1;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            col c;
            unsigned int g;
            g = *pic;
            pic_g[idx++] = *pic;
            pic += 3;
            c.l = (g >> 2) << 5;
            if (show) {
                draw_point(x0 + i, y0 + j, c);
            } else {
                continue;
            }
        }
    }

    return pic_g;
}

unsigned char* RGB_R(int x0, int y0, const unsigned char* pic, int* pic_format, int show) {

    int i, j, idx;

    int width = pic_format[0];
    int height = pic_format[1];
    unsigned char* pic_r = malloc(sizeof(int) * width * height);

    idx = 0;
    pic = pic + 2;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            col c;
            unsigned int r;
            r = *pic;
            pic_r[idx++] = *pic;
            pic += 3;
            c.l = (r >> 3) << 11;
            if (show) {
                draw_point(x0 + i, y0 + j, c);
            } else {
                continue;
            }
        }
    }

    return pic_r;
}

// 提取灰度图
// Gray = 0.299×R + 0.587×G + 0.114×B
unsigned char* RGB2GRAY(int x0, int y0, const unsigned char* pic, int* pic_format, int show) {
    
    int width = pic_format[0];
    int height = pic_format[1];

    int pic_size = width * height;
    unsigned char* pic_b = NULL;
    unsigned char* pic_g = NULL;
    unsigned char* pic_r = NULL;
    unsigned char* pic_gray = malloc(sizeof(unsigned char) * pic_size);

    pic_b = RGB_B(0, 0, pic, pic_format, 0);
    pic_g = RGB_G(0, 0, pic, pic_format, 0);
    pic_r = RGB_R(0, 0, pic, pic_format, 0);

    int i, j;
    for (i = 0; i < pic_size; ++i) {
        pic_gray[i] = (int)((float)(pic_r[i] * 0.299) + 
                            (float)(pic_g[i] * 0.587) + 
                            (float)(pic_b[i] * 0.114));
    }

    if (show) {
        for (j = 0; j < height; ++j) {
            for (i = 0; i < width; ++i) {
                col c;
                int idx = j * width + i;
                unsigned int gray;
                gray = pic_gray[idx];
                c.l = ((gray >> 3) << 0) | ((gray >> 2) << 5) | ((gray >> 3) << 11);
                draw_point(x0 + i, y0 + j, c);
            }
        }
        return pic_gray;
    } else {
        return pic_gray;
    }
}

// 卷积
unsigned char* Conv2D(const unsigned char* input, int input_width, int input_height,
                        const float* kernel, int kernel_width, int kernel_height,
                        int x0, int y0, int show)
{
    // 为保持卷积后图片大小不变, 默认padding
    // padding策略: 补0
    // eg: 512 -> 512 + 2 -> 512 - 3 + 1 -> 512 大小不变
    int padding = 1;
    int padded_width = input_width + 2 * padding;
    int padded_height = input_height + 2 * padding;

    int output_width = padded_width - kernel_width + 1;
    int output_height = padded_height - kernel_height + 1;

    int* padded_input = (int*)malloc(sizeof(int) * padded_width * padded_height);
    unsigned char* output = (unsigned char*)malloc(sizeof(int) * output_width * output_height);

    // padding
    int x, y;
    memset(padded_input, 0, sizeof(int) * padded_width * padded_height);
    for (y = 0; y < input_height; ++y) {
        for (x = 0; x < input_width; ++x) {
            padded_input[(y + padding) * padded_width + (x + padding)] = input[y * input_width + x];
        }
    }

    // Conv2D
    int i, j, m, n;
    for (i = 0; i < output_height; ++i) {
        for (j = 0; j < output_width; ++j) {
            int sum = 0;
            for (m = 0; m < kernel_height; ++m) {
                for (n = 0; n < kernel_width; ++n) {
                    int x = j + n - kernel_width / 2;
                    int y = i + m - kernel_height / 2;

                    // 边界控制
                    if (x >= 0 && x < input_width && y >= 0 && y < input_height) {
                        sum += (int)(input[y * input_width + x] * kernel[m * kernel_width + n]);
                    }
                }
            }

            // 防止溢出
            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;

            output[i * output_width + j] = sum;
        }
    }

    if (show) {
        for (j = 0; j < output_height; ++j) {
            for (i = 0; i < output_width; ++i) {
                col c;
                int idx = j * output_width + i;
                unsigned int gray = output[idx];
                c.l = ((gray >> 3) << 0) | ((gray >> 2) << 5) | ((gray >> 3) << 11);
                draw_point(x0 + i, y0 + j, c);
            }
        }
    }


    return output;
}

// 形态学处理
// OTSU
# define GrayScale 256
unsigned char* OTSU(const unsigned char* input, int input_width, int input_height, 
                        int x0, int y0, int show)
{
    int i, j;
    int threshold = 0;                          // 最佳阈值
    int pixelCount[GrayScale];                  // 每个灰度值所占像素个数
    float pixelPro[GrayScale];                  // 每个灰度值所占像素占总体比例
    int pixelSum = input_height * input_width;  // 像素总个数
    float w0 = 0;                               // 目标像素占总图像比例
    float w1 = 0;                               // 背景像素占总图像比例
    float u0 = 0;                               // 目标平均灰度值
    float u1 = 0;                               // 背景平均灰度值
    float u  = 0;                               // 总平均灰度值
    float u0Sum = 0;                            // 
    float u1Sum = 0;                            // 
    float variance = 0;                         // 类间方差
    float maxVariance = 0;                      // 最大类间方差

    memset(pixelCount, 0, sizeof(int) * GrayScale);
    memset(pixelPro, 0, sizeof(int) * GrayScale);

    // 统计个数和比例
    for (i = 0; i < input_height; ++i) {
        for (j = 0; j < input_width; ++j) {
            pixelCount[input[i * input_width + j]]++;
        }
    }
    for (i = 0; i < GrayScale; ++i){
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
        u += i * pixelPro[i];
    }

    // 二值化
    // 遍历阈值, 确定最大方差 -> 确定最佳阈值
    for (i = 0; i < GrayScale; ++i) {
        for (j = 0; j < GrayScale; ++j) {
            if (j <= i) {
                w0 += pixelPro[j];
                u0Sum += j * pixelPro[j];
            } else {
                w1 += pixelPro[j];
                u1Sum += j * pixelPro[j];
            }
        }
        u0 = u0Sum / w0;
        u1 = u1Sum / w1;
        variance = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);

        if (variance > maxVariance) {
            maxVariance = variance;
            threshold = i;
        }
    }

    // 分割
    unsigned char* output = (unsigned char* )malloc(sizeof(unsigned char) * pixelSum);
    memset(output, 0, pixelSum);
    for (i = 0; i < pixelSum; ++i) {
        if (input[i] > threshold) {
            output[i] = 255;
        }
    }

    // show
    if (show) {
        for (j = 0; j < input_height; ++j) {
            for (i = 0; i < input_width; ++i) {
                col c;
                int idx = j * input_width + i;
                unsigned int gray = output[idx];
                c.l = ((gray >> 3) << 0) | ((gray >> 2) << 5) | ((gray >> 3) << 11);
                draw_point(x0 + i, y0 + input_height - 1 - j, c);
            }
        }
    }


    return output;
}

// canny
unsigned char* canny(const unsigned char* input, int input_width, int input_height, 
                        int x0, int y0, int show) 
{
    

    return NULL;
}

// 腐蚀(全255是255, 只要不是全255, 就置0(腐蚀))
// 只要不全为白色 -> 腐蚀
// 膨胀(全0是0, 只要不是全0, 就置255)
// 只要不全为黑色 -> 膨胀
unsigned char* erosion_dliation(int operation, const unsigned char* input, int input_width, int input_height, 
                                    const int* kernel, int kernel_width, int kernel_height,
                                    int x0, int y0, int show)
{
    int i, j, m, n;
    int kernel_center_x = kernel_width / 2;
    int kernel_center_y = kernel_height / 2;

    unsigned char* output = (unsigned char*)malloc(sizeof(unsigned char) * input_width * input_height);
    for (i = 0; i < input_height * input_width; ++i) {
        output[i] = input[i];
    }

    for (i = 0; i < input_height; ++i) {
        for (j = 0; j < input_width; ++j) {
            int should_operation = 0;

            for (m = 0; m < kernel_height; ++m) {
                for (n = 0; n < kernel_width; ++n) {
                    
                    int y = i + m - kernel_center_y;
                    int x = j + n - kernel_center_x;

                    if (y >= 0 && y < input_height && x >= 0 && x < input_width) {
                        if (kernel[m * kernel_width + n] == 1 &&
                            input[y * input_width + x] != operation) {
                            should_operation = 1;
                            break;
                        }
                    }
                }
                if (should_operation) break;
            }

            if (should_operation) {
                output[i * input_width + j] = 255 - operation;
            }
        }
    }

    // show
    if (show) {
        for (j = 0; j < input_height; ++j) {
            for (i = 0; i < input_width; ++i) {
                col c;
                int idx = j * input_width + i;
                unsigned int gray = output[idx];
                c.l = ((gray >> 3) << 0) | ((gray >> 2) << 5) | ((gray >> 3) << 11);
                draw_point(x0 + i, y0 + input_height - 1 - j, c);
            }
        }
    }


    return output;
}