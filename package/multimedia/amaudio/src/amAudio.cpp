#define TEST_AUDIO_IN_DIRECT_OUT
#ifndef TEST_AUDIO_IN_DIRECT_OUT
#define WRITE_AUDIO_OUT_TO_FILE
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "line_in_select_channel.h"

#define LOG_TAG "amAudio_Test"

#define AUDIO_IN									"/dev/amaudio_in"			//输入设备
#define AUDIO_OUT									"/dev/amaudio_out" 			//输出设备

#define AMAUDIO_IOC_MAGIC							'A'
//Get 输出buffer 大小
#define AMAUDIO_IOC_GET_I2S_OUT_SIZE				_IOW(AMAUDIO_IOC_MAGIC, 0x00, int)
//Get 输出指针(硬件指针)
#define AMAUDIO_IOC_GET_I2S_OUT_PTR					_IOW(AMAUDIO_IOC_MAGIC, 0x01, int)
//Set 输出读指针
#define AMAUDIO_IOC_SET_I2S_OUT_RD_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x02, int)
// Get 输入buffer 大小
#define AMAUDIO_IOC_GET_I2S_IN_SIZE					_IOW(AMAUDIO_IOC_MAGIC, 0x03, int)
//Get 输入指针(硬件指针)
#define AMAUDIO_IOC_GET_I2S_IN_PTR					_IOW(AMAUDIO_IOC_MAGIC, 0x04, int)
//Set  输入读指针
#define AMAUDIO_IOC_SET_I2S_IN_RD_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x05, int)
// Set   输入/ 输出模式。arg = 1 (16bit interleave 模式) ; arg = 0 (32bit block 模式)
#define AMAUDIO_IOC_SET_I2S_IN_MODE					_IOW(AMAUDIO_IOC_MAGIC, 0x06, int)
#define AMAUDIO_IOC_SET_I2S_OUT_MODE				_IOW(AMAUDIO_IOC_MAGIC, 0x07, int)
//Get  输入/ 输出读指针
#define AMAUDIO_IOC_GET_I2S_IN_RD_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x08, int)
#define AMAUDIO_IOC_GET_I2S_OUT_RD_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x09, int)
//Set  输入/ 输出写指针
#define AMAUDIO_IOC_SET_I2S_IN_WR_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x0a, int)
#define AMAUDIO_IOC_SET_I2S_OUT_WR_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x0b, int)
//Get  输入/ 输出写指针
#define AMAUDIO_IOC_GET_I2S_IN_WR_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x0c, int)
#define AMAUDIO_IOC_GET_I2S_OUT_WR_PTR				_IOW(AMAUDIO_IOC_MAGIC, 0x0d, int)
//设置成单声道(左声道) 输出
#define AMAUDIO_IOC_SET_LEFT_MONO					_IOW(AMAUDIO_IOC_MAGIC, 0x0e, int)
//设置成单声道(右声道)  输出
#define AMAUDIO_IOC_SET_RIGHT_MONO					_IOW(AMAUDIO_IOC_MAGIC, 0x0f, int)
//设置成立体声输出
#define AMAUDIO_IOC_SET_STEREO						_IOW(AMAUDIO_IOC_MAGIC, 0x10, int)
//设置输出声道模式，默认是先左后右声道。
#define AMAUDIO_IOC_SET_CHANNEL_SWAP				_IOW(AMAUDIO_IOC_MAGIC, 0x11, int)
//卡拉OK 模式输出，也就是音频直接输出模式，由KARAOK_OFF 和KARAOK_ON控制
#define AMAUDIO_IOC_KARAOK_CTRL						_IOW(AMAUDIO_IOC_MAGIC, 0x12, int)
//音频直接输出左/ 右声道输出增益(0<arg<256)
#define AMAUDIO_IOC_DIRECT_LEFT_GAIN  				_IOW(AMAUDIO_IOC_MAGIC, 0x13, int)
#define AMAUDIO_IOC_DIRECT_RIGHT_GAIN 				_IOW(AMAUDIO_IOC_MAGIC, 0x14, int)

#define KARAOK_OFF	0
#define KARAOK_ON	1

#define M_AUDIO_THREAD_DELAY_TIME					4000

