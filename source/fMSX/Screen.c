#include <string.h>
#include "msx.h"

#include <3DS.h>



typedef unsigned short pixel;

#define ColAve( p1, p2 ) \
	( ( 0x8000 ) | ( (((p1)&0x7BDE) >> 1) + (((p2)&0x7BDE) >> 1) ) )

/*
#define ColAve( p1, p2 ) \
( ( 0x8000 ) | \
	(( ((((uint32)p1)&0x7C00) + (((uint32)p2)&0x7C00)) >>1 )&0x7C00) | \
	(( (((p1)&0x03E0) + ((p2)&0x03E0)) >>1 )&0x03E0) | \
	(( (((p1)&0x001F) + ((p2)&0x001F)) >>1 )&0x001F)   \
)
*/
extern void LOG( char * str , ... );
extern u8 * itoa( int n, u8 * buf );

static byte ZBuf[320];

/** INLINE ***************************************************/
/** Different compilers inline C functions differently.     **/
/*************************************************************/
#ifndef INLINE
#ifdef __GNUC__
#define INLINE inline
#else

#define INLINE
#define register
#endif
#endif
//static int FirstLine = 32;     /* First scanline in the XBuf */

static void  Sprites(byte Y,byte *ZBuf);
static void  ColorSprites(byte Y,byte *ZBuf);
//static pixel *RefreshBorder(byte Y,pixel C);
static pixel * RefreshBorder(byte y, pixel c, int w1 , int w2);
static void ClearLine( pixel *P, pixel C, int w);
static pixel YJKColor(int Y,int J,int K);

extern unsigned int BPal[256],XPal[80],XPal0; 
extern int  XBufType;
extern char *XBuf;
extern char *XBuf2;

/** ClearLine() **********************************************/
/** Clear 256 pixels from P with color C.                   **/
/*************************************************************/
static void ClearLine( pixel *P, pixel C, int w)
{
  register int J;
  for(J=0;J<w;J++) P[J]=C;
/*
	asm volatile ( 
	"loop:	\n\t"
	"strh	%1,[%0], #2		\n\t"
	"subs	%2,%2,#1		\n\t"
	"bne	loop			\n\t"
	:
	:"r" (P),"r" (C),"r" (w)
	);
*/
}

/** YJKColor() ***********************************************/
/** Given a color in YJK format, return the corresponding   **/
/** palette entry.                                          **/
/*************************************************************/
static INLINE pixel YJKColor(register int Y,register int J,register int K)
{
  register int R,G,B;
		
  R=Y+J;
  G=Y+K;
  B=((5*Y-2*J-K)/4);

  R=R<0? 0:R>31? 31:R;
  G=G<0? 0:G>31? 31:G;
  B=B<0? 0:B>31? 31:B;

  return(BPal[(R&0x1C)|((G&0x1C)<<3)|(B>>3)]);
}

