#include <kernel.h>

#include <stdio.h>
#include <png.h>
#include <sys/stat.h>

#include <audsrv.h>
#include <fileXio_rpc.h>

#include <input.h>
#include <graph.h>
#include <gs_psm.h>

#include "hdd.h"
#include "init.h"
#include "interface.h"
#include "lists.h"
#include "settings.h"
#include "video.h"

#include "sal.h"
#include "unzip.h"
#include "zip.h"

#define SAL_FRAME_BUFFER_COUNT	4
#define SOUND_BUFFER_COUNT 	8
#define AUDIO_RATE_COUNT	5
#define MAX_SOUND_LEN 	((48000/60)*2)

extern void *_gp;
static pad_t *pad;

static u32 mSoundSampleCount=0;
static u32 mSoundBufferSize=0;
static u32 mPaletteBuffer[0x100];
static u32 mInputFirst=0;

s32 mAudioRateLookup[AUDIO_RATE_COUNT]={8250,11025,22050,44100,48000};

#include "sal_common.c"
#include "sal_sound.c"
#include "sal_timer.c"
#include "sal_video.c"

s32 sal_DirectoryGetCurrent(s8 *path, u32 size)
{

	// Used only once for the system directory
	settings_t settings = settings_get();

	strcpy(path,settings.home.directory);
	//strcpy(path,"mc0:/");

	//printf("sal path is %s\n", path);

	return SAL_OK;
}

s32 sal_DirectoryGetItemCount(s8 *path, s32 *returnItemCount)
{
	u32 count=1; //always include parent dir entry
	int fd;
	iox_dirent_t buf;

	fd = fileXioDopen(path);

	if (fd > 0)
	{
		while (fileXioDread(fd, &buf) > 0)
		{
			if(buf.stat.mode & FIO_S_IFDIR && (!strcmp(buf.name,".") || !strcmp(buf.name,"..")))
			{
				continue;
			}

			count++;
		}
		fileXioDclose(fd);
	}

	*returnItemCount=count;

	return SAL_OK;
}

// not used
s32 sal_DirectoryGet(s8 *path, struct SAL_DIRECTORY_ENTRY *dir, s32 startIndex, s32 count)
{
	return SAL_ERROR;
}

s32 sal_DirectoryOpen(s8 *path, struct SAL_DIR *d)
{
	printf("path is %s\n",path);

	if(path[0] == 0)
	{
		//special case to get list of drives
		d->needParent=0;
		d->drivesRead=0;
		strcpy((char*)d->path,path);
		return SAL_OK;
	}
	else
	{
		d->fd=fileXioDopen(path);
		d->needParent=1;
		strcpy((char*)d->path,path);
		if(d->fd > 0)
		{
			return SAL_OK;
		}
		else
		{
			printf("Error!\n");
			return SAL_ERROR;
		}
	}

	return SAL_OK;
}

s32 sal_DirectoryClose(struct SAL_DIR *d)
{
	if(d->path[0] == 0)
	{
		d->fd = 0;
		d->path[0]=0;
		return SAL_OK;
	}
	else
	{
		if(d)
		{
			if(d->fd > 0)
			{
				fileXioDclose(d->fd);
				d->fd = 0;
				d->path[0]=0;
				return SAL_OK;
			}
			else
			{
				return SAL_ERROR;
			}
		}
		else
		{
			return SAL_ERROR;
		}
	}

	return SAL_OK;
}