#define ANDROID_AUDIO_IN_SAMPLE_RATE				48000
#define ANDROID_AUDIO_OUT_SAMPLE_RATE				48000
#define AUDIO_BUFFER_SIZE							19200

#ifdef ANDROID 
static AudioRecord *Record = NULL;
static AudioTrack *Track = NULL;
#endif
static int amAudio_InHandle = -1;
static int amAudio_OutHandle = -1;
static int amAudio_InSize = -1;
static int amAudio_OutSize = -1;
static unsigned char *amAudio_BufAddr = NULL;
static unsigned int amAudio_Read = 0;
static unsigned int amAudio_Write = 0;
static pthread_t amAudio_ThreadID = 0;
static int amAudio_ThreadExitFlag = 0;

#ifdef WRITE_AUDIO_OUT_TO_FILE
static FILE *FileOut = NULL;
#endif

static void amAudio_AndroidInCallback (int event, void* user, void *info)
{
#ifdef ANDROID
    AudioRecord::Buffer *buffer = static_cast<AudioRecord::Buffer *>(info);

    if (event != AudioRecord::EVENT_MORE_DATA)
	{
        return;
    }	
    if (buffer == NULL || buffer->size == 0)
	{
		return;
    }

	//音频捕获实际由amAudio_InProc 接口实现，此处为了推动指针前进
#endif				
	return;
}

static int amAudio_AndroidInInit (void)
{
#ifdef ANDROID
	status_t Status;
	
	Record = new AudioRecord();
	
	if (Record == NULL)
	{	
		LOGI("The record creat failure !\n");
		return -1;
	}
	
	//创建录音对象，并设置录音格式

#ifdef Android_4_2	
	Status = Record->set (AUDIO_SOURCE_MIC,
			ANDROID_AUDIO_IN_SAMPLE_RATE,
			AUDIO_FORMAT_PCM_16_BIT,
			AUDIO_CHANNEL_IN_STEREO,
			0,
			amAudio_AndroidInCallback,
			NULL,
			0,
			true,
			0);
#else
	Status = Record->set (AUDIO_SOURCE_MIC,
			ANDROID_AUDIO_IN_SAMPLE_RATE,
			AUDIO_FORMAT_PCM_16_BIT,
			AUDIO_CHANNEL_IN_STEREO,
			0,
			0,
			amAudio_AndroidInCallback,
			NULL,
			0,
			true,
			0);
#endif
	if (Status != NO_ERROR)
	{
		LOGI("Set record parameters failture !\n");
		delete Record;
		Record = NULL;

		return -1;
	}

	Status = Record->initCheck();
	
	if (Status != NO_ERROR)
	{	
		LOGI("Record init Check failture !\n");
		delete Record;
		Record = NULL;

		return -1;
	}

	Status = Record->start();

	if (Status != NO_ERROR)
	{	
		LOGI("Record start failture !\n");
		delete Record;
		Record = NULL;

		return -1;
	}
#endif
	LOGI("Start record success\n");
	
	return 0;
}

static int amAudio_AndroidInFinish (void)
{
#ifdef ANDROID
	if (Record == NULL)
	{
		return -1;
	}

	Record->stop();
	
	//删除录音对象
	
	delete Record;
	Record = NULL;
#endif
	return 0;
}

static void amAudio_AndroidOutCallback (int event, void* user, void *info)
{
#ifdef ANDROID
    AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer *>(info);

    if (event != AudioTrack::EVENT_MORE_DATA)
	{	
        return;
    }
	
    if (buffer == NULL || buffer->size == 0)
	{
		return;
    }

	//清理音频输出，实际音频输出由amAudio_OutProc 接口实现，此处为了推动指针
	memset(buffer->i16, 0, buffer->size);
#endif			
	return;
}

