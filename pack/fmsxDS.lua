-- fmsxDS setting and menu script --

function RGB(A,R,G,B) return (A)*0x8000 + (B)*0x0400+ (G)*0x0020 + (R); end;

function menu2(t)
	local oIndex=t[t.Index].Index;
	local flg = true;
	local n=3;
	if( t.bg == nil )then t.bg = 2; end;
	local function _drw(f)
		oIndex=t[t.Index].Index;
		ui.waitForVBlank(1);
		-- draw group
		if(f==1 or f==3) then
			ui.fill( t.bg, 1,14,253,176, 0x8000 );
			ui.box ( t.bg, 0,13,255,178, 0xFFFF );
			for i=1,#t,1 do
				ui.box( t.bg, (i-1)*55,  0, 56, 14, 0xFFFF );
				if( i == t.Index )then 
					col = 0xFFFF;
					ui.line(t.bg, (t.Index-1)*55 ,13, 54,0 , 0x8000 );  
					local mcnt = #(t[t.Index].Menu);
					for j=1,mcnt,1 do
						ui.drawText2(t.bg, 10, 10+20*j, t[t.Index].Menu[j].MenuName , 0xFFFF, 0x8000 );
						
					end;
				else 
					col = RGB(1,8,8,8);
				end;
				ui.drawText2(t.bg, 3+(i-1)*55, 2, t[i].GroupName , col, 0x8000 );
			end
		end
		-- draw arrow
		if(f==2 or f==3) then
			if( oIndex ~= -1 ) then
				ui.drawText2(t.bg, 10, 10+20*(oIndex          ), t[t.Index].Menu[oIndex          ].MenuName, 0xFFFF, 0x8000 );
				ui.drawText2(t.bg, 10, 10+20*(t[t.Index].Index), t[t.Index].Menu[t[t.Index].Index].MenuName, 0xFFFF, 0xF000 );
			end
		end
	end
	-- menu main routine --
	while( flg ) do
		_drw(n); n=0; oIndex=t[t.Index].Index;
		ui.scanKeys();
		local keys = ui.keysDown();
