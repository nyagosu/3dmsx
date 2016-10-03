//////////////////////////////////////////////////////////////////////
// fmsxDS
//////////////////////////////////////////////////////////////////////
//#include "../../common/ipc2.h"

#include "msx.h"
//#include "sound.h"

#include <3DS.h>

#include <string.h>
#include <stdlib.h>
//#include <NDS.h>
#include <fcntl.h>	
//#include <fat.h>
#include <sys/dir.h>
#include <stdarg.h>

#include "DSDriver.h"
#include "DSGraph.h"
#include "Keybind.h"
#include "DSFileList.h"
#include "DSLua.h"

/** Public parameters ****************************************/
int UseSound  = 8192;          /* Sound driver frequency    */
char *Disks[2][MAXDISKS+1];    /* Disk names for each drive */

extern byte *MemMap[4][4][8];   /* Memory maps [PPage][SPage][Addr] */

int autoFitScreen=0;

/** Various variables ****************************************/
//#define WIDTH  272
//#define HEIGHT 228
int WIDTH  = 272;
int HEIGHT = 228;

extern u8 touchmap[];
extern void waitForVBlank(int);

void createTouchMap();

/** Keyboard bindings ****************************************/
byte WinKeyMap[16];             /* Keyboard map              */
unsigned int JoyKeyMap[2];		/* Joystick Keymap           */
unsigned int MouseMap[2];		/* Mouse Keymap              */

byte hkey[16];					/* Hold key map              */
byte touchMode=0;				/* touch Panel Mode          */
								/*  0:keyboard 1:mouse       */

unsigned int BPal[256],XPal[80],XPal0; 
char *XBuf;
char *XBuf2;
int  XBufType = 1;

int UseStatic = 0;

extern int View_x;
extern int View_y;
extern int UseNarrow;
extern int Usex2Mode;
extern byte TapeHeader[8];

void drawAllKeyboard(void);

int pad_type = 0;

#define SetDownKey(AA) ( WinKeyMap[(AA).code]&=~((AA).mask) )
#define SetUpKey(AA)   ( WinKeyMap[(AA).code]|= ((AA).mask) )
#define SetDownJoy(AA) ( JoyKeyMap[(AA).code]|= ((AA).mask) )
#define SetUpJoy(AA)   ( JoyKeyMap[(AA).code]&=~((AA).mask) )

LLIST * LOGData;

