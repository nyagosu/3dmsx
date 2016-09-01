#include "msx.h"
#include <3DS.h>

void WrVRAM_HW( byte * b, int dt )
{
//	int adr = (int)VRAM - (int)b;

	switch( ScrMode ){
	case 0:
//		if( adr&((VDP[2]&MSK[J].R2)<<10) ){		// pat name table
/*
			FC=xxx;
			BC=xxx;
			T =xxx;
			P =xxx;
			for(X=0;X<8;X++,T++,P+=(250))
			{
				Y=G[(int)*T<<3];
				P[0]=Y&0x80? FC:BC;
				P[1]=Y&0x40? FC:BC;
				P[2]=Y&0x20? FC:BC;
				P[3]=Y&0x10? FC:BC;
				P[4]=Y&0x08? FC:BC;
				P[5]=Y&0x04? FC:BC;
			}
		}
*/
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 10:
	case 11:
	case 12:
		break;
	}
}

int set_screen_HW( int mod )
{
	switch( ScrMode ){
	case 0:
		//256pal bitmap?
	case 1:
		// 16pal BG
	case 2:
		// 16pal BG
	case 3:
		// 16pal BG?
	case 4:
		// 16pal BG
	case 5:
		//256pal bitmap?
	case 6:
		//256pal bitmap?
	case 7:
		//256pal bitmap?
	case 8:
		//256pal bitmap?
	case 10:
		//64kcol bitmap?
	case 11:
		//64kcol bitmap?
	case 12:
		//64kcol bitmap?
	case 13:
		// 256pal bitmap?
	default:
		break;
	}
	return 0;
}

void setPallette_HW()
{
	//screen6,7は中間色もセットする?
	
	
}