--		if keys.A     then flg, ret = t[s+1][2](s,t); n=3; end
		if keys.B     then flg=false; ret=nil; end
		if keys.RIGHT then t.Index=t.Index+1; if(t.Index>#t) then t.Index=1 ; end; n=3; ui.LOG( #t ); end
		if keys.LEFT  then t.Index=t.Index-1; if(t.Index<1 ) then t.Index=#t; end; n=3; end
		if keys.DOWN  then 
			t[t.Index].Index=t[t.Index].Index+1;
			if(t[t.Index].Index>#(t[t.Index].Menu) ) then 
				t[t.Index].Index = #(t[t.Index].Menu); 
			end;
		end;
		if keys.UP    then
			t[t.Index].Index=t[t.Index].Index-1;
			if(t[t.Index].Index<1 ) then 
				t[t.Index].Index = 1; 
			end;
		end;
		if t[t.Index].Index ~= oIndex then n=2;       end
	end
	return ret;
end


function Config_screen_autofit()
	MSX.setParam("scr_x"  ,0);
	MSX.setParam("scr_y"  ,0);
	MSX.setParam("scale_x" ,0);
	MSX.setParam("scale_y" ,0);
	MSX.setParam("AutoFit",1);
	return false, true;
end;

function Config_screen_212()
	MSX.setParam("scr_x"   ,0);
	MSX.setParam("scr_y"   ,0);
	MSX.setParam("scale_x" ,0);
	MSX.setParam("scale_y" ,28);
	MSX.setParam("AutoFit",0);
	return false, true;
end;

function Config_screen_192()
	MSX.setParam("scr_x"   ,0);
	MSX.setParam("scr_y"   ,0);
	MSX.setParam("scale_x" ,0);
	MSX.setParam("scale_y" ,0);
	MSX.setParam("AutoFit",0);
	return false, true;
end;

function Config_screen_user()

	local function _setsc( sx,sy,sw,sh )
		MSX.setParam("scr_x" ,sx);
		MSX.setParam("scr_y" ,sy);
		MSX.setParam("scale_x" ,sw);
		MSX.setParam("scale_y" ,sh);
		buf = "x:"..sx.."   y:"..sy.."  w:"..sw.."  h:"..sh.."       ";
		ui.drawText2( 1, 40, 170, buf, 0xFFFF, RGB(1,0,0,31) );
	end;
	local flg = true;
	local chg = false;
	local x=0;
	local y=0;
	local w=0;
	local h=0;

	MSX.setParam("AutoFit",0);
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

function Config_screen()
	local ret = nil;
	local tbl = { x=30,y=30,col=20,row=5,
					{"戻る"       ,function(n) return false, true;  end },
					{"自動縮小"   ,Config_screen_autofit },
					{"212ドット"  ,Config_screen_212 },
					{"192ドット"  ,Config_screen_192 },
					{"自分で設定" ,Config_screen_user }
				};
	while( ret == nil ) do
		ret = menu2(tbl);
	end;
	return ret;
end


function Config_sound()

	function _psgSet(n,t)
		psg = not psg;
		local buf;
		if( psg ) then buf = "ON"; else buf = "OFF"; end
		t[2][1] = "PSG    "..buf;
		return true, false;
	end;
	function _sccSet(n,t)
		scc = not scc;
		local buf;
		if( scc ) then buf = "ON"; else buf = "OFF"; end
		t[3][1] = "SCC    "..buf;
		return true, false;
	end;
	function _rateSet(n,t)
		local ret = nil;
		local buf = nil;
		local tbl = { x=30,y=30,col=20,row=5,
					{"戻る " ,function(n) return false, true;  end },
					{"32kHz" ,function(n) rate=32768;buf="32kHz";return false, true;  end },
					{"24kHz" ,function(n) rate=24576;buf="24kHz";return false, true;  end },
					{"16kHz" ,function(n) rate=16384;buf="16kHz";return false, true;  end },
					{" 8kHz" ,function(n) rate= 8192;buf=" 8kHz";return false, true;  end },
				};
		while( ret == nil ) do
			ret = menu2(tbl);
		end;
		if( buf ~= nil ) then
			t[4][1] = "レート "..buf;
		end;
		return true, false;
	end;

	local psg = MSX.getParam("PSG");
	local scc = MSX.getParam("SCC");
	local rate= MSX.getParam("smplrate");
	local ret = nil;
	local tbl = { x=30,y=30,col=20,row=4,
					{"戻る" ,function(n) return false, true;  end },
					{"PSG    ON"  ,_psgSet},
					{"SCC    ON"  ,_sccSet},
					{"レート 32k" ,_rateSet},
				};
	while( ret == nil ) do
		ret = menu2(tbl);
	end;
	return ret;
end

function Config_KEY()
	ui.Joybind( 0,0,8,0x01); --Key_A 
	ui.Joybind( 1,0,4,0x08); --Key_B 
	ui.Joybind( 2,0,6,0x20); --Select
	ui.Joybind( 3,2,1,0x00); --Start 
	ui.Joybind( 4,0,8,0x80); --Right
	ui.Joybind( 5,0,8,0x10); --Left
	ui.Joybind( 6,0,8,0x20); --UP
	ui.Joybind( 7,0,8,0x40); --Down
	ui.Joybind( 8,0,0,0x00); --KEY_R 
	ui.Joybind( 9,0,0,0x00); --KEY_L 
	ui.Joybind(10,0,7,0x80); --KEY_X 
	ui.Joybind(11,0,0,0x00); --KEY_Y 
	ui.LOG( "Controler is Keyboard.");
	return true, true;
end

function Config_JOY1()
	ui.Joybind( 0,1,0,0x10); --Key_A = trg1
	ui.Joybind( 1,1,0,0x20); --Key_B = trg2
	ui.Joybind( 2,0,6,0x20); --Select= F1
	ui.Joybind( 3,2,1,0x00); --Start = Menu
	ui.Joybind( 4,1,0,0x08); --Right
	ui.Joybind( 5,1,0,0x04); --Left
	ui.Joybind( 6,1,0,0x01); --UP
	ui.Joybind( 7,1,0,0x02); --Down
	ui.Joybind( 8,1,0,0x00); --KEY_R = none
	ui.Joybind( 9,1,0,0x00); --KEY_L = none
	ui.Joybind(10,1,0,0x00); --KEY_X = none
	ui.Joybind(11,1,0,0x00); --KEY_Y = none
	MSX.setParam( "JoyTypeA"  , 1 );
	ui.LOG( "Controler is Joystick 1.");
	return true, true;
end
function Config_JOY2()
	ui.Joybind( 0,1,1,0x10); --Key_A = trg1
	ui.Joybind( 1,1,1,0x20); --Key_B = trg2
	ui.Joybind( 2,0,6,0x20); --Select= F1
	ui.Joybind( 3,2,1,0x00); --Start = Menu
	ui.Joybind( 4,1,1,0x08); --Right
	ui.Joybind( 5,1,1,0x04); --Left
	ui.Joybind( 6,1,1,0x01); --UP
	ui.Joybind( 7,1,1,0x02); --Down
	ui.Joybind( 8,1,1,0x00); --KEY_R = none
	ui.Joybind( 9,1,1,0x00); --KEY_L = none
	ui.Joybind(10,1,1,0x00); --KEY_X = none
	ui.Joybind(11,1,1,0x00); --KEY_Y = none
	MSX.setParam( "JoyTypeB"  , 1 );
	ui.LOG( "Controler is Joystick 2.");
	return true, true;
end

function Config_MOU1()
	MSX.setParam( "JoyTypeA"  , 2 );
	ui.LOG( "Joystick 1 connected MOUSE.");
	return true, true;
end

function Config_MOU2()
	MSX.setParam( "JoyTypeB"  , 2 );
	ui.LOG( "Joystick 2 connected MOUSE.");
	return true, true;
end

function Config_control()
	local ret = nil;
	local tbl = { x=30,y=30,col=20,row=6,
					{"戻る"       ,function(n) return false, true;  end },
					{"KEYBOARD"   ,Config_KEY  },
					{"JOYSTICK1"  ,Config_JOY1 },
					{"JOYSTICK2"  ,Config_JOY2 },
					{"MOUSE1"     ,Config_MOU1 },
					{"MOUSE2"     ,Config_MOU2 },
				};
	while( ret == nil ) do
		ret = menu2(tbl);
	end;
	return ret;
end

function Config_skip()

	local function _setsc( sx,sy,sw,sh )
		MSX.setParam("scr_x" ,sx);
		MSX.setParam("scr_y" ,sy);
		MSX.setParam("scale_x" ,sw);
		MSX.setParam("scale_y" ,sh);
		buf = "x:"..sx.."   y:"..sy.."  w:"..sw.."  h:"..sh.."       ";
		ui.drawText2( 2, 40, 170, buf, 0xFFFF, RGB(1,0,0,31) );
	end;
	local flg = true;
	local chg = false;
	local x=0;
	local y=0;
	local w=0;
	local h=0;

	MSX.setParam("AutoFit",0);
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
end

function Config()
	local ret = nil;
	local tbl = { x=20,y=20,col=20,row=5, 
				};
	while( ret == nil ) do
		ret = menu2(tbl);
	end;
	return ret;
end

function edit()
	dofile( "edit.lua" );
end

function execlua()
	local r,fn = ui.fileselect( "/","lua" );
	if r==1 then dofile( fn ); end;
end

function MainMenu_RomSelect(n)
	local r,fn = ui.fileselect( ROMPath,"ROM" );
	if r==1 then 
		if MSX.RomChange(n-1,fn) == 0 then
			ui.LOG( "RomChange Error!" );
		end;
	end;
	return true, false;
end

function MainMenu_DskSelect(n)
	local r,fn = ui.fileselect( DSKPath,"DSK" );
	if r==1 then 
		if MSX.DskChange(n-4,fn) == 0 then
			ui.LOG( "DskChange Error!" );
		end;
	end;
	return true, false;
end;

function MainMenu_CasSelect(n)
	local r,fn = ui.fileselect( CASPath,"CAS" );
	if r==1 then MSX.CasChange(fn); end;
	return true, false;
end;

function MainMenu_State()
	local ret = nil;
	local tbl = { x=20,y=20,col=20,row=7, 
					{"戻る"        , function(n) return false, true;  end },
					{"セーブ１"    , function(n) MSX.saveState("state1.sta"); return true, false; end },
					{"セーブ２"    , function(n) MSX.saveState("state2.sta"); return true, false; end },
					{"セーブ３"    , function(n) MSX.saveState("state3.sta"); return true, false; end },
					{"ロード１"    , function(n) MSX.loadState("state1.sta"); return true, false; end },
					{"ロード２"    , function(n) MSX.loadState("state2.sta"); return true, false; end },
					{"ロード３"    , function(n) MSX.loadState("state3.sta"); return true, false; end },
				};
	while( ret == nil ) do
		ret = menu2(tbl);
	end;
	return ret;
end

function MainMenu_ExtRom()
	local ret = nil;
	local tbl = { x=20,y=20,col=20,row=3,
					{"戻る"        , function(n) return false, true;  end },
					{"スロット１"  , function(n) MSX.loadSCC(0); return true, false; end },
					{"スロット２"  , function(n) MSX.loadSCC(1); return true, false; end }
				};
	while( ret == nil ) do
		ret = menu2(tbl);
	end;
	return ret;
end

-- flg = 1:before boot  0:after boot
function MainMenu(flg)
	ui.LOG( "MainMenu" );
	local m1;
	local cnt;
	local ret=nil;

	ui.soundStop();
	-- Draw Infomation --
	ui.cls(3);
	ui.drawText2( 1, 129, 169, "fmsxDS Ver.0.08 ", 0xBDEF, 0x8000 );
	ui.drawText2( 1, 139, 179, "Ported by NYAGOSU", 0xBDEF, 0x8000 );
	ui.drawText2( 1, 130, 170, "fmsxDS Ver.0.08 ", 0xFFFF, 0x0000 );
	ui.drawText2( 1, 140, 180, "Ported by NYAGOSU", 0xFFFF, 0x0000 );

	-- Draw Menu --
	local main_menu = { 
		Index = 1,
		{	GroupName="デバイス",
			Index=1,
			Menu={
				{ MenuName="ロム１    ", MainMenu_RomSelect },
				{ MenuName="ロム２    ", MainMenu_RomSelect },
				{ MenuName="ディスクＡ", MainMenu_DskSelect },
				{ MenuName="ディスクＢ", MainMenu_DskSelect },
				{ MenuName="カセット  ", MainMenu_CasSelect },
			}
		},
		{	GroupName="環境設定", 
			Index=1,
			Menu={
				{ MenuName="フレームスキップ", Config_skip    },
				{ MenuName="スクリーン"      , Config_screen  },
				{ MenuName="コントローラ"    , Config_control },
				{ MenuName="サウンド"        , Config_sound   },
			}
		},
		{	GroupName=" その他 ",
			Index=1,
			Menu={
				{ MenuName=" ステート   ", MainMenu_State },
			}
		},
		{	GroupName="デバッグ", 
			Index=-1,
			MenuFunc = DebugMenu 
		}
	};

--[[
					{ " エディタ   ", edit },
--					{ " lua実行    ", execlua },
					{ " リセット   ", function(n) MSX.exit(); resetflg=1; return false, true; end },
					{ " 電源断     ", function(n) MSX.exit(); resetflg=0; return false, true; end }
				};
	if flg == 1 then 
		tbl[1][1] =   " 実行       ";
		tbl.row = 11;
	else
		tbl[1][1] =   " 戻る       ";
		tbl.row = 13;
	end
]]--
	while( ret == nil ) do
		ret = menu2(main_menu, ret);
	end

	ui.cls(3);
	ui.drawAllKeyboard();
	ui.soundStart();
	return 1;
end

function KeySettings()
end

function dspsts()
	r = {};
	r = MSX.getReg();
	a = 2;
	-- REG --
	ui.box ( 2, 170,0,86,160, RGB(1,5,5,31) );
	ui.drawText( 2, 172, a,string.format( "AF:%.4X (%.4X)",r.AF,r.AF1 ), 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "BC:%.4X (%.4X)",r.BC,r.BC1 ), 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "DE:%.4X (%.4X)",r.DE,r.DE1 ), 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "HL:%.4X (%.4X)",r.HL,r.HL1 ), 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "IX:%.4X IY:%.4X",r.IX,r.IY), 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "PC:%.4X SP:%.4X",r.PC,r.SP), 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "I :%.2X R:%.2X",r.I,r.R )  , 0xFFFF, 0x8000 ); a = a + 7;
	ui.drawText( 2, 172, a,string.format( "IFF:%.4X",r.IFF )          , 0xFFFF, 0x8000 ); a = a + 7;
	-- VDP --
	for i=0,7,1 do
		s = string.format("%2d:%.2X %2d:%.2X %2d:%.2X",i,MSX.getVDP(i),i+8,MSX.getVDP(i+8),i+16,MSX.getVDP(i+16));
		ui.drawText(2,172,i*7+70,s, 0xFFFF, 0x8000 );
	end
	-- DASM --
	ui.box ( 2, 0,0,165,160, RGB(1,5,5,31) );
	adr = r.PC;
	for i=0,16,1 do
		ret, buf = MSX.dasm(adr);
		s = string.format("%.4X %s",adr,buf);
		ui.drawText(2,2,i*7,s, 0xFFFF, 0x8000 );
		adr = adr + ret;
	end
