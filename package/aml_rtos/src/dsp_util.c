// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "hifi4dsp_api.h"


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#define msleep(x)  usleep(x*1000)

#define DEBUG_PRINT		0
#if DEBUG_PRINT
	#define debug_pr(fmt, ...)  printf(fmt, __VA_ARGS__)
#else
	#define debug_pr(fmt, ...)
#endif

#define  CMD_HIFI4DSP_LOAD	0x0100
#define  CMD_HIFI4DSP_RESET	0x0200
#define  CMD_HIFI4DSP_START	0x0400
#define  CMD_HIFI4DSP_STOP	0x0800
#define  CMD_HIFI4DSP_SLEEP	0x1000
#define  CMD_HIFI4DSP_WAKE	0x2000

extern char *optarg;
extern int optind, opterr, optopt;
int lopt;
char dspname[32];

static struct option longOpts[] = {
	{ "load", no_argument, NULL, 'l' },
	{ "reset", no_argument, NULL, 'r' },
	{ "start", no_argument, NULL, 'S' },
	{ "stop", no_argument, NULL, 's' },
	{ "sleep", no_argument, NULL, 'k' },
	{ "wake", no_argument, NULL, 'w' },
	{ "dsp", required_argument, NULL, 'd' },
	{ "firmware", required_argument, NULL, 'f' },
	{ "addr", required_argument, NULL, 'a' },
	{ "test-1", required_argument, &lopt, 1 },
	{ "test-2", required_argument, &lopt, 2 },
	{ "test-3", optional_argument, &lopt, 3 },
	{ "version", no_argument, NULL, 'v' },
	{ "help", no_argument, NULL, 'h' },
	{ 0, 0, 0, 0 }
};
static char optString[]="lrSskwd:f:a:012h";

void showUsage() {
	printf("Usage:[options]\n");
	printf(" -l, --load                 		load firmware to ddr\n");
	printf(" -r, --reset               		reset command\n");
	printf(" -S, --start               		set dsp clk and power on ,reset\n");
	printf(" -s, --stop 				stop dsp\n");
	printf(" -d, --dsp=DSPNAME          		The dspname, hifi4a or hifi4b\n");
	printf(" -f, --firmware=FILE_NAME   		The file name for downloaded file.\n");
	printf(" -a, --addr=address         		The address which used for dsp start \n");
	printf(" --test-1=N                 		Test 1\n");
	printf(" --test-2=N                 		Test 2\n");
	printf(" --test-3=N                		Test 3\n");
	printf(" -v, --version              		Print the version number and exit.\n");
	printf(" -h, --help                 		Print this message and exit.\n");
	printf("URL:\n");
}

int dsp_ctl(struct hifi4dsp_info_t *info, unsigned int cmd)
{
	int i=0,ret=0;
	int fd;
	char path[64];
	sprintf(path, "/dev/hifi4dsp%d", info->id);
	fd = open(path, O_RDWR);
	if (fd < 0)
	{
		printf("open %s fail:%s\n", path, strerror(errno));
		return -1;
	}
	printf("open %s success \n",path);
	if ((ret = ioctl(fd, cmd, info)) < 0)
	{
		printf("ioctl fail:%s\n", strerror(errno));
	}
	close(fd);

	return 0;
}


bool dsp_dev_is_exist(struct hifi4dsp_info_t *info)
{
	int ret = -1;
	const char *pathname;
	char path[256];
	int fd;
	if( (info->id!=0 )&&(info->id!=1 ))
	{
		printf("param error: invalid dsp id (info->id=%d)\n",info->id);
		return 0;
	}
	sprintf(path, "/dev/hifi4dsp%d", info->id);
	pathname=path;

	if(access(pathname,F_OK) != -1){
		printf("dsp dev: %s exist\n", pathname);
	}else{
		printf("dsp dev: %s not exist\n", pathname);
		return 0;
	}
	return 1;
}


