#ifndef DSDRIVER_H
#define DSDRIVER_H

#define printf LOG


typedef struct {
	int siz;
	byte buf[0];
} TapeData;

extern byte touchMode;				/* touch Panel Mode          */

#define RGB16(a,r,g,b)  RGB15(r,g,b)|((a)<<15)

void InitLOG(void);
void LOG( char * str , ... );
void drawAllKeyboard(void);
void createTouchMap();
void changeBGSize(byte V);

#endif
