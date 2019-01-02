#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <directfb.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_api.h>
#include <VX/vx_khr_cnn.h>
#include "post_process.h"
#include "vxc_util.h"
#include "vxc_nn_dynamic_fixed_point_8.h"
#include "vx_utility.h"
#include "libyoloface.h"

#define USE_ASYNC_PROCESS

#ifdef USE_ASYNC_PROCESS
#include <pthread.h>

typedef struct _process_buf_info {
  vx_int8 *buf;
  vx_int32 width;
  vx_int32 height;
} PROCESS_BUF_INFO_t;

static pthread_mutex_t gs_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#define NN_TENSOR_MAX_DIMENSION_NUMBER 4

static DetectResult gDetectResult;
static DetectResult gDetectResult_output;
static vx_tensor                   input = NULL;
static vx_tensor_addressing        inputs_tensor_addressing = NULL;

//static vx_node node;
static vx_context context;
static vx_graph graph;
static vx_bool nn_created = vx_false_e;
static tensors_info_t  outputs_info;

#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }

static IDirectFB *dfb = NULL;
static IDirectFBSurface *primary_surface = NULL;

static char *
dfb_resize(char *buf, int w, int h) {
  IDirectFBSurface *input_surface = NULL;
  DFBSurfaceDescription sdsc;
  DFBSurfaceDescription ddsc;
  DFBRectangle dstrect;
  const char* ddscbuf = NULL;
  char* outbuf = NULL;
  int outpitch = 0;

  if (dfb == NULL) {
    DFBCHECK(DirectFBInit(NULL, NULL));
    DFBCHECK(DirectFBCreate(&dfb));
  }

  sdsc.width = w;
  sdsc.height = h;
  sdsc.flags = DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT;
  sdsc.caps = DSCAPS_NONE;
  sdsc.pixelformat = DSPF_RGB24;
  sdsc.preallocated[0].data = buf;
  sdsc.preallocated[0].pitch = w * 3;
  sdsc.preallocated[1].data = NULL;
  sdsc.preallocated[1].pitch = 0;
  DFBCHECK(dfb->CreateSurface(dfb, &sdsc, &input_surface));

  if (primary_surface == NULL) {
    ddsc.width = NN_INPUT_WIDTH;
    ddsc.height = NN_INPUT_HEIGHT;
    ddsc.flags = DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PIXELFORMAT;
    ddsc.caps = DSCAPS_NONE;
    ddsc.pixelformat = DSPF_RGB24;
    DFBCHECK(dfb->CreateSurface(dfb, &ddsc, &primary_surface));
  }


  dstrect.x = 0; dstrect.y = 0;
  dstrect.w = NN_INPUT_WIDTH;
  dstrect.h = NN_INPUT_HEIGHT;

  DFBCHECK(primary_surface->StretchBlit(primary_surface,
        input_surface, NULL, &dstrect));

  DFBCHECK(primary_surface->Lock(primary_surface, DSLF_READ, (void **)&ddscbuf, &outpitch));

  outbuf = (char *) malloc (NN_INPUT_WIDTH * NN_INPUT_HEIGHT * 3);
  if (outpitch == NN_INPUT_WIDTH * 3) {
    memcpy(outbuf, ddscbuf, outpitch * NN_INPUT_HEIGHT);
  } else {
    const char *src = ddscbuf;
    char *dst = outbuf;
    for (int h = 0; h < NN_INPUT_HEIGHT; h++) {
      memcpy(dst, src, NN_INPUT_WIDTH * 3);
      src += outpitch;
      dst += (NN_INPUT_WIDTH * 3);
    }
  }

  primary_surface->Unlock(primary_surface);


  DFBCHECK(input_surface->Release(input_surface));

  return outbuf;
}

static void dfb_resize_deinit() {
  if (primary_surface) {
    DFBCHECK(primary_surface->Release(primary_surface));
    primary_surface = NULL;
  }
  if (dfb) {
    DFBCHECK(dfb->Release(dfb));
    dfb = NULL;
  }
}

