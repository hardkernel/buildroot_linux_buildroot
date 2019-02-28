#include "onvif_rtsp_common.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctime>

void rtsp_common_create_dir (const char *path) {
  DIR *dir = opendir (path);
  if (!dir) {
    if (mkdir (path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
      perror ("create storage dir");
    }
  }
  closedir (dir);
}

std::string rtsp_common_build_filename (const char *suffix) {
  std::time_t t = std::time (nullptr);
  char time_str[64];
  std::strftime (time_str, sizeof (time_str), "%Y-%m-%d_%H-%M-%S", std::localtime (&t));
  return std::string (time_str) + suffix;
}
