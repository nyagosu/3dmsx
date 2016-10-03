-- fmsxDS setting and menu script --
-- setting path
LUADIR  = BASEDIR .. "LUA/";
BIOSDIR = BASEDIR .. "BIOS/";
ROMDIR  = BASEDIR .. "ROM/";
DSKDIR  = BASEDIR .. "DSK/";
CASDIR  = BASEDIR .. "CAS/";
RESDIR  = BASEDIR .. "RES/";

dofile( LUADIR .. "Debug.lua"   );
dofile( LUADIR .. "Keybind.lua" );
function RGB(A,R,G,B) return (A)*0x8000 + (B)*0x0400+ (G)*0x0020 + (R); end;

DSKeyAssign = "Keyboard";
TouchPanel  = "Keyboard";
ScreenFit   = "Auto Fit";

function menu2(t, c, x, y, kai)
	if( c == nil) then c = 0; end
	if( t.bg == nil )then t.bg = 2; end;
	if( t.row == nil )then t.row = #t; end;
	local s=c;
	local os=c;
	local flg = true;
	local n=3;

	local function _drw(f)
		ui.waitForVBlank(1);
		-- clear
		if ( f==4 ) then
			ui.cls( t.bg );
			f=3;
		end
		-- draw menu
		if(f==1 or f==3) then
			local ena = 0xFFFF;
			ui.fill(t.bg, x, y, (FONT_WH)*t.col+10, (FONT_H+1)*t.row+10,0x8000 );
			ui.box (t.bg, x, y, (FONT_WH)*t.col+10, (FONT_H+1)*t.row+10,0xFFFF );
			for i=0,t.row-1,1 do
				if( t[i+1].Enabled == true ) then ena = 0xFFFF; else ena = RGB(1,7,7,7); end;
				ui.drawText2(t.bg, x+5,y+5+(FONT_H+1)*i, t[i+1].Title, ena, 0x8000 );
			end
			drawInfo();
		end
		-- draw arrow
		if(f==2 or f==3) then
			if( t[os+1].Enabled == true ) then ena = 0xFFFF; else ena = RGB(1,8,8,8); end;
			ui.drawText2(t.bg, x+5,y+5+(FONT_H+1)*os, t[os+1].Title, ena, 0x8000 );
			if( t[s +1].Enabled == true ) then ena = 0xFFFF; else ena = RGB(1,8,8,8); end;
			ui.drawText2(t.bg, x+5,y+5+(FONT_H+1)*s , t[s +1].Title, ena, RGB(1,13,13,7) );
		end
	end

	if t.onInit ~= nil then t.onInit(t); end;

	-- menu main routine --
	while( flg ) do
		_drw(n); n=0; os=s;
		ui.scanKeys();
		local keys = ui.keysDown();
		if keys.A    then 
			if t[s+1].Enabled == true then 
				if t[s+1].Func ~= nil then
					flg, ret = t[s+1].Func(t,t[s+1].Param);
					n=3;
				elseif t[s+1].SubMenu ~= nil then
					menu2( t[s+1].SubMenu, 0, x+10, y+10, kai+1 );
					n=3;
					if kai==0 then n=4; end;
				end;
			end;
		end;
		if keys.B    then flg=false; ret=nil; end
		if keys.UP   then s=s-1;     end
		if keys.DOWN then s=s+1;     end
		if s==-1     then s=t.row-1; end
		if s==t.row  then s=0;       end
		if s ~= os   then n=2;       end
	end
	return ret;
end

function screen_setpram(x,y,sx,sy,fit)
	MSX.scr_x   = x;
	MSX.scr_y   = y;
	MSX.scale_x = sx;
	MSX.scale_y = sy;
	if fit~=nil then MSX.AutoFit = fit; end;
end