bool dsp_firmware_is_exist(struct hifi4dsp_info_t *info)
{
	int ret = -1;
	const char *pathname;
	char path[256];
	int fd;

	strcpy(path, "/lib/firmware/");
	strcat(path, info->fw_name);
	if(strlen(info->fw_name)==0)
	{
		printf("param error: invalid dsp firmware (info->fw_name=NULL)\n");
		return 0;
	}

	sprintf(path, "/lib/firmware/%s", info->fw_name);

	printf(" info->fw_name:%s", path);
	pathname=path;

	if(access(pathname,F_OK) != -1){
		printf("firmware: %s exist\n", pathname);
	}else{
		printf("firmware: invalid dsp firmware (%s not exist)\n", pathname);
		return 0;
	}
	return 1;
}


int dsp_load(struct hifi4dsp_info_t *info)
{
	int err=0;

	if(false == dsp_dev_is_exist(info))
		err -= 1;

	//if(strlen(info->fw_name)==0)
	if (strcmp(info->fw_name, "\0") == 0)
		strcpy(info->fw_name, "dspaboot.bin");

	if(false == dsp_firmware_is_exist(info))
		err -= 1;
	if(err<0)
		return err;
	return dsp_ctl(info, HIFI4DSP_LOAD);
}

int dsp_reset(struct hifi4dsp_info_t *info)
{
	int err=0;
	if(false == dsp_dev_is_exist(info))
		err -= 1;
	if(err<0)
		return err;
	return dsp_ctl(info, HIFI4DSP_RESET);
}
int dsp_start(struct hifi4dsp_info_t *info)
{
	int err=0;
	if(false == dsp_dev_is_exist(info))
		err -= 1;
	if(err<0)
		return err;

	return dsp_ctl(info, HIFI4DSP_START);
}

int dsp_stop(struct hifi4dsp_info_t *info)
{
	int err=0;
	if(false == dsp_dev_is_exist(info))
		err -= 1;
	if(err<0)
		return err;

	return dsp_ctl(info, HIFI4DSP_STOP);
}
int dsp_sleep(struct hifi4dsp_info_t *info)
{
	int err=0;
	if(false == dsp_dev_is_exist(info))
		err -= 1;
	if(err<0)
		return err;

	return dsp_ctl(info, HIFI4DSP_SLEEP);
}
int dsp_wake(struct hifi4dsp_info_t *info)
{
	int err=0;
	if(false == dsp_dev_is_exist(info))
		err -= 1;
	if(err<0)
		return err;

	return dsp_ctl(info, HIFI4DSP_WAKE);
}

int hifi4dsp_load()
{
	int i=0,ret=0;
	struct hifi4dsp_info_t *info=malloc(sizeof(struct hifi4dsp_info_t));
	int fd = open("/dev/hifi4dsp0", O_RDWR);
	if (fd < 0)
	{
		printf("open fail:%s\n", strerror(errno));
		return -1;
	}
//	strcpy(info->fw_name, "hifi4dsp_fw.bin2");
	strcpy(info->fw_name, "dspboot.bin");
	if(false == dsp_firmware_is_exist(info))
		ret -= 1;
	if (ret < 0)
		return ret;
	msleep(50) ;
	if ((ret = ioctl(fd, HIFI4DSP_LOAD, info)) < 0)
	{
		printf("ioctl get fail:%s\n", strerror(errno));
	}

	close(fd);
	return 0;
}

void hifi4dsp_info_init(struct hifi4dsp_info_t *info)
{
	memset(info, 0, sizeof(struct hifi4dsp_info_t));
	info->id=0;
	info->phy_addr=0x3400000;
}