static void generate_detections_results(int num, float thresh, box *boxes, float **probs,  int classes,int width, int height)
{
  int i, detect_num = 0;

#ifdef USE_ASYNC_PROCESS
  pthread_mutex_lock(&gs_mutex);
#endif
  for (i = 0; i < num; ++i)
  {
    int classId = max_index(probs[i], classes);
    float prob = probs[i][classId];
    if (prob > thresh)
    {
      int left  = (boxes[i].x-boxes[i].w/2.)*width;
      int right = (boxes[i].x+boxes[i].w/2.)*width;
      int top   = (boxes[i].y-boxes[i].h/2.)*height;
      int bottom   = (boxes[i].y+boxes[i].h/2.)*height;

      if (left < 0) left = 0;
      if (right > width-1) right = width-1;
      if (top < 0) top = 0;
      if (bottom > height-1) bottom = height-1;

      gDetectResult.pt[detect_num].left = left;
      gDetectResult.pt[detect_num].top = top;
      gDetectResult.pt[detect_num].right = right;
      gDetectResult.pt[detect_num].bottom = bottom;
      detect_num ++;
    }
    if (detect_num >= MAX_DETECT_NUM) {
      break;
    }
  }
  gDetectResult.detect_num= detect_num ;
#ifdef USE_ASYNC_PROCESS
  pthread_mutex_unlock(&gs_mutex);
#endif
}

static int yolo_v2_post_process(float *predictions, int width, int height, int modelWidth, int modelHeight, int input_num, int *input_size )
{
  int i,j,n;
  float threshold = 0.24;
  int num_class = 1;
  int num_box = 5;
  int grid_size = 13;
  float biases[10] = {1.08,1.19,  3.42,4.41,  6.63,11.38,  9.42,5.11,  16.62,10.52};

  box *boxes = (box *)calloc(modelWidth*modelHeight*num_box, sizeof(box));
  float **probs = (float **)calloc(modelWidth*modelHeight*num_box, sizeof(float *));

  for (j = 0; j < modelWidth*modelHeight*num_box; ++j)
    probs[j] = (float *)calloc(num_class+1, sizeof(float *));

  {
    int i,b;
    int coords = 4,classes = 1;
    int size = coords + classes + 1;
    int w = input_size[0];
    int h = input_size[1];
    int n = input_size[2]/size;
    int batch = 1;
    flatten(predictions, w*h, size*n, batch, 1);

    for (b = 0; b < batch; ++b) {
      for (i = 0; i < h*w*n; ++i) {
        int index = size*i + b*input_num;
        predictions[index + 4] = logistic_activate(predictions[index + 4]);
      }
    }

    for (b = 0; b < batch; ++b)
    {
      for (i = 0; i < h*w*n; ++i)
      {
        int index = size*i + b*input_num;
        softmax(predictions + index + 5, classes, 1, predictions + index + 5);
      }
    }
  }


  for (i = 0; i < modelWidth*modelHeight; ++i)
  {
    int row = i / modelWidth;
    int col = i % modelWidth;
    for (n = 0; n < num_box; ++n)
    {
      int index = i*num_box + n;
      int p_index = index * (num_class + 5) + 4;
      float scale = predictions[p_index];
      int box_index = index * (num_class + 5);
      int class_index = 0;
      boxes[index] = get_region_box(predictions, biases, n, box_index, col, row, modelWidth, modelHeight);

      class_index = index * (num_class + 5) + 5;

      for (j = 0; j < num_class; ++j)
      {
        float prob = scale*predictions[class_index+j];
        probs[index][j] = (prob > threshold) ? prob : 0;
      }

    }
  }

  do_nms_sort(boxes, probs, grid_size*grid_size*num_box, num_class, 0.4);

  generate_detections_results(grid_size*grid_size*num_box, threshold, boxes, probs,  num_class,width, height);

  free(boxes);
  boxes = NULL;


  for (j = 0; j < grid_size*grid_size*num_box; ++j)
    free(probs[j]);
  free(probs);

  return 0;
}

static void vx_destory()
{
  dfb_resize_deinit();
  if (nn_created == vx_true_e)
  {
    vxcReleaseNeuralNetwork();
  }

  if (input != NULL)
  {
    vxReleaseTensor(&input);
  }

  if (graph != NULL)
  {
    vxReleaseGraph(&graph);
  }

  if (context != NULL)
  {
    vxReleaseContext(&context);
  }
}