function Config_screen_user()

	local function _setsc( sx,sy,sw,sh )
		screen_setpram(sx,sy,sw,sh);
		buf = "x:"..sx.."   y:"..sy.."  w:"..sw.."  h:"..sh.."       ";
		ui.drawText2( 1, 40, 170, buf, 0xFFFF, RGB(1,0,0,31) );
	end;
	local flg = true;
	local chg = false;
	local x = MSX.scr_x;
	local y = MSX.scr_y;
	local w = MSX.scale_x;
	local h = MSX.scale_y;

	MSX.AutoFit=0;
	_setsc( x,y,w,h );

	ui.fill( 1, 0,0,256,212, RGB(1,0,0,31) );
	ui.box ( 1, 0,0,256,212, 0xFFFF );
	ui.drawText2( 1, 10, 30, "=== Screen Setting === ", 0xFFFF, RGB(1,0,0,31) );
	ui.drawText2( 1, 40, 80, "     Controler - move screen.", 0xFFFF, RGB(1,0,0,31) );
	ui.drawText2( 1, 40, 120, "B + Controler - zoom up or down.", 0xFFFF, RGB(1,0,0,31) );
	ui.drawText2( 1, 40, 140, "            A - return menu.", 0xFFFF, RGB(1,0,0,31) );

	while( flg ) do
		ui.scanKeys();
		local keys = ui.keysDown();
		local hkeys = ui.keysHeld();
		if keys.A     then flg=false; end
		if hkeys.B    then
			if keys.UP    then h=h+1; chg = true; end
			if keys.DOWN  then h=h-1; chg = true; end
			if keys.LEFT  then w=w+1; chg = true; end
			if keys.RIGHT then w=w-1; chg = true; end
		else
			if keys.UP    then y=y+256; chg = true; end
			if keys.DOWN  then y=y-256; chg = true; end
			if keys.LEFT  then x=x+256; chg = true; end
			if keys.RIGHT then x=x-256; chg = true; end
		end;
		if( chg ) then _setsc(x,y,w,h); end;
		chg = false;
		ui.waitForVBlank(1);
	end
	return true;
end;

function _Config_con(n)
	local dat = { 
				--Key_A   --Key_B    --Select   --Start    --Right    --Left     --UP 
				--Down    --KEY_R    --KEY_L    --KEY_X    --KEY_Y 
			{ {0,8,0x01},{0,4,0x08},{0,6,0x20},{2,1,0x00},{0,8,0x80},{0,8,0x10},{0,8,0x20},  --keyboard
			  {0,8,0x40},{0,0,0x00},{0,0,0x00},{0,7,0x80},{0,0,0x00},msg="Keyboard" },
			{ {1,0,0x10},{1,0,0x20},{0,6,0x20},{2,1,0x00},{1,0,0x08},{1,0,0x04},{1,0,0x01},  --joy1
			  {1,0,0x02},{1,0,0x00},{1,0,0x00},{1,0,0x00},{1,0,0x00},msg="Joystick 1" },
			{ {1,1,0x10},{1,1,0x20},{0,6,0x20},{2,1,0x00},{1,1,0x08},{1,1,0x04},{1,1,0x01},  --joy2
			  {1,1,0x02},{1,1,0x00},{1,1,0x00},{1,1,0x00},{1,1,0x00},msg="Joystick 2" }
	};
	local prm= dat[n];
	local i=0;
	for i=0,11,1 do
		local p = prm[i+1];
		MSX.Joybind(i,p[1],p[2],p[3],p[4] );
	end
	DSKeyAssign = prm.msg;
	ui.LOG( "Controler is "..prm.msg );
	return true, true;
end

