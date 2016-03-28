#ifndef NDS_IPC2_INCLUDE
#define NDS_IPC2_INCLUDE

#include <nds/ipc.h>
#include "emu2212.h"
#include "emu2149.h"
#include "emu2413.h"

typedef struct sTransferRegion2 {
  u16 soundCommand;
  int sampleRate;
  PSG  psg;
  SCC  scc;
  OPLL opll;
  int  psgON;
  int  sccON;
} TransferRegion2, * pTransferRegion2;

#define IPC2 ((TransferRegion2 volatile *)(0x027FF000+sizeof(TransferRegion)))

#endif
