//////////////////////////////////////////////////////////////////////
// fmsxDS
//////////////////////////////////////////////////////////////////////
//#include "ipc2.h"
#include <3ds.h>
#include "MSX.h"
#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <NDS.h>
//#include <fat.h>
//#include <unistd.h>
#include "DSGraph.h"
#include "DSLua.h"
#include "DSFileList.h"
#include "cyg-profile.h"

#define MSX_CLK 3579545

void InitLOG(void);
extern void LOG( char * str , ... );

extern char *ROMName[];
extern char *DSKName[];

extern int MainMenu();

//PSG _psg;
//SCC _scc;

extern PSG * psg;
extern SCC * scc;

int psgON = true;
int sccON = true;

extern int NewSampleRate;

int autoFitScreen=0;

#define RGB16(a,r,g,b)  RGB15(r,g,b)|((a)<<15)

void waitForVBlank(int cnt)
{
	int i;
	for(i=0;i<cnt;i++){
//		swiWaitForVBlank();
		gspWaitForVBlank();
	}
}

void HBlankHandler()
	__attribute__ ((no_instrument_function));

void VBlankHandler(void)
{
//	scanKeys();
//	lua_vsync();
//	LOG("vsync");
}


void InitInterruptHandler()
{
//	irqInit();
//	irqSet(IRQ_HBLANK, HBlankHandler);
//	irqSet(IRQ_VBLANK, VBlankHandler);
//	irqEnable( IRQ_HBLANK | IRQ_VBLANK );
}

void ResetSound(int smpl)
{
/*
	IPC2->sampleRate = smpl;
	IPC2->soundCommand = 3;
	while( IPC2->soundCommand ) waitForVBlank(1);
*/
}

void StartSound(void)
{
/*
	IPC2->psgON = psgON;
	IPC2->sccON = sccON;
	IPC2->soundCommand = 2;
	while( IPC2->soundCommand ) waitForVBlank(1);
*/
}

void StopSound(void)
{
/*
	IPC2->soundCommand = 1;
	while( IPC2->soundCommand ) waitForVBlank(1);
*/
}

void InitDS()
{
	gfxInitDefault();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D

	// メモリのウェイトと使用権の設定
//	REG_EXMEMCNT = 0xE880;

	//割り込み初期化
//	InitInterruptHandler();

	//ディスプレイモードの設定
//	videoSetMode   (MODE_5_2D | DISPLAY_BG3_ACTIVE); 
//	videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE);

	// VRAMバンク設定
//	vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_BG, VRAM_C_SUB_BG , VRAM_D_ARM7_0x06000000); 

	// LCDのメインとサブを入れ替える
//	lcdSwap();

/*
	SUB_BG2_CR = BG_BMP16_256x256;
    SUB_BG2_XDX = 1 << 8;
    SUB_BG2_XDY = 0;
    SUB_BG2_YDX = 0;
    SUB_BG2_YDY = 1 << 8;
	SUB_BG2_CX = 0;
	SUB_BG2_CY = 0;

	BG3_CR = BG_BMP16_256x256;
    BG3_XDX = (1 << 8);
    BG3_XDY = 0;
    BG3_YDX = 0;
    BG3_YDY = (1 << 8);
    BG3_CX = 0;
    BG3_CY = 0;
*/
/*
	BACKGROUND.control[3] = BG_BMP16_256x256;
	BACKGROUND.bg3_rotation.hdx = (1 << 8);
	BACKGROUND.bg3_rotation.hdy = 0;
	BACKGROUND.bg3_rotation.vdx = 0;
	BACKGROUND.bg3_rotation.vdy = (1 << 8);
	BACKGROUND.bg3_rotation.dx = 0;
	BACKGROUND.bg3_rotation.dy = 0;

	BACKGROUND_SUB.control[2] = BG_BMP16_256x256;
	BACKGROUND_SUB.bg2_rotation.hdx = (1 << 8);
	BACKGROUND_SUB.bg2_rotation.hdy = 0;
	BACKGROUND_SUB.bg2_rotation.vdx = 0;
	BACKGROUND_SUB.bg2_rotation.vdy = (1 << 8);
	BACKGROUND_SUB.bg2_rotation.dx = 0;
	BACKGROUND_SUB.bg2_rotation.dy = 0;
*/
//	psg = (PSG*)&(IPC2->psg);
//	scc = (SCC*)&(IPC2->scc);

//	psg = &_psg;
//	scc = &_scc;

}



void changeBGSize(byte V)
{
	static int line212flg = -1;
	if( autoFitScreen ){
		if(line212flg != (V&0x80) ){
			line212flg = (V&0x80);
//		    BACKGROUND.bg3_rotation.vdy  = (1 << 8) + (line212flg?32:0);
		}
	}
}

int InitFAT()
{
	int ret;

//	ret = fatInitDefault();
	if( !ret ){
		LOG( "fatInitDefault() Error." );
	    while(1) waitForVBlank(1);
	}
	LOG( "fatInitDefault() OK" );

	return true;
}

int main(int ac, char*av[])
{
	char scriptname[256];
	int ret = 1;
//	IPC2->soundCommand = 0;

	//DS初期化
	InitDS();

	//ログ初期化
//	InitLOG();
	
	//ファイルシステム初期化
//	InitFAT();

//	cygprofile_begin();
//	cygprofile_enable();
	
	//VM 初期化
//	InitMachine();
//	InitMSX();

	// Sound reset
	ResetSound(SampleRate);

	int x,y;
	x = 100;
	y = 100;
	int adr;
	
	strcpy( scriptname, "fmsxDS.lua" );
	while( aptMainLoop() ){
//		if( ret ){
			//	スクリプト実行
//			StartLua(scriptname);
//			cygprofile_disable();
//			cygprofile_end();
//		}
//		ret = fileselect( "fat:/", "LUA", scriptname );

		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) break; // break in order to return to hbmenu

		if (kDown & KEY_UP    ) y = y + 1;
		if (kDown & KEY_DOWN  ) y = y - 1;
		if (kDown & KEY_LEFT  ) x = x - 1;
		if (kDown & KEY_RIGHT ) x = x + 1;

		// Example rendering code that displays a white pixel
		// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
		u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
//		memset(fb, 0, 240*400*3);
		adr = (y+x*240)*3;
		fb[adr  ] = 0xFF;
		fb[adr+1] = 0xFF;
		fb[adr+2] = 0xFF;

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	gfxExit();
	return 0;
}