static vx_status vx_setup(char *model_name)
{
  vx_status status = VX_FAILURE;
  vx_uint32                   num_of_dims;
  vx_uint32                   image_size[NN_TENSOR_MAX_DIMENSION_NUMBER] = {0, 0, 0, 0};
  vx_uint32                   image_stride_size[NN_TENSOR_MAX_DIMENSION_NUMBER] = {0, 0, 0, 0};
  vx_enum                     data_format;
  vx_tensor_create_params_t   tensor_create_params;

  int                         i;
  /*============= Create VX context and build VX Alexnet graph ============ */
  context = vxCreateContext();
  _CHECK_OBJ(context, exit);
  graph = vxCreateGraph(context);
  _CHECK_OBJ(graph, exit);
  /* Prepare input tensor */
  num_of_dims    = NN_INPUT_DIMENSION_NUMBER;
  image_size[0]  = NN_INPUT_WIDTH;
  image_size[1]  = NN_INPUT_HEIGHT;
  image_size[2]  = NN_INPUT_CHANNEL;
  if (num_of_dims > 3) image_size[3]  = 1;
  data_format    = NN_INPUT_DATA_FORMAT;

  tensor_create_params.num_of_dims = num_of_dims;
  tensor_create_params.sizes = (vx_int32 *)image_size;
  tensor_create_params.data_format = data_format;
#if defined(NN_TENSOR_DATA_FORMAT_INT8) || defined(NN_TENSOR_DATA_FORMAT_INT16)
  tensor_create_params.quant_format = NN_INPUT_QUANT_TYPE;
  tensor_create_params.quant_data.dfp.fixed_point_pos = NN_INPUT_FIXED_POINT_POS;
#elif defined(NN_TENSOR_DATA_FORMAT_UINT8)
  tensor_create_params.quant_format = NN_INPUT_QUANT_TYPE;
  tensor_create_params.quant_data.affine.scale = NN_INPUT_AFFINE_SCALE;
  tensor_create_params.quant_data.affine.zeroPoint = NN_INPUT_AFFINE_ZERO_POINT;
#endif
  input = vxCreateTensor2(context, (const vx_tensor_create_params_t*)&tensor_create_params, sizeof(tensor_create_params));
  _CHECK_OBJ(input, exit);

  image_stride_size[0] = vxcGetTypeSize(data_format);
  for (i = 1; i < NN_INPUT_DIMENSION_NUMBER; i++)
  {
    image_stride_size[i] = image_stride_size[i-1] * image_size[i-1];
  }
  inputs_tensor_addressing       = vxCreateTensorAddressing(context, image_size, image_stride_size, NN_INPUT_DIMENSION_NUMBER);
  _CHECK_OBJ(inputs_tensor_addressing, exit);

  status = vxcCreateNeuralNetwork(graph, model_name, input, &outputs_info);
  _CHECK_STATUS(status, exit);
  nn_created = vx_true_e;

  status = vxVerifyGraph(graph);
  _CHECK_STATUS(status, exit);

  return status;;
exit:
  vx_destory();
  return status;
}

