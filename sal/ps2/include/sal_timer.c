#include <sys/time.h>

// clocks per second / hz
static u32 mTimerFreq=576000/60;

u32 sal_TimerRead()
{
	u32 timer_cnt = clock()/mTimerFreq;
	return timer_cnt;
}

s32 sal_TimerInit(s32 freq)
{
	mTimerFreq=(576000)/freq;
	return SAL_OK;
}

void sal_TimerClose(void)
{
}

