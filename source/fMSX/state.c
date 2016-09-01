#include <string.h>
#include "MSX.h"
#include "emu2149.h"
#include "emu2212.h"

extern PSG * psg;
extern SCC * scc;

#ifdef ZLIB
#define fopen           gzopen
#define fclose          gzclose
#define fread(B,L,N,F)  gzread(F,B,(L)*(N))
#define fwrite(B,L,N,F) gzwrite(F,B,(L)*(N))
#endif

/** StateID() ************************************************/
/** Compute 16bit emulation state ID used to identify .STA  **/
/** files.                                                  **/
/*************************************************************/
word StateID(void)
{
  word ID;
  int J,I;

  ID=0x0000;

  /* Add up cartridge ROMs, BIOS, BASIC, ExtBIOS, and DiskBIOS bytes */
  for(I=0;I<MAXSLOTS;++I)
    if(ROMData[I]) for(J=0;J<(ROMMask[I]+1)*0x2000;++J) ID+=I^ROMData[I][J];
  if(MemMap[0][0][0]&&(MemMap[0][0][0]!=EmptyRAM))
    for(J=0;J<0x8000;++J) ID+=MemMap[0][0][0][J];
  if(MemMap[3][1][0]&&(MemMap[3][1][0]!=EmptyRAM))
    for(J=0;J<0x4000;++J) ID+=MemMap[3][1][0][J];
  if(MemMap[3][1][2]&&(MemMap[3][1][2]!=EmptyRAM))
    for(J=0;J<0x4000;++J) ID+=MemMap[3][1][2][J];

  return(ID);
}

/** SaveSTA() ************************************************/
/** Save emulation state to a .STA file.                    **/
/*************************************************************/
int SaveState(const char *FileName)
{
  static byte Header[16] = "STE\032\003\0\0\0\0\0\0\0\0\0\0\0";
  unsigned int State[256],J,I,K;
  FILE *F;

  /* Open state file */
  if(!(F=fopen(FileName,"wb"))) return(0);

  /* Prepare the header */
  J=StateID();
  Header[5] = RAMPages;
  Header[6] = VRAMPages;
  Header[7] = J&0x00FF;
  Header[8] = J>>8;

  /* Write out the header */
  if(fwrite(Header,1,sizeof(Header),F)!=sizeof(Header))
  { fclose(F);return(0); }

  /* Fill out hardware state */
  J=0;
  State[J++] = VDPData;
  State[J++] = PLatch;
  State[J++] = ALatch;
  State[J++] = VAddr;
  State[J++] = VKey;
  State[J++] = PKey;
  State[J++] = WKey;
  State[J++] = IRQPending;
  State[J++] = ScanLine;
  State[J++] = RTCReg;
  State[J++] = RTCMode;
  State[J++] = KanLetter;
  State[J++] = KanCount;
  State[J++] = IOReg;
  State[J++] = PSLReg;
  State[J++] = FMPACKey;

  /* Memory setup */
  for(I=0;I<4;++I)
  {
    State[J++] = SSLReg[I];
    State[J++] = PSL[I];
    State[J++] = SSL[I];
    State[J++] = EnWrite[I];
    State[J++] = RAMMapper[I];
  }  

  /* Cartridge setup */
  for(I=0;I<MAXSLOTS;++I)
  {
    State[J++] = ROMType[I];
    for(K=0;K<4;++K) State[J++]=ROMMapper[I][K];
  }

  /* Write out hardware state */
  if(fwrite(&CPU,1,sizeof(CPU),F)!=sizeof(CPU))
  { fclose(F);return(0); }
  if(fwrite(&PPI,1,sizeof(PPI),F)!=sizeof(PPI))
  { fclose(F);return(0); }
  if(fwrite(VDP,1,sizeof(VDP),F)!=sizeof(VDP))
  { fclose(F);return(0); }
  if(fwrite(VDPStatus,1,sizeof(VDPStatus),F)!=sizeof(VDPStatus))
  { fclose(F);return(0); }
  if(fwrite(Palette,1,sizeof(Palette),F)!=sizeof(Palette))
  { fclose(F);return(0); }
  if(fwrite(&psg,1,sizeof(PSG),F)!=sizeof(PSG))
  { fclose(F);return(0); }
//  if(fwrite(&OPLL,1,sizeof(OPLL),F)!=sizeof(OPLL))
//  { fclose(F);return(0); }
  if(fwrite(&scc,1,sizeof(SCC),F)!=sizeof(SCC))
  { fclose(F);return(0); }
  if(fwrite(State,1,sizeof(State),F)!=sizeof(State))
  { fclose(F);return(0); }

  /* Save memory contents */
  if(fwrite(RAMData,1,RAMPages*0x4000,F)!=RAMPages*0x4000)
  { fclose(F);return(0); }
  if(fwrite(VRAM,1,VRAMPages*0x4000,F)!=VRAMPages*0x4000)
  { fclose(F);return(0); }

  /* Done */
  fclose(F);
  return(1);
}

