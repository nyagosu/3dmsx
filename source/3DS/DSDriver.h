#ifndef DSDRIVER_H
#define DSDRIVER_H

#define printf LOG

typedef struct {
	int siz;
	byte buf[0];
} TapeData;

extern byte touchMode;				/* touch Panel Mode          */

void InitLOG(void);
void ExitLOG(void);
void LOG( char * str , ... );
void drawAllKeyboard(void);
void createTouchMap();
void changeBGSize(byte V);
void InitSDMC(void);
void ExitSDMC(void);

#endif