s32 sal_DirectoryRead(struct SAL_DIR *d, struct SAL_DIRECTORY_ENTRY *dir)
{
	int ret = 0;
	iox_dirent_t buf;
	settings_t settings = settings_get();

	if(d->path[0] == 0)
	{
		switch(d->drivesRead)
		{
			case 0:
				strcpy(dir->filename,"mc0:/");
				strcpy(dir->displayName,"mc0:/");
				dir->type=SAL_FILE_TYPE_DIRECTORY;
				d->drivesRead++;
				return SAL_OK;
			case 1:
				strcpy(dir->filename,"mc1:/");
				strcpy(dir->displayName,"mc1:/");
				dir->type=SAL_FILE_TYPE_DIRECTORY;
				d->drivesRead++;
				return SAL_OK;
			case 2:
				strcpy(dir->filename,"mass:/");
				strcpy(dir->displayName,"mass:/");
				dir->type=SAL_FILE_TYPE_DIRECTORY;
				d->drivesRead++;
				return SAL_OK;
			case 3:
				if (!strncmp(settings.home.directory,"pfs0:",5))
				{
					strcpy(dir->filename,"pfs0:/");
					strcpy(dir->displayName,"pfs0:/");
					dir->type=SAL_FILE_TYPE_DIRECTORY;
					d->drivesRead++;
					return SAL_OK;
				}
				else
				{
					return SAL_ERROR;
				}
			case 4:
				return SAL_ERROR;
		}
		return SAL_OK;
	}
	else
	{
		if(d)
		{
			//if(d->needParent)
			//{
			//	strcpy(dir->filename,"..");
			//	strcpy(dir->displayName,"..");
			//	dir->type=SAL_FILE_TYPE_DIRECTORY;
			//	d->needParent=0;
			//	return SAL_OK;
			//}
			//else
			{
				if(dir)
				{
					ret = fileXioDread(d->fd, &buf);
					if (ret > 0)
					{
						strcpy(dir->filename,buf.name);
						strcpy(dir->displayName,buf.name);
						if (buf.stat.mode & FIO_S_IFDIR)
						{
							dir->type=SAL_FILE_TYPE_DIRECTORY;
						}
						else
						{
							dir->type=SAL_FILE_TYPE_FILE;
						}
						return SAL_OK;
					}
					else
					{
						return SAL_ERROR;
					}
				}
				else
				{
					return SAL_ERROR;
				}
			}
		}
		else
		{
			return SAL_ERROR;
		}
	}
}

void sal_DirectoryGetParent(s8 *path)
{

	printf("parent path is %s\n", path);

	s32 i=0;
	s32 lastDir=-1, firstDir=-1;
	s8 dirSep[2] = {SAL_DIR_SEP};
	s8 dirSepBad[2] = {SAL_DIR_SEP_BAD};
	s32 len=(s32)strlen(path);

	for(i=0;i<len;i++)
	{
		if ((path[i] == dirSep[0]) || (path[i] == dirSepBad[0]))
		{
			//Directory seperator found
			if(lastDir==-1) firstDir = i;
			if(i+1 != len) lastDir = i;
		}
	}

	if (lastDir == firstDir) lastDir++;
	if (lastDir >= 0)
	{
		for(i=lastDir; i<len; i++)
		{
			path[i]=0;
		}
	}
	else
	{
		path[0]=0;
	}
}

u32 sal_InputPoll()
{
	int i;
	u32 inputHeld=0;
	u32 timer = 0;

	pad_get_buttons(pad);

	inputHeld = pad->buttons->btns ^ 0xFFFF;

	timer=sal_TimerRead();
	for (i=0;i<32;i++)
	{
		if (inputHeld&(1<<i))
		{
			if(mInputFirst&(1<<i))
			{
				if (mInputRepeatTimer[i]<timer)
				{
					mInputRepeat|=1<<i;
					mInputRepeatTimer[i]=timer+10;
				}
				else
				{
					mInputRepeat&=~(1<<i);
				}
			}
			else
			{
				//First press of button
				//set timer to expire later than usual
				mInputFirst|=(1<<i);
				mInputRepeat|=1<<i;
				mInputRepeatTimer[i]=timer+50;
			}
		}
		else
		{
			mInputRepeatTimer[i]=timer-10;
			mInputRepeat&=~(1<<i);
			mInputFirst&=~(1<<i);
		}

	}

	if(mInputIgnore)
	{
		//A request to ignore all key presses until all keys have been released has been made
		//check for release and clear flag, otherwise clear inputHeld and mInputRepeat
		if (inputHeld == 0)
		{
			mInputIgnore=0;
		}
		inputHeld=0;
		mInputRepeat=0;
	}

	return inputHeld;
}

