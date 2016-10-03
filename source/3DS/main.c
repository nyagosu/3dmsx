//////////////////////////////////////////////////////////////////////
// fmsxDS
//////////////////////////////////////////////////////////////////////
#include <3ds.h>
//#include "MSX.h"
#include <stdio.h>
#include <string.h>
#include "DSDriver.h"
#include "DSSound.h"
#include "DSLua.h"

void Init3DS()
{
	InitGraph();
	InitSDMC();
	InitLOG();
	InitSound();
}

void Exit3DS(void)
{
	ExitSound();
	ExitLOG();
	InitSDMC();
	ExitGraph();
}


void testloop(){
	int x,y;
	x = 100;
	y = 100;
	int adr;

	while( aptMainLoop() ){
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here

		u32 kDown = hidKeysHeld();
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
}

int main(int ac, char*av[])
{
	// 3DS初期化
	Init3DS();

	char scriptname[256];
	strcpy( scriptname, "fmsxDS.lua" );

	//	スクリプト実行
	printf( "StartLua\r\n" );
	StartLua(scriptname);

	printf( "testloop\r\n" );
	testloop();

	Exit3DS();
	return 0;
}