/** RefreshBorder() ******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** the screen border. It returns a pointer to the start of **/
/** scanline Y in XBuf or 0 if scanline is beyond XBuf.     **/
/*************************************************************/
pixel * RefreshBorder(byte y, pixel c, int w1 , int w2)
{
	pixel *P;
	int K;

	/* Set up the transparent color */
	XPal[0]=(!BGColor||SolidColor0)? XPal0:XPal[BGColor];

	K = (ScanLines212? 212:192);

	// return address
	if( y >= K ){
		P = NULL;
	}else{
		P=(pixel *)XBuf + 256 * y;
	}

	return P;
}
/** Sprites() ************************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** sprites in SCREENs 1-3.                                 **/
/*************************************************************/
void Sprites(register byte Y,register byte *ZBuf)
{
	register byte *P,C;
	register byte B,H,*PT,*AT;
	register unsigned int M;
	register int L,S,K;

	memset((char*)ZBuf+32,0,256);
	if(SpritesOFF) return;

	/* Assign initial values before counting */
	H=Sprites16x16? 16:8;
	B=BigSprites?H*2:H;
	C=0;M=0;L=0;
	AT=SprTab-4;
	Y+=VScroll;
	/* Count displayed sprites */
	do
	{
		M<<=1;AT+=4;L++;	/* Iterating through SprTab      */
		K=AT[0];			/* K = sprite Y coordinate       */
		if(K==208) break;	/* Iteration terminates if Y=208 */
		if(K>256-B) K-=256;	/* Y coordinate may be negative  */

		/* Mark all valid sprites with 1s, break at MAXSPRITE1 sprites */
		if((Y>K)&&(Y<=K+B)) { M|=1;C++;if(C==MAXSPRITE1) break; }
	}
	while(L<32);

	/* Draw all marked sprites */
	for(;M;M>>=1,AT-=4)
	{
		if(M&1)
		{
			C=AT[3];                  /* C = sprite attributes */
			L=C&0x80? AT[1]-32:AT[1]; /* Sprite may be shifted left by 32 */
			C&=0x0F;                  /* C = sprite color */

			if((L<256)&&(L>-B)&&C)
			{
				K=AT[0];                /* K = sprite Y coordinate */
				if(K>256-B) K-=256;     /* Y coordinate may be negative */

				P=ZBuf+32+L;
				PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+(BigSprites?((Y-K-1)>>1):(Y-K-1));
				if(BigSprites){
					S=((int)PT[0]<<8)|(H>8? PT[16]:0x00);               /* Get and clip the sprite data */
					if(S&0xFF00)   /* Draw left 8 pixels of the sprite */
					{
						if(S&0x8000){ P[ 0]=P[ 1]=C; }
						if(S&0x4000){ P[ 2]=P[ 3]=C; }
						if(S&0x2000){ P[ 4]=P[ 5]=C; }
						if(S&0x1000){ P[ 6]=P[ 7]=C; }
						if(S&0x0800){ P[ 8]=P[ 9]=C; }
						if(S&0x0400){ P[10]=P[11]=C; }
						if(S&0x0200){ P[12]=P[13]=C; }
						if(S&0x0100){ P[14]=P[15]=C; }
					}
					if(S&0x00FF)   /* Draw right 8 pixels of the sprite */
					{
						if(S&0x0080){ P[16]=P[17]=C; }
						if(S&0x0040){ P[18]=P[19]=C; }
						if(S&0x0020){ P[20]=P[21]=C; }
						if(S&0x0010){ P[22]=P[23]=C; }
						if(S&0x0008){ P[24]=P[25]=C; }
						if(S&0x0004){ P[26]=P[27]=C; }
						if(S&0x0002){ P[28]=P[29]=C; }
						if(S&0x0001){ P[30]=P[31]=C; }
					}             
				}else{
					K = L>=0? 0x0FFFF:(0x10000>>-L)-1;                 /* Mask 1: clip left sprite boundary */
					if(L>256-H) K^=((0x00200>>(H-8))<<(L-257+H))-1;    /* Mask 2: clip right sprite boundary */
					K&=((int)PT[0]<<8)|(H>8? PT[16]:0x00);             /* Get and clip the sprite data */
					if(K&0xFF00)   /* Draw left 8 pixels of the sprite */
					{
						if(K&0x8000) P[0]=C;if(K&0x4000) P[1]=C;
						if(K&0x2000) P[2]=C;if(K&0x1000) P[3]=C;
						if(K&0x0800) P[4]=C;if(K&0x0400) P[5]=C;
						if(K&0x0200) P[6]=C;if(K&0x0100) P[7]=C;
					}
					if(K&0x00FF)   /* Draw right 8 pixels of the sprite */
					{
						if(K&0x0080) P[8]=C; if(K&0x0040) P[9]=C;
						if(K&0x0020) P[10]=C;if(K&0x0010) P[11]=C;
						if(K&0x0008) P[12]=C;if(K&0x0004) P[13]=C;
						if(K&0x0002) P[14]=C;if(K&0x0001) P[15]=C;
					}
				}
			}
		}
	}
}
/** ColorSprites() *******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** color sprites in SCREENs 4-8. The result is returned in **/
/** ZBuf, whose size must be 304 bytes (32+256+16).         **/
/*************************************************************/
void ColorSprites(register byte Y,byte *ZBuf)
{
  register byte B,C,H,J,OrThem;
  register byte *P,*PT,*AT;
  register int L,K;
  register unsigned int M;

  /* Clear ZBuffer and exit if sprites are off */
  memset((char*)ZBuf+32,0,256);
  if(SpritesOFF) return;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  B=BigSprites?H*2:H;
  C=0;M=0;L=0;
  AT=SprTab-4;
  OrThem=0x00;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;          /* Iterating through SprTab      */
    K=AT[0];                  /* Read Y from SprTab            */
    if(K==216) break;         /* Iteration terminates if Y=216 */
    K=(byte)(K-VScroll);      /* Sprite's actual Y coordinate  */
    if(K>256-B) K-=256;       /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE2 sprites */
    if((Y>K)&&(Y<=K+B)) { M|=1;C++;if(C==MAXSPRITE2) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      K=(byte)(AT[0]-VScroll); /* K = sprite Y coordinate */
      if(K>256-B) K-=256;      /* Y coordinate may be negative */

      J=BigSprites?((Y-K-1)>>1):Y-K-1;
      C=SprTab[-0x0200+((AT-SprTab)<<2)+J];
      OrThem|=C&0x40;

      if(C&0x0F)
      {
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+J;
        P=ZBuf+AT[1]+(C&0x80? 0:32);
        C&=0x0F;
        J=PT[0];
        if(OrThem&0x20)
        {
          if( BigSprites ){
            if(J&0x80){ P[ 0]|=C;P[ 1]|=C; }
            if(J&0x40){ P[ 2]|=C;P[ 3]|=C; }
            if(J&0x20){ P[ 4]|=C;P[ 5]|=C; }
            if(J&0x10){ P[ 6]|=C;P[ 7]|=C; }
            if(J&0x08){ P[ 8]|=C;P[ 9]|=C; }
            if(J&0x04){ P[10]|=C;P[11]|=C; }
            if(J&0x02){ P[12]|=C;P[13]|=C; }
            if(J&0x01){ P[14]|=C;P[15]|=C; }
            if(H>8)
            {
              J=PT[16];
              if(J&0x80){ P[16]|=C;P[17]|=C; }
              if(J&0x40){ P[18]|=C;P[19]|=C; }
              if(J&0x20){ P[20]|=C;P[21]|=C; }
              if(J&0x10){ P[22]|=C;P[23]|=C; }
              if(J&0x08){ P[24]|=C;P[25]|=C; }
              if(J&0x04){ P[26]|=C;P[27]|=C; }
              if(J&0x02){ P[28]|=C;P[29]|=C; }
              if(J&0x01){ P[30]|=C;P[31]|=C; }
            }
          }else{
            if(J&0x80) P[0]|=C;if(J&0x40) P[1]|=C;
            if(J&0x20) P[2]|=C;if(J&0x10) P[3]|=C;
            if(J&0x08) P[4]|=C;if(J&0x04) P[5]|=C;
            if(J&0x02) P[6]|=C;if(J&0x01) P[7]|=C;
            if(H>8)
            {
              J=PT[16];
              if(J&0x80) P[ 8]|=C;if(J&0x40) P[ 9]|=C;
              if(J&0x20) P[10]|=C;if(J&0x10) P[11]|=C;
              if(J&0x08) P[12]|=C;if(J&0x04) P[13]|=C;
              if(J&0x02) P[14]|=C;if(J&0x01) P[15]|=C;
            }
          }
        }
        else
        {
          if( BigSprites ){
            if(J&0x80){ P[ 0]=C;P[ 1]=C; }
            if(J&0x40){ P[ 2]=C;P[ 3]=C; }
            if(J&0x20){ P[ 4]=C;P[ 5]=C; }
            if(J&0x10){ P[ 6]=C;P[ 7]=C; }
            if(J&0x08){ P[ 8]=C;P[ 9]=C; }
            if(J&0x04){ P[10]=C;P[11]=C; }
            if(J&0x02){ P[12]=C;P[13]=C; }
            if(J&0x01){ P[14]=C;P[15]=C; }
            if(H>8)
            {
              J=PT[16];
              if(J&0x80){ P[16]=C;P[17]=C; }
              if(J&0x40){ P[18]=C;P[19]=C; }
              if(J&0x20){ P[20]=C;P[21]=C; }
              if(J&0x10){ P[22]=C;P[23]=C; }
              if(J&0x08){ P[24]=C;P[25]=C; }
              if(J&0x04){ P[26]=C;P[27]=C; }
              if(J&0x02){ P[28]=C;P[29]=C; }
              if(J&0x01){ P[30]=C;P[31]=C; }
            }
          }else{
            if(J&0x80) P[0]=C;if(J&0x40) P[1]=C;
            if(J&0x20) P[2]=C;if(J&0x10) P[3]=C;
            if(J&0x08) P[4]=C;if(J&0x04) P[5]=C;
            if(J&0x02) P[6]=C;if(J&0x01) P[7]=C;
            if(H>8)
            {
              J=PT[16];
              if(J&0x80) P[8]=C; if(J&0x40) P[9]=C;
              if(J&0x20) P[10]=C;if(J&0x10) P[11]=C;
              if(J&0x08) P[12]=C;if(J&0x04) P[13]=C;
              if(J&0x02) P[14]=C;if(J&0x01) P[15]=C;
            }
          }
        }
      }

      /* Update overlapping flag */
      OrThem>>=1;
    }
}

