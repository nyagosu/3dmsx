/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                          MSX.c                          **/
/**                                                         **/
/** This file contains implementation for the MSX-specific  **/
/** hardware: slots, memory mapper, PPIs, VDP, PSG, clock,  **/
/** etc. Initialization code and definitions needed for the **/
/** machine-dependent drivers are also here.                **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2005                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include "MSX.h"
//#include "sound.h"
//#include "ipc2.h"
#include "DSLua.h"
#include "DSDriver.h"
#include "Floppy.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>

//#include <fifocommon.h>

#include "emu2149.h"
#include "emu2212.h"

#define PRINTOK           if(Verbose) LOG("OK")
#define PRINTFAILED       if(Verbose) LOG("FAILED")
#define PRINTRESULT(R)    if(Verbose) LOG((char*)((R)? "OK":"FAILED"))

/** User-defined parameters for fMSX *************************/
int  Mode        = MSX_MSX2|MSX_NTSC; //|MSX_GUESSA|MSX_GUESSB;
byte Verbose     = 1;              /* Debug msgs ON/OFF      */
byte UPeriod     = 100;            /* Interrupts/scr update% */
int  VPeriod     = CPU_VPERIOD;    /* CPU cycles per VBlank  */
int  HPeriod     = CPU_HPERIOD;    /* CPU cycles per HBlank  */
int  RAMPages    = 4;              /* Number of RAM pages    */
int  VRAMPages   = 2;              /* Number of VRAM pages   */
byte Power       = 0;              /* 0:Power off  1:On      */

//int  NewMode     = MSX_MSX2|MSX_NTSC|MSX_GUESSA|MSX_GUESSB;
//int  NewRAMPages = 4;              /* Number of RAM pages    */
//int  NewVRAMPages= 2;              /* Number of VRAM pages   */

/** Main hardware: CPU, RAM, VRAM, mappers *******************/
Z80 CPU;                           /* Z80 CPU state and regs */

byte *VRAM,*VPAGE;                 /* Video RAM              */

byte *RAM[8];                      /* Main RAM (8x8kB pages) */
byte *EmptyRAM;                    /* Empty RAM page (8kB)   */
byte SaveCMOS;                     /* Save CMOS.ROM on exit  */
byte *MemMap[4][4][8];   /* Memory maps [PPage][SPage][Addr] */

byte *RAMData;                     /* RAM Mapper contents    */
byte RAMMapper[4];                 /* RAM Mapper state       */
byte RAMMask;                      /* RAM Mapper mask        */

byte *ROMData[MAXSLOTS];           /* ROM Mapper contents    */
byte ROMMapper[MAXSLOTS][4];       /* ROM Mappers state      */
byte ROMMask[MAXSLOTS];            /* ROM Mapper masks       */
byte ROMType[MAXSLOTS];            /* ROM Mapper types       */

byte EnWrite[4];                   /* 1 if write enabled     */
byte PSL[4],SSL[4];                /* Lists of current slots */
byte PSLReg,SSLReg[4];   /* Storage for A8h port and (FFFFh) */

BIOS bios[MAXBIOS];                /* BIOS settings          */

byte ChrGenType;                  /* BIOS CharcterGen. types */
byte DateFormatType;              /* BIOS Date format types  */
byte KeyboardType;                /* BIOS Keyboard types     */

/** Memory blocks to free in TrashMSX() **********************/
byte *Chunks[MAXCHUNKS];           /* Memory blocks to free  */
int NChunks;                       /* Number of memory blcks */

/** Working directory names **********************************/
char *ProgDir    = NULL;           /* Program directory      */

/** Cartridge files used by fMSX *****************************/
char *ROMName[MAXSLOTS]  = {0,0,0,0,0,0};

/** On-cartridge SRAM data ***********************************/
char *SRAMName[MAXSLOTS] = {0,0,0,0,0,0};/* Filenames (gen-d)*/
byte SaveSRAM [MAXSLOTS] = {0,0,0,0,0,0};/* Save SRAM on exit*/
byte *SRAMData[MAXSLOTS];          /* SRAM (battery backed)  */

/** Disk images used by fMSX *********************************/
char *DSKName[MAXDRIVES] = { NULL, NULL };

/** Emulation state saving ***********************************/
char *StateName  = NULL;           /* State file (autogen-d) */

/** Printer **************************************************/
char *PrnName    = NULL;           /* Printer redirect. file */
FILE *PrnStream;

/** Cassette tape ********************************************/
char *CasName   = NULL;
LLIST* CasData  = NULL;
int CasPtr      = 0;

/** Serial port **********************************************/
char *ComName    = NULL;           /* Serial redirect. file  */
FILE *ComIStream;
FILE *ComOStream;

/** Kanji font ROM *******************************************/
char *KanjiName  = NULL;           /* Kanji.ROM file         */
byte *Kanji;                       /* Kanji ROM 4096x32      */
int  KanLetter;                    /* Current letter index   */
byte KanCount;                     /* Byte count 0..31       */

/** CMOS data  ***********************************************/
char *CMOSName = NULL;             /* CMOS data file name    */

/** Keyboard, joystick, and mouse ****************************/
/*volatile*/ byte MSXKeyMap[16];   /* Keyboard map           */
word JoyState;                     /* Joystick states        */
int  MouState[2];                  /* Mouse states           */
byte MouseDX[2],MouseDY[2];        /* Mouse offsets          */
byte OldMouseX[2],OldMouseY[2];    /* Old mouse coordinates  */
byte MCount[2];                    /* Mouse nibble counter   */

/** General I/O registers: i8255 *****************************/
I8255 PPI;                         /* i8255 PPI at A8h-ABh   */
byte IOReg;                        /* Storage for AAh port   */

/** Disk controller: WD1793 **********************************/
WD1793 FDC;                        /* WD1793 at 7FF8h-7FFFh  */
FDIDisk FDD[4];                    /* Floppy disk images     */

/** Sound hardware: PSG, SCC, OPLL ***************************/
PSG * psg;
byte PSGLatch;

SCC * scc;
byte SCCOn[2];                     /* 1 = SCC page active    */
int  SCCPSlot = -1;                /* SCC+ cart slot No.     */
byte *SCCRAMData;                  /* SCC Mapper RAM data    */
byte SCC_RAM_MODE[4];              /* SCC RAM Mode           */

// OPLL* opll;
byte OPLLLatch;
word FMPACKey;                     /* MAGIC = SRAM active    */

int  SampleRate  = 0x2000;         /* wave form sample rate  */
//int  NewSampleRate = 0x2000;

/** Serial I/O hardware: i8251+i8253 *************************/
I8251 SIO;                         /* SIO registers & state  */

/** Real-time clock ******************************************/
byte RTCReg,RTCMode;               /* RTC register numbers   */
byte RTC[4][13];                   /* RTC registers          */

/** Video processor ******************************************/
byte *ChrGen,*ChrTab,*ColTab;      /* VDP tables (screen)    */
byte *SprGen,*SprTab;              /* VDP tables (sprites)   */
int  ChrGenM,ChrTabM,ColTabM;      /* VDP masks (screen)     */
int  SprTabM;                      /* VDP masks (sprites)    */
word VAddr;                        /* VRAM address in VDP    */
byte VKey,PKey,WKey;               /* Status keys for VDP    */
byte FGColor,BGColor;              /* Colors                 */
byte XFGColor,XBGColor;            /* Second set of colors   */
byte ScrMode;                      /* Current screen mode    */
byte VDP[64],VDPStatus[16];        /* VDP registers          */
byte IRQPending;                   /* Pending interrupts     */
int  ScanLine;                     /* Current scanline       */
byte VDPData;                      /* VDP data buffer        */
byte PLatch;                       /* Palette buffer         */
byte ALatch;                       /* Address buffer         */
int  Palette[16];                  /* Current palette        */
//byte InterlacePage;              /* Chg or Interlace Page  */


/** Places in DiskROM Slot 0:PS 1:SS 2:PAGE ******************/
int PatchSlot[3];

/** Places in DiskROM to be patched with ED FE C9 ************/
static const word DiskPatches[] =
{ 0x4010,0x4013,0x4016,0x401C,0x401F,0 };

/** Places in BIOS to be patched with ED FE C9 ***************/
static const word BIOSPatches[] =
{ 0x00E1,0x00E4,0x00E7,0x00EA,0x00ED,0x00F0,0x00F3,0 };

/** Cartridge map, by primary and secondary slots ************/
byte CartMap[4][4] =
{ { 255,3,4,5 },{ 0,255,255,255 },{ 1,255,255,255 },{ 2,255,255,255 } };

/*** VDP status register states: ***/
static const byte VDPSInit[16] = { 0x9F,0,0x6C,0,0,0,0,0,0,0,0,0,0,0,0,0 };