function DevFileSelect(t,p)
	local path = { ROM=ROMDIR, DSK=DSKDIR, CAS=CASDIR };
	local ext  = { ROM="ROM" , DSK="DSK" , CAS="CAS"  };

	local r,fn = ui.fileselect( path[p.dev], ext[p.dev] );
	if r==1 then 
		if( p.dev=="ROM" ) then 
			MSX.setCart( p.no, fn, "GUESS" );
			if MSX.SCCPSlot == p.no then MSX.SCCPSlot = -1; end;
		elseif( p.dev=="DSK" ) then
			MSX.DskChange(p.no, fn);
		elseif( p.dev=="CAS" ) then
			MSX.CasChange(fn);
		end;
	end;
	return false;
end;

function ViewLog(t, n)
	local top = 1;
	local flg = true;
	local n = 1;
	
	function _drw(flg)
		local cnt;
		if flg==1 then 
			local logdata = ui.LogData();
			ui.cls(2);		-- 面倒なので全クリア
			for cnt=0,19,1  do
				if logdata[top + cnt] ~= nil then 
					ui.drawText2( 2, 0, cnt*10, logdata[top + cnt], 0xFFFF, BGCOLOR[2] );
				end
			end
		end
	end

	while( flg ) do
		_drw(n); n=0;
		ui.scanKeys();
		local keys = ui.keysHeld();
--		if keys.A    then flg=false; end
		if keys.B    then flg=false; ret=nil; end
		if keys.UP   then top=top-1; n=1; end
		if keys.DOWN then top=top+1; n=1; end
		if top==0    then top=1; end
--		if top>#logdata then top=#logdata; end
	end
	ui.cls(2);
	return ret;
end

function execlua()
	local r,fn = ui.fileselect( "/","lua" );
	if r==1 then dofile( fn ); end;
end

function MainMenu_onInit(t)
--	ui.LOG( "MainMenu_onInit" );
	if MSX.Power then			--Power on
		t[ 1].Title   = " 戻る        ";
		t[ 2].Enabled = false;
		t[ 6].Enabled = true;
--		t[10].Enabled = true;
	else						--Power off
		t[ 1].Title   = " 起動        ";
		t[ 2].Enabled = true;
		t[ 6].Enabled = false;
--		t[10].Enabled = false;
	end
end