/** RefreshLineF() *******************************************/
/** Dummy refresh function called for non-existing screens. **/
/*************************************************************/
void RefreshLineF(register byte Y)
{
  register pixel *P;

/*
  if(Verbose>1)
    printf
    (
      "ScrMODE %d: ChrTab=%X ChrGen=%X ColTab=%X SprTab=%X SprGen=%X\n",
      ScrMode,ChrTab-VRAM,ChrGen-VRAM,ColTab-VRAM,SprTab-VRAM,SprGen-VRAM
    );
*/
//  LOG( "RefreshLineF");

  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(P) ClearLine(P,XPal[BGColor],256);
}

/** RefreshLine0() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN0.                 **/
/*************************************************************/
void RefreshLine0(register byte Y)
{
  register pixel *P,FC,BC;
  register byte *T,*G ,X;
  int i;
  
  BC=XPal[BGColor];
  P=RefreshBorder(Y,BC,256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BC,256);
  else
  {
	for(i=0;i<8+HAdjust;i++){
		*P++=BC;
	}

    G=ChrGen+((Y+VScroll)&0x07);
    T=ChrTab+40*(Y>>3);
    FC=XPal[FGColor];

    for(X=0;X<40;X++,T++,P+=6)
    {
      Y=G[(int)*T<<3];
      P[0]=Y&0x80? FC:BC;
      P[1]=Y&0x40? FC:BC;
      P[2]=Y&0x20? FC:BC;
      P[3]=Y&0x10? FC:BC;
      P[4]=Y&0x08? FC:BC;
      P[5]=Y&0x04? FC:BC;
    }

	for(i=0;i<8-HAdjust;i++){
		*P++=BC;
	}
  }
/*
	asm volatile( 
	"\n"
	"	@ --- INLINE ASM START --- @ \n"
	"	mov 	r2,#7					@ r2 = 7		\n"
	"sc0loop0:											\n"
	"	strh	%3,[%4], #+2			@ *P = BC  P+=2 \n"
	"	subs	r2,r2, #1				@ r2--			\n"
	"	bpl	sc0loop0									\n"
	"													\n"
	"	mov 	r2, #40									\n"
	"sc0loop1:											\n"
	"	ldrb	r3,[%1], #+1			@ r2=*T; T++;	\n"
	"	ldrb	r0,[%0, r3, LSL #3]		@ r0=G[r2<<3]   \n"

#define SC0PLOT( _AA ) \
	"	tst 	r0," _AA "				@ r0 & " _AA "  \n"\
	"	strneh	%2,[%4], #+2			@ *P = (r2>0) ? FC:BC   P++; \n"\
	"	streqh	%3,[%4], #+2	\n" 

	SC0PLOT( "#0x80" )
	SC0PLOT( "#0x40" )
	SC0PLOT( "#0x20" )
	SC0PLOT( "#0x10" )
	SC0PLOT( "#0x08" )
	SC0PLOT( "#0x04" )

	"	subs	r2,r2, #1				@ X--  \n"
	"	bne 	sc0loop1				@ goto sc0loop1 \n"
	"\n"
	"	mov 	r2,#7			\n"
	"sc0loop2:					\n"
	"	strh	%3,[%4], #+2	\n"
	"	subs	r2,r2, #1		\n"
	"	bne 	sc0loop2		\n"
	"	@ --- INLINE ASM END --- @ \n"
	"\n"
	:
	:"r"(G),"r"(T),"r"(FC),"r"(BC),"r"(P)
	:"r0","r2","r3"
	);
  }
*/
}
/** RefreshLine1() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN1, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine1(register byte Y)
{
  register pixel *P,FC,BC;
  register byte K,X,XE,C,*T,*G,*R;
  int cnt;

  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor],256);
  else
  {
    Sprites(Y,ZBuf);
    R=ZBuf+32;
    Y+=VScroll;
    G=ChrGen+(Y&0x07);
    T=ChrTab+((int)(Y&0xF8)<<2);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;
		K=ColTab[*T>>3];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=G[(int)*T<<3];

		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x80)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x40)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x20)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x10)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x08)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x04)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x02)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x01)? FC:BC;P++; } R++;
			T++;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}
	}
	for(X=0;X<XE;X++,T++)
	{
	  K=ColTab[*T>>3];
	  FC=XPal[K>>4];
	  BC=XPal[K&0x0F];
	  K=G[(int)*T<<3];

	  C=*R++;*P++ = (C?XPal[C]:(K&0x80)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x40)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x20)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x10)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x08)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x04)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x02)? FC:BC);
	  C=*R++;*P++ = (C?XPal[C]:(K&0x01)? FC:BC);
	}
	if(HAdjust!=0){
		K=ColTab[*T>>3];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=G[(int)*T<<3];

		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x80)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x40)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x20)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x10)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x08)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x04)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x02)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x01)? FC:BC; cnt--;R++;P++; }
		}
	}
  }
}
/** RefreshLine2() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN2, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine2(register byte Y)
{
  register pixel *P,FC,BC;
  register byte K,X,XE,C,*T,*R;
  register int I,J,cnt;

  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor],256);
  else
  {
    Sprites(Y,ZBuf);
    R=ZBuf+32;
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    I=((int)(Y&0xC0)<<5)+(Y&0x07);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;
		J=(int)*T<<3;
		K=ColTab[(I+J)&ColTabM];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=ChrGen[(I+J)&ChrGenM];
		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x80)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x40)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x20)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x10)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x08)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x04)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x02)? FC:BC;P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x01)? FC:BC;P++; } R++;
			T++;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}
	}
	for(X=0;X<XE;X++,T++,P+=8,R+=8)
	{
		J=(int)*T<<3;
		K=ColTab[(I+J)&ColTabM];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=ChrGen[(I+J)&ChrGenM];
		C=R[0];P[0]=C? XPal[C]:(K&0x80)? FC:BC;
		C=R[1];P[1]=C? XPal[C]:(K&0x40)? FC:BC;
		C=R[2];P[2]=C? XPal[C]:(K&0x20)? FC:BC;
		C=R[3];P[3]=C? XPal[C]:(K&0x10)? FC:BC;
		C=R[4];P[4]=C? XPal[C]:(K&0x08)? FC:BC;
		C=R[5];P[5]=C? XPal[C]:(K&0x04)? FC:BC;
		C=R[6];P[6]=C? XPal[C]:(K&0x02)? FC:BC;
		C=R[7];P[7]=C? XPal[C]:(K&0x01)? FC:BC;
	}
	if(HAdjust!=0){
		J=(int)*T<<3;
		K=ColTab[(I+J)&ColTabM];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=ChrGen[(I+J)&ChrGenM];

		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x80)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x40)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x20)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x10)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x08)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x04)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x02)? FC:BC; cnt--;R++;P++; }
			if(cnt){ C=*R;*P=C?XPal[C]:(K&0x01)? FC:BC; cnt--;R++;P++; }
		}
	}
  }
}

/** RefreshLine3() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN3, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine3(register byte Y)
{
  register pixel *P;
  register byte X,XE,C,K,*T,*G,*R;
  int cnt;

  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor],256);
  else
  {
    Sprites(Y,ZBuf);
    R=ZBuf+32;
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    G=ChrGen+((Y&0x1C)>>2);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;
		K=G[(int)*T<<3];
		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K>>4  )]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K>>4  )]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K>>4  )]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K>>4  )]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K&0x0F)]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K&0x0F)]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K&0x0F)]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=XPal[C?C:(K&0x0F)]; P++; } R++;
			T++;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}
	}
	for(X=0;X<XE;X++,T++,P+=8,R+=8)
	{
	  K=G[(int)*T<<3];
	  C=R[0];P[0]=XPal[C?C:(K>>4  )];
	  C=R[1];P[1]=XPal[C?C:(K>>4  )];
	  C=R[2];P[2]=XPal[C?C:(K>>4  )];
	  C=R[3];P[3]=XPal[C?C:(K>>4  )];
	  C=R[4];P[4]=XPal[C?C:(K&0x0F)];
	  C=R[5];P[5]=XPal[C?C:(K&0x0F)];
	  C=R[6];P[6]=XPal[C?C:(K&0x0F)];
	  C=R[7];P[7]=XPal[C?C:(K&0x0F)];
	}
	if(HAdjust!=0){
		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			K=G[(int)*T<<3];
			if(cnt){ C=*R;*P=XPal[C?C:(K>>4  )]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K>>4  )]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K>>4  )]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K>>4  )]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K&0x0F)]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K&0x0F)]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K&0x0F)]; cnt--;P++; R++; }
			if(cnt){ C=*R;*P=XPal[C?C:(K&0x0F)]; cnt--;P++; R++; }
		}
	}
  }
}


/** RefreshLine4() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN4, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine4(register byte Y)
{
  register pixel *P,FC,BC;
  register byte K,X,XE,C,*T,*R;
  register int I,J,cnt;

  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    I=((int)(Y&0xC0)<<5)+(Y&0x07);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;

		J=(int)*T<<3;
		K=ColTab[(I+J)&ColTabM];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=ChrGen[(I+J)&ChrGenM];

		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x80)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x40)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x20)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x10)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x08)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x04)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x02)? FC:BC; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:(K&0x01)? FC:BC; P++; } R++;
			T++;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}
	}
    for(X=0;X<XE;X++,R+=8,P+=8,T++)
    {
      J=(int)*T<<3;
      K=ColTab[(I+J)&ColTabM];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=ChrGen[(I+J)&ChrGenM];

      C=R[0];P[0]=C? XPal[C]:(K&0x80)? FC:BC;
      C=R[1];P[1]=C? XPal[C]:(K&0x40)? FC:BC;
      C=R[2];P[2]=C? XPal[C]:(K&0x20)? FC:BC;
      C=R[3];P[3]=C? XPal[C]:(K&0x10)? FC:BC;
      C=R[4];P[4]=C? XPal[C]:(K&0x08)? FC:BC;
      C=R[5];P[5]=C? XPal[C]:(K&0x04)? FC:BC;
      C=R[6];P[6]=C? XPal[C]:(K&0x02)? FC:BC;
      C=R[7];P[7]=C? XPal[C]:(K&0x01)? FC:BC;
    }
	if(HAdjust!=0){
		J=(int)*T<<3;
		K=ColTab[(I+J)&ColTabM];
		FC=XPal[K>>4];
		BC=XPal[K&0x0F];
		K=ChrGen[(I+J)&ChrGenM];

		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor]; P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x80)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x40)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x20)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x10)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x08)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x04)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x02)? FC:BC; cnt--; R++;P++; }
			if(cnt){ C=*R;*P=C? XPal[C]:(K&0x01)? FC:BC; cnt--; R++;P++; }
		}
	}
  }
}

/** RefreshLine5() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN5, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine5(register byte Y)
{
  register pixel *P;
  register byte I,X,XE,*T,*R;
  int cnt;

  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;

		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[0]>>4];   P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[0]&0x0F]; P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[1]>>4];   P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[1]&0x0F]; P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[2]>>4];   P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[2]&0x0F]; P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[3]>>4];   P++; } R++;
			if(cnt){cnt--;}else{ I=*R;*P=XPal[I? I:T[3]&0x0F]; P++; } R++;
			T+=4;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}
	}
    for(X=0;X<XE;X++,R+=8,P+=8,T+=4)
    {
      I=R[ 0];P[ 0]=XPal[I? I:T[0]>>4];
      I=R[ 1];P[ 1]=XPal[I? I:T[0]&0x0F];
      I=R[ 2];P[ 2]=XPal[I? I:T[1]>>4];
      I=R[ 3];P[ 3]=XPal[I? I:T[1]&0x0F];
      I=R[ 4];P[ 4]=XPal[I? I:T[2]>>4];
      I=R[ 5];P[ 5]=XPal[I? I:T[2]&0x0F];
      I=R[ 6];P[ 6]=XPal[I? I:T[3]>>4];
      I=R[ 7];P[ 7]=XPal[I? I:T[3]&0x0F];
    }
	if( HAdjust!= 0) {
		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ I=*R;*P=XPal[I? I:T[0]>>4];   cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[0]&0x0F]; cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[1]>>4];   cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[1]&0x0F]; cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[2]>>4];   cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[2]&0x0F]; cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[3]>>4];   cnt--; R++;P++; }
			if(cnt){ I=*R;*P=XPal[I? I:T[3]&0x0F]; cnt--; R++;P++; }
		}
	}
  }
}

/** RefreshLine6() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN6, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine6(register byte Y)
{
  register pixel *P;
  register byte X,XE,*T,*R,C;
  int cnt;
  
  P=RefreshBorder(Y,XPal[BGColor&0x03],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor&0x03],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;

		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[0]>>6      ], XPal[(T[0]>>4)&0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[0]>>2)&0x03], XPal[ T[0]    &0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[1]>>6      ], XPal[(T[1]>>4)&0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[1]>>2)&0x03], XPal[ T[1]    &0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[2]>>6      ], XPal[(T[2]>>4)&0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[2]>>2)&0x03], XPal[ T[2]    &0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[3]>>6      ], XPal[(T[3]>>4)&0x03] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[3]>>2)&0x03], XPal[ T[3]    &0x03] ); P++; } R++;

			T+=4;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor&0x03];P++;
			}
		}
	}
	for(X=0;X<XE;X++)
    {
	  C=R[0];P[0]=C?XPal[C]:ColAve( XPal[ T[0]>>6      ], XPal[(T[0]>>4)&0x03] );
      C=R[1];P[1]=C?XPal[C]:ColAve( XPal[(T[0]>>2)&0x03], XPal[ T[0]    &0x03] );
      C=R[2];P[2]=C?XPal[C]:ColAve( XPal[ T[1]>>6      ], XPal[(T[1]>>4)&0x03] );
      C=R[3];P[3]=C?XPal[C]:ColAve( XPal[(T[1]>>2)&0x03], XPal[ T[1]    &0x03] );
      C=R[4];P[4]=C?XPal[C]:ColAve( XPal[ T[2]>>6      ], XPal[(T[2]>>4)&0x03] );
      C=R[5];P[5]=C?XPal[C]:ColAve( XPal[(T[2]>>2)&0x03], XPal[ T[2]    &0x03] );
      C=R[6];P[6]=C?XPal[C]:ColAve( XPal[ T[3]>>6      ], XPal[(T[3]>>4)&0x03] );
      C=R[7];P[7]=C?XPal[C]:ColAve( XPal[(T[3]>>2)&0x03], XPal[ T[3]    &0x03] );

      R+=8;P+=8;T+=4;
    }
	if( HAdjust!= 0) {
		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor&0x03];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[0]>>6      ], XPal[(T[0]>>4)&0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[0]>>2)&0x03], XPal[ T[0]    &0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[1]>>6      ], XPal[(T[1]>>4)&0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[1]>>2)&0x03], XPal[ T[1]    &0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[2]>>6      ], XPal[(T[2]>>4)&0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[2]>>2)&0x03], XPal[ T[2]    &0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[ T[3]>>6      ], XPal[(T[3]>>4)&0x03] ); cnt--; P++; R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[(T[3]>>2)&0x03], XPal[ T[3]    &0x03] ); cnt--; P++; R++; } 

		}
	}
  }
}

/** RefreshLine7() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN7, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine7(register byte Y)
{
  register pixel *P;
  register byte C,X,XE,*T,*R;
  int cnt;
  P=RefreshBorder(Y,XPal[BGColor],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,XPal[BGColor],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;

		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[0]>>4], XPal[T[0]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[1]>>4], XPal[T[1]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[2]>>4], XPal[T[2]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[3]>>4], XPal[T[3]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[4]>>4], XPal[T[4]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[5]>>4], XPal[T[5]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[6]>>4], XPal[T[6]&0xF] ); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C?XPal[C]:ColAve( XPal[T[7]>>4], XPal[T[7]&0xF] ); P++; } R++;

			T+= 8;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}
	}
    for(X=0;X<XE;X++)
    {
	  C=R[0];P[0]=C?XPal[C]:ColAve( XPal[T[0]>>4], XPal[T[0]&0xF] );
      C=R[1];P[1]=C?XPal[C]:ColAve( XPal[T[1]>>4], XPal[T[1]&0xF] );
      C=R[2];P[2]=C?XPal[C]:ColAve( XPal[T[2]>>4], XPal[T[2]&0xF] );
      C=R[3];P[3]=C?XPal[C]:ColAve( XPal[T[3]>>4], XPal[T[3]&0xF] );
      C=R[4];P[4]=C?XPal[C]:ColAve( XPal[T[4]>>4], XPal[T[4]&0xF] );
      C=R[5];P[5]=C?XPal[C]:ColAve( XPal[T[5]>>4], XPal[T[5]&0xF] );
      C=R[6];P[6]=C?XPal[C]:ColAve( XPal[T[6]>>4], XPal[T[6]&0xF] );
      C=R[7];P[7]=C?XPal[C]:ColAve( XPal[T[7]>>4], XPal[T[7]&0xF] );

      R+=8;P+=8;T+=8;
    }
	if( HAdjust!= 0) {
		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = XPal[BGColor];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[0]>>4], XPal[T[0]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[1]>>4], XPal[T[1]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[2]>>4], XPal[T[2]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[3]>>4], XPal[T[3]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[4]>>4], XPal[T[4]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[5]>>4], XPal[T[5]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[6]>>4], XPal[T[6]&0xF] ); cnt--;P++;R++; } 
			if(cnt){ C=*R;*P=C?XPal[C]:ColAve( XPal[T[7]>>4], XPal[T[7]&0xF] ); cnt--;P++;R++; } 
		}
	}
  }
}

/** RefreshLine8() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN8, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine8(register byte Y)
{
  static byte SprToScr[16] =
  {
    0x00,0x02,0x10,0x12,0x80,0x82,0x90,0x92,
    0x49,0x4B,0x59,0x5B,0xC9,0xCB,0xD9,0xDB
  };
  register pixel *P;
  register byte C,X,XE,*T,*R;
  int cnt;

  P=RefreshBorder(Y,BPal[VDP[7]],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BPal[VDP[7]],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

	if(HAdjust==0){
		XE=32;
	}else {
		XE=31;

		if( HAdjust<0 ){
			cnt = -HAdjust;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[0]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[1]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[2]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[3]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[4]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[5]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[6]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=BPal[C? SprToScr[C]:T[7]]; P++; } R++;
			T+= 8;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = BPal[VDP[7]];P++;
			}
		}
	}
    for(X=0;X<XE;X++,T+=8,R+=8,P+=8)
    {
      C=R[0];P[0]=BPal[C? SprToScr[C]:T[0]];
      C=R[1];P[1]=BPal[C? SprToScr[C]:T[1]];
      C=R[2];P[2]=BPal[C? SprToScr[C]:T[2]];
      C=R[3];P[3]=BPal[C? SprToScr[C]:T[3]];
      C=R[4];P[4]=BPal[C? SprToScr[C]:T[4]];
      C=R[5];P[5]=BPal[C? SprToScr[C]:T[5]];
      C=R[6];P[6]=BPal[C? SprToScr[C]:T[6]];
      C=R[7];P[7]=BPal[C? SprToScr[C]:T[7]];
    }
	if( HAdjust!= 0) {
		if( HAdjust<0 ){
			for(X=0;X<-HAdjust;X++ ){
				*P = BPal[VDP[7]];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[0]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[1]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[2]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[3]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[4]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[5]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[6]]; cnt--;P++;R++; }
			if(cnt){ C=*R;*P=BPal[C? SprToScr[C]:T[7]]; cnt--;P++;R++; }
		}
	}
  }
}

/** RefreshLine10() ******************************************/
/** Refresh line Y (0..191/211) of SCREEN10/11, including   **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine10(register byte Y)
{
  register pixel *P;
  register byte C,X,XE,*T,*R;
  register int J,K,cnt;

  P=RefreshBorder(Y,BPal[VDP[7]],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BPal[VDP[7]],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

	if(HAdjust==0){
		XE=63;
		/* Draw first 4 pixels */
		C=R[0];P[0]=C? XPal[C]:BPal[VDP[7]];
		C=R[1];P[1]=C? XPal[C]:BPal[VDP[7]];
		C=R[2];P[2]=C? XPal[C]:BPal[VDP[7]];
		C=R[3];P[3]=C? XPal[C]:BPal[VDP[7]];
		R+=4;P+=4;
	}else {
		XE=62;

		if( HAdjust<0 ){

			cnt = -HAdjust;
		    /* Draw first 4 pixels */
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;

			K=(T[0]&0x07)|((T[1]&0x07)<<3);
			if(K&0x20) K-=64;
			J=(T[2]&0x07)|((T[3]&0x07)<<3);
			if(J&0x20) J-=64;

			if(cnt){cnt--;}else{ C=*R;Y=T[0]>>3;*P=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;Y=T[1]>>3;*P=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;Y=T[2]>>3;*P=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;Y=T[3]>>3;*P=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); P++; } R++;
			T+=4;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = BPal[VDP[7]];P++;
			}
			C=R[0];P[0]=C? XPal[C]:BPal[VDP[7]];
			C=R[1];P[1]=C? XPal[C]:BPal[VDP[7]];
			C=R[2];P[2]=C? XPal[C]:BPal[VDP[7]];
			C=R[3];P[3]=C? XPal[C]:BPal[VDP[7]];
			R+=4;P+=4;
		}
	}
    for(X=0;X<XE;X++,T+=4,R+=4,P+=4)
    {
      K=(T[0]&0x07)|((T[1]&0x07)<<3);
      if(K&0x20) K-=64;
      J=(T[2]&0x07)|((T[3]&0x07)<<3);
      if(J&0x20) J-=64;

      C=R[0];Y=T[0]>>3;P[0]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
      C=R[1];Y=T[1]>>3;P[1]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
      C=R[2];Y=T[2]>>3;P[2]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
      C=R[3];Y=T[3]>>3;P[3]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K);
    }
	if( HAdjust!= 0) {
		if( HAdjust<0 ){
			for(X=0;X<HAdjust;X++ ){
				*P = BPal[VDP[7]];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			for(X=0;X<2;X++){
				K=(T[0]&0x07)|((T[1]&0x07)<<3);
				if(K&0x20) K-=64;
				J=(T[2]&0x07)|((T[3]&0x07)<<3);
				if(J&0x20) J-=64;
				if(cnt){ C=R[0];Y=T[0]>>3;P[0]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); cnt--; } else break;
				if(cnt){ C=R[1];Y=T[1]>>3;P[1]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); cnt--; } else break;
				if(cnt){ C=R[2];Y=T[2]>>3;P[2]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); cnt--; } else break;
				if(cnt){ C=R[3];Y=T[3]>>3;P[3]=C? XPal[C]:Y&1? XPal[Y>>1]:YJKColor(Y,J,K); cnt--; } else break;
				T+=4;R+=4;P+=4;
			}
		}
	}
  }
}