static int amAudio_AndroidOutInit (void)
{
#ifdef ANDROID
	status_t Status;
	
	Track = new AudioTrack();
	if (Track == NULL)
	{
		return -1;
	}

	//创建音频Track, 并设置Audio Track 参数

#ifdef Android_4_2	
	Status = Track->set (AUDIO_STREAM_MUSIC,
			ANDROID_AUDIO_OUT_SAMPLE_RATE,
			AUDIO_FORMAT_PCM_16_BIT,
			AUDIO_CHANNEL_OUT_STEREO,
			0,
			AUDIO_OUTPUT_FLAG_NONE,
			amAudio_AndroidOutCallback,
			NULL,
			0,
			0,
			false,
			0);
#else
	Status = Track->set (AUDIO_STREAM_MUSIC,
			ANDROID_AUDIO_OUT_SAMPLE_RATE,
			AUDIO_FORMAT_PCM_16_BIT,
			AUDIO_CHANNEL_OUT_STEREO,
			0,
			0,
			amAudio_AndroidOutCallback,
			NULL,
			0,
			0,
			AUDIO_SESSION_OUTPUT_MIX);
#endif
	 
	if (Status != NO_ERROR)
	{
		delete Track;
		Track = NULL;

		return -1;
	}

	Status = Track->initCheck();
	if (Status != NO_ERROR)
	{
		delete Track;
		Track = NULL;

		return -1;
	}

	Track->start();
	
	Status = Track->setVolume(1.0, 1.0);
	if (Status != NO_ERROR)
	{
		delete Track;
		Track = NULL;

		return -1;
	}
#endif	
	LOGI("Start audio track success\n");
	
	return 0;
}

static int amAudio_AndroidOutFinish (void)
{
#ifdef ANDROID
	if (Track == NULL)
	{
		return -1;
	}

	Track->stop();

	//删除Audio Tack 对象
	
	delete Track;
	Track = NULL;
#endif
	return 0;
}

static int amAudio_AndroidInit (void)
{
	int Ret;

	Ret = amAudio_AndroidInInit();
		
	if (Ret == 0)
	{
		Ret = amAudio_AndroidOutInit();
		if (Ret == 0)
		{
			return 0;
		}
		else
		{
			amAudio_AndroidInFinish();
			
			return -1;
		}
	}
	return Ret;
}

static int amAudio_AndroidFinish (void)
{
	amAudio_AndroidOutFinish();
	amAudio_AndroidInFinish();
	
	return 0;
}

static int amAudio_InInit(void)
{
	unsigned int hwptr;
	
	hwptr = ioctl (amAudio_InHandle, AMAUDIO_IOC_GET_I2S_IN_PTR);
	if (hwptr == 0xFFFFFFFF)
	{
		LOGI("amAudio_InInit Error!\n");
		return -1;
	}
	//获取输入硬件指针，并将应将指针指向输入读指针
    ioctl (amAudio_InHandle, AMAUDIO_IOC_SET_I2S_IN_RD_PTR, hwptr);

	amAudio_Read = 0;
	amAudio_Write = 0;

    return 0;
}

static int amAudio_OutInit(void)
{
	unsigned int hwptr;

	//获取输出硬件指针
	hwptr= ioctl (amAudio_OutHandle, AMAUDIO_IOC_GET_I2S_OUT_PTR);
	if (hwptr == 0xFFFFFFFF)
	{
		LOGI("amAudio_OutInit Error!\n");
		return -1;
	}
	
	hwptr = (hwptr + 14400) % amAudio_OutSize;

	hwptr = hwptr >> 6 << 6;

	
	ioctl (amAudio_OutHandle, AMAUDIO_IOC_SET_I2S_OUT_WR_PTR, hwptr);

	return 0;
}

