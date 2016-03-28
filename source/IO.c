#include "msx.h"
#include "emu2149.h"
#include "emu2212.h"
#include "DSDriver.h"

//#include <fifocommon.h>

extern PSG * psg;
extern SCC * scc;
//extern OPLL* opll;

/* wrapper functions to actual sound emulation */
void WriteAUDIO(int R,int V) {  }

/** RdZ80() **************************************************/
/** Z80 emulation calls this function to read a byte from   **/
/** address A of Z80 address space. Now moved to Z80.c and  **/
/** made inlined to speed things up.                        **/
/*************************************************************/
byte RdZ80(word A)
{
  /* Filter out everything but [xx11 1111 1xxx 1xxx] */
  if((A&0x3F88)!=0x3F88) return(RAM[A>>13][A&0x1FFF]);

  /* Secondary slot selector */
  if(A==0xFFFF) return(~SSLReg[PSL[3]]);

  /* Floppy disk controller */
  /* 7FF8h..7FFFh Standard DiskROM  */
  /* BFF8h..BFFFh MSX-DOS BDOS      */
  /* 7F80h..7F87h Arabic DiskROM    */
  /* 7FB8h..7FBFh SV738/TechnoAhead */
  if((PSL[A>>14]==3)&&(SSL[A>>14]==1))
    switch(A)
    {
      /* Standard      MSX-DOS       Arabic        SV738            */
      case 0x7FF8: case 0xBFF8: case 0x7F80: case 0x7FB8: /* STATUS */
      case 0x7FF9: case 0xBFF9: case 0x7F81: case 0x7FB9: /* TRACK  */
      case 0x7FFA: case 0xBFFA: case 0x7F82: case 0x7FBA: /* SECTOR */
      case 0x7FFB: case 0xBFFB: case 0x7F83: case 0x7FBB: /* DATA   */
        return(Read1793(&FDC,A&0x0003));
      case 0x7FFF: case 0xBFFF: case 0x7F84: case 0x7FBC: /* SYSTEM */
        return(Read1793(&FDC,WD1793_READY));
    }

  /* Default to reading memory */
  return(RAM[A>>13][A&0x1FFF]);
}

/** WrZ80() **************************************************/
/** Z80 emulation calls this function to write byte V to    **/
/** address A of Z80 address space.                         **/
/*************************************************************/
void WrZ80(word A,byte V)
{
  /* Secondary slot selector */
  if(A==0xFFFF) { SSlot(V);return; }

  /* Floppy disk controller */
  /* 7FF8h..7FFFh Standard DiskROM  */
  /* BFF8h..BFFFh MSX-DOS BDOS      */
  /* 7F80h..7F87h Arabic DiskROM    */
  /* 7FB8h..7FBFh SV738/TechnoAhead */
  if(((A&0x3F88)==0x3F88)&&(PSL[A>>14]==3)&&(SSL[A>>14]==1))
    switch(A)
    {
      /* Standard      MSX-DOS       Arabic        SV738             */
      case 0x7FF8: case 0xBFF8: case 0x7F80: case 0x7FB8: /* COMMAND */
      case 0x7FF9: case 0xBFF9: case 0x7F81: case 0x7FB9: /* TRACK   */
      case 0x7FFA: case 0xBFFA: case 0x7F82: case 0x7FBA: /* SECTOR  */
      case 0x7FFB: case 0xBFFB: case 0x7F83: case 0x7FBB: /* DATA    */
        Write1793(&FDC,A&0x0003,V);
        return;
      case 0xBFFC: /* Standard/MSX-DOS */
      case 0x7FFC: /* Side: [xxxxxxxS] */
        Write1793(&FDC,WD1793_SYSTEM,FDC.Drive|S_DENSITY|(V&0x01? 0:S_SIDE));
        return;
      case 0xBFFD: /* Standard/MSX-DOS  */
      case 0x7FFD: /* Drive: [xxxxxxxD] */
        Write1793(&FDC,WD1793_SYSTEM,(V&0x01)|S_DENSITY|(FDC.Side? 0:S_SIDE));
        return;
      case 0x7FBC: /* Arabic/SV738 */
      case 0x7F84: /* Side/Drive/Motor: [xxxxMSDD] */
        Write1793(&FDC,WD1793_SYSTEM,(V&0x03)|S_DENSITY|(V&0x04? 0:S_SIDE));
        return;
    }
//	int flg = 0;

  /* Write to RAM, if enabled */
  if(EnWrite[A>>14]) { RAM[A>>13][A&0x1FFF]=V;return; }

  /* Switch MegaROM pages */
  if((A>0x3FFF)&&(A<0xC000)) MapROM(A,V);
}

