#ifdef __cplusplus
extern "C" {
#endif

#define MultiSoundCard
//#define TINYALSA_LIB
//#define Android_4_2

#ifdef Android_4_2
#define LOGI ALOGI
#define LOGE ALOGE
#else
#ifndef ANDROID
#undef LOGI
#define LOGI(fmt, ...) printf("I/%s: ", LOG_TAG);printf(fmt, ##__VA_ARGS__);
#undef LOGE
#define LOGE(fmt, ...) printf("E/%s: ", LOG_TAG);printf(fmt, ##__VA_ARGS__);
#undef LOGD
#define LOGD(fmt, ...) printf("D/%s: ", LOG_TAG);printf(fmt, ##__VA_ARGS__);
#endif
#endif

int line_in_select_channel();
int snd_ctl_set(char *control_name, int value);

#ifdef __cplusplus
}
#endif
