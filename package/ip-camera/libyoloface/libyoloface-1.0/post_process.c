#include "post_process.h"

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float box_intersection(box a, box b)
{
	float area = 0;
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if (w < 0 || h < 0)
		return 0;
    area = w*h;
    return area;
}

float box_union(box a, box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

float box_iou(box a, box b)
{
    return box_intersection(a, b)/box_union(a, b);
}

int nms_comparator(const void *pa, const void *pb)
{
    sortable_bbox a = *(sortable_bbox *)pa;
    sortable_bbox b = *(sortable_bbox *)pb;
    float diff = a.probs[a.index][b.classId] - b.probs[b.index][b.classId];
    if (diff < 0) return 1;
    else if (diff > 0) return -1;
    return 0;
}

void do_nms_sort(box *boxes, float **probs, int total, int classes, float thresh)
{
    int i, j, k;
    sortable_bbox *s = (sortable_bbox *)calloc(total, sizeof(sortable_bbox));

    for (i = 0; i < total; ++i) {
        s[i].index = i;
        s[i].classId = 0;
        s[i].probs = probs;
    }

    for (k = 0; k < classes; ++k) {
        for (i = 0; i < total; ++i) {
            s[i].classId = k;
        }
        qsort(s, total, sizeof(sortable_bbox), nms_comparator);
        for (i = 0; i < total; ++i)
		{
            if (probs[s[i].index][k] == 0)
				continue;
            for (j = i+1; j < total; ++j) {
                box b = boxes[s[j].index];
                if (box_iou(boxes[s[i].index], b) > thresh) {
                    probs[s[j].index][k] = 0;
                }
            }
        }
    }
    free(s);
}

int max_index(float *a, int n)
{
	int i, max_i = 0;
    float max = a[0];

    if (n <= 0)
		return -1;

    for (i = 1; i < n; ++i)
	{
        if (a[i] > max)
		{
            max = a[i];
            max_i = i;
        }
    }
    return max_i;
}

float colors[6][3] = { {1,0,1}, {0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };

float get_color(int c, int x, int max)
{
    float ratio = ((float)x/max)*5;
    int i = floor(ratio);
    int j = ceil(ratio);
	float r = 0;
    ratio -= i;
    r = (1-ratio) * colors[i][c] + ratio*colors[j][c];
    //printf("%f\n", r);
    return r;
}

void draw_box(unsigned char *data, int x1, int y1, int x2, int y2, float r, float g, float b, int w, int h)
{
    //normalize_image(a);
	unsigned char ur;
	unsigned char ug;
	unsigned char ub;
    int i;
    if (x1 < 0) x1 = 0;
    if (x1 >= w) x1 = w-1;
    if (x2 < 0) x2 = 0;
    if (x2 >= w) x2 = w-1;

    if (y1 < 0) y1 = 0;
    if (y1 >= h) y1 = h-1;
    if (y2 < 0) y2 = 0;
    if (y2 >= h) y2 = h-1;

	ur = (unsigned char) r * 255;
	ug = (unsigned char) g * 255;
	ub = (unsigned char) b * 255;

    for (i = x1; i <= x2; ++i) {
        data[i + y1*w + 0*w*h] = ur;
        data[i + y2*w + 0*w*h] = ur;

        data[i + y1*w + 1*w*h] = ug;
        data[i + y2*w + 1*w*h] = ug;

        data[i + y1*w + 2*w*h] = ub;
        data[i + y2*w + 2*w*h] = ub;
    }
    for (i = y1; i <= y2; ++i) {
        data[x1 + i*w + 0*w*h] = ur;
        data[x2 + i*w + 0*w*h] = ur;

        data[x1 + i*w + 1*w*h] = ug;
        data[x2 + i*w + 1*w*h] = ug;

        data[x1 + i*w + 2*w*h] = ub;
        data[x2 + i*w + 2*w*h] = ub;
    }
}

void draw_box_width(unsigned char *a, int x1, int y1, int x2, int y2, int w, float r, float g, float b, int width, int height)
{
    int i;
    for (i = 0; i < w; ++i) {
        draw_box(a, x1+i, y1+i, x2-i, y2-i, r, g, b, width, height);
    }
}


float logistic_activate(float x) {return 1./(1. + exp(-x));}

box get_region_box(float *x, float *biases, int n, int index, int i, int j, int w, int h)
{
    box b;

    b.x = (i + logistic_activate(x[index + 0])) / w;
    b.y = (j + logistic_activate(x[index + 1])) / h;
    b.w = exp(x[index + 2]) * biases[2*n]   / w;
    b.h = exp(x[index + 3]) * biases[2*n+1] / h;
    return b;
}


static float Fp16toFp32(const short in)
{
    int t1;
    int t2;
    int t3;
    float out;

    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent

    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position

    t1 += 0x38000000;                       // Adjust bias

    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    *((unsigned int*)&out) = t1;

    return out;
}


void flatten(float *x, int size, int layers, int batch, int forward)
{
    float *swap = (float*)calloc(size*layers*batch, sizeof(float));
    int i,c,b;
    for (b = 0; b < batch; ++b) {
        for (c = 0; c < layers; ++c) {
            for (i = 0; i < size; ++i) {
                int i1 = b*layers*size + c*size + i;
                int i2 = b*layers*size + i*layers + c;
                if (forward) swap[i2] = x[i1];
                else swap[i1] = x[i2];
            }
        }
    }
    memcpy(x, swap, size*layers*batch*sizeof(float));
    free(swap);
}


void softmax(float *input, int n, float temp, float *output)
{
    int i;
    float sum = 0;
    float largest = -FLT_MAX;
    for (i = 0; i < n; ++i) {
        if (input[i] > largest) largest = input[i];
    }
    for (i = 0; i < n; ++i) {
        float e = exp(input[i]/temp - largest/temp);
        sum += e;
        output[i] = e;
    }
    for (i = 0; i < n; ++i) {
        output[i] /= sum;
    }
}