end

function DebugMenu()
	local ret = nil;
	ui.soundStop();
	ui.cls(2);
	dspsts();

	local tbl = 
		{ x=10,y=100,col=6, row=3, 
			{ "trace"     , function(n) return false, 1; end },
			{ "break"     , function(n) return false, 2; end },
			{ "go   "     , function(n) MSX.setParam( "Trace", 0 ); return false, 2; end }
		};
	while( ret == nil ) do
		ret = menu2(tbl);
	end

	if( ret ~= 1 ) then
		ui.cls(2);
		ui.drawAllKeyboard();
	end
	ui.soundStart();
	return 1;
end

-- loadIcon
--ICN = {};
--ICN[0] = ui.loadIcon( "icon.dat", 2048 );

-- loadFont
FONT_H =10;
FONT_WH=6;
FONT_WZ=12;
if( ui.loadFont( "k12x10.bin", FONT_H, FONT_WH, FONT_WZ ) ) then 
	ui.LOG( "Font load OK" );
else
	ui.LOG("Font load NG" );
end;

-- setting path
BIOSPath="fat:/";
ROMPath ="fat:/";
DSKPath ="fat:/";
CASPath ="fat:/";

-- setting MSX Hardware
MSX.setParam( "Version"   , 1 ); -- 0:MSX1 1:MSX2 2:MSX2+
MSX.setParam( "RAMPages"  , 8 );
MSX.setParam( "VRAMPages" , 8 );
MSX.setParam( "UPeriod"   ,100); -- update video %