/** InZ80() **************************************************/
/** Z80 emulation calls this function to read a byte from   **/
/** a given I/O port.                                       **/
/*************************************************************/
byte InZ80(word Port)
{
  /* MSX only uses 256 IO ports */
  Port&=0xFF;

  /* Return an appropriate port value */
  switch(Port)
  {

case 0x90: return(0xFD);                   /* Printer READY signal */
case 0xB5: return(RTCIn(RTCReg));          /* RTC registers        */

case 0xA8: /* Primary slot state   */
case 0xA9: /* Keyboard port        */
case 0xAA: /* General IO register  */
case 0xAB: /* PPI control register */
  PPI.Rin[1]=MSXKeyMap[PPI.Rout[2]&0x0F];
  return(Read8255(&PPI,Port-0xA8));

case 0xFC: /* Mapper page at 0000h */
case 0xFD: /* Mapper page at 4000h */
case 0xFE: /* Mapper page at 8000h */
case 0xFF: /* Mapper page at C000h */
  return(RAMMapper[Port-0xFC]|~RAMMask);

case 0xD9: /* Kanji support */
  Port=Kanji? Kanji[KanLetter+KanCount]:NORAM;
  KanCount=(KanCount+1)&0x1F;
  return(Port);

case 0x80: /* SIO data */
case 0x81:
case 0x82:
case 0x83:
case 0x84:
case 0x85:
case 0x86:
case 0x87:
  return(NORAM);
  /*return(Rd8251(&SIO,Port&0x07));*/

case 0x98: /* VRAM read port */
  /* Read from VRAM data buffer */
  Port=VDPData;
  /* Reset VAddr latch sequencer */
  VKey=1;
  /* Fill data buffer with a new value */
  VDPData=VPAGE[VAddr];
  /* Increment VRAM address */
  VAddr=(VAddr+1)&0x3FFF;
  /* If rolled over, modify VRAM page# */
  if(!VAddr&&(ScrMode>3))
  {
    VDP[14]=(VDP[14]+1)&(VRAMPages-1);
    VPAGE=VRAM+((int)VDP[14]<<14);
  }
  return(Port);

case 0x99: /* VDP status registers */
  /* Read an appropriate status register */
  Port=VDPStatus[VDP[15]];
  /* Reset VAddr latch sequencer */
  VKey=1;
  /* Update status register's contents */
  switch(VDP[15])
  {
    case 0: VDPStatus[0]&=0x5F;SetIRQ(~INT_IE0);break;
    case 1: VDPStatus[1]&=0xFE;SetIRQ(~INT_IE1);break;
    case 7: VDPStatus[7]=VDP[44]=VDPRead();break;
  }
  /* Return the status register value */
  return(Port);

case 0xA2: /* PSG input port */
  /* PSG[14] returns joystick/mouse data */
  if(PSGLatch==14)
  {
    int DX,DY,L,J;

    /* Number of a joystick port */
    Port=PSG_readReg(psg,15)&0x40? 1:0;
    L    = JOYTYPE(Port);

    /* If no joystick, return dummy value */
    if(L==JOY_NONE) return(0x7F);

    /* Compute mouse offsets, if needed */
    if(MCount[Port]==1)
    {
      /* Get new mouse coordinates */
      DX=MouState[Port]&0xFF;
      DY=(MouState[Port]>>8)&0xFF;
      /* Compute offsets and store coordinates  */
      J=OldMouseX[Port]-DX;OldMouseX[Port]=DX;DX=J;
      J=OldMouseY[Port]-DY;OldMouseY[Port]=DY;DY=J;
      /* For 512-wide mode, double horizontal offset */
      if((ScrMode==6)||((ScrMode==7)&&!ModeYJK)||(ScrMode==MAXSCREEN+1)) DX<<=1;
      /* Adjust offsets */
      MouseDX[Port]=(DX>127? 127:(DX<-127? -127:DX))&0xFF;
      MouseDY[Port]=(DY>127? 127:(DY<-127? -127:DY))&0xFF;
    }

    /* Get joystick state */
    J=~(Port? (JoyState>>8):JoyState)&0x3F;

    /* Determine return value */
    switch(MCount[Port])
    {
      case 0: Port=PSG_readReg(psg,15)&(0x10<<Port)? 0x3F:J;break;
      case 1: Port=(MouseDX[Port]>>4)|(J&0x30);break;
      case 2: Port=(MouseDX[Port]&0x0F)|(J&0x30);break;
      case 3: Port=(MouseDY[Port]>>4)|(J&0x30);break;
      case 4: Port=(MouseDY[Port]&0x0F)|(J&0x30);break;
    }

    /* 6th bit is always 1 */
    return(Port|0x40);
  }

  /* PSG[15] resets mouse counters */
  if(PSGLatch==15)
  {
    /* @@@ For debugging purposes */
    /*printf("Reading from PSG[15]\n");*/

    /*MCount[0]=MCount[1]=0;*/
    return(PSG_readReg(psg,15)&0xF0);
  }
  /* Return PSG[0-13] as they are */
//  return(RdData8910(&PSG));
  return PSG_readReg(psg,PSGLatch);

case 0xD0: /* FDC status  */
case 0xD1: /* FDC track   */
case 0xD2: /* FDC sector  */
case 0xD3: /* FDC data    */
case 0xD4: /* FDC IRQ/DRQ */
  /* Brazilian DiskROM I/O ports */
  return(Read1793(&FDC,Port-0xD0));

  }

  /* Return NORAM for non-existing ports */
  if(Verbose&0x20) printf("I/O: Read from unknown PORT[%02Xh]\n",Port);
  return(NORAM);

}

