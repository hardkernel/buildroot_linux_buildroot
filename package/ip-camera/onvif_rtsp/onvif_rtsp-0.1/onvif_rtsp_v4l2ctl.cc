#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>

#define ISP_V4L2_CID_BASE               (0x00f00000 | 0xf000)
#define ISP_V4L2_CID_TEST_PATTERN       (ISP_V4L2_CID_BASE + 0)
#define ISP_V4L2_CID_TEST_PATTERN_TYPE  (ISP_V4L2_CID_BASE + 1)
#define ISP_V4L2_CID_AF_REFOCUS         (ISP_V4L2_CID_BASE + 2)
#define ISP_V4L2_CID_SENSOR_PRESET       (ISP_V4L2_CID_BASE + 3)
#define ISP_V4L2_CID_CUSTOM_SENSOR_WDR_MODE ( ISP_V4L2_CID_BASE + 7 )
#define ISP_V4L2_CID_CUSTOM_SENSOR_EXPOSURE ( ISP_V4L2_CID_BASE + 8 )
#define ISP_V4L2_CID_CUSTOM_SET_FR_FPS ( ISP_V4L2_CID_BASE + 9 )
#define ISP_V4L2_CID_CUSTOM_SET_SENSOR_TESTPATTERN (ISP_V4L2_CID_BASE + 10)
#define ISP_V4L2_CID_CUSTOM_SENSOR_IR_CUT ( ISP_V4L2_CID_BASE + 11 )
#define ISP_V4L2_CID_CUSTOM_SET_AE_ZONE_WEIGHT ( ISP_V4L2_CID_BASE + 12 )
#define ISP_V4L2_CID_CUSTOM_SET_AWB_ZONE_WEIGHT ( ISP_V4L2_CID_BASE + 13 )
#define ISP_V4L2_CID_CUSTOM_SET_MANUAL_EXPOSURE ( ISP_V4L2_CID_BASE + 14 )
#define ISP_V4L2_CID_CUSTOM_SET_SENSOR_INTEGRATION_TIME ( ISP_V4L2_CID_BASE + 15 )
#define ISP_V4L2_CID_CUSTOM_SET_SENSOR_ANALOG_GAIN ( ISP_V4L2_CID_BASE + 16 )
#define ISP_V4L2_CID_CUSTOM_SET_ISP_DIGITAL_GAIN ( ISP_V4L2_CID_BASE + 17 )
#define ISP_V4L2_CID_CUSTOM_SET_STOP_SENSOR_UPDATE ( ISP_V4L2_CID_BASE + 18 )
#define ISP_V4L2_CID_CUSTOM_SET_DS1_FPS ( ISP_V4L2_CID_BASE + 19 )


#define ISP_METERING_ZONES_AE_MAX_H 33
#define ISP_METERING_ZONES_AE_MAX_V 33
#define ISP_METERING_ZONES_AWB_MAX_H 33
#define ISP_METERING_ZONES_AWB_MAX_V 33

static unsigned char ae_zone_weight[ISP_METERING_ZONES_AE_MAX_V][ISP_METERING_ZONES_AE_MAX_H];
static unsigned char awb_zone_weight[ISP_METERING_ZONES_AWB_MAX_V][ISP_METERING_ZONES_AWB_MAX_H];

static void
do_af_refocus (int fd)
{
  struct v4l2_control ctrl;

  ctrl.id = ISP_V4L2_CID_AF_REFOCUS;
  ctrl.value = 0;

  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("Do AF refocus failed");
  }
}

static void
do_sensor_preset (int fd, int preset)
{
  struct v4l2_control ctrl;

  ctrl.id = ISP_V4L2_CID_SENSOR_PRESET;
  ctrl.value = preset;

  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("Do sensor preset failed");
  }
}

static void
do_sensor_wdr_mode (int fd, int mode)
{
  struct v4l2_control ctrl;

  ctrl.id = ISP_V4L2_CID_CUSTOM_SENSOR_WDR_MODE;
  ctrl.value = mode;

  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("Do sensor wdr mode failed\n");
  }
}