-- MenuData
tblMainMenu = 
{	col=13, onInit=MainMenu_onInit, 
	{ Title=" 起動        "  ,Enabled=true , Func=function(n) return false, true; end },
	{ Title=" ロム        "  ,Enabled=true , 
		SubMenu= 
		{	col=13,
			{ Title="ロム１" ,Enabled=true, 
				SubMenu=
				{	col=20,
					{ Title="ROM選択"       , Enabled=true, Func=DevFileSelect, Param={ dev="ROM", no=0 } },
					{ Title="ROMタイプ選択" , Enabled=true,
						SubMenu=
						{	col=20,
							{ Title="自動認識"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"GUESS"); return false; end; },
							{ Title="標準  8K"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"GEN_8"); return false; end; },
							{ Title="標準 16K"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"GEN16"); return false; end; },
							{ Title="KONAMI 5"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"KONA5"); return false; end; },
							{ Title="KONAMI 4"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"KONA4"); return false; end; },
							{ Title="ASCII  8"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"ASC_8"); return false; end; },
							{ Title="ASCII 16"    ,Enabled=true, Func=function(t,p) MSX.setCartType(0,"ASC16"); return false; end; },
						}
					},
					{ Title="SCC+挿入"     ,Enabled=true, Func=function(t,p) MSX.setCart(0,""); MSX.SCCPSlot = 0; return false; end },
					{ Title="ROMを抜く"    ,Enabled=true, Func=function(t,p) MSX.setCart(0,""); MSX.SCCPSlot =-1; return false; end },
				}
			},
			{ Title="ロム２"    ,Enabled=true, 
				SubMenu=
				{	col=20,
					{ Title="ROM選択"       , Enabled=true, Func=DevFileSelect, Param = { dev="ROM", no=1 } },
					{ Title="ROMタイプ選択" , Enabled=true,
						SubMenu=
						{	col=20,
							{ Title="自動認識"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"GUESS"); return false; end; },
							{ Title="標準  8K"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"GEN_8"); return false; end; },
							{ Title="標準 16K"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"GEN16"); return false; end; },
							{ Title="KONAMI 5"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"KONA5"); return false; end; },
							{ Title="KONAMI 4"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"KONA4"); return false; end; },
							{ Title="ASCII  8"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"ASC_8"); return false; end; },
							{ Title="ASCII 16"    ,Enabled=true, Func=function(t,p) MSX.setCartType(1,"ASC16"); return false; end; },
						}
					},
					{ Title="SCC+挿入"     ,Enabled=true, Func=function(t,p) MSX.setCart(1, ""); MSX.SCCPSlot = 1; return false; end },
					{ Title="ROMを抜く"    ,Enabled=true, Func=function(t,p) MSX.setCart(1, ""); MSX.SCCPSlot =-1; return false; end },
				}
			}
		}
	},
	{ Title=" ディスク    "  ,Enabled=true, 
		SubMenu=
		{	col=13,
			{ Title="ディスクＡ",Enabled=true, Func=DevFileSelect, Param={ dev="DSK",no=0 } },
			{ Title="ディスクＢ",Enabled=true, Func=DevFileSelect, Param={ dev="DSK",no=1 } },
		}
	},
	{ Title=" カセット    "  ,Enabled=true,
		SubMenu=
		{	col=13, -- onInit=function(t) if MSX.CASName=="" then t[2].Enabled=false;t[3].Enabled=false;t[4].Enabled=false;t[5].Enabled=false; end;
			{ Title="CAS選択    ",Enabled=true, Func=DevFileSelect, Param={ dev="CAS" } },
			{ Title="先頭       ",Enabled=true, Func=function() MSX.CasTop();  return false; end },
			{ Title="前         ",Enabled=true, Func=function() MSX.CasNext(); return false; end },
			{ Title="次         ",Enabled=true, Func=function() MSX.CasPrev(); return false; end },
			{ Title="最後       ",Enabled=true, Func=function() MSX.CasEnd();  return false; end },
		}
	},
	{ Title=" JOYポート   " ,Enabled=true, 
		SubMenu=
		{	col=13,
			{ Title="ポート1",Enabled=true,
				SubMenu=
				{ col=20,
					{ Title="未接続"        ,Enabled=true,Func=function(n) MSX.JoyTypeA = 0; return false; end  },
					{ Title="Joystick"      ,Enabled=true,Func=function(n) MSX.JoyTypeA = 1; return false; end  },
					{ Title="Mouse"         ,Enabled=true,Func=function(n) MSX.JoyTypeA = 2; return false; end  },
					{ Title="Mouse(JSMode)" ,Enabled=true,Func=function(n) MSX.JoyTypeA = 3; return false; end  },
				}
			},
			{ Title="ポート2",Enabled=true,
				SubMenu=
				{ col=20,
					{ Title="未接続"        ,Enabled=true,Func=function(n) MSX.JoyTypeA = 0; return false; end  },
					{ Title="Joystick"      ,Enabled=true,Func=function(n) MSX.JoyTypeA = 1; return false; end  },
					{ Title="Mouse"         ,Enabled=true,Func=function(n) MSX.JoyTypeA = 2; return false; end  },
					{ Title="Mouse(JSMode)" ,Enabled=true,Func=function(n) MSX.JoyTypeA = 3; return false; end  },
				}
			},
		}
	},
	{ Title=" ステート    "  ,Enabled=true, 
		SubMenu=
		{	col=13,
			{
				Title="セーブ",Enabled=true,
				SubMenu=
				{ col=10,
					{ Title="セーブ１",Enabled=true, Func=function(n) MSX.saveState("state1.sta"); return true, false; end },
					{ Title="セーブ２",Enabled=true, Func=function(n) MSX.saveState("state2.sta"); return true, false; end },
					{ Title="セーブ３",Enabled=true, Func=function(n) MSX.saveState("state3.sta"); return true, false; end },
				}
			},
			{ 
				Title="ロード",Enabled=true,
				SubMenu=
				{ 
					col=10,
					{ Title="ロード１" ,Enabled=true, Func=function(n) MSX.loadState("state1.sta"); return true, false; end },
					{ Title="ロード２" ,Enabled=true, Func=function(n) MSX.loadState("state2.sta"); return true, false; end },
					{ Title="ロード３" ,Enabled=true, Func=function(n) MSX.loadState("state3.sta"); return true, false; end },
				}
			},
		}
	},
	{ Title=" 環境設定    "  ,Enabled=true,
		SubMenu=
		{col=20,
			{
				Title="フレーム描画",Enabled=true,
				SubMenu=
				{	col=10, 
					{ Title="100%",Enabled=true, Func=function() MSX.UPeriod = 100; return false; end; },
					{ Title=" 90%",Enabled=true, Func=function() MSX.UPeriod =  90; return false; end; },
					{ Title=" 80%",Enabled=true, Func=function() MSX.UPeriod =  80; return false; end; },
					{ Title=" 70%",Enabled=true, Func=function() MSX.UPeriod =  70; return false; end; },
					{ Title=" 60%",Enabled=true, Func=function() MSX.UPeriod =  60; return false; end; },
					{ Title=" 50%",Enabled=true, Func=function() MSX.UPeriod =  50; return false; end; },
				}
			},	
			{
				Title="スクリーン"  ,Enabled=true,
				SubMenu=
				{	col=20,
					{ Title="自動縮小  " ,Enabled=true, Func=function() screen_setpram(0,0,0, 0,1); ScreenFit="Auto Fit"; return false; end; },
					{ Title="212ドット " ,Enabled=true, Func=function() screen_setpram(0,0,0,28,0); ScreenFit="212 dot" ; return false; end; },
					{ Title="192ドット " ,Enabled=true, Func=function() screen_setpram(0,0,0, 0,0); ScreenFit="192 dot" ; return false; end; },
					{ Title="自分で設定" ,Enabled=true, Func=function() Config_screen_user(); ScreenFit="User Setting" return false; end; },
					{ Title="上下切替  " ,Enabled=true, Func=function() ui.swapScreen(); return false; end; },
				};
			},
			{
				Title="タッチパネル"  ,Enabled=true,
				SubMenu=
				{	col=20,
					{ Title="keyboard" ,Enabled=true, Func=function() ui.touchMode(0); TouchPanel="Keyboard"; return false; end; },
					{ Title="mouse   " ,Enabled=true, Func=function() ui.touchMode(1); TouchPanel="MouseEmu"; return false; end; },
				};
			},
			{ Title="サウンド"    ,Enabled=true,
				SubMenu=
				{	col=20,
					{ Title="PSG SOUND",Enabled=true,
						SubMenu=
						{	col=10,
							{ Title="ON " ,Enabled=true,Func=function(n) MSX.PSG=1; return false;  end },
							{ Title="OFF" ,Enabled=true,Func=function(n) MSX.PSG=0; return false;  end },
						}
					},
					{ Title="SCC SOUND",Enabled=true,
						SubMenu=
						{	col=10,
							{ Title="ON " ,Enabled=true,Func=function(n) MSX.PSG=1; return false;  end },
							{ Title="OFF" ,Enabled=true,Func=function(n) MSX.PSG=0; return false;  end },
						}
					},
					{ Title="サンプルレート" ,Enabled=true,
						SubMenu=
						{	col=10,
							{ Title="32kHz" ,Enabled=true,Func=function(n) MSX.SampleRate=0x8000;return false;  end },
							{ Title="24kHz" ,Enabled=true,Func=function(n) MSX.SampleRate=0x6000;return false;  end },
							{ Title="16kHz" ,Enabled=true,Func=function(n) MSX.SampleRate=0x4000;return false;  end },
							{ Title=" 8kHz" ,Enabled=true,Func=function(n) MSX.SampleRate=0x2000;return false;  end },
						};
					},
				};
			},
			{ Title="キーアサイン",Enabled=true, 
				SubMenu=
				{	col=20,
					{ Title="KEYBOARD "  ,Enabled=true, Func=function() _Config_con( 1 ); return false; end; },
					{ Title="JOYSTICK1"  ,Enabled=true, Func=function() _Config_con( 2 ); return false; end; },
					{ Title="JOYSTICK2"  ,Enabled=true, Func=function() _Config_con( 3 ); return false; end; },
				};
			},
		},
	},