MSX.setBIOS(0,0,0,BIOSPath.."MSX2.ROM"   ,0x8000);
MSX.setBIOS(3,1,0,BIOSPath.."MSX2EXT.ROM",0x4000);
MSX.setBIOS(3,1,2,BIOSPath.."DISK.ROM"   ,0x4000);
MSX.patchBDOS(3,1,2);
MSX.patchBIOS();

--MSX.setBIOS(3,3,2,"RS232C.ROM" ,0x4000);

--MSX.loadCart (0,"SALA.ROM" );
--MSX.loadCart (1,"GRA2.ROM" );
--MSX.loadFMPAC(1,"FMPAC.ROM");
--MSX.loadMSXDOS2(1,"MSXDOS2.ROM");

--MSX.ChangeDisk(0,"YS1.DSK" );
--MSX.ChangeDisk(1,"USR.DSK" );
--MSX.ChangeCas( "DEZENI.CAS" );

Config_screen_autofit();

-- debug flag
MSX.setParam( "Trace", 0 );

	MSX.RAMSize = 10;
	ui.LOG( MSX.RAMSize );

-- start fmsxDS
exitflg = 1;
resetflg = 0;
while 1 do
	if resetflg==0 then MainMenu(1); end;
	resetflg = 0;
	MSX.boot();
	MSX.exit();
end
