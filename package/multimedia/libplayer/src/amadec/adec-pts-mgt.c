/**
 * \file adec-pts-mgt.c
 * \brief  Functions Of Pts Manage.
 * \version 1.0.0
 * \date 2011-03-08
 */
/* Copyright (C) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <adec-pts-mgt.h>
#include <amconfigutils.h>
#include <sys/time.h>


int adec_pts_droppcm(aml_audio_dec_t *audec);
int vdec_pts_resume(void);
static int vdec_pts_pause(void);

static int set_tsync_enable(int enable)
{
    int fd;
    char *path = "/sys/class/tsync/enable";
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", enable);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    
    return -1;
}
/**
 * \brief calc current pts
 * \param audec pointer to audec
 * \return aurrent audio pts
 */
unsigned long adec_calc_pts(aml_audio_dec_t *audec)
{
    unsigned long pts, delay_pts;
    audio_out_operations_t *out_ops;
    dsp_operations_t *dsp_ops;

    out_ops = &audec->aout_ops;
    dsp_ops = &audec->adsp_ops;

    pts = dsp_ops->get_cur_pts(dsp_ops);
    if (pts == -1) {
        adec_print("get get_cur_pts failed\n");
        return -1;
    }
    dsp_ops->kernel_audio_pts = pts;

    if (out_ops == NULL || out_ops->latency == NULL) {
        adec_print("cur_out is NULL!\n ");
        return -1;
    }

    delay_pts = out_ops->latency(audec) * 90;

    if (delay_pts < pts) {
        pts -= delay_pts;
    } else {
        pts = 0;
    }

    return pts;
}

/**
 * \brief start pts manager
 * \param audec pointer to audec
 * \return 0 on success otherwise -1
 */
int adec_pts_start(aml_audio_dec_t *audec)
{
    unsigned long pts = 0;
    char *file;
    int fd;
    char buf[64];
    dsp_operations_t *dsp_ops;
	char value[CONFIG_VALUE_MAX]={0};

    adec_print("adec_pts_start");
    dsp_ops = &audec->adsp_ops;
    memset(buf, 0, sizeof(buf));

    if (audec->avsync_threshold <= 0) {
        if (am_getconfig_bool("media.libplayer.wfd")) {
            audec->avsync_threshold = SYSTIME_CORRECTION_THRESHOLD * 2 / 3;
            adec_print("use 2/3 default av sync threshold!\n");
        } else {
            audec->avsync_threshold = SYSTIME_CORRECTION_THRESHOLD;
            adec_print("use default av sync threshold!\n");
        }
    }
    adec_print("av sync threshold is %d , no_first_apts=%d,\n", audec->avsync_threshold, audec->no_first_apts);

    dsp_ops->last_pts_valid = 0;
	
	if(am_getconfig("sys.amplayer.drop_pcm",value,NULL) > 0)
		if((!strcmp(value,"1")||!strcmp(value,"true")) && (audec->droppcm_flag))
		{
			adec_pts_droppcm(audec);
			vdec_pts_resume();
			set_tsync_enable(1);
		}
    // before audio start or pts start
    fd = open(TSYNC_EVENT, O_WRONLY);
    if(fd < 0){
      adec_print("unable open file %s, err: %s", TSYNC_EVENT, strerror(errno));
      return -1;
    }
    sprintf(buf, "AUDIO_PRE_START", 0);
    write(fd, buf, strlen(buf));
    close(fd);

    usleep(1000);

	if (audec->no_first_apts) {
		fd = open(TSYNC_APTS, O_RDONLY);
		if (fd < 0) {
			adec_print("unable to open file %s,err: %s", TSYNC_APTS, strerror(errno));
			return -1;
		}
		
		read(fd, buf, sizeof(buf));
		if (sscanf(buf, "0x%lx", &pts) < 1) {
			adec_print("unable to get vpts from: %s", buf);
			return -1;
		}
		close(fd);

	} else {
	    pts = adec_calc_pts(audec);
		
	    if (pts == -1) {

	        adec_print("pts==-1");

	        fd = open(TSYNC_APTS, O_RDONLY);
	        if (fd < 0) {
	            adec_print("unable to open file %s,err: %s", TSYNC_APTS, strerror(errno));
	            return -1;
	        }

	        read(fd, buf, sizeof(buf));
	        close(fd);
	        if (sscanf(buf, "0x%lx", &pts) < 1) {
	            adec_print("unable to get apts from: %s", buf);
	            return -1;
	        }
	    }
	}

    adec_print("audio pts start from 0x%lx", pts);

    fd = open(TSYNC_EVENT, O_WRONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_EVENT, strerror(errno));
        return -1;
    }

    sprintf(buf, "AUDIO_START:0x%lx", pts);
    write(fd, buf, strlen(buf));
    close(fd);

    return 0;
}

