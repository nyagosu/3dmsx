#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>	
#include <sys/dir.h>
#include <3DS.h>

#include "DSFileList.h"
#include "DSGraph.h"

extern void LOG( char * str , ... );
extern void waitForVBlank(int);


int isChildDir( char * path )
{
	char * f;
	char * e;
	f = strchr( path , ':' );
	if( f != NULL ){
		f++;
	}else{
		f = strchr( path , '/' );
	}
	e = strrchr( path , '/' );
	if( f == NULL ) return false;
	if( f != e ){
		return false;
	}
	return true;
}

void addfile( LLIST * l,int typ, char * nm )
{
	FILELIST * fl = (FILELIST *)malloc( sizeof(FILELIST) );
	fl->type = typ;
	strcpy( fl->name, nm );
	LLIST_add( l, fl );
}

LLIST * getfilelist( char * path, char * ext )
{
	char * c;
	LLIST * l;
//	char fname[256];
	struct dirent * st;
	DIR * dir;

//	LOG( "getfilelist1");
	l = LLIST_new();
//	LOG( "getfilelist2[%s]", path);
	/* add parent dir */
	dir = opendir( path );
	addfile( l, 3, "fat0:/" );
	addfile( l, 3, "fat1:/" );
	addfile( l, 3, "fat2:/" );
	addfile( l, 3, "fat3:/" );
	if( dir == NULL ) return l;
//	LOG( "getfilelist3");

	while( (st = readdir(dir)) != NULL ){
//		LOG( "fn[%s]",fname );
		if( st->d_type == DT_DIR ){
			if( strcmp( st->d_name, "." )!=0 ) addfile( l, 2, st->d_name );
		} else {
			if( ext!= NULL ){
				c = strrchr( st->d_name, '.' );
				if( c != NULL ){
					if( strcasecmp( c+1, ext )==0 )	addfile( l, 1, st->d_name );
				}
			}
		}
	}
//	LOG( "getfilelist4");

	closedir(dir);
//	LOG( "getfilelist5");
	return l;
}

FILELIST * getlist( LLIST * lst, int cnt )
{
	int i;
	LLIST_top(lst);
	for(i=0;i<cnt;i++){
		LLIST_next(lst);
	}
	return LLIST_get(lst);
}

#define FSELWIN_X     20
#define FSELWIN_Y     30
#define FSELWIN_W     200
#define FSELWIN_H     120

#define FLIST_X       (FSELWIN_X+10)
#define FLIST_Y       (FSELWIN_Y+10)
#define FLIST_W       120
#define FLIST_H       10
#define FLIST_CNT     10

#define BtnUP_X       (FSELWIN_X+130)
#define BtnUP_Y       (FSELWIN_Y+10)
#define BtnUP_W       20
#define BtnUP_H       20

#define BtnDOWN_X     (FSELWIN_X+130)
#define BtnDOWN_Y     (FSELWIN_Y+90)
#define BtnDOWN_W     20
#define BtnDOWN_H     20

#define BtnOK_X       (FSELWIN_X+155)
#define BtnOK_Y       (FSELWIN_Y+70)
#define BtnOK_W       40
#define BtnOK_H       20

#define BtnCancel_X   (FSELWIN_X+155)
#define BtnCancel_Y   (FSELWIN_Y+90)
#define BtnCancel_W   40
#define BtnCancel_H   20

#define BtnEject_X    (FSELWIN_X+155)
#define BtnEject_Y    (FSELWIN_Y+10)
#define BtnEject_W    40
#define BtnEject_H    20

#define TxtDrv_X	(FSELWIN_X+155)
#define TxtDrv_Y	(FSELWIN_Y+40)

int downCheck( int tx,int ty, int x, int y , int w, int h )
{
	if( (tx > x) && (tx < x + w) ){
		if( (ty > y) && (ty < y + h) ){
			return true;
		}
	}
	return false;
}

void addSlash( char * path )
{
	if( path[strlen(path)-1] != '/' ) strcat( path, "/" );
}

int isRootPath( char * path )
{
	if( strcmp( path, "/" )==0 ) return true;
	if( strncasecmp( path, "fat", 3 )==0 ){
		path += 3;
//		LOG( path );
		if( (*path=='0') || (*path=='1') || (*path=='2') || (*path=='3') ) path++;
		if( strcmp( path,":/" )==0 ) return true;
	}
	return false;
}