int dsp_cmd_parse(int argc, char **argv, struct hifi4dsp_info_t *info)
{
	int cmd=0,err=0;
	int opt;
	int digit_optind = 0;
	int this_option_optind = optind ? optind : 1;
  	int option_index = 0;

	if (argc < 2) showUsage();

	while (1) {
		this_option_optind = optind ? optind : 1;
		opt = getopt_long(argc, argv, ":lrSskwd:f:a:012h", longOpts, &option_index);
		if (opt == -1)
			break;

		switch (opt) {

		case '0':
		case '1':
		case '2':
			if (digit_optind != 0 && digit_optind != this_option_optind)
			debug_pr("digits occur in two different argv-elements.\n");
			digit_optind = this_option_optind;
			debug_pr("option %c\n", opt);
		break;
		case 'l':
			cmd += CMD_HIFI4DSP_LOAD;
			debug_pr("option=l, value=%s\n", optarg);
		break;
		case 'r':
			cmd += CMD_HIFI4DSP_RESET;
			debug_pr("option=r, value=%s\n", optarg);
		break;
		case 'S':
			cmd += CMD_HIFI4DSP_START;
			debug_pr("option=r, value=%s\n", optarg);
		break;
		case 's':
			cmd += CMD_HIFI4DSP_STOP;
			debug_pr("option=r, value=%s\n", optarg);
		break;
		case 'k':
			cmd += CMD_HIFI4DSP_SLEEP;
			debug_pr("option=r, value=%s\n", optarg);
		break;
		case 'w':
			cmd += CMD_HIFI4DSP_WAKE;
			debug_pr("option=r, value=%s\n", optarg);
		break;
		case 'd':	/*dsp id*/
			//cmd |= 0x1;
			cmd |= 0x10;
			//sscanf(optarg, "%d", &(info->id));
			//info->id = optarg[1] - '0';
			strcpy(dspname, optarg);
			printf("dspname:%s \n",dspname);
			if (strcmp(dspname, "hifi4a") == 0)
				info->id = 0;
			else if (strcmp(dspname, "hifi4b") == 0)
				info->id = 1;
			else
			{
				printf("#########wrong dsp name######### \n"
				         "please input : hifi4a or hifi4b \n");
				showUsage();
				return 0;
			}
			debug_pr("option=d, value=%s, info.id=%d\n", optarg, info->id);
		break;

		case 'f': /*firwmare name*/
			cmd |= 0x20;
			strcpy((info->fw_name), optarg);
			debug_pr("option=f, value=%s, info->fw_name=%s\n", optarg, info->fw_name);
		break;

		case 'a': /*address*/
			cmd |= 0x40;
			sscanf(optarg, "%x", &(info->phy_addr));
			debug_pr("option=a, value=%s, info.phy_addr=0x%x\n", optarg, info->phy_addr);
		break;
		case 'h':
			showUsage();
		break;
		case '?':
			printf("****cmd option error: invalid option in cmd\n");
			err=(err<0?err-1:-1);
			//showUsage();
		break;
		case ':':
			printf("****cmd option error: param is requeired for option\n");
			err=(err<0?err-1:-1);
			//showUsage();
		break;
		default:
			debug_pr("getopt returned non-option code 0%o\n", opt);
		}

		debug_pr("opt = %c\t", opt);
		debug_pr("optarg = %-15s\t",optarg);
		debug_pr("optind = %-3d\t",optind);
		debug_pr("argv[%d] =%-15s\t", optind, argv[optind]);
		debug_pr("option_index = %-3d\n",option_index);
    }
	if (optind < argc) {
		debug_pr("have non-option ARGV-elements: ");
		while (optind < argc){
			showUsage();
			debug_pr("%s ", argv[optind++]);
		}
		debug_pr("\n");
		return 0;
	}
	if (err<0){
		showUsage();
		return err;
	}
	debug_pr("%s cmd=0x%08x\n",__func__,cmd);
	return cmd;
}

#if 0
int main(int argc, char **argv)
{
	int cmd=0;
	struct hifi4dsp_info_t info={0};
//	hifi4dsp_info_init(&info);
	printf("dsp driver test! \n");
	hifi4dsp_load();
	//dsp_load(&info);
	dsp_start(&info);
	return 0;
}
#endif

int main(int argc, char **argv)
{
	int cmd=0;
	struct hifi4dsp_info_t info={0};
	cmd = dsp_cmd_parse(argc, argv, &info);
	if(cmd<0)
		return 0;
	if(cmd&CMD_HIFI4DSP_LOAD){
		dsp_load(&info);
	}
	if(cmd&CMD_HIFI4DSP_RESET){
		dsp_reset(&info);
	}
	if(cmd&CMD_HIFI4DSP_START){
		dsp_start(&info);
	}
	if(cmd&CMD_HIFI4DSP_STOP){
		dsp_stop(&info);
	}
	if(cmd&CMD_HIFI4DSP_SLEEP){
		dsp_sleep(&info);
	}
	if(cmd&CMD_HIFI4DSP_WAKE){
		dsp_wake(&info);
	}
	return 0;
}