int adec_pts_droppcm(aml_audio_dec_t *audec)
{
    unsigned long vpts, apts;
    int drop_size;
    int fd;
    int ret;
    char buf[32];
    char buffer[8*1024];
	char value[CONFIG_VALUE_MAX]={0};
	
    fd = open(TSYNC_VPTS, O_RDONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_VPTS, strerror(errno));
        return -1;
    }

    read(fd, buf, sizeof(buf));
    if (sscanf(buf, "0x%lx", &vpts) < 1) {
        adec_print("unable to get vpts from: %s", buf);
        return -1;
    }
    close(fd);

    apts = adec_calc_pts(audec);
	int diff = (apts > vpts)?(apts-vpts):(vpts-apts);
	adec_print("before drop --apts 0x%x,vpts 0x%x,apts %s, diff 0x%x\n",apts,vpts,(apts>vpts)?"big":"small",diff);
	int audio_ahead = 0;
	unsigned pts_ahead_val = SYSTIME_CORRECTION_THRESHOLD;

        if (am_getconfig_bool("media.libplayer.wfd")) {
            pts_ahead_val = pts_ahead_val * 2 / 3;
        }

	if(am_getconfig("media.amplayer.apts",value,NULL) > 0){
		if(!strcmp(value,"slow")){
			audio_ahead = -1;
		}
		else if(!strcmp(value,"fast")){
			audio_ahead = 1;
		}
	}
	memset(value,0,sizeof(value));
	if(am_getconfig("media.amplayer.apts_val",value,NULL) > 0){
		pts_ahead_val = atoi(value);
	}	
	adec_print("audio ahead %d,ahead pts value %d \n",	audio_ahead,pts_ahead_val);

	struct timeval  new_time,old_time;
    long new_time_mseconds;
    long old_time_mseconds;
    //old time
    gettimeofday(&old_time, NULL);
    old_time_mseconds = (old_time.tv_usec / 1000 + old_time.tv_sec * 1000);
#if 1
    if(apts < vpts){
        drop_size = ((vpts - apts+pts_ahead_val*audio_ahead)/90) * (audec->samplerate/1000) * audec->channels *2;
    	
		int nDropCount=0;
		adec_print("==drop_size=%d, nDropCount:%d -----------------\n",drop_size, nDropCount);
		while(drop_size > 0){
			ret = audec->adsp_ops.dsp_read(&audec->adsp_ops, buffer, MIN(drop_size, 8192));
			//apts = adec_calc_pts(audec);
			//adec_print("==drop_size=%d, ret=%d, nDropCount:%d apts=0x%x,-----------------\n",drop_size, ret, nDropCount,apts);
			if(ret==0)//no data in pcm buf
			{
				if(nDropCount>=5)
					break;
				else 
					nDropCount++;
				adec_print("==ret:0 no pcm nDropCount:%d \n",nDropCount);
			}
			else
			{
				nDropCount=0;
				drop_size -= ret;
			}
		}
    }
