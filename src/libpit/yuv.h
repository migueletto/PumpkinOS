#ifndef PIT_YUV_H
#define PIT_YUV_H

#ifdef __cplusplus
extern "C" {
#endif

int i420_yuyv(unsigned char *i420, int i420_len, unsigned char *yuyv, int width);

void i420_gray(unsigned char *i420, int i420_len, unsigned char *gray);

int yuyv_i420(unsigned char *yuyv, int yuyv_len, unsigned char *i420, int width);

void yuyv_gray(unsigned char *yuyv, int yuyv_len, unsigned char *gray);

void yuyv_rgb(unsigned char *yuyv, int yuyv_len, unsigned char *rgb);

void yuyv_rgba(unsigned char *yuyv, int yuyv_len, unsigned char *rgba);

void yuyv_rgb565(unsigned char *yuyv, int yuyv_len, unsigned char *rgb);

void uyvy_yuyv(unsigned char *uyvy, int uyvy_len, unsigned char *yuyv);

void rgb_yuyv(unsigned char *rgb, int rgb_len, unsigned char *yuyv);

void rgb_gray(unsigned char *rgb, int rgb_len, unsigned char *gray);

void bgra_rgba(unsigned char *bgra, int rgba_len, unsigned char *rgba);

void bgra_rgb(unsigned char *bgra, int bgra_len, unsigned char *rgb);

void rgb_rgba(unsigned char *rgb, int rgb_len, unsigned char *rgba);

void gray_yuyv(unsigned char *gray, int gray_len, unsigned char *yuyv);

void gray_rgb(unsigned char *gray, int gray_len, unsigned char *rgb);

void gray_rgba(unsigned char *gray, int gray_len, unsigned char *rgba);

void rgba_yuyv(unsigned char *rgba, int rgba_len, unsigned char *yuyv);

void rgba_gray(unsigned char *rgba, int rgba_len, unsigned char *gray);

void rgba_rgb(unsigned char *rgba, int rgba_len, unsigned char *rgb);

void desaturate_i420(unsigned char *buf, int len);

void desaturate_yuyv(unsigned char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