static int amAudio_InProc(void)
{
	int Ret, ProcSize, WritePos, WriteSize;
	int hwptr;
	int rdptr;
	int Bytes;

	//获取硬件输入指针
	if(amAudio_InHandle == -1){
		LOGE("amAudio_InHandle error: %d\n", -1);
	}
		
	hwptr = ioctl (amAudio_InHandle, AMAUDIO_IOC_GET_I2S_IN_PTR);
	if (hwptr == 0xFFFFFFFF)
	{
		return -1;
	}
	
	//获取输入读指针
	rdptr = ioctl (amAudio_InHandle, AMAUDIO_IOC_GET_I2S_IN_RD_PTR);
	
	Bytes = (hwptr - rdptr + amAudio_InSize) % amAudio_InSize;
		
	Bytes = Bytes >> 2 << 2;
	if (Bytes == 0)
	{
		return 0;
	}
	
	if (rdptr < hwptr)
	{	
		ProcSize = Bytes;

		WriteSize = AUDIO_BUFFER_SIZE - (amAudio_Write - amAudio_Read);
		if (WriteSize > ProcSize)
		{
			WriteSize = ProcSize;
		}
		WritePos = amAudio_Write % AUDIO_BUFFER_SIZE;

		if (WritePos + WriteSize > AUDIO_BUFFER_SIZE)
		{
			Ret = read (amAudio_InHandle, amAudio_BufAddr + WritePos, AUDIO_BUFFER_SIZE - WritePos);
			if (Ret != AUDIO_BUFFER_SIZE - WritePos)
			{
				LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
				return -1;
			}
			amAudio_Write += Ret;
			Ret = read (amAudio_InHandle, amAudio_BufAddr, WriteSize - (AUDIO_BUFFER_SIZE - WritePos));
			if (Ret != WriteSize - (AUDIO_BUFFER_SIZE - WritePos))
			{
				LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
				return -1;
			}
			amAudio_Write += Ret;
		}
		else
		{
			Ret = read (amAudio_InHandle, amAudio_BufAddr + WritePos, WriteSize);
			if (Ret != WriteSize)
			{
				LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
				return -1;
			}
			amAudio_Write += Ret;
		}
	}
	else
	{
		ProcSize = amAudio_InSize - rdptr;
		WriteSize = AUDIO_BUFFER_SIZE - (amAudio_Write - amAudio_Read);
		if (WriteSize > ProcSize)
		{
			WriteSize = ProcSize;
		}
		WritePos = amAudio_Write % AUDIO_BUFFER_SIZE;

		if (WritePos + WriteSize > AUDIO_BUFFER_SIZE)
		{
			Ret = read (amAudio_InHandle, amAudio_BufAddr + WritePos, AUDIO_BUFFER_SIZE - WritePos);
			if (Ret != AUDIO_BUFFER_SIZE - WritePos)
			{
				LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
				return -1;
			}
			amAudio_Write += Ret;
			Ret = read (amAudio_InHandle, amAudio_BufAddr, WriteSize - (AUDIO_BUFFER_SIZE - WritePos));
			if (Ret != WriteSize - (AUDIO_BUFFER_SIZE - WritePos))
			{
				LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
				return -1;
			}
			amAudio_Write += Ret;
		}
		else
		{
			Ret = read (amAudio_InHandle, amAudio_BufAddr + WritePos, WriteSize);
			if (Ret != WriteSize)
			{
				LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
				return -1;
			}
			amAudio_Write += Ret;
		}

		ProcSize = Bytes - ProcSize;
		if (ProcSize > 0)
		{
			WriteSize = AUDIO_BUFFER_SIZE - (amAudio_Write - amAudio_Read);
			if (WriteSize > ProcSize)
			{
				WriteSize = ProcSize;
			}
			WritePos = amAudio_Write % AUDIO_BUFFER_SIZE;
			
			if (WritePos + WriteSize > AUDIO_BUFFER_SIZE)
			{
				Ret = read (amAudio_InHandle, amAudio_BufAddr + WritePos, AUDIO_BUFFER_SIZE - WritePos);
				if (Ret != AUDIO_BUFFER_SIZE - WritePos)
				{
					LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
					return -1;
				}
				amAudio_Write += Ret;
				Ret = read (amAudio_InHandle, amAudio_BufAddr, WriteSize - (AUDIO_BUFFER_SIZE - WritePos));
				if (Ret != WriteSize - (AUDIO_BUFFER_SIZE - WritePos))
				{
					LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
					return -1;
				}
				amAudio_Write += Ret;
			}
			else
			{
				Ret = read (amAudio_InHandle, amAudio_BufAddr + WritePos, WriteSize);
				if (Ret != WriteSize)
				{
					LOGI("amAudio_InProc Error Line No = %d!\n", __LINE__);
					return -1;
				}
				amAudio_Write += Ret;
			}
		}
	}

	return 0;
}

