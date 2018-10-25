#define MAX_DETECT_NUM 100

typedef struct _DetectPoint {
  int left;
  int top;
  int right;
  int bottom;
} DetectPoint;

typedef struct _DetectResult {
   int  detect_num;
   DetectPoint pt[MAX_DETECT_NUM];
} DetectResult;

int yoloface_init(const char *model_path);
int yoloface_process(const char *buf, int width, int height);
void yoloface_deinit();
DetectResult *yoloface_get_detection_result();