/** OutZ80() *************************************************/
/** Z80 emulation calls this function to write byte V to a  **/
/** given I/O port.                                         **/
/*************************************************************/
void OutZ80(word Port,byte Value)
{
  register byte I,J;  

  Port&=0xFF;
  switch(Port)
  {

case 0x7C: OPLLLatch=Value;return;               /* OPLL Register# */
//case 0x7D: OPLL_writeReg(opll,OPLLLatch,Value);return;    /* OPLL Data      */
//case 0x91: Printer(Value);return;               /* Printer Data   */
case 0xA0: PSGLatch=Value;return;		          /* PSG Register#  */
case 0xB4: RTCReg=Value&0x0F;return;              /* RTC Register#  */ 
case 0xC0: WriteAUDIO(0,Value);return;            /* AUDIO Register#*/
case 0xC1: WriteAUDIO(1,Value);return;            /* AUDIO Data     */

case 0xD8: /* Upper bits of Kanji ROM address */
  KanLetter=(KanLetter&0x1F800)|((int)(Value&0x3F)<<5);
  KanCount=0;
  return;

case 0xD9: /* Lower bits of Kanji ROM address */
  KanLetter=(KanLetter&0x007E0)|((int)(Value&0x3F)<<11);
  KanCount=0;
  return;

case 0x80: /* SIO data */
case 0x81:
case 0x82:
case 0x83:
case 0x84:
case 0x85:
case 0x86:
case 0x87:
  return;
  /*Wr8251(&SIO,Port&0x07,Value);
  return;*/

case 0x98: /* VDP Data */
  VKey=1;
  if(WKey)
  {
    /* VDP set for writing */
    VDPData=VPAGE[VAddr]=Value;
    VAddr=(VAddr+1)&0x3FFF;
  }
  else
  {
    /* VDP set for reading */
    VDPData=VPAGE[VAddr];
    VAddr=(VAddr+1)&0x3FFF;
    VPAGE[VAddr]=Value;
  }
  /* If VAddr rolled over, modify VRAM page# */
  if(!VAddr&&(ScrMode>3)) 
  {
    VDP[14]=(VDP[14]+1)&(VRAMPages-1);
    VPAGE=VRAM+((int)VDP[14]<<14);
  }
  return;

case 0x99: /* VDP Address Latch */
  if(VKey) { ALatch=Value;VKey=0; }
  else
  {
    VKey=1;
    switch(Value&0xC0)
    {
      case 0x80:
        /* Writing into VDP registers */
        VDPOut(Value&0x3F,ALatch);
        break;
      case 0x00:
      case 0x40:
        /* Set the VRAM access address */
        VAddr=(((word)Value<<8)+ALatch)&0x3FFF;
        /* WKey=1 when VDP set for writing into VRAM */
        WKey=Value&0x40;
        /* When set for reading, perform first read */
        if(!WKey)
        {
          VDPData=VPAGE[VAddr];
          VAddr=(VAddr+1)&0x3FFF;
          if(!VAddr&&(ScrMode>3))
          {
            VDP[14]=(VDP[14]+1)&(VRAMPages-1);
            VPAGE=VRAM+((int)VDP[14]<<14);
          }
        }
        break;
    }
  }
  return;

case 0x9A: /* VDP Palette Latch */
  if(PKey) { PLatch=Value;PKey=0; }
  else
  {
    byte R,G,B;
    /* New palette entry written */
    PKey=1;
    J=VDP[16];
    /* Compute new color components */
    R=(PLatch&0x70)*255/112;
    G=(Value&0x07)*255/7;
    B=(PLatch&0x07)*255/7;
    /* Set new color for palette entry J */
    Palette[J]=RGB2INT(R,G,B);
    SetColor(J,R,G,B);
    /* Next palette entry */
    VDP[16]=(J+1)&0x0F;
  }
  return;

case 0x9B: /* VDP Register Access */
  J=VDP[17]&0x3F;
  if(J!=17) VDPOut(J,Value);
  if(!(VDP[17]&0x80)) VDP[17]=(J+1)&0x3F;
  return;

case 0xA1: /* PSG Data */
  /* PSG[15] is responsible for joystick/mouse */
  if(PSGLatch==15)
  {
    /* @@@ For debugging purposes */
    /*printf("Writing PSG[15] <- %02Xh\n",Value);*/

    /* For mouse, update nibble counter      */
    /* For joystick, set nibble counter to 0 */
    if((Value&0x0C)==0x0C) MCount[1]=0;
    else if((JOYTYPE(1)==JOY_MOUSE)&&((Value^PSG_readReg(psg,15))&0x20))
           MCount[1]+=MCount[1]==4? -3:1;

    /* For mouse, update nibble counter      */
    /* For joystick, set nibble counter to 0 */
    if((Value&0x03)==0x03) MCount[0]=0;
    else if((JOYTYPE(0)==JOY_MOUSE)&&((Value^PSG_readReg(psg,15))&0x10))
           MCount[0]+=MCount[0]==4? -3:1;
  }

  /* Put value into a register */
//  WrData8910(&PSG,Value);
  PSG_writeReg(psg,PSGLatch,Value);
//  fifoSendValue32( FIFO_USER_01,(4<<24)|(PSGLatch<<8)|Value );

  return;

case 0xA8: /* Primary slot state   */
case 0xA9: /* Keyboard port        */
case 0xAA: /* General IO register  */
case 0xAB: /* PPI control register */
  /* Write to PPI */
  Write8255(&PPI,Port-0xA8,Value);
  /* If general I/O register has changed... */
  if(PPI.Rout[2]!=IOReg) { PPIOut(PPI.Rout[2],IOReg);IOReg=PPI.Rout[2]; }
  /* If primary slot state has changed... */
  if(PPI.Rout[0]!=PSLReg) PSlot(PPI.Rout[0]);
  /* Done */  
  return;

case 0xB5: /* RTC Data */
  if(RTCReg<13)
  {
    /* J = register bank# now */
    J=RTCMode&0x03;
    /* Store the value */
    RTC[J][RTCReg]=Value;
    /* If CMOS modified, we need to save it */
    if(J>1) SaveCMOS=1;
    return;
  }
  /* RTC[13] is a register bank# */
  if(RTCReg==13) RTCMode=Value;
  return;

case 0xD0: /* FDC command */
case 0xD1: /* FDC track   */
case 0xD2: /* FDC sector  */
case 0xD3: /* FDC data    */
  /* Brazilian DiskROM I/O ports */
  Write1793(&FDC,Port-0xD0,Value);
  return;

case 0xD4: /* FDC system  */
  /* Brazilian DiskROM drive/side: [xxxSxxDx] */
  Value=((Value&0x02)>>1)|S_DENSITY|(Value&0x10? 0:S_SIDE);
  Write1793(&FDC,WD1793_SYSTEM,Value);
  return;

case 0xFC: /* Mapper page at 0000h */
case 0xFD: /* Mapper page at 4000h */
case 0xFE: /* Mapper page at 8000h */
case 0xFF: /* Mapper page at C000h */
  J=Port-0xFC;
  Value&=RAMMask;
  if(RAMMapper[J]!=Value)
  {
//    if(Verbose&0x08) printf("RAM-MAPPER: block %d at %Xh\n",Value,J*0x4000);
    I=J<<1;
    RAMMapper[J]      = Value;
    MemMap[3][2][I]   = RAMData+((int)Value<<14);
    MemMap[3][2][I+1] = MemMap[3][2][I]+0x2000;
    if((PSL[J]==3)&&(SSL[J]==2))
    {
      EnWrite[J] = 1;
      RAM[I]     = MemMap[3][2][I];
      RAM[I+1]   = MemMap[3][2][I+1];
    }
  }
  return;

  }
}