/*** VDP control register states: ***/
static const byte VDPInit[64]  =
{
  0x00,0x10,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


/** Screen Mode Handlers [number of screens + 1] *************/
void (*RefreshLine[MAXSCREEN+2])(byte Y) =
{
  RefreshLine0,   /* SCR 0:  TEXT 40x24  */
  RefreshLine1,   /* SCR 1:  TEXT 32x24  */
  RefreshLine2,   /* SCR 2:  BLK 256x192 */
  RefreshLine3,   /* SCR 3:  64x48x16    */
  RefreshLine4,   /* SCR 4:  BLK 256x192 */
  RefreshLine5,   /* SCR 5:  256x192x16  */
  RefreshLine6,   /* SCR 6:  512x192x4   */
  RefreshLine7,   /* SCR 7:  512x192x16  */
  RefreshLine8,   /* SCR 8:  256x192x256 */
  0,              /* SCR 9:  NONE        */
  RefreshLine10,  /* SCR 10: YAE 256x192 */
  RefreshLine10,  /* SCR 11: YAE 256x192 */
  RefreshLine12,  /* SCR 12: YJK 256x192 */
  RefreshLineTx80 /* SCR 0:  TEXT 80x24  */
};

int SCR_X = 0;
int SCR_Y = 0;

/** VDP Address Register Masks *******************************/
static const struct { byte R2,R3,R4,R5,M2,M3,M4,M5; } MSK[MAXSCREEN+2] =
{
  { 0x7F,0x00,0x3F,0x00,0x00,0x00,0x00,0x00 }, /* SCR 0:  TEXT 40x24  */
  { 0x7F,0xFF,0x3F,0xFF,0x00,0x00,0x00,0x00 }, /* SCR 1:  TEXT 32x24  */
  { 0x7F,0x80,0x3C,0xFF,0x00,0x7F,0x03,0x00 }, /* SCR 2:  BLK 256x192 */
  { 0x7F,0x00,0x3F,0xFF,0x00,0x00,0x00,0x00 }, /* SCR 3:  64x48x16    */
  { 0x7F,0x80,0x3C,0xFC,0x00,0x7F,0x03,0x03 }, /* SCR 4:  BLK 256x192 */
  { 0x60,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 5:  256x192x16  */
  { 0x60,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 6:  512x192x4   */
  { 0x20,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 7:  512x192x16  */
  { 0x20,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 8:  256x192x256 */
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* SCR 9:  NONE        */
  { 0x20,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 10: YAE 256x192 */
  { 0x20,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 11: YAE 256x192 */
  { 0x20,0x00,0x00,0xFC,0x1F,0x00,0x00,0x03 }, /* SCR 12: YJK 256x192 */
  { 0x7C,0xF8,0x3F,0x00,0x03,0x07,0x00,0x00 }  /* SCR 0:  TEXT 80x24  */
};

/** MegaROM Mapper Names *************************************/
char *ROMNames[MAXMAPPERS+1] = 
{ 
  "GENERIC/8kB","GENERIC/16kB","KONAMI5/8kB",
  "KONAMI4/8kB","ASCII/8kB","ASCII/16kB",
  "GMASTER2/SRAM","FMPAC/SRAM","SCC SOUND","UNKNOWN"
};

/** Keyboard Mapping *****************************************/
/** This keyboard mapping is used by KBD_SET()/KBD_RES()    **/
/** macros to modify KeyMap[] bits.                         **/
/*************************************************************/
#if 0
const byte Keys[][2] =
{
  { 0,0x00 },{ 8,0x10 },{ 8,0x20 },{ 8,0x80 }, /* None,LEFT,UP,RIGHT */
  { 8,0x40 },{ 6,0x01 },{ 6,0x02 },{ 6,0x04 }, /* DOWN,SHIFT,CONTROL,GRAPH */
  { 7,0x20 },{ 7,0x08 },{ 6,0x08 },{ 7,0x40 }, /* BS,TAB,CAPSLOCK,SELECT */
  { 8,0x02 },{ 7,0x80 },{ 8,0x08 },{ 8,0x04 }, /* HOME,ENTER,DELETE,INSERT */
  { 6,0x10 },{ 7,0x10 },{ 6,0x20 },{ 6,0x40 }, /* COUNTRY,STOP,F1,F2 */
  { 6,0x80 },{ 7,0x01 },{ 7,0x02 },{ 9,0x08 }, /* F3,F4,F5,PAD0 */
  { 9,0x10 },{ 9,0x20 },{ 9,0x40 },{ 7,0x04 }, /* PAD1,PAD2,PAD3,ESCAPE */
  { 9,0x80 },{10,0x01 },{10,0x02 },{10,0x04 }, /* PAD4,PAD5,PAD6,PAD7 */
  { 8,0x01 },{10,0x08 },{10,0x10 },{ 0,0x00 }, /* SPACE,PAD8,PAD9,None */
  { 0,0x00 },{ 0,0x00 },{ 0,0x00 },{ 2,0x01 }, /* None,None,None,['] */
  { 0,0x00 },{ 0,0x00 },{ 0,0x00 },{ 0,0x00 }, /* 0x28 */
  { 2,0x04 },{ 1,0x04 },{ 2,0x08 },{ 2,0x10 }, /* [,],[-],[.],[/] */
  { 0,0x01 },{ 0,0x02 },{ 0,0x04 },{ 0,0x08 }, /* 0,1,2,3 */
  { 0,0x10 },{ 0,0x20 },{ 0,0x40 },{ 0,0x80 }, /* 4,5,6,7 */
  { 1,0x01 },{ 1,0x02 },{ 0,0x00 },{ 1,0x80 }, /* 8,9,None,[;] */
  { 0,0x00 },{ 1,0x08 },{ 0,0x00 },{ 0,0x00 }, /* None,[=],None,None */
  { 0,0x00 },{ 2,0x40 },{ 2,0x80 },{ 3,0x01 }, /* None,A,B,C */
  { 3,0x02 },{ 3,0x04 },{ 3,0x08 },{ 3,0x10 }, /* D,E,F,G */
  { 3,0x20 },{ 3,0x40 },{ 3,0x80 },{ 4,0x01 }, /* H,I,J,K */
  { 4,0x02 },{ 4,0x04 },{ 4,0x08 },{ 4,0x10 }, /* L,M,N,O */
  { 4,0x20 },{ 4,0x40 },{ 4,0x80 },{ 5,0x01 }, /* P,Q,R,S */
  { 5,0x02 },{ 5,0x04 },{ 5,0x08 },{ 5,0x10 }, /* T,U,V,W */
  { 5,0x20 },{ 5,0x40 },{ 5,0x80 },{ 1,0x20 }, /* X,Y,Z,[[] */
  { 1,0x10 },{ 1,0x40 },{ 0,0x00 },{ 0,0x00 }, /* [\],[]],None,None */
  { 2,0x02 },{ 2,0x40 },{ 2,0x80 },{ 3,0x01 }, /* [`],a,b,c */
  { 3,0x02 },{ 3,0x04 },{ 3,0x08 },{ 3,0x10 }, /* d,e,f,g */
  { 3,0x20 },{ 3,0x40 },{ 3,0x80 },{ 4,0x01 }, /* h,i,j,k */
  { 4,0x02 },{ 4,0x04 },{ 4,0x08 },{ 4,0x10 }, /* l,m,n,o */
  { 4,0x20 },{ 4,0x40 },{ 4,0x80 },{ 5,0x01 }, /* p,q,r,s */
  { 5,0x02 },{ 5,0x04 },{ 5,0x08 },{ 5,0x10 }, /* t,u,v,w */
  { 5,0x20 },{ 5,0x40 },{ 5,0x80 },{ 0,0x00 }, /* x,y,z,None */
  { 0,0x00 },{ 0,0x00 },{ 0,0x00 },{ 0,0x00 }  /* 0x7C */
};
#endif
/** Internal Functions ***************************************/
/** These functions are defined and internally used by the  **/
/** code in MSX.c.                                          **/
/*************************************************************/
byte *LoadROM(const char *Name,int Size,byte *Buf);
int  EjectCart(int Slot);
int  LoadCart(int Slot);
int  GuessROM(const byte *Buf,int Size);
void SetMegaROM(int Slot,byte P0,byte P1,byte P2,byte P3);
void MapROM(word A,byte V);       /* Switch MegaROM banks            */
void PSlot(byte V);               /* Switch primary slots            */
void SSlot(byte V);               /* Switch secondary slots          */
void VDPOut(byte R,byte V);       /* Write value into a VDP register */
void Printer(byte V);             /* Send a character to a printer   */
void PPIOut(byte New,byte Old);   /* Set PPI bits (key click, etc.)  */
void CheckSprites(void);          /* Check collisions and 5th sprite */
byte RTCIn(byte R);               /* Read RTC registers              */
byte SetScreen(void);             /* Change screen mode              */
word SetIRQ(byte IRQ);            /* Set/Reset IRQ                   */
word StateID(void);               /* Compute emulation state ID      */

//static int stricmpn(const char *S1,const char *S2,int Limit);
byte *GetMemory(int Size); /* Get memory chunk                */
void FreeMemory(byte *Ptr);/* Free memory chunk               */
static void FreeAllMemory(void);  /* Free all memory chunks          */

byte system_RTCIn( byte R );
void RefreshScreen( void );

void ResetSound(int smpl);
void StartSound(void);

/** stricmpn() ***********************************************/
/** Case-indifferent comparison of up to Length characters. **/
/*************************************************************/
/*
static int stricmpn(const char *S1,const char *S2,int Limit)
{
  for(;*S1&&*S2&&Limit&&(toupper(*S1)==toupper(*S2));++S1,++S2,--Limit);
  return(Limit? toupper(*S1)-toupper(*S2):0);
}
*/
/** GetMemory() **********************************************/
/** Allocate a memory chunk of given size using malloc().   **/
/** Store allocated address in Chunks[] for later disposal. **/
/*************************************************************/
byte *GetMemory(int Size)
{
  byte *P;

  if((Size<=0)||(NChunks>=MAXCHUNKS)) return(0);
  P=(byte *)malloc(Size);
  if(P) Chunks[NChunks++]=P;
  return(P);
}

/** FreeMemory() *********************************************/
/** Free memory allocated by a previous GetMemory() call.   **/
/*************************************************************/
void FreeMemory(byte *Ptr)
{
  int J;

  for(J=0;(J<NChunks)&&(Ptr!=Chunks[J]);++J);
  if(J<NChunks)
  {
    for(--NChunks;J<NChunks;++J) Chunks[J]=Chunks[J+1];
    free(Ptr);
  }
}

/** FreeAllMemory() ******************************************/
/** Free all memory allocated by GetMemory() calls.         **/
/*************************************************************/
static void FreeAllMemory(void)
{
  int J;
  for(J=0;J<NChunks;++J) free((void *)Chunks[J]);
  NChunks=0;
}

/** InitMSX() ************************************************/
/** Allocate memory, load ROM images, initialize hardware,  **/
/** CPU and start the emulation. This function returns 0 in **/
/** the case of failure.                                    **/
/*************************************************************/
int InitMSX(void)
{
  /*** Joystick types: ***/
/*
  static const char *JoyTypes[] =
  {
    "nothing","normal joystick",
    "mouse in joystick mode","mouse in real mode"
  };
*/
  int *T;

  LOG( "InitMSX..." );

  /*** STARTUP CODE starts here: ***/

  T=(int *)"\01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
#ifdef LSB_FIRST
  if(*T!=1)
  {
    printf("********** This machine is high-endian. **********\n");
    printf("Take #define LSB_FIRST out and compile fMSX again.\n");
    return(0);
  }
#else
  if(*T==1)
  {
    printf("********* This machine is low-endian. **********\n");
    printf("Insert #define LSB_FIRST and compile fMSX again.\n");
    return(0);
  }
#endif

  LOG( " Endian check OK." );

  /* Allocate 16kB for the empty space (scratch RAM) */
  if(Verbose) printf("Allocating 16kB for empty space...");
  if(!(EmptyRAM=(byte*)malloc(0x4000))) { PRINTFAILED;return(0); }
  memset(EmptyRAM,NORAM,0x4000);

  return 1;
}

/** LoadBios() ************************************************/
/* load BIOS file and mapping Memorymap                      */
/*************************************************************/
int LoadBios(int a, int b, int c, char * fn, int siz )
{
	byte *P;
	P=LoadROM(fn,siz,0);
	if(!P) return(0);
	MemMap[a][b][c]=P;
	if( siz>0x2000)MemMap[a][b][c+1]=P+0x2000;
	if( siz>0x4000)MemMap[a][b][c+2]=P+0x4000;
	if( siz>0x6000)MemMap[a][b][c+3]=P+0x6000;

	// MAIN BIOS ROM?
	if( (a==0)&&(b==0)&&(c==0) ){
		/* マシンの情報をMainBiosからセットする */
		Mode=(Mode&~MSX_MODEL)|(P[0x2D]&MSX_MODEL);					/* MSX Version */
		Mode=(Mode&~MSX_VIDEO)|((P[0x2B]&0x80)?MSX_PAL:MSX_NTSC);	/* Video Mode */
		ChrGenType     = P[0x2B]&0x0F;								/* キャラジェネのtype 0:日本 1:アメリカ 2:ロシア */
		DateFormatType =(P[0x2B]&0x70)>>4;							/* 日付フォーマット   0:YYMMDD  1:MMDDYY 2:DDMMYY */
		KeyboardType   = P[0x2C]&0x0F;								/* キーボード         0:日本 1:アメリカ 2:フランス 3:イギリス 4:ドイツ 5:ロシア 6:スペイン */
	}
	return 1;
}
/** loadSCCSound() ********************************************/
/**************************************************************/
int loadSCCSound(int slot )
{
	// SCC Sound cartridge
	ROMType[slot]=MAP_SCC;
	SCC_RAM_MODE[0] = 0;			/* モードレジスタ = bank select */
	SCC_RAM_MODE[1] = 0;			/* モードレジスタ = bank select */
	SCC_RAM_MODE[2] = 0;			/* モードレジスタ = bank select */
	SCC_RAM_MODE[3] = 0;			/* モードレジスタ = bank select */
	MemMap[slot+1][0][2]=SCCRAMData;
	MemMap[slot+1][0][3]=SCCRAMData + 0x2000;
	MemMap[slot+1][0][4]=SCCRAMData + 0x4000;
	MemMap[slot+1][0][5]=SCCRAMData + 0x6000;
	ROMMask[slot] = 0xF;			/* 16ページ */
	ROMMapper[slot][0]=0;
	ROMMapper[slot][1]=1;
	ROMMapper[slot][2]=2;
	ROMMapper[slot][3]=3;
    SCC_set_type( scc, SCC_ENHANCED );
	LOG( "SCC Cart insert Slot %d", slot + 1 );
	return 1;
}

#if 0
/** OpenPrinter() ********************************************/
/*************************************************************/
/*
int OpenPrinter()
{
  if(!PrnName) PrnStream=stdout;
  else
  {
    if(Verbose) printf("Redirecting printer output to %s...",PrnName);
    if(!(PrnStream=fopen(PrnName,"wb"))) PrnStream=stdout;
    PRINTRESULT(PrnStream!=stdout);
  }
}
*/
/** OpenSerial() *********************************************/
/*************************************************************/
/*
int OpenSerial()
{
	if(!ComName) 
	{
		ComIStream=stdin;
		ComOStream=stdout;
	} else {
		if(Verbose) printf("Redirecting serial I/O to %s...",ComName);
		if(!(ComOStream=ComIStream=fopen(ComName,"r+b")))
		{ ComIStream=stdin;ComOStream=stdout; }
		PRINTRESULT(ComOStream!=stdout);
	}
}
*/

/** initPallette() *******************************************/
/*************************************************************/
int initPallette(void)
{
	/*** Initial palette: ***/
	static unsigned int PalInit[16] =
	{
	  0x00000000,0x00000000,0x0020C020,0x0060E060,
	  0x002020E0,0x004060E0,0x00A02020,0x0040C0E0,
	  0x00E02020,0x00E06060,0x00C0C020,0x00C0C080,
	  0x00208020,0x00C040A0,0x00A0A0A0,0x00E0E0E0
	};
	int J;
	for(J=0;J<16;J++)
	{
		Palette[J]=PalInit[J];
    	SetColor(J,(Palette[J]>>16)&0xFF,(Palette[J]>>8)&0xFF,Palette[J]&0xFF);
	}
	return 1;
}
#endif
/** bootMSX() ************************************************/
/*************************************************************/
int bootMSX(void)
{
	int I,J,K;
	byte *P1, *P;
//	word A;

	/*** Initial palette: ***/
	static const unsigned int PalInit[16] =
	{
		0x00000000,0x00000000,0x0020C020,0x0060E060,
		0x002020E0,0x004060E0,0x00A02020,0x0040C0E0,
		0x00E02020,0x00E06060,0x00C0C020,0x00C0C080,
		0x00208020,0x00C040A0,0x00A0A0A0,0x00E0E0E0
	};

	/*** CMOS ROM default values: ***/
	static const byte RTCInit[4][13]  =
	{
		{  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{  0, 0, 0, 0,40,80,15, 4, 4, 0, 0, 0, 0 },
		{  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};

	LOG( "bootMSX" );

	/* Zero everyting */
//	CasStream = NULL;
	PrnStream = NULL;
	ComIStream= NULL;
	ComOStream= NULL;
	VRAM      = NULL;
	Kanji     = NULL;
	SaveCMOS  = 0;
	FMPACKey  = 0x0000;
	NChunks   = 0;

	/* Zero cartridge related data */
	for(J=0;J<MAXCARTS;J++)
	{
		ROMMask[J]  = 0;
		ROMData[J]  = NULL;
		ROMType[J]  = MAP_GUESS;
		SRAMData[J] = NULL;
		SRAMName[J] = NULL;
		SaveSRAM[J] = 0; 
	}
//	LOG( "bootMSX 1" );

	/* Reset memory map to the empty space */
	for(I=0;I<4;I++)
		for(J=0;J<4;J++)
			for(K=0;K<8;K++)
				MemMap[I][J][K]=EmptyRAM;
	
	/* Calculate IPeriod from VPeriod/HPeriod */
	if(UPeriod<1) UPeriod=100;
	VPeriod = (VIDEO(MSX_PAL)? CPU_V313:CPU_V262);
	HPeriod = CPU_HPERIOD;

	CPU.TrapBadOps = Verbose&0x10;
	CPU.IPeriod    = CPU_H240;
	CPU.IAutoReset = 0;

//	LOG( "bootMSX 2" );

	//RAM Pages
	if((RAMPages<(MODEL(MSX_MSX1)? 4:8))||(RAMPages>256)){
		RAMPages=MODEL(MSX_MSX1)? 4:8;
	}else{
		/* Number of RAM pages should be power of 2 */
		/* Calculate RAMMask=(2^RAMPages)-1 */
		for(J=1;J<RAMPages;J<<=1);
		RAMPages=J;
	}
	RAMMask=RAMPages-1;

	if(Verbose) printf("Allocating %dkB for RAM...",RAMPages*16);
	if((P1=GetMemory(RAMPages*0x4000))!=NULL)
	{
		memset(P1,NORAM,RAMPages*0x4000);
		FreeMemory(RAMData);
		RAMData  = P1;
	}
	PRINTRESULT(P1);

	//VRAM Pages
	if((VRAMPages<(MODEL(MSX_MSX1)? 2:8))||(VRAMPages>8)){
		VRAMPages=MODEL(MSX_MSX1)? 2:8;
	}else{
		/* Number of VRAM pages should be a power of 2 */
		for(J=1;J<VRAMPages;J<<=1);
		VRAMPages=J;
	}
	if(Verbose) printf("Allocating %dkB for VRAM...",VRAMPages*16);
	if((P1=GetMemory(VRAMPages*0x4000))!=NULL)
	{
		memset(P1,0x00,VRAMPages*0x4000);
		FreeMemory(VRAM);
		VRAM      = P1;
	}
	PRINTRESULT(P1);

//	LOG( "bootMSX 3" );

	// load bios
	for(I=0;I<16;I++){
		if( bios[I].name != NULL ){
			LOG( "BIOS Load[%s]", bios[I].name );
			LoadBios( bios[I].PS,bios[I].SS,bios[I].PG, bios[I].name, bios[I].siz );
		}
	}

//	LOG( "bootMSX 4" );

	// load cart
	for(I=0;I<MAXSLOTS;I++) LoadCart( I );

//	LOG( "bootMSX 5" );

	// load SCC+ cart
	if( (SCCPSlot != -1) && (ROMName[SCCPSlot]==NULL) ){
		loadSCCSound( SCCPSlot );
		LOG( "Insert SCC+ Cart in Slot %d", SCCPSlot );
	}

//	LOG( "bootMSX 6" );

	// load Kanji rom
	if( KanjiName ) Kanji=LoadROM(KanjiName,0,0);

	// patch BIOS
	if(Mode&MSX_PATCHBDOS){
		for(J=0;BIOSPatches[J];J++)
		{
			P=MemMap[0][0][0]+BIOSPatches[J];
			P[0]=0xED;P[1]=0xFE;P[2]=0xC9;
		}

		if( MemMap[PatchSlot[0]][PatchSlot[1]][PatchSlot[2]]!=EmptyRAM ){
			for(J=0;DiskPatches[J];J++)
			{
				P=MemMap[PatchSlot[0]][PatchSlot[0]][PatchSlot[0]]+DiskPatches[J]-0x4000;
				P[0]=0xED;P[1]=0xFE;P[2]=0xC9;
			}
		}
		LOG( "Patched DISK BIOS." );
	}

//	LOG( "bootMSX 7" );

	
	// CMOS loading
    if(CMOSName)
    {
		P1 = LoadROM(CMOSName, sizeof(RTC),(byte *)RTC);
		if(P1){
			LOG( "CMOS data loaded." );
		}else{
			memcpy(RTC,RTCInit,sizeof(RTC));
			LOG( "CMOS data use default." );
		}
    }else{
		memcpy(RTC,RTCInit,sizeof(RTC));
		LOG( "CMOS data use default." );
    }

	for(J=0;J<4;J++)
	{
		EnWrite[J]=0;                      /* Write protect ON for all slots */
		PSL[J]=SSL[J]=0;                   /* PSL=0:0:0:0, SSL=0:0:0:0       */
		MemMap[3][2][J*2]   = RAMData+(3-J)*0x4000;        /* RAMMap=3:2:1:0 */
		MemMap[3][2][J*2+1] = MemMap[3][2][J*2]+0x2000;
		RAMMapper[J]        = 3-J;
		RAM[J*2]            = MemMap[0][0][J*2];           /* Setting RAM    */
		RAM[J*2+1]          = MemMap[0][0][J*2+1];
	}

//	LOG( "bootMSX 8" );

  /* For all MegaROMs... */
  for(J=0;J<MAXSLOTS;++J)
    if((I=ROMMask[J]+1)>4)
    {
      /* For normal MegaROMs, set first four pages */
      if((ROMData[J][0]=='A')&&(ROMData[J][1]=='B'))
        SetMegaROM(J,0,1,2,3);
      /* Some MegaROMs default to last pages on reset */
      else if((ROMData[J][(I-2)<<13]=='A')&&(ROMData[J][((I-2)<<13)+1]=='B'))
        SetMegaROM(J,I-2,I-1,I-2,I-1);
      /* If 'AB' signature is not found at the beginning or the end */
      /* then it is not a MegaROM but rather a plain 64kB ROM       */
    }
	
	if(Verbose) printf("Initializing sound driver.");
	ResetSound(SampleRate);

//	LOG( "bootMSX 9" );

	/* Reset serial I/O */
	//Reset8251(&SIO,ComIStream,ComOStream);

	/* Reset PPI chips and slot selectors */
	//Reset8255(&PPI);

	PPI.Rout[0]=PSLReg=0x00;
	PPI.Rout[2]=IOReg=0x00;
	SSLReg[0]=0x00;
	SSLReg[1]=0x00;
	SSLReg[2]=0x00;
	SSLReg[3]=0x00;

	/* Reset floppy disk controller */
	Reset1793(&FDC,FDD,WD1793_KEEP);
	
	memcpy(VDP,VDPInit,sizeof(VDP));
	memcpy(VDPStatus,VDPSInit,sizeof(VDPStatus));

	/* Reset Keyboard */
	memset(MSXKeyMap,0xFF,16);               /* Keyboard         */

	/* Reset initial pallette */
	for(J=0;J<16;++J)
	{
	Palette[J]=PalInit[J];
	SetColor(J,(Palette[J]>>16)&0xFF,(Palette[J]>>8)&0xFF,Palette[J]&0xFF);
	}

	/* Reset mouse coordinates/counters */
	for(J=0;J<2;++J)
		MouState[J]=MouseDX[J]=MouseDY[J]=OldMouseX[J]=OldMouseY[J]=MCount[J]=0;
	
	IRQPending=0x00;                      /* No IRQs pending  */
	SCCOn[0]=SCCOn[1]=0;                  /* SCCs off for now */
	RTCReg=RTCMode=0;                     /* Clock registers  */
	KanCount=0;KanLetter=0;               /* Kanji extension  */
	ChrTab=ColTab=ChrGen=VRAM;            /* VDP tables       */
	SprTab=SprGen=VRAM;
	ChrTabM=ColTabM=ChrGenM=SprTabM=~0;   /* VDP addr. masks  */
	VPAGE=VRAM;                           /* VRAM page        */
	FGColor=BGColor=XFGColor=XBGColor=0;  /* VDP colors       */
	ScrMode=(KeyboardType==0?1:0);        /* Screen mode      */
	VKey=PKey=1;WKey=0;                   /* VDP keys         */
	VAddr=0x0000;                         /* VRAM access addr */
	ScanLine=0;                           /* Current scanline */
	VDPData=NORAM;                        /* VDP data buffer  */

	/* Set "V9958" VDP version for MSX2+ */
	if(MODEL(MSX_MSX2P)) VDPStatus[1]|=0x04;
//	LOG( "bootMSX 10" );

	/* Reset CPU */
	ResetZ80(&CPU);
	LOG( "Reset Z80" );

	/* keyborad 表示 */
//	drawAllKeyboard();
	/* keybind設定 */
	lua_setKeybind();

//	LOG( "bootMSX 12" );

	StartSound();
	
//	LOG( "bootMSX 13" );
	
	if(Verbose)
	{
		LOG("  %d CPU cycles per HBlank",HPeriod);
		LOG("  %d CPU cycles per VBlank",VPeriod);
		LOG("  %d scanlines",VPeriod/HPeriod);
		LOG("  Video Mode is %s",VIDEO(MSX_NTSC)?"NTSC":"PAL" );
	}

//	LOG( "bootMSX 14" );

	/* Start execution of the code */
	if(Verbose) LOG("Boot MSX.");
	Power = 1;
	/* A= */ RunZ80(&CPU);
	LOG( "Exited MSX." );

	TrashMSX();

	return(1);
}

/** TrashMSX() ***********************************************/
/** Free resources allocated by StartMSX().                 **/
/*************************************************************/
void TrashMSX(void)
{
	FILE* F;
	int J;
//	char buf[256];

  LOG( "Trash MSX." );

  /* Save CMOS RAM, if present */
  if(SaveCMOS)
  {
    if(Verbose) printf("Writing CMOS data...");
    F=fopen(CMOSName,"wb");
    if(F==NULL)
     SaveCMOS=0;
    else
    {
      if(fwrite(RTC,1,sizeof(RTC),F)!=sizeof(RTC)) SaveCMOS=0;
      fclose(F);
    }
    PRINTRESULT(SaveCMOS);
  }

  /* Shut down sound logging */
//  TrashMIDI();
  
  /* Close all IO streams */
/*
  if(PrnStream&&(PrnStream!=stdout))   fclose(PrnStream);
  if(ComOStream&&(ComOStream!=stdout)) fclose(ComOStream);
  if(ComIStream&&(ComIStream!=stdin))  fclose(ComIStream);
*/
  /* Eject all cartridges (will save SRAM) */
  for(J=0;J<MAXSLOTS;++J) EjectCart(J);
  LOG( "Eject Cart" );

  /* Eject all disks */
  for(J=0;J<MAXDRIVES;++J) ChangeDisk(J,0);
  LOG( "Eject Disk" );

  /* Eject all cassettes */
  ChangeCas(0);
  LOG( "Eject Cassette." );

  /* Free all remaining allocated memory */
  FreeAllMemory();

}

/** MapROM() *************************************************/
/** Switch ROM Mapper pages. This function is supposed to   **/
/** be called when ROM page registers are written to.       **/
/*************************************************************/
void MapROM(register word A,register byte V)
{
  byte I,J,PS,SS,*P;

/* @@@ For debugging purposes
printf("(%04Xh) = %02Xh at PC=%04Xh\n",A,V,CPU.PC.W);
*/

  J  = A>>14;           /* 16kB page number 0-3  */
  PS = PSL[J];          /* Primary slot number   */
  SS = SSL[J];          /* Secondary slot number */
  I  = CartMap[PS][SS]; /* Cartridge number      */

  /* Drop out if no cartridge in that slot */
  if(I>=MAXSLOTS) return;

  /* SCC: enable/disable for no cart */
/*
  if(!ROMData[I]){
	  if((A==0x9000)){
		SCCOn[I]=(V==0x3F)? 1:0;
*/
//		SCC_write(scc, R,V);
/*
	  }
  }
*/
  /* SCC: types 0, 2, or no cart */
//  if(((A&0xFF00)==0x9800)&&SCCOn[I])
//  {
    /* Compute SCC register number */
//    J=A&0x00FF;
    /* When no MegaROM present, we allow the program */
    /* to write into SCC wave buffer using EmptyRAM  */
    /* as a scratch pad.                             */
//    if(!ROMData[I]&&(J<0x80)) EmptyRAM[0x1800+J]=V;
    /* Output data to SCC chip */
//    WriteSCC(&SCChip,J,V);
//    SCC_write(scc, A,V);
//    return;
//  }

  /* SCC+: types 0, 2, or no cart */
//  if(((A&0xFF00)==0xB800)&&SCCOn[I])
//  {
    /* Compute SCC register number */
//    J=A&0x00FF;

    /* When no MegaROM present, we allow the program */
    /* to write into SCC wave buffer using EmptyRAM  */
    /* as a scratch pad.                             */
//    if(!ROMData[I]&&(J<0xA0)) EmptyRAM[0x1800+J]=V;

    /* Output data to SCC chip */
//    WriteSCCP(&SCChip,J,V);
//    SCC_write(scc, A,V);
//    return;
//  }

  /* If no cartridge or no mapper, exit */
  if(!ROMData[I]||!ROMMask[I]) return;

  switch(ROMType[I])
  {
    case MAP_GEN8: /* Generic 8kB cartridges (Konami, etc.) */
      /* Only interested in writes to 4000h-BFFFh */
      SCC_write(scc, A,V);
//      fifoSendValue32( FIFO_USER_01,(5<<24)|(A<<8)|V );

  	if((A<0x4000)||(A>0xBFFF)) break;
      J=(A-0x4000)>>13;
      /* Turn SCC on/off on writes to 8000h-9FFFh */
      if(J==2) SCCOn[I]=(V==0x3F)? 1:0;
      /* Switch ROM pages */
      V&=ROMMask[I];
      if(V!=ROMMapper[I][J])
      {
        RAM[J+2]=MemMap[PS][SS][J+2]=ROMData[I]+((int)V<<13);
        ROMMapper[I][J]=V;
      }

//      if(Verbose&0x08)
//        printf("ROM-MAPPER %c: 8kB ROM page #%d at %04Xh\n",I+'A',V,J*0x2000+0x4000);
      return;

    case MAP_GEN16: /* Generic 16kB cartridges (MSXDOS2, HoleInOneSpecial) */
      /* Only interested in writes to 4000h-BFFFh */
      if((A<0x4000)||(A>0xBFFF)) break;
      J=(A&0x8000)>>14;
      /* Switch ROM pages */
      V=(V<<1)&ROMMask[I];
      if(V!=ROMMapper[I][J])
      {
        RAM[J+2]=MemMap[PS][SS][J+2]=ROMData[I]+((int)V<<13);
        RAM[J+3]=MemMap[PS][SS][J+3]=RAM[J+2]+0x2000;
        ROMMapper[I][J]=V;
      }
//      if(Verbose&0x08)
//        printf("ROM-MAPPER %c: 16kB ROM page #%d at %04Xh\n",I+'A',V>>1,J*0x2000+0x4000);
      return;

    case MAP_SCC: /* KONAMI SCC Sound cartridges */
      // if((A<0x4000)||(A>0xBFFF)) break;
      SCC_write(scc, A,V);
//      fifoSendValue32 FIFO_USER_01,(5<<24)|(A<<8)|V );

      if(A==0xBFFE||A==0xBFFF){
         /* RAM Mode (0:ROM 0!:RAM) */
         SCC_RAM_MODE[0]=(V&0x11)						;	/* BANK 1 */
         SCC_RAM_MODE[1]=(V&0x12)						;	/* BANK 2 */
         SCC_RAM_MODE[2]=((V&0x24)==0x24)||(V&0x10)		;	/* BANK 3 */
         SCC_RAM_MODE[3]=(V&0x10)						;	/* BANK 4 */
         return;
      }

      J=(A-0x4000)>>13;		// 0x4000->0  0x6000->1 ...

      if( !SCC_RAM_MODE[J] ){									/* bank select mode  */
        if((A<0x5000)||(A>0xB000)||((A&0x1FFF)!=0x1000)) break;
        J=(A-0x5000)>>13;
        V&=ROMMask[I];
        if(V!=ROMMapper[I][J])
        {
            /* Switch RAM */
            RAM[J+2]=MemMap[PS][SS][J+2]=SCCRAMData+((int)V<<13);
            ROMMapper[I][J]=V;
        }
      }else{											        /* RAM mode */
        RAM[J+2][A&0x1FFF]=V;
      }
//      if(Verbose&0x08)
//        printf("ROM-MAPPER %c: 8kB ROM page #%d at %04Xh\n",I+'A',V,J*0x2000+0x4000);
      return;

    case MAP_KONAMI5: /* KONAMI5 8kB cartridges */
      SCC_write(scc, A,V);
      /* Only interested in writes to 5000h/7000h/9000h/B000h */
      if((A<0x5000)||(A>0xB000)||((A&0x1FFF)!=0x1000)) break;
      J=(A-0x5000)>>13;
      /* Turn SCC on/off on writes to 9000h */
      if(J==2) SCCOn[I]=(V==0x3F)? 1:0;
      /* Switch ROM pages */
      V&=ROMMask[I];
      if(V!=ROMMapper[I][J])
      {
        RAM[J+2]=MemMap[PS][SS][J+2]=ROMData[I]+((int)V<<13);
        ROMMapper[I][J]=V;
      }

//      if(Verbose&0x08)
//        printf("ROM-MAPPER %c: 8kB ROM page #%d at %04Xh\n",I+'A',V,J*0x2000+0x4000);
      return;

    case MAP_KONAMI4: /* KONAMI4 8kB cartridges */
      /* Only interested in writes to 6000h/8000h/A000h */
      /* (page at 4000h is fixed) */
      if((A<0x6000)||(A>0xA000)||(A&0x1FFF)) break;
      J=(A-0x4000)>>13;
      /* Switch ROM pages */
      V&=ROMMask[I];
      if(V!=ROMMapper[I][J])
      {
        RAM[J+2]=MemMap[PS][SS][J+2]=ROMData[I]+((int)V<<13);
        ROMMapper[I][J]=V;
      }
//      if(Verbose&0x08)
//        printf("ROM-MAPPER %c: 8kB ROM page #%d at %04Xh\n",I+'A',V,J*0x2000+0x4000);
      return;

    case MAP_ASCII8: /* ASCII 8kB cartridges */
      /* If switching pages... */
      if((A>=0x6000)&&(A<0x8000))
      {
        J=(A&0x1800)>>11;
        /* If selecting SRAM... */
        if(V&(ROMMask[I]+1))
        {
          /* Select SRAM page */
          V=0xFF;
          P=SRAMData[I];
//          if(Verbose&0x08)
//            printf("ROM-MAPPER %c: 8kB SRAM at %04Xh\n",I+'A',J*0x2000+0x4000);
        }
        else
        {
          /* Select ROM page */
          V&=ROMMask[I];
          P=ROMData[I]+((int)V<<13);
//          if(Verbose&0x08)
//            printf("ROM-MAPPER %c: 8kB ROM page #%d at %04Xh\n",I+'A',V,J*0x2000+0x4000);
        }
        /* If page was actually changed... */
        if(V!=ROMMapper[I][J])
        {
          MemMap[PS][SS][J+2]=P;
          ROMMapper[I][J]=V;
          /* Only update memory when cartridge's slot selected */
          if((PSL[(J>>1)+1]==PS)&&(SSL[(J>>1)+1]==SS)) RAM[J+2]=P;
        }
        /* Done with page switch */
        return;
      }
      /* Write to SRAM */
      if((A>=0x8000)&&(A<0xC000)&&(ROMMapper[I][((A>>13)&1)+2]==0xFF))
      {
        RAM[A>>13][A&0x1FFF]=V;
        SaveSRAM[I]=1;
        /* Done with SRAM write */
        return;
      }
      break;

    case MAP_ASCII16: /*** ASCII 16kB cartridges ***/
      /* If switching pages... */
      if((A>=0x6000)&&(A<0x8000))
      {
        J=(A&0x1000)>>11;
        /* If selecting SRAM... */
        if(V&(ROMMask[I]+1))
        {
          /* Select SRAM page */
          V=0xFF;
          P=SRAMData[I];
//          if(Verbose&0x08)
//            printf("ROM-MAPPER %c: 2kB SRAM at %04Xh\n",I+'A',J*0x2000+0x4000);
        }
        else
        {
          /* Select ROM page */
          V=(V<<1)&ROMMask[I];
          P=ROMData[I]+((int)V<<13);
//          if(Verbose&0x08)
//            printf("ROM-MAPPER %c: 16kB ROM page #%d at %04Xh\n",I+'A',V>>1,J*0x2000+0x4000);
        }
        /* If page was actually changed... */
        if(V!=ROMMapper[I][J])
        {
          MemMap[PS][SS][J+2]=P;
          MemMap[PS][SS][J+3]=P+0x2000;
          ROMMapper[I][J]=V;
          /* Only update memory when cartridge's slot selected */
          if((PSL[(J>>1)+1]==PS)&&(SSL[(J>>1)+1]==SS))
          {
            RAM[J+2]=P;
            RAM[J+3]=P+0x2000;
          }
        }
        /* Done with page switch */
        return;
      }
      /* Write to SRAM */
      if((A>=0x8000)&&(A<0xC000)&&(ROMMapper[I][2]==0xFF))
      {
        P=RAM[A>>13];
        A&=0x07FF;
        P[A+0x0800]=P[A+0x1000]=P[A+0x1800]=
        P[A+0x2000]=P[A+0x2800]=P[A+0x3000]=
        P[A+0x3800]=P[A]=V;
        SaveSRAM[I]=1;
        /* Done with SRAM write */
        return;
      }
      break;

    case MAP_GMASTER2: /* Konami GameMaster2+SRAM cartridge */
      /* Switch ROM and SRAM pages, page at 4000h is fixed */
      if((A>=0x6000)&&(A<=0xA000)&&!(A&0x1FFF))
      {
        /* Figure out which ROM page gets switched */
        J=(A-0x4000)>>13;
        /* If changing SRAM page... */
        if(V&0x10)
        {
          /* Select SRAM page */
          RAM[J+2]=MemMap[PS][SS][J+2]=SRAMData[I]+(V&0x20? 0x2000:0);
          /* SRAM is now on */
          ROMMapper[I][J]=0xFF;
//          if(Verbose&0x08)
//            printf("GMASTER2 %c: 4kB SRAM page #%d at %04Xh\n",I+'A',(V&0x20)>>5,J*0x2000+0x4000);
        }
        else
        {
          /* Compute new ROM page number */
          V&=ROMMask[I];
          /* If ROM page number has changed... */
          if(V!=ROMMapper[I][J])
          {
            RAM[J+2]=MemMap[PS][SS][J+2]=ROMData[I]+((int)V<<13);
            ROMMapper[I][J]=V;
          }
//          if(Verbose&0x08)
//            printf("GMASTER2 %c: 8kB ROM page #%d at %04Xh\n",I+'A',V,J*0x2000+0x4000);
        }
        /* Done with page switch */
        return;
      }
      /* Write to SRAM */
      if((A>=0xB000)&&(A<0xC000)&&(ROMMapper[I][3]==0xFF))
      {
        RAM[5][(A&0x0FFF)|0x1000]=RAM[5][A&0x0FFF]=V;
        SaveSRAM[I]=1;
        /* Done with SRAM write */
        return;
      }
      break;

    case MAP_FMPAC: /* Panasonic FMPAC+SRAM cartridge */
      /* See if any switching occurs */
      switch(A)
      {
        case 0x7FF7: /* ROM page select */
          V=(V<<1)&ROMMask[I];
          ROMMapper[I][0]=V;
          /* 4000h-5FFFh contains SRAM when correct FMPACKey supplied */
          if(FMPACKey!=FMPAC_MAGIC)
          {
            P=ROMData[I]+((int)V<<13);
            RAM[2]=MemMap[PS][SS][2]=P;
            RAM[3]=MemMap[PS][SS][3]=P+0x2000;
          }
//          if(Verbose&0x08)
//            printf("FMPAC %c: 16kB ROM page #%d at 4000h\n",I+'A',V>>1);
          return;
        case 0x7FF6: /* OPL1 enable/disable? */
//          if(Verbose&0x08)
//            printf("FMPAC %c: (7FF6h) = %02Xh\n",I+'A',V);
          V&=0x11;
          return;
        case 0x5FFE: /* Write 4Dh, then (5FFFh)=69h to enable SRAM */
        case 0x5FFF: /* (5FFEh)=4Dh, then write 69h to enable SRAM */
          FMPACKey=A&1? ((FMPACKey&0x00FF)|((int)V<<8))
                      : ((FMPACKey&0xFF00)|V);
          P=FMPACKey==FMPAC_MAGIC?
            SRAMData[I]:(ROMData[I]+((int)ROMMapper[I][0]<<13));
          RAM[2]=MemMap[PS][SS][2]=P;
          RAM[3]=MemMap[PS][SS][3]=P+0x2000;
//          if(Verbose&0x08)
//            printf("FMPAC %c: 8kB SRAM %sabled at 4000h\n",I+'A',FMPACKey==FMPAC_MAGIC? "en":"dis");
          return;
      }
      /* Write to SRAM */
      if((A>=0x4000)&&(A<0x5FFE)&&(FMPACKey==FMPAC_MAGIC))
      {
        RAM[A>>13][A&0x1FFF]=V;
        SaveSRAM[I]=1;
        return;
      }
      break;
  }

  /* No MegaROM mapper or there is an incorrect write */
//  if(Verbose&0x08) printf("MEMORY: Bad write (%04Xh) = %02Xh\n",A,V);
}

/** PSlot() **************************************************/
/** Switch primary memory slots. This function is called    **/
/** when value in port A8h changes.                         **/
/*************************************************************/
void PSlot(register byte V)
{
  register byte J,I;
  
  if(PSLReg!=V)
    for(PSLReg=V,J=0;J<4;++J,V>>=2)
    {
      I          = J<<1;
      PSL[J]     = V&3;
      SSL[J]     = (SSLReg[PSL[J]]>>I)&3;
      RAM[I]     = MemMap[PSL[J]][SSL[J]][I];
      RAM[I+1]   = MemMap[PSL[J]][SSL[J]][I+1];
      EnWrite[J] = (PSL[J]==3)&&(SSL[J]==2)&&(MemMap[3][2][I]!=EmptyRAM);
    }
}

/** SSlot() **************************************************/
/** Switch secondary memory slots. This function is called  **/
/** when value in (FFFFh) changes.                          **/
/*************************************************************/
void SSlot(register byte V)
{
  register byte J,I;
  
  /* Cartridge slots do not have subslots, fix them at 0:0:0:0 */
  if((PSL[3]==1)||(PSL[3]==2)) V=0x00;
  
  if(SSLReg[PSL[3]]!=V)
    for(SSLReg[PSL[3]]=V,J=0;J<4;++J,V>>=2)
    {
      if(PSL[J]==PSL[3])
      {
        I          = J<<1;
        SSL[J]     = V&3;
        RAM[I]     = MemMap[PSL[J]][SSL[J]][I];
        RAM[I+1]   = MemMap[PSL[J]][SSL[J]][I+1];
        EnWrite[J] = (PSL[J]==3)&&(SSL[J]==2)&&(MemMap[3][2][I]!=EmptyRAM);
      }
    }
}

/** SetIRQ() *************************************************/
/** Set or reset IRQ. Returns IRQ vector assigned to        **/
/** CPU.IRequest. When upper bit of IRQ is 1, IRQ is reset. **/
/*************************************************************/
word SetIRQ(register byte IRQ)
{
  if(IRQ&0x80) IRQPending&=IRQ; else IRQPending|=IRQ;
  CPU.IRequest=IRQPending? INT_IRQ:INT_NONE;
  return(CPU.IRequest);
}

/** SetScreen() **********************************************/
/** Change screen mode. Returns new screen mode.            **/
/*************************************************************/
byte SetScreen(void)
{
  register byte I,J;

  switch(((VDP[0]&0x0E)>>1)|(VDP[1]&0x18))
  {
    case 0x10: J=0;break;
    case 0x00: J=1;break;
    case 0x01: J=2;break;
    case 0x08: J=3;break;
    case 0x02: J=4;break;
    case 0x03: J=5;break;
    case 0x04: J=6;break;
    case 0x05: J=7;break;
    case 0x07: J=8;break;
    case 0x12: J=MAXSCREEN+1;break;
    default:   J=ScrMode;break;
  }

  /* Recompute table addresses */
  I=(J>6)&&(J!=MAXSCREEN+1)? 11:10;
  ChrTab  = VRAM+((int)(VDP[2]&MSK[J].R2)<<I);
  ChrGen  = VRAM+((int)(VDP[4]&MSK[J].R4)<<11);
  ColTab  = VRAM+((int)(VDP[3]&MSK[J].R3)<<6)+((int)VDP[10]<<14);
  SprTab  = VRAM+((int)(VDP[5]&MSK[J].R5)<<7)+((int)VDP[11]<<15);
  SprGen  = VRAM+((int)VDP[6]<<11);
  ChrTabM = ((int)(VDP[2]|~MSK[J].M2)<<I)|((1<<I)-1);
  ChrGenM = ((int)(VDP[4]|~MSK[J].M4)<<11)|0x007FF;
  ColTabM = ((int)(VDP[3]|~MSK[J].M3)<<6)|0x1C03F;
  SprTabM = ((int)(VDP[5]|~MSK[J].M5)<<7)|0x1807F;

  /* Return new screen mode */
  ScrMode=J;
  return(J);
}

/** SetMegaROM() *********************************************/
/** Set MegaROM pages for a given slot. SetMegaROM() always **/
/** assumes 8kB pages.                                      **/
/*************************************************************/
void SetMegaROM(int Slot,byte P0,byte P1,byte P2,byte P3)
{
  byte PS,SS;

  /* @@@ ATTENTION: MUST ADD SUPPORT FOR SRAM HERE!   */
  /* @@@ The FFh value must be treated as a SRAM page */

  /* Slot number must be valid */
  if((Slot<0)||(Slot>=MAXSLOTS)) return;
  /* Find primary/secondary slots */
  for(PS=0;PS<4;++PS)
  {
    for(SS=0;(SS<4)&&(CartMap[PS][SS]!=Slot);++SS);
    if(SS<4) break;
  }
  /* Drop out if slots not found */
  if(PS>=4) return;

  /* Apply masks to ROM pages */
  P0&=ROMMask[Slot];
  P1&=ROMMask[Slot];
  P2&=ROMMask[Slot];
  P3&=ROMMask[Slot];
  /* Set memory map */
  MemMap[PS][SS][2]=ROMData[Slot]+P0*0x2000;
  MemMap[PS][SS][3]=ROMData[Slot]+P1*0x2000;
  MemMap[PS][SS][4]=ROMData[Slot]+P2*0x2000;
  MemMap[PS][SS][5]=ROMData[Slot]+P3*0x2000;
  /* Set ROM mappers */
  ROMMapper[Slot][0]=P0;
  ROMMapper[Slot][1]=P1;
  ROMMapper[Slot][2]=P2;
  ROMMapper[Slot][3]=P3;
}

/** VDPOut() *************************************************/
/** Write value into a given VDP register.                  **/
/*************************************************************/
void VDPOut(register byte R,register byte V)
{ 
  register byte J;

  switch(R)  
  {
    case  0: /* Reset HBlank interrupt if disabled */
             if((VDPStatus[1]&0x01)&&!(V&0x10))
             {
               VDPStatus[1]&=0xFE;
               SetIRQ(~INT_IE1);
             }
             /* Set screen mode */
             if(VDP[0]!=V) { VDP[0]=V;SetScreen(); }
             break;
    case  1: /* Set/Reset VBlank interrupt if enabled or disabled */
             if(VDPStatus[0]&0x80) SetIRQ(V&0x20? INT_IE0:~INT_IE0);
             /* Set screen mode */
             if(VDP[1]!=V) { VDP[1]=V;SetScreen(); }
             break;
    case  2: J=(ScrMode>6)&&(ScrMode!=MAXSCREEN+1)? 11:10;
             ChrTab  = VRAM+((int)(V&MSK[ScrMode].R2)<<J);
             ChrTabM = ((int)(V|~MSK[ScrMode].M2)<<J)|((1<<J)-1);
             break;
    case  3: ColTab  = VRAM+((int)(V&MSK[ScrMode].R3)<<6)+((int)VDP[10]<<14);
             ColTabM = ((int)(V|~MSK[ScrMode].M3)<<6)|0x1C03F;
             break;
    case  4: ChrGen  = VRAM+((int)(V&MSK[ScrMode].R4)<<11);
             ChrGenM = ((int)(V|~MSK[ScrMode].M4)<<11)|0x007FF;
             break;
    case  5: SprTab  = VRAM+((int)(V&MSK[ScrMode].R5)<<7)+((int)VDP[11]<<15);
             SprTabM = ((int)(V|~MSK[ScrMode].M5)<<7)|0x1807F;
             break;
    case  6: V&=0x3F;SprGen=VRAM+((int)V<<11);break;
    case  7: FGColor=V>>4;BGColor=V&0x0F;break;
	case  9: changeBGSize(V);break;	/* add change NDS BG size */
    case 10: V&=0x07;
             ColTab=VRAM+((int)(VDP[3]&MSK[ScrMode].R3)<<6)+((int)V<<14);
             break;
    case 11: V&=0x03;
             SprTab=VRAM+((int)(VDP[5]&MSK[ScrMode].R5)<<7)+((int)V<<15);
             break;
    case 14: V&=VRAMPages-1;VPAGE=VRAM+((int)V<<14);
             break;
    case 15: V&=0x0F;break;
    case 16: V&=0x0F;PKey=1;break;
    case 17: V&=0xBF;break;
    case 25: VDP[25]=V;
             SetScreen();
             break;
    case 44: VDPWrite(V);break;
    case 46: VDPDraw(V);break;
  }

  /* Write value into a register */
  VDP[R]=V;
} 

/** Printer() ************************************************/
/** Send a character to the printer.                        **/
/*************************************************************/
//void Printer(byte V) { fputc(V,PrnStream); }
void Printer(byte V)
{
//  if(!PrnStream)
//  {
//    PrnStream = PrnName?   fopen(PrnName,"ab"):0;
//    PrnStream = PrnStream? PrnStream:stdout;
//  }
//  fputc(V,PrnStream);
}

/** PPIOut() *************************************************/
/** This function is called on each write to PPI to make    **/
/** key click sound, motor relay clicks, and so on.         **/
/*************************************************************/
void PPIOut(register byte New,register byte Old)
{
//	/* Keyboard click bit */
//  if((New^Old)&0x80) Drum(DRM_CLICK,64);
//  /* Motor relay bit */
//  if((New^Old)&0x10) Drum(DRM_CLICK,255);
}

/** RTCIn() **************************************************/
/** Read value from a given RTC register.                   **/
/*************************************************************/
byte RTCIn(register byte R)
{
  register byte J;

  /* Only 16 registers/mode */
  R&=0x0F;

  /* Bank mode 0..3 */
  J=RTCMode&0x03;

  if(R>12) J=R==13? RTCMode:NORAM;
  else
    if(J) J=RTC[J][R];
    else  J = system_RTCIn(R);

  /* Four upper bits are always high */
  return(J|0xF0);
}

/** LoopZ80() ************************************************/
/** Refresh screen, check keyboard and sprites. Call this   **/
/** function on each interrupt.                             **/
/*************************************************************/
word LoopZ80(Z80 *R)
{
  static byte BFlag=0;
  static byte BCount=0;
  static int  UCount=100;
  static byte ACount=0;
  static byte Drawing=0;
  register int J;
//  static byte InterlaceCount=0;

//  LOG( "LOOP z80 Start" );

  /* Flip HRefresh bit */
  VDPStatus[2]^=0x20;

  /* If HRefresh is now in progress... */
  if(!(VDPStatus[2]&0x20))
  {
    /* HRefresh takes most of the scanline */
    R->IPeriod=!ScrMode||(ScrMode==MAXSCREEN+1)? CPU_H240:CPU_H256;

    /* New scanline */
    ScanLine=ScanLine<(PALVideo? 312:261)? ScanLine+1:0;

    /* If first scanline of the screen... */
    if(!ScanLine)
    {
//	  LOG( "LOOP z80 2" );
      /* Drawing now... */
      Drawing=1;

      /* Reset VRefresh bit */
      VDPStatus[2]&=0xBF;

      /* Refresh display */
      if(UCount<100) UCount+= UPeriod;
      else
      {
        UCount -= 100;
        RefreshScreen();
      }

//	  LOG( "LOOP z80 3" );

      /* Interlace and Change page for GRP4-7 */
 /*     if(InterlaceCount) InterlaceCount--;
      else
      {
        InterlacePage=!InterlacePage;
        if(VDP[13]) InterlacePage=0;
        {
          InterlaceCount=(InterlacePage ?VDP[13]&0x0F:VDP[13]>>4);
          if(!(VDP[9]&0x04))InterlaceCount *= 10;
		}
      }
*/      
      /* Blinking for TEXT80 */
      if(BCount) BCount--;
      else
      {
        BFlag=!BFlag;
        if(!VDP[13]) { XFGColor=FGColor;XBGColor=BGColor; }
        else
        {
          BCount=(BFlag? VDP[13]&0x0F:VDP[13]>>4)*10;
          if(BCount)
          {
            if(BFlag) { XFGColor=FGColor;XBGColor=BGColor; }
            else      { XFGColor=VDP[12]>>4;XBGColor=VDP[12]&0x0F; }
          }
        }
      }
    }

    /* Line coincidence is active at 0..255 */
    /* in PAL and 0..234/244 in NTSC        */
    J=PALVideo? 256:ScanLines212? 245:235;
//	  LOG( "LOOP z80 4" );

    /* When reaching end of screen, reset line coincidence */
    if(ScanLine==J)
    {
      VDPStatus[1]&=0xFE;
      SetIRQ(~INT_IE1);
    }

    /* When line coincidence is active... */
    if(ScanLine<J)
    {
      /* Line coincidence processing */
      J=(((ScanLine+VScroll)&0xFF)-VDP[19])&0xFF;
      if(J==2)
      {
        /* Set HBlank flag on line coincidence */
        VDPStatus[1]|=0x01;
        /* Generate IE1 interrupt */
        if(VDP[0]&0x10) SetIRQ(INT_IE1);
      }
      else
      {
        /* Reset flag immediately if IE1 interrupt disabled */
        if(!(VDP[0]&0x10)) VDPStatus[1]&=0xFE;
      }
    }
//	  LOG( "LOOP z80 5" );

    /* Return whatever interrupt is pending */
    R->IRequest=IRQPending? INT_IRQ:INT_NONE;
    return(R->IRequest);
  }

  /*********************************/
  /* We come here for HBlanks only */
  /*********************************/

  /* HBlank takes HPeriod-HRefresh */
  R->IPeriod=!ScrMode||(ScrMode==MAXSCREEN+1)? CPU_H240:CPU_H256;
  R->IPeriod=HPeriod-R->IPeriod;

  /* If last scanline of VBlank, see if we need to wait more */
  J=PALVideo? 313:262;
  if(ScanLine>=J-1)
  {
    J*=CPU_HPERIOD;
    if(VPeriod>J) R->IPeriod+=VPeriod-J;
  }

  /* If first scanline of the bottom border... */
  if(ScanLine==(ScanLines212? 212:192)) Drawing=0;

  /* If first scanline of VBlank... */
  J=PALVideo? (ScanLines212? 212+42:192+52):(ScanLines212? 212+18:192+28);
  if(!Drawing&&(ScanLine==J))
  {
    /* Set VBlank bit, set VRefresh bit */
    VDPStatus[0]|=0x80;
    VDPStatus[2]|=0x40;

    /* Generate VBlank interrupt */
    if(VDP[1]&0x20) SetIRQ(INT_IE0);
  }

  /* Run V9938 engine */
  LoopVDP();

//  LOG( "LOOP z80 6" );

  /* Refresh scanline, possibly with the overscan */
  if(UCount>=100&&Drawing&&(ScanLine<256))
  {
    if(!ModeYJK||(ScrMode<7)||(ScrMode>8)){
//  LOG( "LOOP z80 7" );
      (RefreshLine[ScrMode])(ScanLine);
    }else{
//  LOG( "LOOP z80 8" );
      if(ModeYAE) (RefreshLine[10])(ScanLine);
      else (RefreshLine[12])(ScanLine);
    }
  }

  /* Keyboard, sound, and other stuff always runs at line 192    */
  /* This way, it can't be shut off by overscan tricks (Maarten) */
  if(ScanLine==192)
  {
//  LOG( "LOOP z80 9" );
    /* Check sprites and set Collision, 5Sprites, 5thSprite bits */
    if(!SpritesOFF&&ScrMode&&(ScrMode<MAXSCREEN+1)) CheckSprites();

    /* Count MIDI ticks */
//    MIDITicks(VPeriod/CPU_CLOCK);

    /* Update AY8910 state every VPeriod/CPU_CLOCK milliseconds */
//    Loop8910(&PSG,VPeriod/CPU_CLOCK);

    /* Flush changes to the sound channels */
//    Sync8910(&PSG,AY8910_FLUSH|(UseDrums? AY8910_DRUMS:0));
//    SyncSCC(&SCChip,SCC_FLUSH);
//    Sync2413(&OPLL,YM2413_FLUSH);

    /* Check joystick */
    JoyState=Joystick();

    /* Check keyboard */
    Keyboard_proc();

    /* Exit emulation if requested */
    if(!Power) return(INT_QUIT);

    if(JOYTYPE(0)>=JOY_MOUSE)
    {
      /* Get new mouse state */
      MouState[0]=Mouse(0);
      /* Merge mouse buttons into joystick buttons */
      JoyState=((JoyState&~0x30)|((MouState[0]>>12)&0x0030));
      /* If mouse-as-joystick... */
      if(JOYTYPE(0)==JOY_MOUSTICK)
      {
        J=MouState[0]&0xFF;
        JoyState|=J>OldMouseX[0]? 0x0008:J<OldMouseX[0]? 0x0004:0;
        OldMouseX[0]=J;
        J=(MouState[0]>>8)&0xFF;
        JoyState|=J>OldMouseY[0]? 0x0002:J<OldMouseY[0]? 0x0001:0;
        OldMouseY[0]=J;
      }
    }

    /* Check mouse in joystick port #2 */
    if(JOYTYPE(1)>=JOY_MOUSE)
    {
      /* Get new mouse state */
      MouState[1]=Mouse(1);
      /* Merge mouse buttons into joystick buttons */
      JoyState=((JoyState&(~0x3000))|((MouState[1]>>4)&0x3000));
      /* If mouse-as-joystick... */
      if(JOYTYPE(1)==JOY_MOUSTICK)
      {
        J=MouState[1]&0xFF;
        JoyState|=J>OldMouseX[1]? 0x0800:J<OldMouseX[1]? 0x0400:0;
        OldMouseX[1]=J;
        J=(MouState[1]>>8)&0xFF;
        JoyState|=J>OldMouseY[1]? 0x0200:J<OldMouseY[1]? 0x0100:0;
        OldMouseY[1]=J;
      }
    }
//	LOG("JOY=[%X] MouState[0]=[%X] MouState[1]=[%X]",JoyState,MouState[0],MouState[1]);
    /* If any autofire options selected, run autofire counter */
    if(OPTION(MSX_AUTOSPACE|MSX_AUTOFIREA|MSX_AUTOFIREB))
      if((ACount=(ACount+1)&0x07)>3)
      {
        /* Autofire spacebar if needed */
//        if(OPTION(MSX_AUTOSPACE)) KBD_RES(' ');
        /* Autofire FIRE-A if needed */
        if(OPTION(MSX_AUTOFIREA)) JoyState&=~(JST_FIREA|(JST_FIREA<<8));
        /* Autofire FIRE-B if needed */
        if(OPTION(MSX_AUTOFIREB)) JoyState&=~(JST_FIREB|(JST_FIREB<<8));
      }
  }
//  LOG( "LOOP z80 10" );

  /* Return whatever interrupt is pending */
  R->IRequest=IRQPending? INT_IRQ:INT_NONE;
  return(R->IRequest);
}

/** CheckSprites() *******************************************/
/** Check for sprite collisions and 5th/9th sprite in a     **/
/** row.                                                    **/
/*************************************************************/
void CheckSprites(void)
{
  register word LS,LD;
  register byte DH,DV,*PS,*PD,*T;
  byte I,J,N,M,*S,*D;

  /* Clear 5Sprites, Collision, and 5thSprite bits */
  VDPStatus[0]=(VDPStatus[0]&0x9F)|0x1F;

  for(N=0,S=SprTab;(N<32)&&(S[0]!=208);N++,S+=4);
  M=SolidColor0;

  if(Sprites16x16)
  {
    for(J=0,S=SprTab;J<N;J++,S+=4)
      if((S[3]&0x0F)||M)
        for(I=J+1,D=S+4;I<N;I++,D+=4)
          if((D[3]&0x0F)||M) 
          {
            DV=S[0]-D[0];
            if((DV<16)||(DV>240))
	    {
              DH=S[1]-D[1];
              if((DH<16)||(DH>240))
	      {
                PS=SprGen+((int)(S[2]&0xFC)<<3);
                PD=SprGen+((int)(D[2]&0xFC)<<3);
                if(DV<16) PD+=DV; else { DV=256-DV;PS+=DV; }
                if(DH>240) { DH=256-DH;T=PS;PS=PD;PD=T; }
                while(DV<16)
                {
                  LS=((word)*PS<<8)+*(PS+16);
                  LD=((word)*PD<<8)+*(PD+16);
                  if(LD&(LS>>DH)) break;
                  else { DV++;PS++;PD++; }
                }
                if(DV<16) { VDPStatus[0]|=0x20;return; }
              }
            }
          }
  }
  else
  {
    for(J=0,S=SprTab;J<N;J++,S+=4)
      if((S[3]&0x0F)||M)
        for(I=J+1,D=S+4;I<N;I++,D+=4)
          if((D[3]&0x0F)||M)
          {
            DV=S[0]-D[0];
            if((DV<8)||(DV>248))
            {
              DH=S[1]-D[1];
              if((DH<8)||(DH>248))
              {
                PS=SprGen+((int)S[2]<<3);
                PD=SprGen+((int)D[2]<<3);
                if(DV<8) PD+=DV; else { DV=256-DV;PS+=DV; }
                if(DH>248) { DH=256-DH;T=PS;PS=PD;PD=T; }
                while((DV<8)&&!(*PD&(*PS>>DH))) { DV++;PS++;PD++; }
                if(DV<8) { VDPStatus[0]|=0x20;return; }
              }
            }
          }
  }
}

/** GuessROM() ***********************************************/
/** Guess MegaROM mapper of a ROM.                          **/
/*************************************************************/
int GuessROM(const byte *Buf,int Size)
{
  int J,I,ROMCount[MAXMAPPERS];
  int K;
  char S[256];
  FILE *F;

  /* Compute ROM's CRC */
  for(J=K=0;J<Size;J++) K+=Buf[J];
  /* Try opening file with CRCs */
  F=fopen("CARTS.CRC","rb");
  if(F!=NULL)
  {
    /* Scan file comparing CRCs */
    while(fgets(S,sizeof(S)-4,F))
      if(sscanf(S,"%08X %d",&J,&I)==2)
        if(K==J) return(I);
    /* Nothing found */
    fclose(F);
  }

  /* Clear all counters */
  for(J=0;J<MAXMAPPERS;J++) ROMCount[J]=1;
  /* Generic 8kB mapper is default */
  ROMCount[MAP_GEN8]+=1;
  /* ASCII 16kB preferred over ASCII 8kB */
  ROMCount[MAP_ASCII16]-=1;

  /* Count occurences of characteristic addresses */
  for(J=0;J<Size-2;J++)
  {
    I=Buf[J]+((int)Buf[J+1]<<8)+((int)Buf[J+2]<<16);
    switch(I)
    {
      case 0x500032: ROMCount[MAP_KONAMI5]++;break;
      case 0x900032: ROMCount[MAP_KONAMI5]++;break;
      case 0xB00032: ROMCount[MAP_KONAMI5]++;break;
      case 0x400032: ROMCount[MAP_KONAMI4]++;break;
      case 0x800032: ROMCount[MAP_KONAMI4]++;break;
      case 0xA00032: ROMCount[MAP_KONAMI4]++;break;
      case 0x680032: ROMCount[MAP_ASCII8]++;break;
      case 0x780032: ROMCount[MAP_ASCII8]++;break;
      case 0x600032: ROMCount[MAP_KONAMI4]++;
                     ROMCount[MAP_ASCII8]++;
                     ROMCount[MAP_ASCII16]++;
                     break;
      case 0x700032: ROMCount[MAP_KONAMI5]++;
                     ROMCount[MAP_ASCII8]++;
                     ROMCount[MAP_ASCII16]++;
                     break;
      case 0x77FF32: ROMCount[MAP_ASCII16]++;break;
    }
  }

  /* Find which mapper type got more hits */
  for(I=0,J=0;J<MAXMAPPERS;J++)
    if(ROMCount[J]>ROMCount[I]) I=J;

  /* Return the most likely mapper type */
  return(I);
}

/** ChangePrinter() ******************************************/
/** Change printer output to a given file. The previous     **/
/** file is closed. ChangePrinter(0) redirects output to    **/
/** stdout. Returns 1 on success, 0 on failure.             **/
/*************************************************************/
void ChangePrinter(const char *FileName)
{
//  if(PrnStream&&(PrnStream!=stdout)) fclose(PrnStream);
//  PrnName   = FileName;
//  PrnStream = 0;
}

/** ChangeDisk() *********************************************/
/** Change disk image in a given drive. Closes current disk **/
/** image if Name=0 was given. Creates a new disk image if  **/
/** Name="" was given. Returns 1 on success or 0 on failure.**/
/*************************************************************/
byte ChangeDisk(byte N,const char *FileName)
{
//	byte *P;

	/* We only have MAXDRIVES drives */
	if(N>=MAXDRIVES) return(0);

	/* Reset FDC, in case it was running a command */
	Reset1793(&FDC,FDD,WD1793_KEEP);

	if( DSKName[N] ){
		LOG( "Disk flashing ... [%s]" ,DSKName[N] );
		SaveFDI( &FDD[N],DSKName[N],FMT_DSK);
		EjectFDI(&FDD[N]);
		FreeMemory((byte*)DSKName[N]);
		DSKName[N]=NULL;
	}

	/* Eject disk if requested */
	if( !FileName || !(*FileName) ) return(1);

	/* If FileName not empty, try loading disk image */
	LOG( "Disk image loading ... [%s]" ,FileName );
	if( !LoadFDI(&FDD[N],FileName,FMT_DSK) ){
#if 0
		/** Failed to open as a plain file */
		/* Create a new 720kB disk image */
		P = NewFDI(&FDD[N],
			DSK_SIDS_PER_DISK,
			DSK_TRKS_PER_SIDE,
			DSK_SECS_PER_TRCK,
			DSK_SECTOR_SIZE
		);
		/* If FileName not empty, treat it as directory, otherwise new disk */
		if(P&&!(DSKLoad(FileName,P):DSKCreate(P))){
			EjectFDI(&FDD[N]);
			return(0);
		}
#endif
		LOG( "Disk image loading failed." );
		return 0;
	}
	/* Done */
	DSKName[N]= (char*)GetMemory(strlen(FileName)+1);
	strcpy( DSKName[N], FileName );
	
	return(1);
}
/** LoadROM() ************************************************/
/** Load a file, allocating memory as needed. Returns addr. **/
/** of the alocated space or 0 if failed.                   **/
/*************************************************************/
byte *LoadROM(const char *Name,int Size,byte *Buf)
{
  FILE* F;
  byte *P;
  LOG( "LoadROM Start [%s]",(char*)Name );

  if( Name == NULL ) return(0);

  /* Can't give address without size! */
  if(Buf&&!Size) return(0);

  /* Open file */
  F=fopen(Name,"r");
  if( F==NULL ){
    LOG( "LoadROM:File Open err." );
	return(0);
  }

  /* Determine data size, if wasn't given */
  if(!Size)
  {
    /* Determine size via fseek()/ftell() or by reading */
    /* entire [GZIPped] stream                          */
    if(fseek(F,0,SEEK_END)>0) Size=ftell(F);
//    else while(fgetc(F)!=EOF) Size++;
    /* Rewind file to the beginning */
    fseek(F,0,SEEK_SET);
  }

  /* Allocate memory */
  P=Buf? Buf:GetMemory(Size);
  if(!P)
  {
    fclose(F);
    LOG( "LoadROM:Mem Alloc err." );
    return(0);
  }

  /* Read data */
  if((int)fread(P,1,Size,F)!=Size)
  {
    if(!Buf) free(P);
    fclose(F);
    LOG( "LoadROM:read err." );
    return(0);
  }

  /* Done */
  fclose(F);
  LOG( "LoadROM [%s]", Name );
  return(P);
}

/** EjectCart() **********************************************/
/** Eject cartridge to given slot.                          **/
/** Returns 1 on success, 0 on failure.                     **/
/*************************************************************/
int EjectCart(int Slot)
{
	byte PS,SS;
	int C1;
	FILE* F;

	LOG( "EjectCart slot[%d]", Slot );

	if(!ROMName[Slot]) return(0);

	/* Check slot #, try to open file */
	if((Slot<0)||(Slot>=MAXCARTS)) return(0);
	/* Find primary/secondary slots */
	for(PS=0;PS<4;++PS)
	{
		for(SS=0;(SS<4)&&(CartMap[PS][SS]!=Slot);++SS);
		if(SS<4) break;
	}
	/* Drop out if slots not found */
	if(PS>=4) return(0);
	LOG( "EjectCart 2" );

	/* If there is a SRAM in this cartridge slot... */
	if(SRAMData[Slot]&&SaveSRAM[Slot]&&SRAMName[Slot])
	{
    	/* Open .SAV file */
//		if(Verbose) printf("Writing %s...",SRAMName[Slot]);
		if(!(F=fopen(SRAMName[Slot],"wb"))) SaveSRAM[Slot]=0;
		else
		{
			/* Write .SAV file */
			switch(ROMType[Slot])
			{
				case MAP_ASCII8:
				case MAP_FMPAC:
					if(fwrite(SRAMData[Slot],1,0x2000,F)!=0x2000) SaveSRAM[Slot]=0;
					break;
				case MAP_ASCII16:
					if(fwrite(SRAMData[Slot],1,0x0800,F)!=0x0800) SaveSRAM[Slot]=0;
					break;
				case MAP_GMASTER2:
					if(fwrite(SRAMData[Slot],1,0x1000,F)!=0x1000)        SaveSRAM[Slot]=0;
					if(fwrite(SRAMData[Slot]+0x2000,1,0x1000,F)!=0x1000) SaveSRAM[Slot]=0;
					break;
			}
			/* Done with .SAV file */
			fclose(F);
		}
		/* Done saving SRAM */
		LOG("%s SRAM saved.",SRAMName[Slot]);
	}

	/* Free memory if present */
	FreeMemory(ROMData[Slot]);
	ROMData[Slot] = 0;
	ROMMask[Slot] = 0;

	/* Free previous SRAM resources */
	FreeMemory(SRAMData[Slot]);
	FreeMemory((byte*)SRAMName[Slot]);

	/* Set memory map to dummy RAM */
	for(C1=0;C1<8;++C1) MemMap[PS][SS][C1]=EmptyRAM;
	/* Restart MSX */
//	ResetMSX(Mode,RAMPages,VRAMPages);
	/* Cartridge ejected */
//	if(Verbose) printf("Ejected cartridge from slot %c\n",Slot+'A');
	FreeMemory((byte*)ROMName[Slot]);
	ROMName[Slot] = NULL;
	
	/* remove SCC+ cartridge */
	SCCPSlot = -1;
	
	return(1);
}

/** LoadCart() ***********************************************/
/** Load cartridge into given slot. Returns cartridge size  **/
/** in 16kB pages on success, 0 on failure.                 **/
/*************************************************************/
int LoadCart(int Slot)
{
  int C1,C2,Pages,ROM64;
  byte *P,PS,SS;
  long Len;
  FILE* F;
//  u8 buf[16];

  LOG( "LoadCart Slot[%d] name[%s]",Slot, ROMName[Slot] );

  /* Check slot #, try to open file */
  if((Slot<0)||(Slot>=MAXCARTS)) return(0);
  /* Find primary/secondary slots */
  for(PS=0;PS<4;++PS)
  {
    for(SS=0;(SS<4)&&(CartMap[PS][SS]!=Slot);++SS);
    if(SS<4) break;
  }
  /* Drop out if slots not found */
  if(PS>=4) return(0);

  if( ROMName[Slot]==NULL ) return(0);

  /* Try opening file */
  F=fopen(ROMName[Slot],"r");
  if( F==NULL ) return(0);
  if(Verbose) LOG("Found %s:\n",ROMName[Slot]);

  if(fseek(F,0,SEEK_END)>=0) 
  	Len=ftell(F);
  else
  {
    // Determine file length by reading entire [GZIPped] stream
    fseek(F,0,SEEK_SET);
    for(Len=0;(C2=fread(EmptyRAM,1,0x4000,F))==0x4000;Len+=C2);
    if(C2>=0) Len+=C2;
    // Clean up the EmptyRAM! 
    memset(EmptyRAM,NORAM,0x4000);
  }

  /* Rewind file */
  rewind(F);

  if(Verbose) LOG( "CART SIZE[%ld]", Len );

  Len>>=13;
  /* Calculate 2^n closest to number of pages */
  for(Pages=1;Pages<Len;Pages<<=1);

  /* Check "AB" signature in a file */

  ROM64=0;
  C1=fgetc(F);
  C2=fgetc(F);

  LOG( "LoadCart 4" );

  // Maybe this is a flat 64kB ROM? *
  if((C1!='A')||(C2!='B'))
    if(fseek(F,0x4000,SEEK_SET)>=0)
    {
      C1=fgetc(F);
      C2=fgetc(F);
      ROM64=(C1=='A')&&(C2=='B');
    }

  // Maybe it is the last page that contains "AB" signature?
  if((Len>=2)&&((C1!='A')||(C2!='B')))
    if(fseek(F,0x2000*(Len-2),SEEK_SET)>=0)
    {
      C1=fgetc(F);
      C2=fgetc(F);
    }

  LOG( "LoadCart 5" );

  // If we can't find "AB" signature, drop out     
  if((C1!='A')||(C2!='B'))
  {
    if(Verbose) printf("  Not a valid cartridge ROM");
    fclose(F);
    return(0);
  }

//  if(Verbose) printf("  Cartridge %c: ",'A'+Slot);

  /* Done with the file */
  fclose(F);

  /* Show ROM type and size */
/*  if(Verbose)
    printf
    (
      "%dkB %s ROM..",Len*8,
      !ROM64&&(Len>4)? ROMNames[ROMType[Slot]]:"NORMAL"
    );
*/
  LOG( "LoadCart 6" );

  /* Assign ROMMask for MegaROMs */
  ROMMask[Slot]=!ROM64&&(Len>4)? (Pages-1):0x00;
  /* Allocate space for the ROM */
  ROMData[Slot]=GetMemory(Pages<<13);
  if(!ROMData[Slot]) { PRINTFAILED;return(0); }

  /* Try loading ROM */
  if(!LoadROM(ROMName[Slot],Len<<13,ROMData[Slot])) { PRINTFAILED;return(0); }

  /* Mirror ROM if it is smaller than 2^n pages */
  if(Len<Pages)
    memcpy
    (
      ROMData[Slot]+Len*0x2000,
      ROMData[Slot]+(Len-Pages/2)*0x2000,
      (Pages-Len)*0x2000
    ); 

  LOG( "LoadCart 7" );


  /* Set memory map depending on the ROM size */
  switch(Len)
  {
    case 1:
      /* 8kB ROMs are mirrored 8 times: 0:0:0:0:0:0:0:0 */
      MemMap[PS][SS][0]=ROMData[Slot];
      MemMap[PS][SS][1]=ROMData[Slot];
      MemMap[PS][SS][2]=ROMData[Slot];
      MemMap[PS][SS][3]=ROMData[Slot];
      MemMap[PS][SS][4]=ROMData[Slot];
      MemMap[PS][SS][5]=ROMData[Slot];
      MemMap[PS][SS][6]=ROMData[Slot];
      MemMap[PS][SS][7]=ROMData[Slot];
      break;

    case 2:
      /* 16kB ROMs are mirrored 4 times: 0:1:0:1:0:1:0:1 */
      MemMap[PS][SS][0]=ROMData[Slot];
      MemMap[PS][SS][1]=ROMData[Slot]+0x2000;
      MemMap[PS][SS][2]=ROMData[Slot];
      MemMap[PS][SS][3]=ROMData[Slot]+0x2000;
      MemMap[PS][SS][4]=ROMData[Slot];
      MemMap[PS][SS][5]=ROMData[Slot]+0x2000;
      MemMap[PS][SS][6]=ROMData[Slot];
      MemMap[PS][SS][7]=ROMData[Slot]+0x2000;
      break;

    case 3:
    case 4:
      /* 24kB and 32kB ROMs are mirrored twice: 0:1:0:1:2:3:2:3 */
      MemMap[PS][SS][0]=ROMData[Slot];
      MemMap[PS][SS][1]=ROMData[Slot]+0x2000;
      MemMap[PS][SS][2]=ROMData[Slot];
      MemMap[PS][SS][3]=ROMData[Slot]+0x2000;
      MemMap[PS][SS][4]=ROMData[Slot]+0x4000;
      MemMap[PS][SS][5]=ROMData[Slot]+0x6000;
      MemMap[PS][SS][6]=ROMData[Slot]+0x4000;
      MemMap[PS][SS][7]=ROMData[Slot]+0x6000;
      break;

    default:
      if(ROM64)
      {
        /* 64kB ROMs are loaded to fill slot: 0:1:2:3:4:5:6:7 */
        MemMap[PS][SS][0]=ROMData[Slot];
        MemMap[PS][SS][1]=ROMData[Slot]+0x2000;
        MemMap[PS][SS][2]=ROMData[Slot]+0x4000;
        MemMap[PS][SS][3]=ROMData[Slot]+0x6000;
        MemMap[PS][SS][4]=ROMData[Slot]+0x8000;
        MemMap[PS][SS][5]=ROMData[Slot]+0xA000;
        MemMap[PS][SS][6]=ROMData[Slot]+0xC000;
        MemMap[PS][SS][7]=ROMData[Slot]+0xE000;
      }
      break;
  }
  LOG( "LoadCart 8" );

  /* Show starting address */
/*
  if(Verbose)
    printf
    (
      "starts at %04Xh..",
      MemMap[Slot+1][0][2][2]+256*MemMap[Slot+1][0][2][3]
    );
*/
  /* Guess MegaROM mapper type if not given */
  if(ROMType[Slot]==MAP_GUESS&&(ROMMask[Slot]+1>4))
  {
    ROMType[Slot]=GuessROM(ROMData[Slot],0x2000*(ROMMask[Slot]+1));
    if(Verbose) LOG("guessed %s..",ROMNames[Slot]);
//    if(Slot<MAXCARTS) SETROMTYPE(Slot,Type);
  }

  /* Save MegaROM type */
//  ROMType[Slot]=Type;

  /* For Generic/16kB carts, set ROM pages as 0:1:N-2:N-1 */
  if((ROMType[Slot]==MAP_GEN16)&&(ROMMask[Slot]+1>4))
    SetMegaROM(Slot,0,1,ROMMask[Slot]-1,ROMMask[Slot]);

//  LOG( "LoadCart 9" );

  /* If cartridge may need a SRAM... */
  if(MAP_SRAM(ROMType[Slot]))
  {
    /* Get SRAM memory */
    SRAMData[Slot]=GetMemory(0x4000);
    if(!SRAMData[Slot])
    {
      if(Verbose) printf("scratch SRAM..");
      SRAMData[Slot]=EmptyRAM;
    }
    else
    {
      if(Verbose) printf("got 16kB SRAM..");
      memset(SRAMData[Slot],NORAM,0x4000);
    }

    /* Generate SRAM file names and load SRAM contents */
    if((SRAMName[Slot]=(char*)GetMemory(strlen(ROMName[Slot])+5))!=NULL)
    {
      /* Compose SRAM file name */
      strcpy(SRAMName[Slot],ROMName[Slot]);      
      P=(byte*)strrchr(SRAMName[Slot],'.');
      if(P) strcpy((char*)P,".sav");
      else strcat(SRAMName[Slot],".sav");
      /* Try opening file... */
      F=fopen(SRAMName[Slot],"r");
      if(F!=NULL)
      {
        /* Read SRAM file */
        Len=fread(SRAMData[Slot],1,0x4000,F);
        fclose(F);
        /* Print information if needed */
//        if(Verbose) printf("loaded %d bytes from %s..",Len,SRAMName[Slot]);
        /* Mirror data according to the mapper type */
        P=SRAMData[Slot];
        switch(ROMType[Slot])
        {
          case MAP_FMPAC:
            memset(P+0x2000,NORAM,0x2000);
            P[0x1FFE]=FMPAC_MAGIC&0xFF;
            P[0x1FFF]=FMPAC_MAGIC>>8;
            break;
          case MAP_GMASTER2:
            memcpy(P+0x2000,P+0x1000,0x1000);
            memcpy(P+0x3000,P+0x1000,0x1000);
            memcpy(P+0x1000,P,0x1000);
            break;
          case MAP_ASCII16:
            memcpy(P+0x0800,P,0x0800);
            memcpy(P+0x1000,P,0x0800);
            memcpy(P+0x1800,P,0x0800);
            memcpy(P+0x2000,P,0x0800);
            memcpy(P+0x2800,P,0x0800);
            memcpy(P+0x3000,P,0x0800);
            memcpy(P+0x3800,P,0x0800);
            break;
        }
      }
    } 
  }

  /* Done setting up cartridge */
//  ResetMSX(Mode,RAMPages,VRAMPages);
//  PRINTOK;

#if 0
	/* If first used user slot... */
  if(!Slot||((Slot==1)&&!ROMData[0]))
  {
    /* Remove old state name */
    FreeMemory((byte*)StateName);

    /* If memory for StateName gets allocated... */
    if((StateName=(char*)GetMemory(strlen(ROMName[Slot])+5))!=NULL)
    {
      /* Copy name */
      strcpy(StateName,ROMName[Slot]);
      /* Find extension */
      P=(byte*)strrchr(StateName,'.');
      /* Either replace extension with ".sta" or add ".sta" */
      if(P) strcpy((char*)P,".sta"); else strcat(StateName,".sta");
      /* Try loading state */
      if(Verbose) printf("Loading state from %s...",StateName);
      C1=LoadState(StateName);
      PRINTRESULT(C1);
    }
  }
#endif
  LOG( "LoadCart success." );

  /* Done loading cartridge */
//  PRINTOK;
  return(Pages);
}

