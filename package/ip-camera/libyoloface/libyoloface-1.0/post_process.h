#ifndef  POST_PROCESS_H_
#define  POST_PROCESS_H_

#ifdef _cplusplus
extern "C" {
#endif

typedef struct{
    float x, y, w, h;
} box;


typedef struct{
    int index;
    int classId;
    float **probs;
} sortable_bbox;

box get_region_box(float *x, float *biases, int n, int index, int i, int j, int w, int h);
void do_nms_sort(box *boxes, float **probs, int total, int classes, float thresh);
int max_index(float *a, int n);
void flatten(float *x, int size, int layers, int batch, int forward);
float logistic_activate(float x);
void softmax(float *input, int n, float temp, float *output);
#ifdef _cplusplus
}
#endif
#endif

