/*
https://github.com/plzombie/depress/issues/2
*/

#ifndef THRESHOLD_H_
#define THRESHOLD_H_
#define THRESHOLD_VERSION "3.1"

#include <stdbool.h>
#include <math.h>

#ifdef THRESHOLD_STATIC
#define THRESHOLDAPI static
#else
#define THRESHOLDAPI extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

THRESHOLDAPI int ImageThreshold(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, unsigned int threshold);
THRESHOLDAPI int ImageThresholdBimod(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, float part, float delta);
THRESHOLDAPI int ImageThresholdSauvola(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, int radius, float sensitivity, float part, int lower_bound, int upper_bound, float delta);
THRESHOLDAPI int ImageThresholdBlur(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, float raduis, float part, float delta, float sensitivity);
THRESHOLDAPI int ImageThresholdEdgePlus(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, float raduis, float part, float delta, float sensitivity);

#ifdef __cplusplus
}
#endif

#ifdef THRESHOLD_IMPLEMENTATION

static void ImageHist(unsigned char* buf, unsigned long long int* histogram, unsigned int width, unsigned int height, unsigned int channels, unsigned int histsize)
{
    unsigned int y, x, d, im;
    unsigned long int k;

    for (im = 0; im < histsize; im++)
    {
        histogram[im] = 0;
    }
    k = 0;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            im = 0;
            for (d = 0; d < channels; d++)
            {
                im += (int)buf[k];
                k++;
            }
            im += (channels >> 1);
            im /= channels;
            im = (im < histsize) ? im : (histsize - 1);
            histogram[im]++;
        }
    }
}

static int HistBiMod (unsigned long long int* histogram, unsigned int histsize, float part)
{
    unsigned int k, T, Tn;
    unsigned long long int im, iw, ib, Tw, Tb;
    int threshold = 0;

    part = (part < 0.0f || part > 1.0f) ? 0.5f : part;
    T = (unsigned)(part * (float)histsize + 0.5f);
    Tn = 0;
    while ( T != Tn )
    {
        Tn = T;
        Tb = Tw = ib = iw = 0;
        for (k = 0; k < T; k++)
        {
            im = histogram[k];
            Tb += (im * k);
            ib += im;
        }
        for (k = T; k < histsize; k++)
        {
            im = histogram[k];
            Tw += (im * k);
            iw += im;
        }
        Tb /= (ib > 1) ? ib : 1;
        Tw /= (iw > 1) ? iw : 1;
        if (iw == 0 && ib == 0)
        {
            T = Tn;
        }
        else if (iw == 0)
        {
            T = (unsigned)Tb;
        }
        else if (ib == 0)
        {
            T = (unsigned)Tw;
        }
        else
        {
            T = (unsigned)(part * (float)Tw + (1.0f - part) * (float)Tb + 0.5f);
        }
    }
    threshold = (int)T;

    return threshold;
}

static void ImageCopy(unsigned char* buf, unsigned char* bufb, unsigned int width, unsigned int height, unsigned int channels)
{
    unsigned int y, x, d;
    unsigned long int k;

    k = 0;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            for (d = 0; d < channels; d++)
            {
                bufb[k] = buf[k];
                k++;
            }
        }
    }
}

static void GaussLineMatrix (float *cmatrix, float radius)
{
    float std_dev;
    float sum, t, tt, step;
    int i, j, iradius, n = 50;

    if (radius < 0)
    {
        radius = -radius;
    }
    std_dev = radius * radius;
    step = 1.0f / (float)n;
    iradius = (int)(2.0f * radius + 0.5f) + 1;
    if (iradius > 1)
    {
        for (i = 0; i < iradius; i++)
        {
            sum = 0;
            for (j = 0; j <= n; j++)
            {
                t = (float)i;
                t -= 0.5f;
                t += step * j;
                tt = -(t * t) / (2.0f * std_dev);
                sum += (float)(exp(tt));
            }
            cmatrix[i] = sum * step;
        }
        sum = cmatrix[0];
        for (i = 1; i < iradius; i++)
        {
            sum += 2.0f * cmatrix[i];
        }
        for (i = 0; i < iradius; i++)
        {
            cmatrix[i] = cmatrix[i] / sum;
        }
    }
    else
    {
        cmatrix[0] = 1.0f;
    }
}

