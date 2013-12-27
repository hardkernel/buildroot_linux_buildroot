
#include "amlstreaminfo.h"
#include "amlvideoinfo.h"

//media stream info class ,in clude audio and video
//typedef int (*AmlCallBackFunc)(void *data);

int amlStreamInfoWriteHeader(AmlStreamInfo *info, codec_para_t *pcodec)
{
    g_print("[%s:%d]\n", __FUNCTION__, __LINE__);
    if(NULL == info->configdata){
        g_print("[%s:%d] configdata is null\n", __FUNCTION__, __LINE__);
        return 0;
    }
    g_print("[%s:%d]\n", __FUNCTION__, __LINE__);
    guint8 *configbuf = GST_BUFFER_DATA(info->configdata);
    gint configsize = GST_BUFFER_SIZE(info->configdata);
    g_print("[%s:%d]configsize=%d\n", __FUNCTION__, __LINE__, configsize);
    if(configbuf && (configsize > 0)){
        codec_write(pcodec, configbuf, configsize);
    }
    return 0;
}

AmlStreamInfo *createStreamInfo(gint size)
{
    AmlStreamInfo *info = g_malloc(size);
    info->init = NULL;
    info->writeheader = amlStreamInfoWriteHeader;
    info->add_startcode = NULL;
    info->finalize = amlStreamInfoFinalize;
    info->configdata = NULL;
    g_print("[%s:%d]\n", __FUNCTION__, __LINE__);
    return info;
}

void amlStreamInfoFinalize(AmlStreamInfo *info)
{
    if(info){
        g_free(info);
    }
}

typedef struct{
    const gchar *name;
    AmlStreamInfo *(*newStreamInfo)();
}AmlStreamInfoPool;

static const AmlStreamInfoPool amlStreamInfoPool[] = {
    /*******video format information*******/
    {"video/x-h264", newAmlInfoH264},
    {"video/mpeg", newAmlInfoMpeg},
    {"video/x-msmpeg", newAmlInfoMsmpeg},
    {"video/x-h263", newAmlInfoH263},
    {"video/x-jpeg", newAmlInfoJpeg},
    {"video/x-wmv", newAmlInfoWmv},
    {"video/x-divx", newAmlInfoDivx},
    {"video/x-xvid", newAmlInfoXvid},
    /*******audio format information*******/
    {NULL, NULL}
};

AmlStreamInfo *amlStreamInfoInterface(gchar *format)
{
    AmlStreamInfoPool *p = amlStreamInfoPool;
    AmlStreamInfo *info = NULL;
    int i = 0;
    while(p->name){
        if(!strcmp(p->name, format)){
            g_print("[%s:%d]name=%s\n", __FUNCTION__, __LINE__, p->name);
            if(p->newStreamInfo){
                info = p->newStreamInfo();
            }
            break;
        }
        p++;
    }
    return info;
}


