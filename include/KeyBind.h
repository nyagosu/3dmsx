#ifndef __KEYBIND_H__
#define __KEYBIND_H__

typedef struct {
	u8  type;
	u8  code;
	u8  mask;
} JOYBIND;

typedef struct {
	u16 x_mgn;				/* keytop text x pos from key disp pos (dot) */
	u16 y_mgn;				/* keytop text y pos from key disp pos (dot) */
	u8  fnttype;			/* 0:MSX ROM Font  1:SYSTEM Font */
	char text[8];			/* keytop text */
} KEYTOP;

typedef struct _KEYBIND {
	u8  type;				/* 0:KEYBOARD 1:JOYSTICK */
	u8  code;				/* Keycode */
	u8  mask;				/* key bit mask */
	u8  Hold;				/* key hold number (ex. shift, graph key) */
	u16 x;					/* key disp x pos (x8) */
	u16 y;					/* key disp y pos (x8) */
	u16 width;				/* key disp width (x8) */
	u16 height;				/* key disp height(x8) */
	KEYTOP keytop[10];		/* key top text */
} KEYBIND;

extern JOYBIND JoyBinds[16];
extern KEYBIND KeyBinds[128];

//extern int DownSoftKey( int );
//extern int DownJoy( word vKey );
//extern int UpJoy( word vKey );
// extern void ReadKeyBind( char *, KEYBIND * );

#endif
