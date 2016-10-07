static u16 video_buffer[SAL_SCREEN_WIDTH*SAL_SCREEN_HEIGHT] __attribute__((aligned(128)));

u32 sal_VideoInit(u32 bpp, u32 color, u32 refreshRate)
{
	settings_t settings = settings_get();

	int mode = settings.display.mode;

	if (mode == GRAPH_MODE_HDTV_1080I)
	{
		video_init_screen(0,0,SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT,1,mode);
	}
	else
	{
		video_init_screen(0,0,SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT,0,mode);
	}

	video_init_draw_env(SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT);

	//video_send_packet(SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT,(void*)&video_buffer[0],NULL);
	video_draw_packet(SAL_SCREEN_WIDTH,SAL_SCREEN_HEIGHT,GS_PSM_16S,0);

	return SAL_OK;
}

void sal_VideoFlip(s32 vsync)
{

	video_send_texture();
	video_draw_texture();

	video_sync_flip();
	//video_sync_wait();
}

u32 sal_VideoSetScaling(s32 width, s32 height)
{
	settings_t settings = settings_get();

	int mode = settings.display.mode;

	if (mode == GRAPH_MODE_HDTV_1080I)
	{
		video_init_screen(0,0,width,height,1,mode);
	}
	else
	{
		video_init_screen(0,0,width,height,0,mode);
	}

	video_init_draw_env(width,height);

	video_send_packet(width,height,(void*)&video_buffer[0],NULL);
	video_draw_packet(width,height,GS_PSM_16S,0);

	printf("screen = %dx%d\n",width,height);

	return SAL_OK;
}

void *sal_VideoGetBuffer()
{
	return (void*)&video_buffer[0];
}

void sal_VideoPaletteSync()
{
	//TODO
}

void sal_VideoPaletteSet(u32 index, u32 color)
{
	mPaletteBuffer[index]=color;
}