static float GaussBlurFilterY (unsigned char *src, unsigned int height, unsigned int width, unsigned int channels, float radius)
{
    int iradius, y, x, yp, yn, i, dval;
    unsigned long int k, kp, kn, line = width * channels;
    unsigned int d;
    float imc, sc, gaussval = 0.0f;
    float *gaussmat;
    unsigned char *temp = NULL;

    if (radius < 0)
    {
        radius = -radius;
    }
    iradius = (int)(2.0f * radius + 0.5f) + 1;

    if (!(gaussmat = (float*)malloc((iradius) * sizeof(float))))
    {
        return 0.0f;
    }
    if (!(temp = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char))))
    {
        return 0.0f;
    }

    GaussLineMatrix (gaussmat, radius);

    if (iradius > 1)
    {
        k = 0;
        for (y = 0; y < (int)height; y++)
        {
            for (x = 0; x < (int)width; x++)
            {
                for (d = 0; d < channels; d++)
                {
                    kp = k;
                    kn = k;
                    imc = (float)src[k + d];
                    sc = imc * gaussmat[0];
                    for (i = 1; i < iradius; i++)
                    {
                        yp = y - i;
                        if (yp < 0)
                        {
                            yp = 0;
                        }
                        else
                        {
                            kp -= line;
                        }
                        yn = y + i;
                        if (yn < (int)height)
                        {
                            kn += line;
                        }
                        else
                        {
                            yn = 0;
                        }
                        imc = (float)src[kp + d];
                        sc += imc * gaussmat[i];
                        imc = (float)src[kn + d];
                        sc += imc * gaussmat[i];
                    }
                    sc += 0.5f;
                    sc = (sc < 0.0f) ? 0.0f : (sc < 255.0f) ? sc : 255.0f;
                    temp[k + d] = (unsigned char)sc;
                }
                k += channels;
            }
        }
        k = 0;
        for (y = 0; y < (int)height; y++)
        {
            for (x = 0; x < (int)width; x++)
            {
                for (d = 0; d < channels; d++)
                {
                    dval = (int)src[k] - (int)temp[k];
                    gaussval += (dval < 0) ? -dval : dval;
                    src[k] = temp[k];
                    k++;
                }
            }
        }
        gaussval /= (float)k;
    }

    free(temp);
    free(gaussmat);

    return gaussval;
}

static float GaussBlurFilterX (unsigned char *src, unsigned int height, unsigned int width, unsigned int channels, float radius)
{
    int iradius, y, x, xp, xn, i, dval;
    unsigned long int k, kp, kn;
    unsigned int d;
    float imc, sc, gaussval = 0.0f;
    float *gaussmat;
    unsigned char *temp = NULL;

    if (radius < 0)
    {
        radius = -radius;
    }
    iradius = (int)(2.0f * radius + 0.5f) + 1;

    if (!(gaussmat = (float*)malloc((iradius) * sizeof(float))))
    {
        return 0.0f;
    }
    if (!(temp = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char))))
    {
        return 0.0f;
    }

    GaussLineMatrix (gaussmat, radius);

    if (iradius > 1)
    {
        k = 0;
        for (y = 0; y < (int)height; y++)
        {
            for (x = 0; x < (int)width; x++)
            {
                for (d = 0; d < channels; d++)
                {
                    kp = k;
                    kn = k;
                    imc = (float)src[k + d];
                    sc = imc * gaussmat[0];
                    for (i = 1; i < iradius; i++)
                    {
                        xp = x - i;
                        if (xp < 0)
                        {
                            xp = 0;
                        }
                        else
                        {
                            kp -= channels;
                        }
                        xn = x + i;
                        if (xn < (int)width)
                        {
                            kn += channels;
                        }
                        else
                        {
                            xn = 0;
                        }
                        imc = (float)src[kp + d];
                        sc += imc * gaussmat[i];
                        imc = (float)src[kn + d];
                        sc += imc * gaussmat[i];
                    }
                    sc += 0.5f;
                    sc = (sc < 0.0f) ? 0.0f : (sc < 255.0f) ? sc : 255.0f;
                    temp[k + d] = (unsigned char)sc;
                }
                k += channels;
            }
        }
        k = 0;
        for (y = 0; y < (int)height; y++)
        {
            for (x = 0; x < (int)width; x++)
            {
                for (d = 0; d < channels; d++)
                {
                    dval = (int)src[k] - (int)temp[k];
                    gaussval += (dval < 0) ? -dval : dval;
                    src[k] = temp[k];
                    k++;
                }
            }
        }
        gaussval /= (float)k;
    }

    free(temp);
    free(gaussmat);

    return gaussval;
}

static float GaussBlurFilter(unsigned char *src, unsigned int width, unsigned int height, unsigned int channels, float radiusy, float radiusx)
{
    float gaussval = 0.0f;
    gaussval = GaussBlurFilterY(src, height, width, channels, radiusy);
    gaussval += GaussBlurFilterX(src, height, width, channels, radiusx);
    gaussval /= 255.0f;

    return gaussval;
}