#ifdef USE_ASYNC_PROCESS
void *vx_image_process(void* param)
#else
static vx_status vx_image_process(vx_uint8 * src,int width,int height)
#endif
{
  vx_status status = VX_FAILURE;
  vx_uint32                   output_size[NN_TENSOR_MAX_DIMENSION_NUMBER];
  vx_uint32                   output_stride_size[NN_TENSOR_MAX_DIMENSION_NUMBER];
  vx_int8 *input_data_ptr = NULL;
  vx_int32 tmpdata; //zxw
  vx_uint32                   num_of_dims = 3;
  int                         size;
  vx_enum                     data_format;
  vx_enum                     quant_format;
  vx_int32                    zero_point;
  vx_float32                  scale;
  vx_uint8                    fix_point_pos;
  int channels = NN_INPUT_CHANNEL;
  int i=0, j=0, offset=0;
  vx_tensor                   output = NULL;
  void*                       output_ptr = NULL;
  vx_tensor_addressing        output_user_addr = NULL;
  //unsigned char *orgPixel = NULL;
  vx_float32 * outBuf = NULL;

#ifdef USE_ASYNC_PROCESS
  PROCESS_BUF_INFO_t *buf_info = (PROCESS_BUF_INFO_t *)param;
  vx_uint8* src = buf_info->buf;
  vx_int32 width = buf_info->width;
  vx_int32 height = buf_info->height;
#else
  src = dfb_resize(src, width, height);
#endif

  data_format    = NN_INPUT_DATA_FORMAT;
  if (input_data_ptr == NULL)
    input_data_ptr = (vx_int8*) malloc(NN_INPUT_WIDTH * NN_INPUT_HEIGHT * channels * sizeof(vx_int8));

  _CHECK_OBJ(input_data_ptr, process_exit);

  for (i = 0; i < channels; i++)
  {
    offset = NN_INPUT_WIDTH * NN_INPUT_HEIGHT *( channels -1 - i);  // prapare BGR input data
    for (j = 0; j < NN_INPUT_WIDTH * NN_INPUT_HEIGHT; j++)
    {
      tmpdata = (src[j * channels + i]>>1);
      input_data_ptr[j  + offset] = (vx_int8)((tmpdata >  127) ? 127 : (tmpdata < -128) ? -128 : tmpdata);
    }
  }

  free(src);

  status = vxCopyTensorPatch(input, NULL, inputs_tensor_addressing, input_data_ptr, VX_WRITE_ONLY, 0);  //input_data_ptr to src
  _CHECK_STATUS(status, process_exit);

  // do some clean up

  status = vxProcessGraph(graph);
  _CHECK_STATUS(status, process_exit);

  for (j=0; j < outputs_info.size;j++)
  {
    /* process result */
    output = outputs_info.tensors[j];
    status = vxQueryTensor(output, VX_TENSOR_NUM_OF_DIMS, &num_of_dims, sizeof(num_of_dims));
    _CHECK_STATUS(status, process_exit);

    status = vxQueryTensor(output, VX_TENSOR_DIMS, output_size, sizeof(output_size));
    _CHECK_STATUS(status, process_exit);

    status = vxQueryTensor(output, VX_TENSOR_DATA_TYPE, &data_format, sizeof(data_format));
    _CHECK_STATUS(status, process_exit);
    status = vxQueryTensor(output, VX_TENSOR_FIXED_POINT_POS, &fix_point_pos, sizeof(fix_point_pos));
    _CHECK_STATUS(status, process_exit);
    status = vxQueryTensor(output, VX_TENSOR_QUANT_FORMAT, &quant_format, sizeof(quant_format));
    status = vxQueryTensor(output, VX_TENSOR_ZERO_POINT, &zero_point, sizeof(zero_point));
    status = vxQueryTensor(output, VX_TENSOR_SCALE, &scale, sizeof(scale));

    output_stride_size[0] = vxcGetTypeSize(data_format);
    size = output_stride_size[0] * output_size[0];
    for (i = 1; i < (int)num_of_dims; i++)
    {
      output_stride_size[i] = output_stride_size[i-1] * output_size[i-1];
      size *= output_size[i];
    }
    output_ptr = malloc(size);

    _CHECK_OBJ(output_ptr, process_exit);

    output_user_addr = vxCreateTensorAddressing(
        context,
        &output_size[0],
        &output_stride_size[0],
        num_of_dims
        );
    _CHECK_OBJ(output_user_addr, process_exit);

    status = vxCopyTensorPatch(
        output,
        NULL,
        output_user_addr,
        output_ptr,
        VX_READ_ONLY,
        0
        );
    _CHECK_STATUS(status, process_exit);

    outBuf = showResult(output_ptr, size / output_stride_size[0],j,num_of_dims,output_size,data_format, quant_format,fix_point_pos, zero_point, scale, outputs_info.size);
    yolo_v2_post_process(outBuf,width,height,13,13,size/output_stride_size[0], (int *)output_size);
  }

process_exit:
  if (outBuf != NULL)
  {
    free(outBuf);
    outBuf = NULL;
  }
  if (input_data_ptr != NULL)
  {
    free(input_data_ptr);
    input_data_ptr = NULL;
  }

  if (output_ptr != NULL)
  {
    free(output_ptr);
    output_ptr = NULL;
  }

  if (output_user_addr)
  {
    vxReleaseTensorAddressing(&output_user_addr);
  }

#ifdef USE_ASYNC_PROCESS
  buf_info->buf = NULL;
  return NULL;
#else
  return status;
#endif
}

#define DEFAULT_MODULE_PATH "/etc/yoloface_model.dat"

int yoloface_init(const char *model_path) {
  gDetectResult.detect_num = 0;
  if (model_path == NULL) {
    model_path = DEFAULT_MODULE_PATH;
  }
  return vx_setup((char *)model_path) == VX_SUCCESS;
}

PROCESS_BUF_INFO_t process_buf = {0};

int yoloface_process(const char *buf, int width, int height) {
#ifdef USE_ASYNC_PROCESS
  if (process_buf.buf != NULL) {
    // previous process not finished
    return 1;
  }

  process_buf.buf = (vx_int8 *)dfb_resize((char *)buf, width, height);
  process_buf.width = width;
  process_buf.height = height;

  pthread_t tid;
  pthread_attr_t attr;
  int rc = pthread_attr_init(&attr);
  rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  rc = pthread_create(&tid, &attr, vx_image_process, (void*)&process_buf);
  return 1;
#else
  return vx_image_process(buf, width, height) == VX_SUCCESS;
#endif
}

void yoloface_deinit() {
  vx_destory();
}

DetectResult *yoloface_get_detection_result() {
#ifdef USE_ASYNC_PROCESS
  pthread_mutex_lock(&gs_mutex);
  memcpy(&gDetectResult_output, &gDetectResult, sizeof(gDetectResult));
  pthread_mutex_unlock(&gs_mutex);
  return &gDetectResult_output;
#else
  return &gDetectResult;
#endif
}

