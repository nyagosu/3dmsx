//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

#include <3DS.h>
#include <string.h>

#include "emu2212.h"
#include "emu2149.h"
#include "emu2413.h"

PSG * psg;
SCC * scc;

#define MSX_CLK 3579545
#define SOUNDBUFSIZ ((u16)(1024))   //367
int samplerate = 0x2000;	// 8kHz

s16 * soundbuf_psg[2];
s16 * soundbuf_scc[2];
int SoundPtr    = 0; // データ書込位置
int SoundBufCnt = 0; // データ書込バッファ
volatile int SoundProcTiming = 0;
int SoundPause = true;
int SoundInit  = false;
int psgON      = true;
int sccON      = true;
int adrflg = 0;

void InitSound(void)
{
	printf( "InitSound\r\n" );

/*
	SoundPause = true;
	SoundInit  = false;
	REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);

	// Sound buffer setting
	soundbuf_psg[0] = (s16*)VRAM;
	soundbuf_psg[1] = (s16*)VRAM + SOUNDBUFSIZ    ;
	soundbuf_scc[0] = (s16*)VRAM + SOUNDBUFSIZ * 2;
	soundbuf_scc[1] = (s16*)VRAM + SOUNDBUFSIZ * 3;

	irqSet(IRQ_TIMER0, IrqTimer0 );
*/
}

void ExitSound(void)
{
	printf( "ExitSound\r\n" );
}

void StartSound(void)
{

}

void ResetSound(void)
{
/*
	StopSound();

	// PSG initialize
	PSG_new(psg, MSX_CLK,samplerate);
	PSG_reset( psg );

	// SCC initialize
	SCC_new(scc, MSX_CLK,samplerate);
	SCC_reset( scc );
*/
}

static int _range( int num, int min, int max ){
	return (num>max)?num=max:(num<min)?num=min:num;
}

void CalcSound( void )
{
/*
	if( SoundProcTiming > 0 ){
		if( psgON ){
			s16 * P = soundbuf_psg[SoundPtr] + SoundBufCnt;
			*P = (s16)_range( (int)PSG_calc(psg)*2,-32768, 32767 );
		}
		if( sccON ){
			s16 * S = soundbuf_scc[SoundPtr] + SoundBufCnt;
			*S = (s16)_range( (int)SCC_calc(scc)*2,-32768, 32767 );
		}
		SoundBufCnt++;
		SoundProcTiming --;

		if( SoundBufCnt >= SOUNDBUFSIZ ) {
			// Set Sound Register
			SCHANNEL_CR(SoundPtr*2  ) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(63) | SOUND_FORMAT_16BIT;
			SCHANNEL_CR(SoundPtr*2+1) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(63) | SOUND_FORMAT_16BIT;

			SoundPtr = SoundPtr==0?1:0;

			SCHANNEL_CR(SoundPtr*2  ) = 0;
			SCHANNEL_CR(SoundPtr*2+1) = 0;

			SoundBufCnt = 0;
		}
	}
*/
	return;
}

void PlaySound()
{
/*
	int i;

	SoundPause = false;
	SoundProcTiming = 0;
	SoundPtr        = 0;
	SoundBufCnt     = 0;

	// clear sound buffer
	for(i=0;i<SOUNDBUFSIZ*4;i++ ){
		*((s16*)VRAM+i) = 0;
	}

	// Sound REG setting
	for(i=0;i<4;i++){
		SCHANNEL_CR(i)           = 0;
		SCHANNEL_TIMER(i)        = SOUND_FREQ(samplerate);
		SCHANNEL_LENGTH(i)       = SOUNDBUFSIZ*2 >> 2;
		SCHANNEL_REPEAT_POINT(i) = 0;
	}

	SCHANNEL_SOURCE(0) = (uint32)soundbuf_psg[0];
	SCHANNEL_SOURCE(1) = (uint32)soundbuf_scc[0];
	SCHANNEL_SOURCE(2) = (uint32)soundbuf_psg[1];
	SCHANNEL_SOURCE(3) = (uint32)soundbuf_scc[1];

	// Timer setting
	TIMER0_DATA = TIMER_FREQ(samplerate);
	TIMER0_CR = TIMER_DIV_1   | TIMER_ENABLE | TIMER_IRQ_REQ;

//	TIMER1_DATA = (u16)(0x10000 - SOUNDBUFSIZ);
//	TIMER1_CR = TIMER_CASCADE | TIMER_ENABLE | TIMER_IRQ_REQ;

	irqEnable( IRQ_TIMER0 );
*/
}

void StopSound()
{
/*
	int i;
	if( !SoundPause ){
		SoundPause = true;
		SoundProcTiming = 0;
		SoundPtr        = 0;
		SoundBufCnt     = 0;

		for(i=0;i<4;i++){
			SCHANNEL_CR(i) = 0;
		}

		irqDisable( IRQ_TIMER0 );
	}
*/
}

void MSXsoundAddressHandler(void * address, void* userdata)
{
	if( adrflg == 0 ) psg = (PSG*)address;
	if( adrflg == 1 ) scc = (SCC*)address;
}

void MSXsoundCommandHandler(u32 command, void* userdata)
{
/*
	int cmd  = command >> 24;
	int addr = (command & 0x00FFFF00) >> 8;
	int data = command & 0xFFFF;
	
	switch(cmd)
	{
		case 1: StopSound();  break;	// Stop
		case 2: PlaySound();  break;	// Play
		case 3: ResetSound(); break;	// Reset
		case 4: PSG_writeReg(psg,addr,data); break;
		case 5: SCC_write(scc,addr,data); break;
		case 6: adrflg = data; break;	// psg:0 scc:1
		case 7: psgON = (data==1)?true:false;  break;
		case 8: sccON = (data==1)?true:false;  break;
	}
*/
}

void installMSXSoundFIFO() {
/*
	fifoSetAddressHandler(FIFO_USER_01, MSXsoundAddressHandler, 0);
	fifoSetValue32Handler(FIFO_USER_01, MSXsoundCommandHandler, 0);
*/
}