#endif	

	    //new time 
    gettimeofday(&new_time, NULL);
    new_time_mseconds = (new_time.tv_usec / 1000 + new_time.tv_sec * 1000);
    adec_print("==old time  sec :%d usec:%d \n", old_time.tv_sec  ,old_time.tv_usec );
    adec_print("==new time  sec:%d usec:%d \n", new_time.tv_sec  ,new_time.tv_usec  ); 
    adec_print("==old time ms is :%d  new time ms is:%d   diff:%d  \n",old_time_mseconds ,new_time_mseconds ,new_time_mseconds- old_time_mseconds);
    fd = open(TSYNC_VPTS, O_RDONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_VPTS, strerror(errno));
        return -1;
    }

    read(fd, buf, sizeof(buf));
    close(fd);
    if (sscanf(buf, "0x%lx", &vpts) < 1) {
        adec_print("unable to get vpts from: %s", buf);
        return -1;
    }

    apts = adec_calc_pts(audec);
	diff = (apts > vpts)?(apts-vpts):(vpts-apts);
	adec_print("after drop pcm:--apts 0x%x,vpts 0x%x,apts %s, diff 0x%x\n",apts,vpts,(apts>vpts)?"big":"small",diff);
	
#if 0
    while(apts < vpts){
        audec->adsp_ops.dsp_read(&audec->adsp_ops, buffer, 8192);
	 apts = adec_calc_pts(audec);
    }
#endif

    return 0;
}

/**
 * \brief pause pts manager
 * \return 0 on success otherwise -1
 */
int adec_pts_pause(void)
{
    int fd;
    char buf[32];

    fd = open(TSYNC_EVENT, O_WRONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_EVENT, strerror(errno));
        return -1;
    }

    sprintf(buf, "AUDIO_PAUSE");
    write(fd, buf, strlen(buf));
    close(fd);

    return 0;
}

/**
 * \brief resume pts manager
 * \return 0 on success otherwise -1
 */
int adec_pts_resume(void)
{
    int fd;
    char buf[32];

    adec_print("adec_pts_resume");
    memset(buf, 0, sizeof(buf));

    fd = open(TSYNC_EVENT, O_WRONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_EVENT, strerror(errno));
        return -1;
    }

    sprintf(buf, "AUDIO_RESUME");
    write(fd, buf, strlen(buf));
    close(fd);

    return 0;
}

/**
 * \brief refresh current audio pts
 * \param audec pointer to audec
 * \return 0 on success otherwise -1
 */
 static int apts_interrupt=0;
int adec_refresh_pts(aml_audio_dec_t *audec)
{
    unsigned long pts;
    unsigned long systime;
    unsigned long last_pts = audec->adsp_ops.last_audio_pts;
    unsigned long last_kernel_pts = audec->adsp_ops.kernel_audio_pts;
    int fd = -1;
    char buf[64];

    if (audec->auto_mute == 1) {
        return 0;
    }

    memset(buf, 0, sizeof(buf));

    /* get system time */
    fd = open(TSYNC_PCRSCR, O_RDWR);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_PCRSCR, strerror(errno));
        return -1;
    }

    read(fd, buf, sizeof(buf));
    if(fd>=0){
        close(fd);
        fd = -1;
    }

    if (sscanf(buf, "0x%lx", &systime) < 1) {
        adec_print("unable to getsystime %s", buf);
        //close(fd);
        return -1;
    }

    /* get audio time stamp */
    pts = adec_calc_pts(audec);
    if (pts == -1 || last_pts == pts) {
        //close(fd);
        //if (pts == -1) {
        return -1;
        //}
    }

    //adec_print("adec_get_pts() pts=%x\n",pts);

    if ((abs(pts - last_pts) > APTS_DISCONTINUE_THRESHOLD) && (audec->adsp_ops.last_pts_valid)) {
        /* report audio time interruption */
        adec_print("pts = %lx, last pts = %lx\n", pts, last_pts);

        fd = open(TSYNC_EVENT, O_RDWR);
        if (fd < 0) {
            adec_print("unable to open file %s,err: %s", TSYNC_EVENT, strerror(errno));
            return -1;
        }

        adec_print("audio time interrupt: 0x%lx->0x%lx, 0x%lx\n", last_pts, pts, abs(pts - last_pts));

        sprintf(buf, "AUDIO_TSTAMP_DISCONTINUITY:0x%lx", pts);
        write(fd, buf, strlen(buf));
        close(fd);
	 fd = -1;
        audec->adsp_ops.last_audio_pts = pts;
        audec->adsp_ops.last_pts_valid = 1;
        adec_print("set automute!\n");
        audec->auto_mute = 1;
        apts_interrupt=10;
        return 0;
    }

    if (last_kernel_pts == audec->adsp_ops.kernel_audio_pts) {
        return 0;
    }

    audec->adsp_ops.last_audio_pts = pts;
    audec->adsp_ops.last_pts_valid = 1;

    if (abs(pts - systime) < audec->avsync_threshold) {
        apts_interrupt=0;
        return 0;
    }
    else if(apts_interrupt>0){
        apts_interrupt --;
        return 0;
        }

    /* report apts-system time difference */
    fd = open(TSYNC_APTS, O_RDWR);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_APTS, strerror(errno));
        return -1;
    }

    adec_print("report apts as %ld,system pts=%ld, difference= %ld\n", pts, systime, (pts - systime));

    sprintf(buf, "0x%lx", pts);
    write(fd, buf, strlen(buf));
    close(fd);
    fd = -1;
    return 0;
}