/** RefreshLine12() ******************************************/
/** Refresh line Y (0..191/211) of SCREEN12, including      **/
/** sprites in this line.                                   **/
/*************************************************************/
void RefreshLine12(register byte Y)
{
  register pixel *P;
  register byte C,X,XE,*T,*R;
  register int J,K,cnt;

  P=RefreshBorder(Y,BPal[VDP[7]],256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BPal[VDP[7]],256);
  else
  {
    ColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    if(HScroll512&&(HScroll>255)) T=(byte *)((int)T^0x10000);
    T+=HScroll&0xFC;

	if(HAdjust==0){
		XE=63;
		/* Draw first 4 pixels */
		C=R[0];P[0]=C? XPal[C]:BPal[VDP[7]];
		C=R[1];P[1]=C? XPal[C]:BPal[VDP[7]];
		C=R[2];P[2]=C? XPal[C]:BPal[VDP[7]];
		C=R[3];P[3]=C? XPal[C]:BPal[VDP[7]];
		R+=4;P+=4;
	}else {
		XE=62;

		if( HAdjust<0 ){
			cnt = -HAdjust;
			/* Draw first 4 pixels */
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:BPal[VDP[7]]; P++; } R++;

			K=(T[0]&0x07)|((T[1]&0x07)<<3);
			if(K&0x20) K-=64;
			J=(T[2]&0x07)|((T[3]&0x07)<<3);
			if(J&0x20) J-=64;

			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:YJKColor(T[0]>>3,J,K); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:YJKColor(T[1]>>3,J,K); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:YJKColor(T[2]>>3,J,K); P++; } R++;
			if(cnt){cnt--;}else{ C=*R;*P=C? XPal[C]:YJKColor(T[3]>>3,J,K); P++; } R++;
			T++;
		}else{
			for(X=0;X<HAdjust;X++ ){
				*P = BPal[VDP[7]];P++;
			}
			/* Draw first 4 pixels */
			C=R[0];P[0]=C? XPal[C]:BPal[VDP[7]];
			C=R[1];P[1]=C? XPal[C]:BPal[VDP[7]];
			C=R[2];P[2]=C? XPal[C]:BPal[VDP[7]];
			C=R[3];P[3]=C? XPal[C]:BPal[VDP[7]];
			R+=4;P+=4;
		}
	}
    for(X=0;X<XE;X++,T+=4,R+=4,P+=4)
    {
      K=(T[0]&0x07)|((T[1]&0x07)<<3);
      if(K&0x20) K-=64;
      J=(T[2]&0x07)|((T[3]&0x07)<<3);
      if(J&0x20) J-=64;

      C=R[0];P[0]=C? XPal[C]:YJKColor(T[0]>>3,J,K);
      C=R[1];P[1]=C? XPal[C]:YJKColor(T[1]>>3,J,K);
      C=R[2];P[2]=C? XPal[C]:YJKColor(T[2]>>3,J,K);
      C=R[3];P[3]=C? XPal[C]:YJKColor(T[3]>>3,J,K);
    }
	if( HAdjust!= 0) {
		if( HAdjust<0 ){
			for(X=0;X<HAdjust;X++ ){
				*P = BPal[VDP[7]];P++;
			}
		}else{
			cnt = 8 - HAdjust;
			for(X=0;X<2;X++){
				K=(T[0]&0x07)|((T[1]&0x07)<<3);
				if(K&0x20) K-=64;
				J=(T[2]&0x07)|((T[3]&0x07)<<3);
				if(J&0x20) J-=64;
				if(cnt){ C=R[0];P[0]=C? XPal[C]:YJKColor(T[0]>>3,J,K); cnt--; }else break;
				if(cnt){ C=R[1];P[1]=C? XPal[C]:YJKColor(T[1]>>3,J,K); cnt--; }else break;
				if(cnt){ C=R[2];P[2]=C? XPal[C]:YJKColor(T[2]>>3,J,K); cnt--; }else break;
				if(cnt){ C=R[3];P[3]=C? XPal[C]:YJKColor(T[3]>>3,J,K); cnt--; }else break;
				T+=4,R+=4,P+=4;
			}
		}
	}
  }
}

