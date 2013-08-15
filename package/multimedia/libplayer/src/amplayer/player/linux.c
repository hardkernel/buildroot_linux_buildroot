#include <amconfigutils.h>

int GetSystemSettingString(const char *path, char *value, char *defaultv)
{
    return am_getconfig(path, value, defaultv);
}

