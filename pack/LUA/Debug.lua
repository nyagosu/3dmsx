-- fmsxDS Debug Menu Script --
function dspsts()
	r = {};
	r = DEBUG.getReg();
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
		s = string.format("%2d:%.2X %2d:%.2X %2d:%.2X",i,DEBUG.getVDP(i),i+8,DEBUG.getVDP(i+8),i+16,DEBUG.getVDP(i+16));
		ui.drawText(2,172,i*7+70,s, 0xFFFF, 0x8000 );
	end
	-- DASM --
	ui.box ( 2, 0,0,165,160, RGB(1,5,5,31) );
	adr = r.PC;
	for i=0,16,1 do
		ret, buf = DEBUG.dasm(adr);
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