static int amAudio_OutProc(void)
{
	int hwptr;
	int wrptr;
	int Bytes, ProcSize;
	int ReadPos, ReadSize;
	int Ret;
	
	hwptr = ioctl (amAudio_OutHandle, AMAUDIO_IOC_GET_I2S_OUT_PTR);
	if (hwptr == 0xFFFFFFFF)
	{
		return -1;
	}

	hwptr = (hwptr + 14400) % amAudio_OutSize;
	hwptr = hwptr >> 6 << 6;
	wrptr = ioctl (amAudio_OutHandle, AMAUDIO_IOC_GET_I2S_OUT_WR_PTR);
	
	Bytes = (hwptr - wrptr + amAudio_OutSize) % amAudio_OutSize;

	if (Bytes > 9600)
	{
		wrptr = (hwptr + amAudio_OutSize - 1920) % amAudio_OutSize;
		wrptr = wrptr >> 6 << 6;
		ioctl (amAudio_OutHandle, AMAUDIO_IOC_SET_I2S_OUT_WR_PTR, wrptr);
		Bytes = 1920;
	}
	
	while (amAudio_Write - amAudio_Read > 15360)					// 48 * 80 * 4
	{
		amAudio_Read += 4800;
	}

	if (Bytes == 0)
	{
		return 0;
	}

	if (wrptr < hwptr)
	{
		ProcSize = Bytes;
		ReadSize = amAudio_Write - amAudio_Read;
		if (ReadSize > ProcSize)
		{
			ReadSize = ProcSize;
		}
		
		ReadSize = ReadSize >> 6 << 6;
		if (ReadSize == 0)
		{
			return 0;
		}

		ReadPos = amAudio_Read % AUDIO_BUFFER_SIZE;
		if (ReadPos + ReadSize > AUDIO_BUFFER_SIZE)
		{
			Ret = write (amAudio_OutHandle, amAudio_BufAddr + ReadPos, AUDIO_BUFFER_SIZE - ReadPos);
			if (Ret != AUDIO_BUFFER_SIZE - ReadPos)
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, AUDIO_BUFFER_SIZE - ReadPos, Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr + ReadPos, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
			Ret = write (amAudio_OutHandle, amAudio_BufAddr, ReadSize - (AUDIO_BUFFER_SIZE - ReadPos));
			if (Ret != ReadSize - (AUDIO_BUFFER_SIZE - ReadPos))
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, ReadSize - (AUDIO_BUFFER_SIZE - ReadPos), Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
		}
		else
		{
			Ret = write (amAudio_OutHandle, amAudio_BufAddr + ReadPos, ReadSize);
			if (Ret != ReadSize)
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, ReadSize, Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr + ReadPos, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
		}
	}
	else
	{
		ProcSize = amAudio_OutSize - wrptr;
		ReadSize = amAudio_Write - amAudio_Read;
		if (ReadSize > ProcSize)
		{
			ReadSize = ProcSize;
		}
		
		ReadSize = ReadSize >> 6 << 6;
		if (ReadSize == 0)
		{
			return 0;
		}

		ReadPos = amAudio_Read % AUDIO_BUFFER_SIZE;
		if (ReadPos + ReadSize > AUDIO_BUFFER_SIZE)
		{
			Ret = write (amAudio_OutHandle, amAudio_BufAddr + ReadPos, AUDIO_BUFFER_SIZE - ReadPos);
			if (Ret != AUDIO_BUFFER_SIZE - ReadPos)
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, AUDIO_BUFFER_SIZE - ReadPos, Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr + ReadPos, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
			Ret = write (amAudio_OutHandle, amAudio_BufAddr, ReadSize - (AUDIO_BUFFER_SIZE - ReadPos));
			if (Ret != ReadSize - (AUDIO_BUFFER_SIZE - ReadPos))
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, ReadSize - (AUDIO_BUFFER_SIZE - ReadPos), Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
		}
		else
		{
			Ret = write (amAudio_OutHandle, amAudio_BufAddr + ReadPos, ReadSize);
			if (Ret != ReadSize)
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, ReadSize, Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr + ReadPos, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
		}

		ProcSize = Bytes - (amAudio_OutSize - wrptr);
		ReadSize = amAudio_Write - amAudio_Read;
		if (ReadSize > ProcSize)
		{
			ReadSize = ProcSize;
		}
		
		ReadSize = ReadSize >> 6 << 6;
		if (ReadSize == 0)
		{
			return 0;
		}

		ReadPos = amAudio_Read % AUDIO_BUFFER_SIZE;
		if (ReadPos + ReadSize > AUDIO_BUFFER_SIZE)
		{
			Ret = write (amAudio_OutHandle, amAudio_BufAddr + ReadPos, AUDIO_BUFFER_SIZE - ReadPos);
			if (Ret != AUDIO_BUFFER_SIZE - ReadPos)
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, AUDIO_BUFFER_SIZE - ReadPos, Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr + ReadPos, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
			Ret = write (amAudio_OutHandle, amAudio_BufAddr, ReadSize - (AUDIO_BUFFER_SIZE - ReadPos));
			if (Ret != ReadSize - (AUDIO_BUFFER_SIZE - ReadPos))
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, ReadSize - (AUDIO_BUFFER_SIZE - ReadPos), Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
		}
		else
		{
			Ret = write (amAudio_OutHandle, amAudio_BufAddr + ReadPos, ReadSize);
			if (Ret != ReadSize)
			{
				LOGI("amAudio_OutProc Error Line No = %d ReadSize = %d Ret = %d!\n", __LINE__, ReadSize, Ret);
				return -1;
			}
#ifdef WRITE_AUDIO_OUT_TO_FILE
			if (FileOut != NULL)
			{
				fwrite (amAudio_BufAddr + ReadPos, 1, Ret, FileOut);
			}
#endif
			amAudio_Read += Ret;
		}

	}

	return 0;
}