void sal_CpuSpeedSet(u32 mhz)
{
	//Screws with TV out mode so disabling cpu scaling for the moment
	//u16 tempCore=mhz*1000000;
	//cpu_clock_set(tempCore);
}

u32 sal_CpuSpeedNext(u32 currSpeed)
{
	u32 newSpeed=currSpeed+1;
	if(newSpeed > 500) newSpeed = 500;
	return newSpeed;
}

u32 sal_CpuSpeedPrevious(u32 currSpeed)
{
	u32 newSpeed=currSpeed-1;
	if(newSpeed > 500) newSpeed = 0;
	return newSpeed;
}

u32 sal_CpuSpeedNextFast(u32 currSpeed)
{
	u32 newSpeed=currSpeed+10;
	if(newSpeed > 500) newSpeed = 500;
	return newSpeed;
}

u32 sal_CpuSpeedPreviousFast(u32 currSpeed)
{
	u32 newSpeed=currSpeed-10;
	if(newSpeed > 500) newSpeed = 0;
	return newSpeed;
}

#if 1
#define ALARM_INTERVAL 70
//#define ALARM_INTERVAL 625

static u8 sound_thread_stack[0x2000] __attribute__ ((aligned(16)));
static int sound_thread_id = -1;

static u8 dispatcher_thread_stack[0x2000] __attribute__ ((aligned(16)));
static int dispatcher_thread_id = -1;
// original
void dispatcher(void* arg)
{
	while(1)
	{
		iRotateThreadReadyQueue(30);
		SleepThread();
	}
}

void alarmfunction(s32 id, u16 time, void *arg)
{
	iWakeupThread(dispatcher_thread_id);
	iSetAlarm(ALARM_INTERVAL,alarmfunction,NULL);
}
#endif
s32 sal_Init(void)
{
	ee_thread_t thread;
	settings_t settings;

	settings = settings_get();

	video_packets_init();

	video_init_framebuffer(SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT);

	video_init_texbuffer(SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT,GS_PSM_16S,0);

	sal_TimerInit(60);
	memset(mInputRepeatTimer,0,sizeof(mInputRepeatTimer));

	pad = pad_open(settings.input.port,settings.input.slot,MODE_DIGITAL,MODE_UNLOCKED);

	SetAlarm(ALARM_INTERVAL, alarmfunction, NULL);
	ChangeThreadPriority(GetThreadId(), 29);

	thread.func = (void*)dispatcher;
	thread.stack = (void*)dispatcher_thread_stack;
	thread.stack_size = 0x2000;
	thread.gp_reg = &_gp;
	thread.initial_priority = 0;
	dispatcher_thread_id = CreateThread(&thread);
	StartThread (dispatcher_thread_id, NULL);

	thread.func = (void*)SoundThread;
	thread.stack = (void*)sound_thread_stack;
	thread.stack_size = 0x40000;
	thread.gp_reg = &_gp;
	thread.initial_priority = 30;
	sound_thread_id = CreateThread(&thread);
	StartThread(sound_thread_id, NULL);

	ChangeThreadPriority(GetThreadId(),30);
	printf("Pad (%d,%d) is open\n", settings.input.port,settings.input.slot);

	return SAL_OK;
}

void sal_Reset(void)
{
	sal_AudioClose();
}

int mainEntry(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	settings_t settings;

	parse_args(argc,argv);

	init("pocketsnes.cfg");

	init_sound_modules(NULL);

	settings = settings_get();

	strcpy(argv[0],"");

    printf("main entry\n");
	mainEntry(0,NULL);

	__asm__ __volatile__
	(
		"	li $3, 0x04;"
		"	syscall;"
		"	nop;"
	);

	return 0;
}
