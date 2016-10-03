require "editbuf"
require "keyboard"
local ebuf = editbuf:new();
local kb   = keyboard:new();

local i;
local KEY_MAX=78;
local flg = 1;
local chg = 1;
local filename = "";
local rbuf;
local spkey;
local FONT_W=6;
local FONT_H=10;
ebuf.SCRWIDTH  = 42;
ebuf.SCRHEIGHT = 18;

local function drawScreen(chg)
	ui.waitForVBlank(1);
	local s=1;
	local e=ebuf.SCRHEIGHT;
--	if( chg == 2 ) then
--		s = self.cursor.y - self.scrpos.y -1;
--		e = self.cursor.y - self.scrpos.y +1;
--	end
	for i=s,e,1 do
		local b = ebuf:getscreenbuf(i);
		ui.drawText2( 1, 0,(i-1)*FONT_H, b, 0xFFFF, RGB(1,0,10,10) );
	end
	local sx, sy = ebuf:getscreenpos();
	ui.line( 1, (sx-1)*FONT_W,(sy-1)*FONT_H,0,FONT_H, RGB(1,31,31,5) );
	ui.drawText2(1,0,182,string.sub( "x:"..sx.." y:"..sy.." | "..filename..ebuf.SCRWIDTH,1,ebuf.SCRWIDTH),0xFFFF,RGB(1,0,0,31) );
end

local function inputFilename()
	local fn = "";
	local flg=true;
	local cx = 0;
	local chg = 1;
	local touchnum = -1;
	local touchnum_o= -1;
	local shift=1;
	local rbuf = "";
	local spkey = "";
	drawAllKeyboard();
	ui.fill( 1, 5,100, 250, 30, 0x8000 );
	ui.box(  1, 5,100, 250, 30, RGB(1,31,31,31) );
	ui.drawText( 1, 10,98," input file name ", 0xFFFF,0x8000 );

	while( flg ) do 
		ui.scanKeys();
		local keys = ui.keysDown();
		if keys.RIGHT then 
			if( cx < string.len(fn) ) then cx = cx + 1; chg = 1; end
		end
		if keys.LEFT then
			if( cx > 0 ) then cx = cx - 1; chg = 1; end
		end
		if keys.A then ret=true; flg=false; end
		if keys.B then ret=false; fn=""; flg=false; end

		rbuf, spkey = kb:softKeyInput();
		if( spkey == "RET" ) then ret=true; flg=false; k="" end
		if( spkey == "DEL" ) then
			fn = string.sub(fn,1,cx)..string.sub(fn,cx+2);
			chg = 1;
			k = ""
		end
		if( spkey == "BS" ) then
			if( cx > 0 ) then
				fn = string.sub(fn,1,cx-1)..string.sub(fn,cx+1);
				chg = 1;
			end
			k = ""
		end;
		k = string.gsub(k,"[\"\|\\:\*\/\<\>]","");
		if( k ~= "" ) then
			fn = string.sub(fn,1,cx)..k..string.sub(fn,cx+1);
			cx = cx + 1;
			chg = 1;
		end
		ui.waitForVBlank(1);
		if( chg==1 ) then 
			ui.drawText( 1, 8,110,string.sub(fn..string.rep(" ",48),1,48), 0xFFFF,0x8000 ); 
			ui.line( 1, 8+cx*5, 110, 0, 6, 0xFFFF );
			chg = 0;
		end
	end
	return ret, fn;
end

local function dzmenu()
	-- New
	local function onNew(n)
		ebuf:clear();
		filename="";
		return false, true;	-- menu end, no exit
	end
	-- Open
	local function onOpen(n)
		local r,fn = ui.fileselect( "fat:/","lua" );
		if( r==1 ) then
			ebuf:load( fn );
			filename = fn;
		end
		return false, true;	-- menu end, no exit
	end
	-- SaveAs
	local function onSaveAs(n)
		local ret,fn = inputFilename();
		if( ret ) then 
			filename = fn;
			ebuf:save( filename );
			ui.LOG( filename.." saved." );
		end
		return true, true;	-- menu loop
	end
	-- Save
	local function onSave(n)
		if( filename == "" ) then
			onSaveAs();
		else
			ebuf:save( filename );
			ui.LOG( filename.." saved." );
		end
		return true, true;	-- menu loop
	end
	local tbl = {bg=2,x=10,y=10,col=10,row=5, 
					{"Close"   , function(n) return false, true;  end },
					{"New"     , onNew     },
					{"Open"    , onOpen    },
					{"Save"    , onSave    },
					{"Save as" , onSaveAs  },
					{"Exit"    , function(n) return false, false; end }
				};
	ret = menu2(tbl);
	ui.cls(3);
	kb:drawKeyboardScreen();
	return ret;
end

-- main --
ui.cls(3);
kb:drawKeyboardScreen();
ui.keysSetRepeat(20,6);
while( flg ) do 
	ui.scanKeys();
	local keys = ui.keysDownRepeat();
	if keys.RIGHT then chg = ebuf:right(); end
	if keys.LEFT  then chg = ebuf:left() ; end
	if keys.UP    then chg = ebuf:up()   ; end
	if keys.DOWN  then chg = ebuf:down() ; end
	-- START button --
	if keys.START then 
		flg = dzmenu();
		chg = 1;
	end
	-- touchpanel --
	rbuf, spkey = kb:softKeyInput();
	if( spkey == "ESC" ) then k="" end;
	if( spkey == "RET" ) then chg=1; ebuf:separateLine(); ebuf:right(); end;
	if( spkey == "DEL" ) then chg=1; ebuf:delete(); end;
	if( spkey == "BS"  ) then chg=1; ebuf:left(); ebuf:delete(); end;
	if( rbuf  ~= ""    ) then chg=1; ebuf:input( rbuf ); end
	if( chg > 0 ) then
		drawScreen(chg);
		chg = 0;
	else
		ui.waitForVBlank(1);
	end
end