void waitForVBlank(int cnt)
{
	int i;
	for(i=0;i<cnt;i++){
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

void InitSDMC(void)
{
	printf( "InitSDMC\r\n" );
}

void ExitSDMC(void)
{
	printf( "ExitSDMC\r\n" );

}
void ExitLOG(void)
{
	printf( "ExitLOG\r\n" );

}

void InitLOG(void)
{
	printf( "InitLOG\r\n" );
//	LOGData = LLIST_new();
}

#define LOG_CHR_NUM  51
void LOG( char * str , ... )
{
	va_list args;
	char buf[256];
	char *a;
	char * n;
	int l;
	va_start(args,str);
	vprintf(str,args);
	va_end(args);
/*
	n = buf;
	l = strlen( buf );
	while(l){
		if( l > LOG_CHR_NUM ){
			a = malloc( LOG_CHR_NUM + 1 );
			strncpy(a, n,LOG_CHR_NUM);
			n+=LOG_CHR_NUM;
			l=strlen(n);
		}else{
			a = malloc( strlen(n) + 1 );
			strcpy( a, n );
			l=0;
		}
		copyGraph( GFX_BOTTOM, GFX_LEFT, 0, 160, GFX_BOTTOM, GFX_LEFT, 0, 168, 256, 24 );
		fill( GFX_BOTTOM,GFX_LEFT, 0, 184, 256, 8, clsColor[1] );
		drawText( GFX_BOTTOM,GFX_LEFT, 0, 184, (u8*)a, 0xFFFF, clsColor[1] );
		LLIST_add( LOGData, a );
	}
	*/
}

void RomSelect( int num )
{
	char fn[256];
	int ret = fileselect( "fat:/", "ROM", fn );
	if( ret ){
		if( !ROMName[num] ) ROMName[num] = (char*)malloc(256);
		strcpy( ROMName[num], fn );
	}
}

void DiskSelect( int num )
{
	char fn[256];
	int ret = fileselect( "fat:/", "DSK", fn );
	if( ret && ChangeDisk(num,fn) ){
		if( !DSKName[num] ) DSKName[num] = (char*)malloc(256);
		strcpy( DSKName[num], fn );
	}
}

void CasSelect( void )
{
	char fn[256];
	int ret = fileselect( "fat:/", "CAS", fn );
	if( ret ){
		if( !CasName ) CasName = (char*)malloc(256);
		strcpy( CasName, fn );
		ChangeCas( CasName );
	}
}

byte system_RTCIn(byte R)
{
	byte J;
	time_t t = time(NULL);
	struct tm * tm = localtime( &t );

	switch(R)
    {
    	case 0:  J=tm->tm_sec  % 10   ;break;
        case 1:  J=tm->tm_sec  / 10   ;break;
        case 2:  J=tm->tm_min  % 10   ;break;
        case 3:  J=tm->tm_min  / 10   ;break;
        case 4:  J=tm->tm_hour % 10   ;break;
        case 5:  J=tm->tm_hour / 10   ;break;
        case 6:  J=tm->tm_wday        ;break;
        case 7:  J=tm->tm_mday % 10   ;break;
        case 8:  J=tm->tm_mday / 10   ;break;
        case 9:  J=tm->tm_mon  % 10   ;break;
        case 10: J=tm->tm_mon  / 10   ;break;
        case 11: J=tm->tm_year % 10   ;break;
        case 12: J=tm->tm_year / 10   ;break;
        default: J=0x0F;break;
    }                   
	return J;
}

void ReadKeyBind( char * fn, KEYBIND * kb )
{
}

void vid_close()
{
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

/** InitMachine() ********************************************/
/** Allocate resources needed by the machine-dependent code.**/
/************************************ TO BE WRITTEN BY USER **/
int InitMachine(void)
{
	int J;

	/* keybord init */ 
	memset((char*)WinKeyMap,0xFF,16);
	memset((char*)hkey,0xFF,16);

	JoyKeyMap[0] = 0;
	JoyKeyMap[1] = 0;

	MouseMap[0]  = 0;
	MouseMap[1]  = 0;

	/* Reset the palette */
	for(J=0;J<16;J++) XPal[J]=0;
	XPal0=0;

	/* Set SCREEN8 colors */
	for(J=0;J<256;J++ ){
		BPal[J] = RGB565(J&0x1C, (J&0xE0)>>3, (J&0x03)<<3 );
	}

//	XBuf  = (char*) BG_GFX;
//	XBuf2 = (char*) BGFX_BOTTOM,GFX_LEFT;

	createTouchMap();

	return 1;
}

/** TrashMachine() *******************************************/
/** Deallocate all resources taken by InitMachine().        **/
/************************************ TO BE WRITTEN BY USER **/
void TrashMachine(void)
{
//  vid_close();		/* video close */
//  StopSound();		/* sound stop  */
}

#define LINECOL 0xFFFF
#define OFFCOL  0xF39C
#define ONCOL   0xA108
#define FONTCOL 0x8000

#define KB_X	0
#define KB_Y	32
#define KB_CHIP 8

int checkKey( u16 x, u16 y )
{
	x = (u16)( x / KB_CHIP);
	y = (u16)( y / KB_CHIP);
	return touchmap[ y * 32 + x ]; 
}

// extern byte RdZ80(register word Addr);
#define LOCK_KANA 	(Power?RdZ80(0xFCAC):0)
#define LOCK_CAPS 	(Power?RdZ80(0xFCAB):0)
#define LOCK_SHIFT ( !(hkey[6]&1) )
#define LOCK_CTRL  ( !(hkey[6]&2) )
#define LOCK_GRAPH ( !(hkey[6]&4) )

void drawKey( int k, u32 col )
{
//	LOG( "drawKey" );
	if( k == 255 ) return;
	int x = KeyBinds[k].x * KB_CHIP;
	int y = KeyBinds[k].y * KB_CHIP;
	int w = KeyBinds[k].width  * KB_CHIP;
	int h = KeyBinds[k].height * KB_CHIP;
	
//	LOG( "fill" );
	fill( GFX_BOTTOM,GFX_LEFT,x,y,w,h,col );
//	LOG( "box" );
	box ( GFX_BOTTOM,GFX_LEFT,x,y,w,h,0x00DDDDDD );
//	LOG( "drawKey 2" );

	int num;
	if     ( LOCK_CTRL  ){ num = 9; }
	else if( LOCK_GRAPH ){ num = 8; }
	else if( LOCK_KANA  ){
		if( LOCK_CAPS ){
			num = LOCK_SHIFT?7:6;
		} else {
			num = LOCK_SHIFT?5:4;
		}
	}else{
		if( LOCK_CAPS ){
			num = LOCK_SHIFT?3:2;
		} else {
			num = LOCK_SHIFT?1:0;
		}
	}
//	LOG( "%d", num );
	if( KeyBinds[k].keytop[num].text ){
		char * buf = KeyBinds[k].keytop[num].text;
		x += KeyBinds[k].keytop[num].x_mgn;
		y += KeyBinds[k].keytop[num].y_mgn;
		if( KeyBinds[k].keytop[num].fnttype == 1 ){
			drawText   ( GFX_BOTTOM,GFX_LEFT, x, y, (u8*)buf, 0x00, 0);
		}else{
			drawTextMSX( GFX_BOTTOM,GFX_LEFT, x, y, (u8*)buf, 0x00, 0);
		}
	}
//	LOG( "drawKey end" );
}

void drawAllKeyboard(void)
{
	int i = 0;
	LOG( "drawAllKeyboard" );
	if( !Power ) return ;
	while( KeyBinds[i].mask ){
		drawKey( i, 0xFFFFFF );
		i++;
	}
}

void Keyboard_proc(void)
{
//	static int pen=false;
	static int ok = 255;
	int i;
	int chgflg = false;
	int DrawAllFlg = false;
	int btn1=0;
	int btn2=0;

//	int tx = 0,ty = 0;
//	u8 buf[16];
	u32 k;
	static touchPosition tp;
	static int oldLockKana = false;
	static int oldLockCaps = false;
	static int toggleLButtonDown = false;

//	LOG( "Keyboard" );
//    memset((char*)WinKeyMap, 0xFF, 16 );
//	dispTouch();
//	printText( "cnt  ", 0, 10, IPC2->cnt   );

/*
    uint16 specialKeysPressed = ~IPC->buttons;
	if(specialKeysPressed & IPC_PEN_DOWN ){
*/
	scanKeys();
	if( touchMode ){	// Mouse MODE
		// 押した瞬間
		k = keysDown();
		if( k&KEY_X ){
			toggleLButtonDown = !toggleLButtonDown;
			LOG( "touch L Btn Down %s",toggleLButtonDown?"[ture]":"[false]" );
		}else
		if( k&KEY_START ) lua_LuaFunc(1);	// MainMenu
		// 押し続け
		k = keysHeld();
		if( k&KEY_TOUCH ){
//			tp = touchReadXY();
			touchRead(&tp);
			LOG( "x[%d] y[%d]",tp.px, tp.py );
			if( toggleLButtonDown ) btn1 = 1;
		}
		if(k&KEY_L   ) btn1 = 1;
		if(k&KEY_DOWN) btn2 = 1;

		// 離した
		k = keysUp();
		if( k&KEY_TOUCH ){
			LOG("keyup touch");
			if( toggleLButtonDown ) btn1 = 0;
		}

		MouseMap[0] = ((btn2<<17)|(btn1<<16)|(tp.py<<8)|(tp.px));
		MouseMap[1] = ((btn2<<17)|(btn1<<16)|(tp.py<<8)|(tp.px));
	}else{
		k = keysDown();
		if( k|0x0FFF ){
			for(i=0;i<12;i++ ){
				if( k&BIT(i) ) {
					switch( JoyBinds[i].type ){
					  case 0: SetDownKey(JoyBinds[i]);chgflg=true;break;
					  case 1: SetDownJoy(JoyBinds[i]); break;
					}
				}
			}
		}
		if( k&KEY_TOUCH ){
//			touchPosition tp = touchReadXY();
			touchRead(&tp);
			k = checkKey( tp.px, tp.py );
			if( k != ok ){
//				pen = true;
				if( ok != k ){
					drawKey( ok, RGB565(31,31,31) );
					drawKey(  k, RGB565(28,19,19) );
					SetUpKey(KeyBinds[ok]);
					SetDownKey(KeyBinds[k]);
				}
				chgflg = true;
				ok = k;
			}
		}
		k = keysUp();
		if( k|0x0FFF ){
			for(i=0;i<12;i++ ){
				if( k&BIT(i) ) {
					switch( JoyBinds[i].type ){
					  case 0: SetUpKey(JoyBinds[i]);chgflg=true;break;
					  case 1: SetUpJoy(JoyBinds[i]); break;
					  case 2: lua_LuaFunc(JoyBinds[i].code);break;
					}
				}
			}
		}
		if( k&KEY_TOUCH ){
			drawKey( ok, RGB565(31,31,31) );
			SetUpKey(KeyBinds[ok]);
			if( KeyBinds[ok].Hold&1 ){
				hkey[KeyBinds[ok].code] ^= KeyBinds[ok].mask;
			}
			if( KeyBinds[ok].Hold ){
				DrawAllFlg = true;
			}
			chgflg = true;
			ok = 255;
//			pen = false;
		}

		if( chgflg ){
		    memcpy((char*)MSXKeyMap, (char*)WinKeyMap, 16 );
			for(i=0;i<16;i++ ) MSXKeyMap[i] &= hkey[i];
		}

		// かなとCAPSキーのLOCK状態をチェック */
		if( LOCK_KANA ){
			if( !oldLockKana ){
				DrawAllFlg = true;
				oldLockKana = LOCK_KANA;
			}
		}else{
			if( oldLockKana ){
				DrawAllFlg = true;
				oldLockKana = LOCK_KANA;
			}
		}
		if( LOCK_CAPS ){
			if( !oldLockCaps ){
				DrawAllFlg = true;
				oldLockCaps = LOCK_CAPS;
			}
		}else{
			if( oldLockCaps ){
				DrawAllFlg = true;
				oldLockCaps = LOCK_CAPS;
			}
		}

		if(DrawAllFlg){
			drawAllKeyboard();
		}
	}
}

/** Joystick() ***********************************************/
/** Query position of a joystick connected to port N.       **/
/** Returns 0.0.B2.A2.R2.L2.D2.U2.0.0.B1.A1.R1.L1.D1.U1.    **/  
/************************************ TO BE WRITTEN BY USER **/
unsigned int Joystick(void)
{
//	LOG( "JoyKeyMap[0]=%X JoyKeyMap[1]=%X",JoyKeyMap[0],JoyKeyMap[1]);
	return (JoyKeyMap[1]<<8)|JoyKeyMap[0];
}

/** Mouse() **************************************************/
/** Query coordinates of a mouse connected to port N.       **/
/** Returns F2.F1.Y.Y.Y.Y.Y.Y.Y.Y.X.X.X.X.X.X.X.X.          **/
/************************************ TO BE WRITTEN BY USER **/
unsigned int Mouse(byte N)
{
//	LOG( "MouseMap[%d]=%X",N,MouseMap[N]);
	return MouseMap[N];
}

/** SetColor() ***********************************************/
/** Set color N (0..15) to (R,G,B).                         **/
/************************************ TO BE WRITTEN BY USER **/
void SetColor(byte N,byte R,byte G,byte B)
{
  unsigned int J;
  if(UseStatic) J=BPal[((7*R/255)<<2)|((7*G/255)<<5)|(3*B/255)];
  else J=( ((int)(B/8)<<10)|((int)(G/8)<<5)|(int)(R/8)|BIT(15) );

  if(N) XPal[N]=J; else XPal0=J;
}

/** RefreshScreen() ******************************************/
/** Refresh screen. This function is called in the end of   **/
/** refresh cycle to show the entire screen.                **/
/************************************ TO BE WRITTEN BY USER **/
void RefreshScreen( void )
{
//	dPrint( "RefreshScreen\n");
//    BitBlt( hDC, View_x, View_y, sw, sh, hbDC,0,0,SRCCOPY ); 
//	swiWaitForVBlank();
}

/** search tape header **/
/************************/
int SearchTapeHeader(FILE * fp)
{
	char buf[8];
	
	int l = ftell(fp);
	if( l&7 ) fseek(fp,8-(l&7),SEEK_CUR );

	while( fread( buf, 1, 8, fp )==8 ){
		if( memcmp(buf, TapeHeader, 8)==0 ){
			return ftell(fp) - 8;
		}
	}
	return -1;
}

/** read tape data ******/
/************************/
TapeData * readTapeData(FILE * fp, int * head )
{
	int s, e;
	int siz, asiz;
	fseek( fp,*head,SEEK_SET );
	s = SearchTapeHeader(fp);
	if( s == -1 ) return NULL;
	e = SearchTapeHeader(fp);
	if( e == -1 ){
		e = ftell(fp);
		*head = -1;
	}else{
		*head = e;
	}
	siz  = e-s-8;
	asiz = siz + ((siz%8)?8-siz%8:0);
	TapeData * td = (TapeData*)calloc(1, sizeof(TapeData) + asiz );
	if( td == NULL ) return NULL;
	td->siz = asiz;
	LOG( "getTapeData  s[%d] e[%d] size[%d] alloc[%d]", s, e, siz, asiz );
	fseek( fp,s+8,SEEK_SET );
	if( fread( td->buf, 1, siz , fp ) < siz ){
		free( td );
		return NULL;
	}
	return td;
}

/** ChangeCas() **********************************************/
/************************************ TO BE WRITTEN BY USER **/
int ChangeCas( const char * fname )
{
	FILE * fp = NULL;
	int ret;

    if( CasData ){
		/* save */
		fp= fopen(fname,"wb");
		if( fp == NULL ){
			LOG("tape save error.");
			return 0;
		}
		ret = LLIST_top(CasData);
		while( ret == LLIST_OK ){
			TapeData * obj =(TapeData *)LLIST_get(CasData);
			if( obj == NULL ) break;
			fwrite( TapeHeader, 1,8, fp );
			fwrite( obj->buf,1,obj->siz, fp );
			ret = LLIST_next(CasData);
		}
		fclose(fp);

		/* free */
		LLIST_free(CasData);
		CasData=NULL;
		FreeMemory( (byte*)CasName );
		CasName=NULL;
		CasPtr =0;
    }

	if( (fname==NULL) || ((*fname)=='\0') ) return 1;	/* eject only */

	CasData = LLIST_new();
	
	fp= fopen(fname,"rb");
	if(fp==NULL){
		LOG("new tape %s.",fname);
		TapeData * td = (TapeData*)calloc(1, sizeof(TapeData) + 8 );
		if( td == NULL ) return 0;
		td->siz = 8;
		LLIST_add(CasData,td);
		return 0;
    }

	int head = 0;
	while( head != -1 ){
		TapeData * obj = readTapeData(fp, &head );
		if( obj == NULL ){
			fclose(fp);
			LLIST_free(CasData);
			CasData = NULL;
			return 0;
		}
		LLIST_add(CasData, (void*)obj);
	}
	fclose(fp);

	CasName = (char*)malloc(strlen(fname)+1);
	if( !CasName ){
		LLIST_free(CasData);
		CasData = NULL;
		LOG("CAS name memory allocate error." );
		fclose(fp);
		return 0;
	}
	strcpy( CasName, fname );
	LLIST_top( CasData );
	return 1;
}

/** CasTop() *************************************************/
/*************************************************************/
void CasTop( void )
{
	if( CasName ){
		LLIST_top( CasData );
		CasPtr = 0;
		LOG( "Rewind tape to top.");
	}
}

/** CasEnd() *************************************************/
/*************************************************************/
void CasEnd( void )
{
	if( CasName ){
		LLIST_end( CasData );
		CasPtr = 0;
		LOG( "Feed tape to end.");
	}
}

/** CasPrev() ************************************************/
/*************************************************************/
void CasPrev( void )
{
	if( CasName ){
		LLIST_prev( CasData );
		CasPtr = 0;
		LOG( "rewind tape previous header.");
	}
}

/** CasNext() ************************************************/
/*************************************************************/
void CasNext( void )
{
	if( CasName ){
		LLIST_next( CasData );
		CasPtr = 0;
		LOG( "Feed tape next header.");
	}
}

/** createTouchMap() *****************************************/
/*************************************************************/
void createTouchMap()
{
	int i,j,k;
	for( i=0;i<32*24;i++ ) touchmap[i]=255;

	for( i=0;i<128;i++ ){
		for(j=0;j<KeyBinds[i].height;j++ ){
			for(k=0;k<KeyBinds[i].width;k++){
				touchmap[(KeyBinds[i].y+j) * 32 + (KeyBinds[i].x + k) ] = i;
			}
		}
	}
}

u8 touchmap[32*24];

JOYBIND JoyBinds[16] =
{
	{0,8,0x01},		// Key_A = Space
	{0,4,0x08},		// Key_B = N
	{0,6,0x20},		// Select= F1
	{2,1,0x00},		// Start = MainMenu
	{0,8,0x80},		// Right
	{0,8,0x10},		// Left
	{0,8,0x20},		// UP
	{0,8,0x40},		// Down
	{0,0,0x00},		// KEY_R = none
	{0,0,0x00},		// KEY_L = none
	{0,7,0x80},		// KEY_X = return
	{0,0,0x00}		// KEY_Y = none
};

KEYBIND KeyBinds[128];
#if 0
	=
{
//TP LN MSK LCK X  Y W H FxFy Ft  NML    SHIFT  CAPS   CP+SFT  KANA  KN+SH  KN+CP KN+CP+SH GRAPH  CTRL
  {0, 6,0x20,0, 1, 0,3,2,1, 4,0,{"F01" ,"F06" ,"F01" ,"F06" ,"F01" ,"F06" ,"F01" ,"F06" ,"F01" ,"F01"  } },  // F1
  {0, 6,0x40,0, 4, 0,3,2,1, 4,0,{"F02" ,"F07" ,"F02" ,"F07" ,"F02" ,"F07" ,"F02" ,"F07" ,"F02" ,"F02"  } },  // F2
  {0, 6,0x80,0, 7, 0,3,2,1, 4,0,{"F03" ,"F08" ,"F03" ,"F08" ,"F03" ,"F08" ,"F03" ,"F08" ,"F03" ,"F03"  } },  // F3
  {0, 7,0x01,0,10, 0,3,2,1, 4,0,{"F04" ,"F09" ,"F04" ,"F09" ,"F04" ,"F09" ,"F04" ,"F09" ,"F04" ,"F04"  } },  // F4
  {0, 7,0x02,0,13, 0,3,2,1, 4,0,{"F05" ,"F10" ,"F05" ,"F10" ,"F05" ,"F10" ,"F05" ,"F10" ,"F05" ,"F05"  } },  // F5

  {0, 7,0x10,0,18, 0,3,2,1, 4,1,{"STOP","STOP","STOP","STOP","STOP","STOP","STOP","STOP","STOP","^STOP"} },  // STOP
  {0, 8,0x02,0,21, 0,3,2,1, 4,1,{"HOME","CLS" ,"HOME","CLS" ,"HOME","CLS" ,"HOME","CLS" ,"HOME","HOME" } },  // HOME/CLS  
  {0, 7,0x40,0,25, 0,2,2,1, 4,1,{"SEL" ,"SEL" ,"SEL" ,"SEL" ,"SEL" ,"SEL" ,"SEL" ,"SEL" ,"SEL" ,"SEL"  } },  // SELECT    
  {0, 8,0x04,0,27, 0,2,2,1, 4,1,{"INS" ,"INS" ,"INS" ,"INS" ,"INS" ,"INS" ,"INS" ,"INS" ,"INS" ,"INS"  } },  // INSERT    
  {0, 8,0x08,0,29, 0,2,2,1, 4,1,{"DEL" ,"DEL" ,"DEL" ,"DEL" ,"DEL" ,"DEL" ,"DEL" ,"DEL" ,"DEL" ,"DEL"  } },  // DELETE    

  {0, 7,0x04,0, 1, 2,2,2,1, 5,1,{"ESC" ,"ESC" ,"ESC" ,"ESC" ,"ESC" ,"ESC" ,"ESC" ,"ESC" ,"ESC" ,"ESC"  } },  // ESC
  {0, 0,0x02,0, 3, 2,2,2,6, 5,0,{"1"   ,"!"   ,"1"   ,"!"   ,"\x91","\x87","\xB1","\xA7","\x07","1"    } },  // '1'
  {0, 0,0x04,0, 5, 2,2,2,6, 5,0,{"2"   ,"\""  ,"2"   ,"\""  ,"\x92","\x88","\xB2","\xA8","\x01","2"    } },  // '2'
  {0, 0,0x08,0, 7, 2,2,2,6, 5,0,{"3"   ,"#"   ,"3"   ,"#"   ,"\x93","\x89","\xB3","\xA9","\x02","3"    } },  // '3'
  {0, 0,0x10,0, 9, 2,2,2,6, 5,0,{"4"   ,"$"   ,"4"   ,"$"   ,"\x94","\x8A","\xB4","\xAA","\x03","4"    } },  // '4'
  {0, 0,0x20,0,11, 2,2,2,6, 5,0,{"5"   ,"%"   ,"5"   ,"%"   ,"\x95","\x8B","\xB5","\xAB","\x04","5"    } },  // '5'
  {0, 0,0x40,0,13, 2,2,2,6, 5,0,{"6"   ,"&"   ,"6"   ,"&"   ,"\xE5","\xE5","\xC5","\xC5","\x05","6"    } },  // '6'
  {0, 0,0x80,0,15, 2,2,2,6, 5,0,{"7"   ,"'"   ,"7"   ,"'"   ,"\xE6","\xE6","\xC6","\xC6","\x06","7"    } },  // '7'
  {0, 1,0x01,0,17, 2,2,2,6, 5,0,{"8"   ,"("   ,"8"   ,"("   ,"\xE7","\xE7","\xC7","\xC7","\x0D","8"    } },  // '8'
  {0, 1,0x02,0,19, 2,2,2,6, 5,0,{"9"   ,")"   ,"9"   ,")"   ,"\xE8","\xE8","\xC8","\xC8","\x0E","9"    } },  // '9'
  {0, 0,0x01,0,21, 2,2,2,6, 5,0,{"0"   ,""    ,"0"   ,""    ,"\xE9","\xE9","\xC9","\xC9","\x0F","0"    } },  // '0'
  {0, 1,0x04,0,23, 2,2,2,6, 5,0,{"-"   ,"="   ,"-"   ,"="   ,"\xF7","\xF7","\xD7","\xD7","\x17","-"    } },  // -
  {0, 1,0x08,0,25, 2,2,2,6, 5,0,{"^"   ,"~"   ,"^"   ,"~"   ,"\xF8","\xF8","\xD8","\xD8",""    ,"U"    } },  // ^
  {0, 1,0x10,0,27, 2,2,2,6, 5,0,{"\\"  ,"|"   ,"\\"  ,"|"   ,"\xF8","\xF8","\xD8","\xD8","\x09","L"    } },  // "\"
  {0, 7,0x20,0,29, 2,2,2,3, 5,1,{"BS"  ,"BS"  ,"BS"  ,"BS"  ,"BS"  ,"BS"  ,"BS"  ,"BS"  ,"BS"  ,"BS"   } },  // BS
//25                       
  {0, 7,0x08,0, 1, 4,3,2,1, 5,1,{"TAB" ,"TAB" ,"TAB" ,"TAB" ,"TAB" ,"TAB" ,"TAB" ,"TAB" ,"TAB" ,"TAB"  } },  // TAB
  {0, 4,0x40,0, 4, 4,2,2,6, 5,0,{"q"   ,"Q"   ,"Q"   ,"q"   ,"\x96","\x96","\xB6","\xB6",""    ,""     } },  // 'Q'
  {0, 5,0x10,0, 6, 4,2,2,6, 5,0,{"w"   ,"W"   ,"W"   ,"w"   ,"\x97","\x97","\xB7","\xB7",""    ,""     } },  // 'W'
  {0, 3,0x04,0, 8, 4,2,2,6, 5,0,{"e"   ,"E"   ,"E"   ,"e"   ,"\x98","\x98","\xB8","\xB8","\x18","^E"   } },  // 'E'
  {0, 4,0x80,0,10, 4,2,2,6, 5,0,{"r"   ,"R"   ,"R"   ,"r"   ,"\x99","\x99","\xB9","\xB9","\x12","^R"   } },  // 'R'
  {0, 5,0x02,0,12, 4,2,2,6, 5,0,{"t"   ,"T"   ,"T"   ,"t"   ,"\x9A","\x9A","\xBA","\xBA","\x19",""     } },  // 'T'
  {0, 5,0x40,0,14, 4,2,2,6, 5,0,{"y"   ,"Y"   ,"Y"   ,"y"   ,"\xEA","\xEA","\xDA","\xDA","\x08",""     } },  // 'Y'
  {0, 5,0x04,0,16, 4,2,2,6, 5,0,{"u"   ,"U"   ,"U"   ,"u"   ,"\xEB","\xEB","\xDB","\xDB",""    ,""     } },  // 'U'
  {0, 3,0x40,0,18, 4,2,2,6, 5,0,{"i"   ,"I"   ,"I"   ,"i"   ,"\xEC","\xEC","\xDC","\xDC","\x16","^I"   } },  // 'I'
  {0, 4,0x10,0,20, 4,2,2,6, 5,0,{"o"   ,"O"   ,"O"   ,"o"   ,"\xED","\xED","\xDD","\xDD",""    ,""     } },  // 'O'
  {0, 4,0x20,0,22, 4,2,2,6, 5,0,{"p"   ,"P"   ,"P"   ,"p"   ,"\xEE","\xEE","\xDE","\xDE","\x10",""     } },  // 'P'
  {0, 1,0x20,0,24, 4,2,2,6, 5,0,{"@"   ,"`"   ,"@"   ,"`"   ,"\xFA","\xFA","\xDA","\xDA",""    ,""     } },  // @
  {0, 1,0x40,0,26, 4,2,2,6, 5,0,{"["   ,"{"   ,"["   ,"{"   ,"\xFB","\xA2","\xDB","\xA2","\x84",""     } },  // [
  {0, 7,0x80,0,28, 4,3,4,6,10,1,{"RET" ,"RET" ,"RET" ,"RET" ,"RET","RET"  ,"RET" ,"RET" ,"RET" ,"RET"  } },  // RETURN
//39                                                                                                  
  {0, 6,0x02,1, 1, 6,3,2,1, 5,1,{"CTRL","CTRL","CTRL","CTRL","CTRL","CTRL","CTRL","CTRL","CTRL","CTRL" } },  // CTRL 
  {0, 2,0x40,0, 4, 6,2,2,6, 5,0,{"a"   ,"A"   ,"A"   ,"a"   ,"\x9B","\x9B","\xBB","\xAB",""    ,""     } },  // 'A'
  {0, 5,0x01,0, 6, 6,2,2,6, 5,0,{"s"   ,"S"   ,"S"   ,"s"   ,"\x9C","\x9C","\xBC","\xAC","\x0C",""     } },  // 'S'
  {0, 3,0x02,0, 8, 6,2,2,6, 5,0,{"d"   ,"D"   ,"D"   ,"d"   ,"\x9D","\x9D","\xBD","\xAD","\x14",""     } },  // 'D'
  {0, 3,0x08,0,10, 6,2,2,6, 5,0,{"f"   ,"F"   ,"F"   ,"f"   ,"\x9E","\x9E","\xBE","\xAE","\x15","^F"   } },  // 'F'
  {0, 3,0x10,0,12, 6,2,2,6, 5,0,{"g"   ,"G"   ,"G"   ,"g"   ,"\x9F","\x9F","\xBF","\xAF","\x13","^G"   } },  // 'G'
  {0, 3,0x20,0,14, 6,2,2,6, 5,0,{"h"   ,"H"   ,"H"   ,"h"   ,"\xEF","\xEF","\xCF","\xAF","\x0A","^H"   } },  // 'H'
  {0, 3,0x80,0,16, 6,2,2,6, 5,0,{"j"   ,"J"   ,"J"   ,"j"   ,"\xF0","\xF0","\xD0","\xD0",""    ,"^J"   } },  // 'J'
  {0, 4,0x01,0,18, 6,2,2,6, 5,0,{"k"   ,"K"   ,"K"   ,"k"   ,"\xF1","\xF1","\xD1","\xD1",""    ,"^K"   } },  // 'K'
  {0, 4,0x02,0,20, 6,2,2,6, 5,0,{"l"   ,"L"   ,"L"   ,"l"   ,"\xF2","\xF2","\xD2","\xD2","\x1E","^L"   } },  // 'L'
  {0, 1,0x80,0,22, 6,2,2,6, 5,0,{";"   ,"+"   ,";"   ,"+"   ,"\xF3","\xF3","\xD3","\xD3","\x82",""     } },  // +
  {0, 2,0x01,0,24, 6,2,2,6, 5,0,{":"   ,"*"   ,":"   ,"*"   ,"\xDE","\xB0","\xDE","\xB0","\x81",""     } },  // :
  {0, 2,0x02,0,26, 6,2,2,6, 5,0,{"]"   ,"}"   ,"]"   ,"}"   ,"\xDF","\xA3","\xDF","\xA3","\x85",""     } },  // ]
//50,2                                                                                                
  {0, 6,0x01,1, 1, 8,4,2,1, 5,1,{"SHIFT","SHIFT","SHIFT","SHIFT","SHIFT","SHIFT","SHIFT","SHIFT","SHIFT","SHIFT"} },  // SHIFT  
  {0, 5,0x80,0, 5, 8,2,2,6, 5,0,{"z"   ,"Z"   ,"Z"   ,"z"   ,"\xE0","\xE0","\xC0","\xE0",""    ,""    } },  // 'Z'
  {0, 5,0x20,0, 7, 8,2,2,6, 5,0,{"x"   ,"X"   ,"X"   ,"x"   ,"\xE1","\xE1","\xC1","\xE1","\x1C","^X"  } },  // 'X'
  {0, 3,0x01,0, 9, 8,2,2,6, 5,0,{"c"   ,"C"   ,"C"   ,"c"   ,"\xE2","\x8F","\xC2","\x8F","\x1A","^C"  } },  // 'C'
  {0, 5,0x08,0,11, 8,2,2,6, 5,0,{"v"   ,"V"   ,"V"   ,"v"   ,"\xE3","\xE3","\xC3","\xE3","\x11","^V"  } },  // 'V'
  {0, 2,0x80,0,13, 8,2,2,6, 5,0,{"b"   ,"B"   ,"B"   ,"b"   ,"\xE4","\xE4","\xC4","\xE4","\x1B","^B"  } },  // 'B'
  {0, 4,0x08,0,15, 8,2,2,6, 5,0,{"n"   ,"N"   ,"N"   ,"n"   ,"\xF4","\x8C","\xD4","\x8C",""    ,"^N"  } },  // 'N'
  {0, 4,0x04,0,17, 8,2,2,6, 5,0,{"m"   ,"M"   ,"M"   ,"m"   ,"\xF5","\x8D","\xD5","\x8D","\x0B","^M"  } },  // 'M'
  {0, 2,0x04,0,19, 8,2,2,6, 5,0,{","   ,"<"   ,","   ,"<"   ,"\xF6","\x8E","\xD6","\x8E","\x1F",""    } },  // ,
  {0, 2,0x08,0,21, 8,2,2,6, 5,0,{"."   ,">"   ,"."   ,">"   ,"\xFC","\xA4","\xFC","\xA4","\x1D",""    } },  // .
  {0, 2,0x10,0,23, 8,2,2,6, 5,0,{"/"   ,"?"   ,"/"   ,"?"   ,"\x86","\xA1","\x86","\xA1","\x80",""    } },  // /
  {0, 2,0x20,0,25, 8,2,2,6, 5,0,{""    ,"_"   ,""    ,"_"   ,"\xFD","\x85","\xFD","\x85","\x83",""    } },  // _
//60,5
  {0, 6,0x08,2, 1,10, 3,2,1,5,1,{"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" ,"CAPS" } },  // CAPS
  {0, 6,0x04,1, 4,10, 3,2,1,5,1,{"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" ,"GRPH" } },  // GRAPH
  {0, 6,0x10,2, 7,10, 3,2,6,5,0,{"\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5","\x96\xE5"} },  // かな
  {0, 8,0x01,0,10,10,11,2,6,5,0,{"SPACE","SPACE","SPACE","SPACE","SPACE","SPACE","SPACE","SPACE","SPACE","SPACE"} },  // SPACE

  {0, 8,0x20,0,27,12, 2,2,6,5,1,{"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" } },  // UP
  {0, 8,0x10,0,25,14, 2,2,6,5,1,{"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" } },  // LEFT
  {0, 8,0x80,0,29,14, 2,2,6,5,1,{"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" } },  // RIGHT
  {0, 8,0x40,0,27,16, 2,2,6,5,1,{"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" } },  // DOWN
  {0, 8,0x30,0,25,12, 2,2,6,5,1,{""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     } },  // UP LEFT
  {0, 8,0xA0,0,29,12, 2,2,6,5,1,{""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     } },  // UP RIGHT
  {0, 8,0x50,0,25,16, 2,2,6,5,1,{""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     } },  // DOWN LEFT
  {0, 8,0xC0,0,29,16, 2,2,6,5,1,{""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     ,""     } },  // DOWN RIGHT

  {0,10,0x04,0,15,12, 2,2,6,5,0,{"7" ,"7" ,"7" ,"7" ,"7" ,"7" ,"7" ,"7" ,"7" ,"7" } }, //83 NUM 7
  {0,10,0x08,0,17,12, 2,2,6,5,0,{"8" ,"8" ,"8" ,"8" ,"8" ,"8" ,"8" ,"8" ,"8" ,"8" } }, //84 NUM 8
  {0,10,0x10,0,19,12, 2,2,6,5,0,{"9" ,"9" ,"9" ,"9" ,"9" ,"9" ,"9" ,"9" ,"9" ,"9" } }, //85 NUM 9
  {0, 9,0x04,0,21,12, 2,2,6,5,0,{"/" ,"/" ,"/" ,"/" ,"/" ,"/" ,"/" ,"/" ,"/" ,"/" } }, //75 NUM /
  {0, 9,0x80,0,15,14, 2,2,6,5,0,{"4" ,"4" ,"4" ,"4" ,"4" ,"4" ,"4" ,"4" ,"4" ,"4" } }, //80 NUM 4
  {0,10,0x01,0,17,14, 2,2,6,5,0,{"5" ,"5" ,"5" ,"5" ,"5" ,"5" ,"5" ,"5" ,"5" ,"5" } }, //81 NUM 5
  {0,10,0x02,0,19,14, 2,2,6,5,0,{"6" ,"6" ,"6" ,"6" ,"6" ,"6" ,"6" ,"6" ,"6" ,"6" } }, //82 NUM 6
  {0, 9,0x01,0,21,14, 2,2,6,5,0,{"*" ,"*" ,"*" ,"*" ,"*" ,"*" ,"*" ,"*" ,"*" ,"*" } }, //73 NUM *
  {0, 9,0x10,0,15,16, 2,2,6,5,0,{"1" ,"1" ,"1" ,"1" ,"1" ,"1" ,"1" ,"1" ,"1" ,"1" } }, //77 NUM 1
  {0, 9,0x20,0,17,16, 2,2,6,5,0,{"2" ,"2" ,"2" ,"2" ,"2" ,"2" ,"2" ,"2" ,"2" ,"2" } }, //78 NUM 2
  {0, 9,0x40,0,19,16, 2,2,6,5,0,{"3" ,"3" ,"3" ,"3" ,"3" ,"3" ,"3" ,"3" ,"3" ,"3" } }, //79 NUM 3
  {0,10,0x20,0,21,16, 2,2,6,5,0,{"-" ,"-" ,"-" ,"-" ,"-" ,"-" ,"-" ,"-" ,"-" ,"-" } }, //86 NUM -
  {0, 9,0x08,0,15,18, 2,2,6,5,0,{"0" ,"0" ,"0" ,"0" ,"0" ,"0" ,"0" ,"0" ,"0" ,"0" } }, //76 NUM 0
  {0,10,0x80,0,17,18, 2,2,6,5,0,{"." ,"." ,"." ,"." ,"." ,"." ,"." ,"." ,"." ,"." } }, //88 NUM .
  {0,10,0x40,0,19,18, 2,2,6,5,0,{"," ,"," ,"," ,"," ,"," ,"," ,"," ,"," ,"," ,"," } }, //87 NUM ,
  {0, 9,0x02,0,21,18, 2,2,6,5,0,{"+" ,"+" ,"+" ,"+" ,"+" ,"+" ,"+" ,"+" ,"+" ,"+" } }, //74 NUM +
  {0, 0,0x00,0, 0, 0, 0,0,0,0,0,{"" ,"" ,"" ,"" ,"" ,"" ,"" ,"" ,"" ,"" } }, // END
};
#endif
	
#if 0
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //89 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //90 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //91 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //92 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //93 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //94 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //95 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //96 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //97 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //98 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //99 reserved
	{ 0,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //100 reserved
	/* joy */
	{ 1,0x00,0x01,1,{"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" } }, //101 joy1 UP
	{ 1,0x00,0x02,1,{"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" } }, //102 joy1 DOWN
	{ 1,0x00,0x04,1,{"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" } }, //103 joy1 LEFT
	{ 1,0x00,0x08,1,{"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" } }, //104 joy1 RIGHT
	{ 1,0x00,0x05,1,{"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" } }, //105 joy1 UP LEFT
	{ 1,0x00,0x09,1,{"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" } }, //106 joy1 UP RIGHT
	{ 1,0x00,0x06,1,{"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" } }, //107 joy1 DOWN LEFT
	{ 1,0x00,0x0A,1,{"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" } }, //108 joy1 DOWN RIGHT
	{ 1,0x00,0x10,1,{"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   } }, //109 joy1 BTN1
	{ 1,0x00,0x20,1,{"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   } }, //110 joy1 BTN2
	{ 1,0x01,0x01,1,{"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" } }, //111 joy2 UP
	{ 1,0x01,0x02,1,{"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" } }, //112 joy2 DOWN
	{ 1,0x01,0x04,1,{"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" } }, //113 joy2 LEFT
	{ 1,0x01,0x08,1,{"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" } }, //114 joy2 RIGHT
	{ 1,0x00,0x05,1,{"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" ,"\x7F" } }, //115 joy1 UP LEFT
	{ 1,0x00,0x09,1,{"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" ,"\x80" } }, //116 joy1 UP RIGHT
	{ 1,0x00,0x06,1,{"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" ,"\x82" } }, //117 joy1 DOWN LEFT
	{ 1,0x00,0x0A,1,{"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" ,"\x81" } }, //118 joy1 DOWN RIGHT
	{ 1,0x01,0x10,1,{"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   ,"T1"   } }, //119 joy2 BTN1
	{ 1,0x01,0x20,1,{"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   ,"T2"   } }, //120 joy2 BTN2
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //121 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //122 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //123 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //124 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //125 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //126 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //127 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //128 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //129 reserved
	{ 1,0x00,0x00,0,{0,0,0,0,0,0,0,0,0,0} }, //130 reserved
	/* main menu */
	{ 2,MAINMENU, 0,00 }
};
#endif