int fileselect( char * p, char * ext, char * fname )
{
	int top = 0;
	int sel = -1;
//	int y=0;
	int i;
	u32 k;
	int chg = true;
	int ret = false;
//	int col;
	int loopflg = true;
	int firstflg = true;
	int evt;
	int tx = 0, ty = 0;
	LLIST * lst;
	FILELIST * cur;
	int pendownflg = false;
	char path[256];

	strcpy( path, p );

	drawWindow( GFX_BOTTOM,GFX_LEFT,FSELWIN_X,FSELWIN_Y,FSELWIN_W,FSELWIN_H );
	drawButton( GFX_BOTTOM,GFX_LEFT,BtnUP_X    ,BtnUP_Y    ,BtnUP_W    ,BtnUP_H    ,(u8*)"\x7F"   );
	drawButton( GFX_BOTTOM,GFX_LEFT,BtnDOWN_X  ,BtnDOWN_Y  ,BtnDOWN_W  ,BtnDOWN_H  ,(u8*)"\x80"   );
	drawButton( GFX_BOTTOM,GFX_LEFT,BtnEject_X ,BtnEject_Y ,BtnEject_W ,BtnEject_H ,(u8*)"Eject"  );
	drawButton( GFX_BOTTOM,GFX_LEFT,BtnOK_X    ,BtnOK_Y    ,BtnOK_W    ,BtnOK_H    ,(u8*)"OK"     );
	drawButton( GFX_BOTTOM,GFX_LEFT,BtnCancel_X,BtnCancel_Y,BtnCancel_W,BtnCancel_H,(u8*)"Cancel" );

	box ( GFX_BOTTOM,GFX_LEFT,FLIST_X,FLIST_Y,FLIST_W,FLIST_H*FLIST_CNT, 0x7f7f7f );

//	LOG( "fileselect 1" );
	lst = getfilelist( path, ext );
//	LOG( "fileselect 2" );

	while(loopflg){

		if( chg ){

			fill( GFX_BOTTOM,GFX_LEFT,FLIST_X+1,FLIST_Y+1,FLIST_W-2,FLIST_H*FLIST_CNT-2, 0xFFFFFF );

			cur = getlist( lst , top );
//			LOG( "fileselect 3" );
			for( i=0;i<FLIST_CNT;i++ ){
//				LOG( cur->name );
				char buf[32];
				if( cur == NULL ) break;
				if( top + i == sel ) fill( GFX_BOTTOM,GFX_LEFT,FLIST_X+1,FLIST_Y+FLIST_H*i+1,FLIST_W-2,FLIST_H-2, 0xff7f7f );
				if( cur->type == 1 ){
					strcpy( buf, cur->name );
				}else{
					strcpy( buf, "[" );
					strcat( buf, cur->name );
					strcat( buf, "]" );
				}
				drawText( GFX_BOTTOM,GFX_LEFT,FLIST_X+2,FLIST_Y+2+FLIST_H*i, (u8*)buf, 0x00, 0 );
				ret =LLIST_next(lst);
				if( ret!=LLIST_OK ) break;
				cur = LLIST_get(lst);
			}
			chg = false;
		}

//		LOG( "fileselect 4" );

		scanKeys();
		k = keysDown();
		evt = 0;
		if( k&KEY_TOUCH ){
			pendownflg = true;
			touchPosition t;
			touchRead(&t);
			tx = t.px;
			ty = t.py;
			/* pset(GFX_BOTTOM,GFX_LEFT, tx, ty , RGB16(1,0,0,0) ); */
		}else{
			if( pendownflg ){
				if( downCheck( tx, ty, FLIST_X,FLIST_Y,FLIST_W, FLIST_H*FLIST_CNT ) ){
					sel = top + ( ty - FLIST_Y ) / 10;
					chg = true;
				}else
				if( downCheck(tx,ty, BtnOK_X, BtnOK_Y, BtnOK_W, BtnOK_H) ){
					evt = 1;
				}else
				if( downCheck(tx,ty, BtnCancel_X, BtnCancel_Y, BtnCancel_W, BtnCancel_H) ){
					evt = 2;
				}else
				if( downCheck(tx,ty, BtnEject_X, BtnEject_Y, BtnEject_W, BtnEject_H) ){
					evt = 3;
				}else
				if( downCheck(tx,ty, BtnUP_X, BtnUP_Y, BtnUP_W, BtnUP_H) ){
					if( top > 0 ){
						top --;
						chg = true;
					}
				}else
				if( downCheck(tx,ty, BtnDOWN_X, BtnDOWN_Y, BtnDOWN_W, BtnDOWN_H) ){
					if( top < lst->count - 10 ){
						top ++;
						chg = true;
					}
				}
			}
			pendownflg = false;
		}
		if(k&KEY_DOWN ){
			if( sel < lst->count-1 ){
				sel++;
				if( top+9 < sel ){
					top+= sel-(top+9);
				}
				chg = true;
			}
		}
		if(k&KEY_UP   ){
			if( sel > 0 ){
				sel--;
				if( top > sel ){
					top = sel;
				}
				chg = true;
			}
		}
		if(k&KEY_A  ){
			if( !firstflg ) evt=1;
		}else{ 
			firstflg = false;
		}
		if(k&KEY_B ) evt=2;
		if(k&KEY_X ) evt=3;
		
		switch( evt ){
			case 1:	// OK button
				if( sel == -1 ) break; 
				FILELIST * wk = getlist( lst, sel );
				switch( wk->type ){
				case 1:	/* file */
					strcpy( fname, path );
					addSlash(fname);
					strcat( fname, wk->name );
//					LOG( "%s selected.", fname );
					ret = true;
					loopflg = false;
					break;
				case 2:	/* dir */
					if( strcmp( wk->name , ".." ) == 0 ){	/* parent */
						char * wk;
						wk = strrchr( path, '/' );
						*(wk+1) = '\0';
						if( !isRootPath(path) ) *wk = '\0';
					}else{									/* child dir */
						addSlash(path);
						strcat( path, wk->name );
					}
//					LOG( path );
					LLIST_free( lst );
					lst = getfilelist( path, ext );
//					LOG( "fileselect after getfilelist" );
					top = 0;
					sel = -1;
					chg = true;
					break;
				case 3:
					strcpy( path, wk->name );
//					LOG( path );
					LLIST_free( lst );
					lst = getfilelist( path, ext );
					top = 0;
					sel = -1;
					chg = true;
					break;
				}
				break;
			case 2: // cancel
				ret = false;
				loopflg = false;
				break;
			case 3: // eject
//				LOG( "Ejected." );
				strcpy( fname, "" );
				ret = true;
				loopflg = false;
				break;
		}

		waitForVBlank(6);
	}

	LLIST_free( lst );
	return ret;
}