/**
 * \brief Disable or Enable av sync
 * \param e 1 =  enable, 0 = disable
 * \return 0 on success otherwise -1
 */
int avsync_en(int e)
{
    int fd;
    char  bcmd[16];

    fd = open(TSYNC_ENABLE, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", e);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }
    return -1;
}

/**
 * \brief calc pts when switch audio track
 * \param audec pointer to audec
 * \return 0 on success otherwise -1
 *
 * When audio track switch occurred, use this function to judge audio should
 * be played or not. If system time fall behind audio pts , and their difference
 * is greater than SYSTIME_CORRECTION_THRESHOLD, auido should wait for
 * video. Otherwise audio can be played.
 */
int track_switch_pts(aml_audio_dec_t *audec)
{
    unsigned long vpts;
    unsigned long apts;
    unsigned long pcr;
    char buf[32];
    int fd = -1;

    memset(buf, 0, sizeof(buf));

    fd = open(TSYNC_PCRSCR, O_RDWR);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_PCRSCR, strerror(errno));
        return 1;
    }

    read(fd, buf, sizeof(buf));
    close(fd);

    if (sscanf(buf, "0x%lx", &pcr) < 1) {
        adec_print("unable to get pcr %s", buf);
        close(fd);
        return 1;
    }

    apts = adec_calc_pts(audec);
    if (apts == -1) {
        close(fd);
        adec_print("unable to get apts");
        return 1;
    }
    
    if((apts > pcr) && (apts - pcr > 0x100000))
        return 0;
		
    if (abs(apts - pcr) < audec->avsync_threshold || (apts <= pcr)) {
        return 0;
    } else {
        return 1;
    }

}
static int vdec_pts_pause(void)
{
    int fd;
    char buf[32];

    fd = open("/sys/class/video_pause", O_WRONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_EVENT, strerror(errno));
        return -1;
    }

    sprintf(buf, "1");
    write(fd, buf, strlen(buf));
    close(fd);

    return 0;
}
int vdec_pts_resume(void)
{
    int fd;
    char buf[32];

    adec_print("vdec_pts_resume\n");
    memset(buf, 0, sizeof(buf));

    fd = open(TSYNC_EVENT, O_WRONLY);
    if (fd < 0) {
        adec_print("unable to open file %s,err: %s", TSYNC_EVENT, strerror(errno));
        return -1;
    }

    sprintf(buf, "VIDEO_PAUSE:0x0");
    write(fd, buf, strlen(buf));
    close(fd);

    return 0;
}