static void ImageMathDivide(unsigned char *buf, unsigned char *div, unsigned char *res, unsigned int width, unsigned int height, unsigned int channels, float delta)
{
    unsigned int y, x, d;
    unsigned long int k;
    float im, imm;

    k = 0;
    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            for (d = 0; d < channels; d++)
            {
                im = (float)buf[k];
                im++;
                imm = (float)div[k];
                imm++;
                im /= imm;
                im *= 256.0f;
                im += delta;
                im -= 0.5f;
                im = (im < 0.0f) ? 0.0f : (im < 255.0f) ? im : 255.0f;
                res[k] = (unsigned char)im;
                k++;
            }
        }
    }
}

/*
ImageThreshold()

input:
buf - unsigned char* image (height * width * channels)
threshold - threshold value

output:
bufmask - bool* image mask (height * width)
threshold - threshold value

Use:
int threshold = ImageThreshold(buf, bufmask, width, height, channels, threshold);
*/

THRESHOLDAPI int ImageThreshold(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, unsigned int threshold)
{
    unsigned int y, x, d, im;
    unsigned long int k, km;

    k = 0;
    km = 0;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            im = 0;
            for (d = 0; d < channels; d++)
            {
                im += (unsigned int)buf[k];
                k++;
            }
            im += (channels >> 1);
            im /= channels;
            bufmask[km] = (im < threshold);
            km++;
        }
    }

    return threshold;
}

/*
ImageThresholdBimod()

input:
buf - unsigned char* image (height * width * channels)
part = 1.0f [1:1]
delta = 0.0f [off, regulator]

output:
bufmask - bool* image mask (height * width)
threshold - threshold value

Use:
int threshold = ImageThresholdBimod(buf, bufmask, width, height, channels, part, delta);
*/

THRESHOLDAPI int ImageThresholdBimod(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, float part, float delta)
{
    unsigned int Tmax = 256;
    int threshold = 0;
    unsigned long long int histogram[256] = {0};

    ImageHist(buf, histogram, width, height, channels, Tmax);
    part *= 0.5f;
    threshold = HistBiMod (histogram, Tmax, part);
    threshold += (int)delta;
    threshold = ImageThreshold(buf, bufmask, width, height, channels, threshold);

    return threshold;
}

/*
ImageThresholdSauvola()

input:
buf - unsigned char* image (height * width * channels)
radius = 3
part = 1.0f [1:1]
sensitivity = 0.2;
lower_bound = 0;
upper_bound = 255;
delta = 0.0f [off, regulator]

output:
bufmask - bool* image mask (height * width)
threshold - threshold value

Use:
int threshold = ImageThresholdSauvola(buf, bufmask, width, height, channels, int radius, sensitivity, part, lower_bound, upper_bound, delta);
*/

THRESHOLDAPI int ImageThresholdSauvola(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, int radius, float sensitivity, float part, int lower_bound, int upper_bound, float delta)
{
    unsigned int x, y, d, y1, x1, y2, x2, yf, xf;
    float imx, imm, imv, ima, t, st = 0, sn = 0;
    float dynamic_range;
    int threshold = 0;
    unsigned long int k, kf, km, n;

    dynamic_range = (part > 0.0f) ? (127.5f * part * channels) : 1.0f;

    if (radius < 0)
    {
        radius = -radius;
    }
    if (upper_bound < lower_bound)
    {
        upper_bound += lower_bound;
        lower_bound = upper_bound - lower_bound;
        upper_bound -= lower_bound;
    }
    lower_bound *= channels;
    upper_bound *= channels;

    k = 0;
    km = 0;
    for (y = 0; y < height; y++)
    {
        y1 = (y < (unsigned int)radius) ? 0 : (y - (unsigned int)radius);
        y2 = (y + radius + 1 < height) ? (y + radius + 1) : height;
        for (x = 0; x < width; x++)
        {
            x1 = (x < (unsigned int)radius) ? 0 : (x - (unsigned int)radius);
            x2 = (x + radius + 1 < width) ? (x + radius + 1) : width;
            imm = 0;
            imv = 0;
            n = 0;
            for (yf = y1; yf < y2; yf++)
            {
                for (xf = x1; xf < x2; xf++)
                {
                    kf = (yf * width + xf) * channels;
                    imx = 0.0f;
                    for (d = 0; d < channels; d++)
                    {
                        imx += (float)buf[kf + d];
                    }
                    imm += imx;
                    imv += (imx * imx);
                    n++;
                }
            }
            n = (n > 0) ? n : 1;
            imm /= n;
            imv /= n;
            imv -= (imm * imm);
            imv = (imv < 0) ? -imv : imv;
            imv = (float)(sqrt(imv));
            ima = 1.0f - imv / dynamic_range;
            imx = 0.0f;
            for (d = 0; d < channels; d++)
            {
                imx += (float)buf[k];
                k++;
            }
            if (imx < (float)lower_bound)
            {
                t = (float)lower_bound;
            }
            else if (imx > (float)upper_bound)
            {
                t = (float)upper_bound;
            }
            else
            {
                t = imm * (1.0f - sensitivity * ima) + delta;
            }
            bufmask[km] = (imx < t);
            km++;
            st += t;
            sn++;
        }
    }
    sn = (sn > 0.0f) ? sn : 1.0f;
    threshold = (int)(st / sn + 0.5f);

    return threshold;
}