static void *amAudio_Thread(void *data)
{
	int Ret;
	int Status;
	
	Status = 0;
	
	while (1)
	{
		if (amAudio_ThreadExitFlag == 1)
		{
			amAudio_ThreadExitFlag = 0;
			break;
		}

		switch (Status)
		{
			case 0:
				Ret = amAudio_InInit();
				if (Ret == 0)
				{
					Status = 1;
				}
				break;
			case 1:
				Ret = amAudio_OutInit();
				if (Ret == 0)
				{
					Status = 2;
				}
				break;
			case 2:
				Ret = amAudio_InProc();
				
				if (Ret == 0)
				{
					Ret = amAudio_OutProc();
					if (Ret != 0)
					{
						Status = 0;
					}
				}
				else
				{
					Status = 0;
				}
				break;
		}

		usleep (M_AUDIO_THREAD_DELAY_TIME);
	}

	pthread_exit (0);
	
	return ((void*)0);
}

static int amAudio_AmlogicInit (void)
{
	pthread_attr_t		attr;
	struct sched_param	param;
	int					Ret;
	
	//打开输入输出设备
	amAudio_InHandle = open (AUDIO_IN, O_RDWR);
	
	if (amAudio_InHandle < 0)
	{
		LOGI("The device amaudio_in cant't be opened!\n");
		return -1;
	}

	amAudio_OutHandle = open (AUDIO_OUT, O_RDWR);
	if (amAudio_OutHandle < 0)
	{
		close (amAudio_InHandle);
		amAudio_InHandle = -1;
		LOGI("The device amaudio_out cant't be opened!\n");
		return -1;
	}

	//设置音频模式以及参数
	
	ioctl (amAudio_InHandle, AMAUDIO_IOC_SET_I2S_IN_MODE, 1);
	ioctl (amAudio_OutHandle, AMAUDIO_IOC_SET_I2S_OUT_MODE, 1);

	ioctl (amAudio_InHandle, AMAUDIO_IOC_KARAOK_CTRL, KARAOK_ON);

	
#ifdef TEST_AUDIO_IN_DIRECT_OUT
	ioctl (amAudio_InHandle, AMAUDIO_IOC_DIRECT_LEFT_GAIN, 256);
	ioctl (amAudio_InHandle, AMAUDIO_IOC_DIRECT_RIGHT_GAIN, 256);
#else 
	//如果希望在录音写文件的同时音频直通，把麦克混音比例打开(0-256)
	ioctl (amAudio_InHandle, AMAUDIO_IOC_DIRECT_LEFT_GAIN, 0);
	ioctl (amAudio_InHandle, AMAUDIO_IOC_DIRECT_RIGHT_GAIN, 0);
#endif

#ifndef TEST_AUDIO_IN_DIRECT_OUT

	//获取输入输出数据大小
	amAudio_InSize = ioctl (amAudio_InHandle, AMAUDIO_IOC_GET_I2S_IN_SIZE, 0);
	if (amAudio_InSize == -1)
	{
		close (amAudio_OutHandle);
		amAudio_OutHandle = -1;

		close (amAudio_InHandle);
		amAudio_InHandle = -1;
		return -1;
	}
	
	amAudio_OutSize = ioctl (amAudio_OutHandle, AMAUDIO_IOC_GET_I2S_OUT_SIZE, 0);
	if (amAudio_OutSize == -1)
	{
		close (amAudio_OutHandle);
		amAudio_OutHandle = -1;

		close (amAudio_InHandle);
		amAudio_InHandle = -1;
		return -1;
	}
	
	//分配amAudio buffer 空间
	amAudio_BufAddr = (unsigned char *)malloc (AUDIO_BUFFER_SIZE);
	if (amAudio_BufAddr == NULL)
	{
		close (amAudio_OutHandle);
		amAudio_OutHandle = -1;

		close (amAudio_InHandle);
		amAudio_InHandle = -1;
		return -1;
	}

	amAudio_Read = 0;
	amAudio_Write = 0;
	
	//建立线程
	pthread_attr_init (&attr);
	pthread_attr_setschedpolicy (&attr, SCHED_RR);
	param.sched_priority = sched_get_priority_max (SCHED_RR);
	pthread_attr_setschedparam (&attr, &param);
	amAudio_ThreadExitFlag = 0;
	Ret = pthread_create (&amAudio_ThreadID, &attr, &amAudio_Thread, NULL);
	pthread_attr_destroy (&attr);

	if (Ret != 0)
	{
		free(amAudio_BufAddr);
		amAudio_BufAddr = NULL;
		
		close (amAudio_OutHandle);
		amAudio_OutHandle = -1;

		close (amAudio_InHandle);
		amAudio_InHandle = -1;
		return -1;
	}
#endif
	return 0;
}