--	{ Title=" デバッグ    " ,Enabled=true, Func=function(n) MSX.Trace=1; return false, true; end },
	{ Title=" LOG         " ,Enabled=true, Func=ViewLog },
--	{ Title=" Edit        " ,Enabled=true, Func=execlua },
--	{ Title=" lua実行     " ,Enabled=true, Func=function(n) dofile( "edit.lua" ); return false; end },
--	{ Title=" リセット    " ,Enabled=true, Func=function(n) MSX.exit(); resetflg=1; return false, true; end },
	{ Title=" 終了        " ,Enabled=true, Func=function(n) MSX.exit(); resetflg=-1; return false, true; end },
	{ Title=" 電源OFF     " ,Enabled=true, Func=function(n) MSX.exit(); resetflg=0; return false, true; end },
};

function fn( s )
	if s == "" then res = "[NONE]"; return res; end;
	local res = "";
	for i=-1,-#s,-1 do
		local c = string.sub(s,i,i);
		if c == "/" then break; end
		res = c .. res;
	end;
	return res;
end

function drawInfo()
	local rt = { "GEN 8","GEN16","KONA5","KONA4","ASC 8","ASC16","GMST2","FMPAC","SCC","GUESS" };
	local joy= { "None", "Joystick", "Mouse", "Mouse(Joystick)" };
	local snd= { "OFF", "ON" }
	ui.cls(1);
	ui.box ( 1, 5, 5,245, 15, RGB(1,10,10,31) );
	ui.drawText2( 1, 10, 10, "fmsxDS Ver.0.09" , 0xFFFF , 0 );
	ui.box ( 1, 5,25,245,160, RGB(1,10,10,31) );
	ui.drawText2( 1, 10, 30, "ROM1 :" .. rt[MSX.ROMType[0]+1] .. ": " .. fn(MSX.ROMName[0]) , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 40, "ROM2 :" .. rt[MSX.ROMType[1]+1] .. ": " .. fn(MSX.ROMName[1]) , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 50, "DISK A     : " .. fn(MSX.DSKName0  ) , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 60, "DISK B     : " .. fn(MSX.DSKName1  ) , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 70, "CASSETTE   : " .. fn(MSX.CASName   ) , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 80, "JOYPORT 1  : " .. joy[MSX.JoyTypeA+1] , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 90, "JOYPORT 2  : " .. joy[MSX.JoyTypeB+1] , 0xFFFF , 0 );

	ui.drawText2( 1, 10, 105,"DRAW FRIQ. : " .. MSX.UPeriod .. "%" , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 115,"SCREEN     : " .. ScreenFit   , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 125,"TOUCHPANEL : " .. TouchPanel  , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 135,"KEY ASSIGN : " .. DSKeyAssign , 0xFFFF , 0 );

	ui.drawText2( 1, 10, 150,"PSG        : " .. snd[MSX.PSG+1] , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 160,"SCC        : " .. snd[MSX.SCC+1] , 0xFFFF , 0 );
	ui.drawText2( 1, 10, 170,"SAMPLERATE : " .. MSX.SampleRate , 0xFFFF , 0 );