/*
ImageThresholdBlur()

input:
buf - unsigned char* image (height * width * channels)
part = 1.0f [1:1]
delta = 0.0f [off, regulator]
sensitivity = 0.2;

output:
bufmask - bool* image mask (height * width)
threshold - threshold value

Use:
int threshold = ImageThresholdBlur(buf, bufmask, width, height, channels, raduis, part, delta, sensitivity);
*/

THRESHOLDAPI int ImageThresholdBlur(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, float raduis, float part, float delta, float sensitivity)
{
    int threshold = 0;
    unsigned int y, x, d;
    float imx;
    unsigned long int k;
    unsigned char *bufb = NULL;

    if ((bufb = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char))))
    {
        ImageCopy(buf, bufb, width, height, channels);
        (void)GaussBlurFilter(bufb, width, height, channels, raduis, raduis);
        ImageMathDivide(bufb, buf, bufb, width, height, channels, -127.0f);
        sensitivity += 1.0f;
        k = 0;
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                for (d = 0; d < channels; d++)
                {
                    imx = (float)bufb[k];
                    imx -= 255.0f;
                    imx *= sensitivity;
                    imx += 255.0f;
                    imx = (imx < 0.0f) ? 0.0f : (imx < 255.0f) ? imx : 255.0f;
                    bufb[k] = (unsigned char)imx;
                    k++;
                }
            }
        }
        ImageMathDivide(buf, bufb, bufb, width, height, channels, -127.0f);
        threshold = ImageThresholdBimod(bufb, bufmask, width, height, channels, part, delta);
        free(bufb);
    }
    else
    {
        threshold = ImageThresholdBimod(buf, bufmask, width, height, channels, part, delta);
    }

    return threshold;
}

/*
ImageThresholdEdgePlus()

input:
buf - unsigned char* image (height * width * channels)
part = 1.0f [1:1]
delta = 0.0f [off, regulator]
sensitivity = 0.2;

output:
bufmask - bool* image mask (height * width)
threshold - threshold value

Use:
int threshold = ImageThresholdEdgePlus(buf, bufmask, width, height, channels, raduis, part, delta, sensitivity);
*/

THRESHOLDAPI int ImageThresholdEdgePlus(unsigned char* buf, bool* bufmask, unsigned int width, unsigned int height, unsigned int channels, float raduis, float part, float delta, float sensitivity)
{
    int threshold = 0;
    unsigned int y, x, d;
    float imo, imx, edge, edgeplus;
    unsigned long int k;
    unsigned char *bufb = NULL;

    if ((bufb = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char))))
    {
        ImageCopy(buf, bufb, width, height, channels);
        (void)GaussBlurFilter(bufb, width, height, channels, raduis, raduis);
        k = 0;
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                for (d = 0; d < channels; d++)
                {
                    imo = (float)buf[k];
                    imx = (float)bufb[k];
                    edge = (imo + 1.0f) / (imx + 1.0f) - 0.5f;
                    edgeplus = imo * edge;
                    imx = sensitivity * edgeplus + (1.0f - sensitivity) * imo;
                    imx = (imx < 0.0f) ? 0.0f : (imx < 255.0f) ? imx : 255.0f;
                    bufb[k] = (unsigned char)imx;
                    k++;
                }
            }
        }
        threshold = ImageThresholdBimod(bufb, bufmask, width, height, channels, part, delta);
        free(bufb);
    }
    else
    {
        threshold = ImageThresholdBimod(buf, bufmask, width, height, channels, part, delta);
    }

    return threshold;
}

#endif /* THRESHOLD_IMPLEMENTATION */

#endif /* THRESHOLD_H_ */
