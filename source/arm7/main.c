/*---------------------------------------------------------------------------------

	fmsxDS ARM7 core

---------------------------------------------------------------------------------*/
#include <3ds.h>
//#include <dswifi7.h>
#include "msxsnd.h"

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void VcountHandler() {
	inputGetAndSend();
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void VblankHandler(void) {
//	Wifi_Update();
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	irqInit();
	fifoInit();

	SetYtrigger(80);

//	installWifiFIFO();
//	installSoundFIFO();

//	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	
	installMSXSoundFIFO();
	
	installSystemFIFO();
	
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	// Start the RTC tracking IRQ
	initClockIRQ();

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK );

	// read User Settings from firmware
	readUserSettings();

	// ARM7 full power calc MSX sound :)
	while (1){
		if( !SoundPause ){
			CalcSound();
		}else{
			swiWaitForVBlank();
		}
	}
}
