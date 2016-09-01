#ifndef DSDRIVER_H
#define DSDRIVER_H

#define printf LOG

typedef struct {
	int siz;
	byte buf[0];
} TapeData;

extern byte touchMode;				/* touch Panel Mode          */

void LOG( char * str , ... );
void drawAllKeyboard(void);
void createTouchMap();
void changeBGSize(byte V);

#endif