/** RefreshLineTx80() ****************************************/
/** Refresh line Y (0..191/211) of TEXT80.                  **/
/*************************************************************/
void RefreshLineTx80(register byte Y)
{
  register pixel *P,FC,BC;
  register byte X,M,*T,*C,*G;
  register int i;

  BC=XPal[BGColor];
  P=RefreshBorder(Y,BC,256,272);
  if(!P) return;

  if(!ScreenON) ClearLine(P,BC,256);
  else
  {
	for(i=0;i<8+HAdjust;i++){
		*P++=BC;
	}
    G=ChrGen+((Y+VScroll)&0x07);
    T=ChrTab+((80*(Y>>3))&ChrTabM);
    C=ColTab+((10*(Y>>3))&ColTabM);
    P+=9;
	
    for(X=0,M=0x00;X<80;X++,T++,P+=3)
    {
      if(!(X&0x07)) M=*C++;
      if(M&0x80) { FC=XPal[XFGColor];BC=XPal[XBGColor]; }
      else       { FC=XPal[FGColor];BC=XPal[BGColor]; }
      M<<=1;
      Y=*(G+((int)*T<<3));
      P[0]=Y&0xC0? ColAve(Y&0x80?FC:BC,Y&0x40?FC:BC):BC;
      P[1]=Y&0x30? ColAve(Y&0x20?FC:BC,Y&0x10?FC:BC):BC;
      P[2]=Y&0x0C? ColAve(Y&0x08?FC:BC,Y&0x04?FC:BC):BC;
    }
	for(i=0;i<8-HAdjust;i++){
		*P++=XPal[BGColor];
	}
  }
}