end

-- flg = 1:power off  0:power on
function MainMenu()
	ui.LOG( "MainMenu" );
	local m1;
	local cnt;
	local ret=nil;

	ui.soundStop();
	ui.cls(3);

	while( ret == nil ) do
		ret = menu2(tblMainMenu, ret, 10,10, 0);
	end

	ui.cls(3);
	ui.drawAllKeyboard();
	ui.soundStart();
	return 1;
end

function InitSettings()

	-- background color
	BGCOLOR = { RGB(1,5,5,15), RGB(1,5,5,15) };
	ui.clsColor( BGCOLOR[1], BGCOLOR[2] );

	-- loadFont
	FONT_H =10;
	FONT_WH=5;
	FONT_WZ=10;
	if( ui.loadFont( RESDIR .. "maru10.bin", FONT_H, FONT_WH, FONT_WZ ) ) then 
		ui.LOG( "Font load OK" );
	else
		ui.LOG("Font load NG" );
	end;

	-- loadIcon
	--ICN = {};
	--ICN[0] = ui.loadIcon( "icon.dat", 2048 );

end

-- setting default MSX Hardware
function MSXHWSettings()
	ui.LOG( "HW Setting..." );

	MSX.RAMPages   = 8;									-- RAM  Pages( 16KB/page )
	MSX.VRAMPages  = 8;									-- VRAM Pages( 16KB/page )
	MSX.UPeriod    = 100; 								-- update video % ( MAX 100% )

	MSX.SampleRate = 0x2000;							-- 8kHz

	MSX.setBIOS(0,0,0,BIOSDIR.."MSX2J.ROM"   ,0x8000);	--MAIN BIOS ROM
	MSX.setBIOS(3,1,0,BIOSDIR.."MSX2JEXT.ROM",0x4000);	--SUB  BIOS ROM(not required MSX1)
	MSX.setBIOS(3,1,2,BIOSDIR.."DISK.ROM"   ,0x4000);	--DISK BIOS ROM
	--MSX.setBIOS(3,3,2,BIOSDIR.."RS232C.ROM" ,0x4000);			--RS232C BIOS(HW not imprement)

	MSX.patchBDOS(true, 3,1,2);							--Patched DISK BIOS

