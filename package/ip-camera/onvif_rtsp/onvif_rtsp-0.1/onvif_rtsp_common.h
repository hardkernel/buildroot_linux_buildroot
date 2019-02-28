#ifndef _ONVIF_RTSP_COMMON_H_
#define _ONVIF_RTSP_COMMON_H_

#include <string>

void rtsp_common_create_dir (const char *path);
std::string rtsp_common_build_filename (const char *suffix);

#endif /* _ONVIF_RTSP_COMMON_H_ */