static void
do_fr_fps (int fd, int fps)
{
  struct v4l2_control ctrl;

  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_FR_FPS;
  ctrl.value = fps;

  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("Do sensor fps failed");
  }
}

static void
do_ds1_fps (int fd, int fps)
{
  struct v4l2_control ctrl;

  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_DS1_FPS;
  ctrl.value = fps;

  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("Do sensor fps failed");
  }
}

static void
do_fr_set_ae_zone_weight (int fd)
{
  struct v4l2_ext_controls ctrls;
  struct v4l2_ext_control ctrl;

  memset (&ctrls, 0, sizeof (ctrls));
  memset (&ctrls, 0, sizeof (ctrl));

  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_AE_ZONE_WEIGHT;
  ctrl.ptr = ae_zone_weight;

  ctrls.which = 0;
  ctrls.count = 1;
  ctrls.controls = &ctrl;

  if (-1 == ioctl (fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
    printf ("Do set ae zone weight failed\n");
  }
}

static void
do_fr_set_awb_zone_weight (int fd)
{
  struct v4l2_ext_controls ctrls;
  struct v4l2_ext_control ctrl;

  memset (&ctrls, 0, sizeof (ctrls));
  memset (&ctrls, 0, sizeof (ctrl));

  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_AWB_ZONE_WEIGHT;
  ctrl.ptr = awb_zone_weight;

  ctrls.which = 0;
  ctrls.count = 1;
  ctrls.controls = &ctrl;

  if (-1 == ioctl (fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
    printf ("Do set awb zone weight failed\n");
  }
}

static void
do_sensor_exposure (int fd, int exp)
{
  struct v4l2_control ctrl;

  ctrl.id = ISP_V4L2_CID_CUSTOM_SENSOR_EXPOSURE;
  ctrl.value = exp;

  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("Do sensor exposure failed\n");
  }
}

static void
set_manual_exposure (int fd, int enable)
{
  struct v4l2_control ctrl;
  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_MANUAL_EXPOSURE;
  ctrl.value = enable;
  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("set_manual_exposure failed\n");
  }
}

static void
do_sensor_ir_cut (int fd, int ircut_state)
{
  struct v4l2_control ctrl;
  ctrl.id = ISP_V4L2_CID_CUSTOM_SENSOR_IR_CUT;
  ctrl.value = ircut_state;
  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("do_sensor_ir_cut failed\n");
  }
}

static void
set_manual_sensor_integration_time (int fd, unsigned int sensor_integration_time_state)
{
  struct v4l2_control ctrl;
  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_SENSOR_INTEGRATION_TIME;
  ctrl.value = sensor_integration_time_state;
  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("set_manual_sensor_integration_time failed\n");
  }
}

static void
set_manual_sensor_analog_gain (int fd, unsigned int sensor_analog_gain_state)
{
  struct v4l2_control ctrl;
  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_SENSOR_ANALOG_GAIN;
  ctrl.value = sensor_analog_gain_state;
  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("set_manual_sensor_analog_gain failed\n");
  }
}

static void
set_stop_sensor_update (int fd, unsigned int stop_sensor_update_state)
{
  struct v4l2_control ctrl;
  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_STOP_SENSOR_UPDATE;
  ctrl.value = stop_sensor_update_state;
  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("set_stop_sensor_update failed\n");
  }
}

static void
set_manual_isp_digital_gain (int fd, unsigned int isp_digital_gain_state)
{
  struct v4l2_control ctrl;
  ctrl.id = ISP_V4L2_CID_CUSTOM_SET_ISP_DIGITAL_GAIN;
  ctrl.value = isp_digital_gain_state;
  if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
    printf ("set_manual_isp_digital_gain failed\n");
  }
}

void
onvif_rtsp_v4l2ctl_wdr (int fd, bool enable) {
  int mode, exposure;
  if (enable) {
    mode = 2; exposure = 2;
  } else {
    mode = 0; exposure = 1;
  }
  do_sensor_wdr_mode (fd, mode);
  do_sensor_exposure (fd, exposure);
}

void
onvif_rtsp_v4l2ctl_fps (int fd, int fps) {
  do_fr_fps (fd, fps);
}