int amAudio_AmlogicFinish (void)
{
#ifndef TEST_AUDIO_IN_DIRECT_OUT
	amAudio_ThreadExitFlag = 1;
	pthread_join(amAudio_ThreadID, NULL);
	
	free(amAudio_BufAddr);
	amAudio_BufAddr = NULL;
#endif

	ioctl (amAudio_InHandle, AMAUDIO_IOC_KARAOK_CTRL, KARAOK_OFF);
	
	close (amAudio_OutHandle);
	amAudio_OutHandle = -1;
	
	close (amAudio_InHandle);
	amAudio_InHandle = -1;

	return 0;
}

int amAudio_Init (void)
{
	int Ret;

#ifdef MultiSoundCard

#ifdef TINYALSA_LIB
		line_in_select_channel();
#else

	snd_ctl_set ((char *)"Audio In Source", 0);
	snd_ctl_set ((char *)"Left LINEIN Select", 6);
	snd_ctl_set ((char *)"Right LINEIN Select", 6);
#endif

#endif

	LOGI("------------Test start!----------------------\n");

#ifdef WRITE_AUDIO_OUT_TO_FILE
	FileOut = fopen ("/data/test.pcm", "wb");
	if(FileOut == NULL)
		LOGI("The file can't be opened!\n");
#endif
	
	Ret = amAudio_AndroidInit ();

	if (Ret == 0)
	{
		Ret = amAudio_AmlogicInit ();

		if (Ret == 0)
		{
			return 0;
		}
		else
		{
			amAudio_AndroidFinish ();
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

int amAudio_Finish (void)
{
	amAudio_AmlogicFinish ();
	amAudio_AndroidFinish ();

	LOGI("------------Test finish!----------------------\n");
	
#ifdef WRITE_AUDIO_OUT_TO_FILE
	if (FileOut != NULL)
	{
		fclose (FileOut);
		FileOut = NULL;
	}
#endif
	
	return 0;
}

int main (void)
{
	amAudio_Init ();
	while (1);
	//sleep(200);
	amAudio_Finish ();

	return 0;
}