--**** CartMap *****
--        SUB  | 0   1   2   3
--    ---------+--------------
--    SLOT #0  | x,  3,  4,  5
--    SLOT #1  | 0,  x,  x,  x
--    SLOT #2  | 1,  x,  x,  x
--    SLOT #3  | 2,  x,  x,  x
--
-- x:BIOS use  0,1:UserCart    2-5:InternalCart

	--MSX.setCart( 2,BIOSDIR.."MSXDOS2.ROM", "GEN16" );			--MSXDOS2      (not test)
	--MSX.setCart( 3,BIOSDIR.."FMPAC.ROM"  , "FMPAC" );			--FMPAC        (not test)
	--MSX.setCart( 4,BIOSDIR.."PAINTER.ROM", "GUESS" );			--Painter      (not test)
	--MSX.setCart( 5,BIOSDIR.."GMASTER.ROM", "GMST2" );			--Game Master2 (not test)

	MSX.CMOSName = BIOSDIR.."CMOS.ROM";			--Blank: use default.

	MSX.Trace = 0;								--debug flag ( set 1 to calling 'DebugMenu' function.)(not test)

	screen_setpram(0,0,0,0,1);					-- Auto fit screen height.
end

-- Do not change function name --
function LuaFunc(x)
	local f = { MainMenu, func2, func3, func4, func5 };
	f[x]();
end

-----------------
-- Boot fmsxDS --
-----------------
InitSettings();
MSXHWSettings();

exitflg = 1;
resetflg = 0;
while 1 do
	if resetflg==0 then MainMenu(); end;
	if resetflg==-1 then break; end;
	resetflg = 0;
	MSX.boot();
	MSX.exit();
end