/** LoadSTA() ************************************************/
/** Load emulation state from a .STA file.                  **/
/*************************************************************/
int LoadState(const char *FileName)
{
  unsigned int State[256],J,I,K;
  byte Header[16];
  FILE *F;

  /* Open state file */
  if(!(F=fopen(FileName,"rb"))) return(0);

  /* Read the header */
  if(fread(Header,1,sizeof(Header),F)!=sizeof(Header))
  { fclose(F);return(0); }

  /* Verify the header */
  if(memcmp(Header,"STE\032\003",5))
  { fclose(F);return(0); }
  if(Header[7]+Header[8]*256!=StateID())
  { fclose(F);return(0); }
  if((Header[5]!=(RAMPages&0xFF))||(Header[6]!=(VRAMPages&0xFF)))
  { fclose(F);return(0); }

  /* Read the hardware state */
  if(fread(&CPU,1,sizeof(CPU),F)!=sizeof(CPU))
  { fclose(F);return(0); }
  if(fread(&PPI,1,sizeof(PPI),F)!=sizeof(PPI))
  { fclose(F);return(0); }
  if(fread(VDP,1,sizeof(VDP),F)!=sizeof(VDP))
  { fclose(F);return(0); }
  if(fread(VDPStatus,1,sizeof(VDPStatus),F)!=sizeof(VDPStatus))
  { fclose(F);return(0); }
  if(fread(Palette,1,sizeof(Palette),F)!=sizeof(Palette))
  { fclose(F);return(0); }
  if(fread(&psg,1,sizeof(PSG),F)!=sizeof(PSG))
  { fclose(F);return(0); }
//  if(fread(&OPLL,1,sizeof(OPLL),F)!=sizeof(OPLL))
//  { fclose(F);return(0); }
  if(fread(&scc,1,sizeof(SCC),F)!=sizeof(SCC))
  { fclose(F);return(0); }
  if(fread(State,1,sizeof(State),F)!=sizeof(State))
  { fclose(F);return(0); }

  /* Load memory contents */
  if(fread(RAMData,1,Header[5]*0x4000,F)!=Header[5]*0x4000)
  { fclose(F);return(0); }
  if(fread(VRAM,1,Header[6]*0x4000,F)!=Header[6]*0x4000)
  { fclose(F);return(0); }

  /* Done with the file */
  fclose(F);

  /* Parse hardware state */
  J=0;
  VDPData    = State[J++];
  PLatch     = State[J++];
  ALatch     = State[J++];
  VAddr      = State[J++];
  VKey       = State[J++];
  PKey       = State[J++];
  WKey       = State[J++];
  IRQPending = State[J++];
  ScanLine   = State[J++];
  RTCReg     = State[J++];
  RTCMode    = State[J++];
  KanLetter  = State[J++];
  KanCount   = State[J++];
  IOReg      = State[J++];
  PSLReg     = State[J++];
  FMPACKey   = State[J++];

  /* Memory setup */
  for(I=0;I<4;++I)
  {
    SSLReg[I]       = State[J++];
    PSL[I]          = State[J++];
    SSL[I]          = State[J++];
    EnWrite[I]      = State[J++];
    RAMMapper[I]    = State[J++];
  }  

  /* Cartridge setup */
  for(I=0;I<MAXSLOTS;++I)
  {
    ROMType[I]      = State[J++];
    for(K=0;K<4;++K) ROMMapper[I][K]=State[J++];
  }

  /* Set RAM mapper pages */
  if(RAMMask)
    for(I=0;I<4;++I)
    {
      RAMMapper[I]       &= RAMMask;
      MemMap[3][2][I*2]   = RAMData+RAMMapper[I]*0x4000;
      MemMap[3][2][I*2+1] = MemMap[3][2][I*2]+0x2000;
    }

  /* Set ROM mapper pages */
  for(I=0;I<MAXSLOTS;++I)
    if(ROMData[I]&&ROMMask[I])
      SetMegaROM(I,ROMMapper[I][0],ROMMapper[I][1],ROMMapper[I][2],ROMMapper[I][3]);

  /* Set main address space pages */
  for(I=0;I<4;++I)
  {
    RAM[2*I]   = MemMap[PSL[I]][SSL[I]][2*I];
    RAM[2*I+1] = MemMap[PSL[I]][SSL[I]][2*I+1];
  }

  /* Set palette */
  for(I=0;I<16;++I)
    SetColor(I,(Palette[I]>>16)&0xFF,(Palette[I]>>8)&0xFF,Palette[I]&0xFF);

  /* Set screen mode and VRAM table addresses */
  SetScreen();

  /* Set some other variables */
  VPAGE    = VRAM+((int)VDP[14]<<14);
  FGColor  = VDP[7]>>4;
  BGColor  = VDP[7]&0x0F;
  XFGColor = FGColor;
  XBGColor = BGColor;

  /* Done */
  return(1);
}

#ifdef ZLIB
#undef fopen
#undef fclose
#undef fread
#undef fwrite
#endif
