#ifndef _GST_AML_VIDEOCTL_H_
#define  _GST_AML_VIDEOCTL_H_
#include <gst/gst.h>

G_BEGIN_DECLS
typedef enum{
    AmlStateNormal,
    AmlStateFastForward,
    AmlStateSlowForward,
}AmlState;

int set_sysfs_str(const char *path, const char *val);
int get_sysfs_str(const char *path, char *valstr, int size);
int set_sysfs_int(const char *path, int val);
int get_sysfs_int(const char *path);
int set_black_policy(int blackout);
int get_black_policy();
int set_tsync_enable(int enable);
int get_tsync_enable(void);
int set_fb0_blank(int blank);
int set_fb1_blank(int blank);

G_END_DECLS
#endif
