
typedef void (*play_callback)(void);

static play_callback play_func = NULL;

void PlayFunction(void)
{

	audsrv_wait_audio(mSoundBufferSize);
	audsrv_play_audio((char*)mSoundBuffer[mSb],mSoundBufferSize);

	if(mSb+1>=SOUND_BUFFER_COUNT) mSb=0;
	else mSb++;
}

void SoundThread()
{

	while(1)
	{
		PlayFunction();
	}

}

s32 sal_AudioInit(s32 rate, s32 bits, s32 stereo, s32 Hz)
{
	audsrv_fmt_t format;

	format.channels = stereo + 1;
	format.bits = bits;
	format.freq = rate;

	mSoundSampleCount=(rate/Hz) * format.channels;
	mSoundBufferSize=mSoundSampleCount*((bits==16)?2:1);

	audsrv_set_format(&format);

	//play_func = PlayFunction;
	printf("AUDIO_INITIALIZED\n");
	return SAL_OK;
}


void sal_AudioClose(void)
{
	printf("AUDIO_CLOSED\n");
	//play_func = NULL;

	audsrv_set_volume(0);
	audsrv_stop_audio();

	//DeleteThread(sound_thread_id);

}

void sal_AudioSetVolume(s32 l, s32 r)
{
	s32 volume = (100 * l) / 31;

	audsrv_set_volume(volume);
}

