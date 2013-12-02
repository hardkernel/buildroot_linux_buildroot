#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/fb.h>
#include "gstamlsysctl.h"

int set_sysfs_str(const char *path, const char *val)
{
    int fd;
    int bytes;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    } else {
    }
    return -1;
}
int  get_sysfs_str(const char *path, char *valstr, int size)
{
    int fd;
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, valstr, size - 1);
        valstr[strlen(valstr)] = '\0';
        close(fd);
    } else {
        sprintf(valstr, "%s", "fail");
        return -1;
    };
    //log_print("get_sysfs_str=%s\n", valstr);
    return 0;
}

int set_sysfs_int(const char *path, int val)
{
    int fd;
    int bytes;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", val);
        bytes = write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}
int get_sysfs_int(const char *path)
{
    int fd;
    int val = 0;
    char  bcmd[16];
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, sizeof(bcmd));
        val = strtol(bcmd, NULL, 16);
        close(fd);
    }
    return val;
}


int set_black_policy(int blackout)
{
    return set_sysfs_int("/sys/class/video/blackout_policy", blackout);
}

int get_black_policy()
{
    return get_sysfs_int("/sys/class/video/blackout_policy") & 1;
}

int set_tsync_enable(int enable)
{
    return set_sysfs_int("/sys/class/tsync/enable", enable);

}
int get_tsync_enable(void)
{
    char buf[32];
    int ret = 0;
    int val = 0;
    ret = get_sysfs_str("/sys/class/tsync/enable", buf, 32);
    if (!ret) {
        sscanf(buf, "%d", &val);
    }
    return val == 1 ? val : 0;
}

int set_fb0_blank(int blank)
{
    return  set_sysfs_int("/sys/class/graphics/fb0/blank", blank);
}

int set_fb1_blank(int blank)
{
    return  set_sysfs_int("/sys/class/graphics/fb1/blank", blank);
}

/*property functions*/
static void aml_register_propfunc (GHashTable **propTable, gint key, AmlPropFunc func)
{
  gpointer ptr = (gpointer) func;

  if (propTable && !(*propTable)){
    *propTable = g_hash_table_new (g_direct_hash, g_direct_equal);
  }
  if (!g_hash_table_lookup (*propTable, key))
    g_hash_table_insert (*propTable, key, (gpointer) ptr);
}
static void aml_unregister_propfunc(GHashTable *propTable)
{
    if(propTable){
        g_hash_table_destroy(propTable);
    }
}

AmlPropFunc aml_find_propfunc (GHashTable *propTable, gint key)
{
    AmlPropFunc func = NULL;
    func = (AmlPropFunc)g_hash_table_lookup(propTable, key);
    return func;
}

void aml_Install_Property(
    GObjectClass *kclass, 
    GHashTable **getPropTable, 
    GHashTable **setPropTable, 
    AmlPropType *prop_pool)
{
    GObjectClass*gobject_class = (GObjectClass *) kclass;
    AmlPropType *p = prop_pool;

    while(p && (p->propID != -1)){
        if(p->installprop){
            p->installprop(gobject_class, p->propID);
        }
        if(p->getprop){
            aml_register_propfunc(getPropTable, p->propID, p->getprop);
        }
        if(p->setprop){
            aml_register_propfunc(setPropTable, p->propID, p->setprop);
        }
        p++;
    }

}

void aml_Uninstall_Property(GHashTable *getPropTable, GHashTable *setPropTable)
{
    aml_unregister_propfunc(getPropTable);
    aml_unregister_propfunc(setPropTable);
}
