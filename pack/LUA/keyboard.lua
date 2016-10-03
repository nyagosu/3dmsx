module( "keyboard", package.seeall )

function new()
	keyb = { 
		kmode  = false;
		KEY_MAX=78;
		shift = 1;

		kb = {
		  {"��","","","","","","��","�J","","","","DEL","BS",
		   "1","2","3","4","5","6","7","8","9","0","-","^","\\",
		   "q","w","e","r","t","y","u","i","o","p","@","[","RET",
		   "a","s","d","f","g","h","j","k","l",";",":","]","RET",
		   "z","x","c","v","b","n","m",",",".","/","","","",
		   "SFT","","","","","SPC","SPC","��","","","","","" },

		  {"��","","","","","","��","�J","","","","DEL","BS",
		   "!","\"","#","$","%","&","\'","(",")","","=","~","|",
		   "Q","W","E","R","T","Y","U","I","O","P","`","{","RET",
		   "A","S","D","F","G","H","J","K","L","+","*","}","RET",
		   "Z","X","C","V","B","N","M","<",">","?","_","","",
		   "SFT","","","","","SPC","SPC","��","","","","","" }
		};

		romadic = {
		-- 1
		{
			{"A","��"},{"I","��"},{"U","��"},{"E","��"},{"O","��"},
			{"!", "�I"},{"\"", "�h"},{"#", "��"},{"$", "��"},{"%", "��"},{"&", "��"},
			{"'", "�f"},{"(", "�i"},{")", "�j"},{"-", "�["},{"^", "�O"},{"-","�["},
			{"=", "��"},{"~", "�`"},{"|", "�b"},{"@", "��"},{"[", "�u"},{"`", "�M"},
			{"{", "�o"},{";", "�G"},{":", "�F"},{"]", "�v"},{"+", "�{"},{"*", "��"},
			{"}", "�p"},{",", "�A"},{".", "�B"},{"\\", "��"},{"<", "��"},{">", "��"},
			{"?", "�H"},{"_", "�Q"},{"/", "�E"},
		},
		-- 2
		{
			{"KA", "��"},{"KI", "��"},{"KU", "��"},{"KE", "��"},{"KO", "��"},
			{"GA", "��"},{"GI", "��"},{"GU", "��"},{"GE", "��"},{"GO", "��"},
			{"SA", "��"},{"SI"  ,"��"},{"SU"  ,"��"},{"SE"  ,"��"},{"SO"  ,"��"},
			{"ZA", "��"},{"ZI"  ,"��"},{"JI"  ,"��"},{"ZU"  ,"��"},{"ZE"  ,"��"},{"ZO"  ,"��"},
			{"TA", "��"},{"TI", "��"},{"TU", "��"},{"TE", "��"},{"TO", "��"},
			{"DA", "��"},{"DI", "��"},{"DU", "��"},{"DE", "��"},{"DO", "��"},
			{"NA", "��"},{"NI", "��"},{"NU", "��"},{"NE", "��"},{"NO", "��"},
			{"HA", "��"},{"HI", "��"},{"HU", "��"},{"FU", "��"},{"HE", "��"},{"HO", "��"},
			{"BA", "��"},{"BI", "��"},{"BU", "��"},{"BE", "��"},{"BO", "��"},
			{"FA", "�ӂ�"},{"FI", "�ӂ�"},{"FYU","�ӂ�"},{"FE", "�ӂ�"},{"FO", "�ӂ�"},
			{"PA", "��"},{"PI", "��"},{"PU", "��"},{"PE", "��"},{"PO", "��"},
			{"MA", "��"},{"MI", "��"},{"MU", "��"},{"ME", "��"},{"MO", "��"},
			{"YA", "��"},{"YU", "��"},{"YO", "��"},
			{"RA", "��"},{"RI", "��"},{"RU", "��"},{"RE", "��"},{"RO", "��"},
			{"WA","��"},{"WI","��"},{"WE","��"},{"WO","��"},{"NN","��"},{"XN","��"},
			{"JA"  ,"����"},{"JU"  ,"����"},{"JE"  ,"����"},{"JO " ,"����"},
			{"LA","��"},{"LI","��"},{"LU","��"},{"LE","��"},{"LO","��"},
			{"XA","��"},{"XI","��"},{"XU","��"},{"XE","��"},{"XO","��"},
			{"WI","����"},{"WE","����"},
			{"VA","���J��"},{"VI","���J��"},{"VU","���J"},{"VE","���J��"},{"VO","���J��"},
		},
		-- 3
		{
			{"SHI" ,"��"},{"CHI","��"},{"TSU","��"},{"LTU","��"},
			{"KYA","����"},{"KYU","����"},{"KYO","����"},
			{"SYA" ,"����"},{"SYU" ,"����"},{"SYE" ,"����"},{"SYO" ,"����"},
			{"SHA" ,"����"},{"SHU" ,"����"},{"SHE" ,"����"},{"SHO" ,"����"},
			{"ZYA" ,"����"},{"ZYU" ,"����"},{"ZYE" ,"����"},{"ZYO" ,"����"},
			{"JYA" ,"����"},{"JYU" ,"����"},{"JYE" ,"����"},{"JYO" ,"����"},
			{"GYA","����"},{"GYU","����"},{"GYO","����"},
			{"GWA","����"},{"GWI","����"},{"GWE","����"},{"GWO","����"},
			{"TYA","����"},{"TYU","����"},{"TYE","����"},{"TYO","����"},
			{"CYA","����"},{"CYU","����"},{"CYE","����"},{"CYO","����"},
			{"CHA","����"},{"CHU","����"},{"CHE","����"},{"CHO","����"},
			{"DYA","����"},{"DYU","����"},{"DYE","����"},{"DYO","����"},
			{"THA","�Ă�"},{"THI","�Ă�"},{"THU","�Ă�"},{"THE","�Ă�"},{"THO","�Ă�"},
			{"DHA","�ł�"},{"DHI","�ł�"},{"DHU","�ł�"},{"DHE","�ł�"},{"DHO","�ł�"},
			{"NYA","�ɂ�"},{"NYU","�ɂ�"},{"NYE","�ɂ�"},{"NYO","�ɂ�"},
			{"HYA","�Ђ�"},{"HYU","�Ђ�"},{"HYE","�Ђ�"},{"HYO","�Ђ�"},
			{"BYA","�т�"},{"BYU","�т�"},{"BYE","�т�"},{"BYO","�т�"},
			{"PYA","�҂�"},{"PYU","�҂�"},{"PYE","�҂�"},{"PYO","�҂�"},
			{"MYA","�݂�"},{"MYU","�݂�"},{"MYE","�݂�"},{"MYO","�݂�"},
			{"LYA","��"},{"XYA","��"},{"LYU","��"},{"XYU","��"},{"LYO","��"},{"XYO","��"},
			{"RYA","���"},{"RYU","���"},{"RYE","�肥"},{"RYO","���"},
		},
		-- 4
		{
			{"LTSU","��"}
		}
		};
	}
	function keyb:roma( rk )
		local c;
		local r={};
		for c,r in ipairs(self.romadic[#rk]) do 
			if( r[1] == rk ) then
				return r[2], "";
			end
		end
		return "", rk;
	end

	function keyb:dkey(num,d)
		local y = math.floor(num /13);
		local x = num % 13;
		local col;
		if( d==1 ) then col = RGB(1,31,15,15); else col = RGB(1,0,0,0) end;
		ui.fill( 2, x*19, y*19+15, 17, 17, col ); 
		ui.box ( 2, x*19, y*19+15, 17, 17, 0xFFFF );
		ui.drawText2( 2, x*19+3,y*19+18, keyb.kb[self.shift][num+1], 0xFFFF,0 );
	end

	function keyb:drawKeyboardScreen()
		ui.box ( 2, 1, 1, 254, 13, 0xFFFF );
		for i=0,13*6-1,1 do keyb:dkey( i, 0 ); end;
	end

	function keyb:softKeyInput()
		local flg    = true;
		local touch  = false;
		local kinp   = false;
		local touchnum   = -1;
		local touchnum_o = -1;
		local buf = "";
		local rkbuf = "";
		local spkey = "";
		while( flg ) do
			flg = false;
			ui.scanKeys();
			local keys = ui.keysDownRepeat();
			if( keys.TOUCH ) then
				touch = true;
				local t = ui.touchReadXY();
				touchnum = math.floor((t.py-15)/19)*13 + math.floor(t.px/19)
				if( touchnum >= self.KEY_MAX ) then touchnum = -1 end
				if( touchnum_o > -1 ) then keyb:dkey(touchnum_o, 0 ) end
			if( touchnum   > -1 ) then keyb:dkey(touchnum  , 1 ) end
				touchnum_o = touchnum;
			else
				touch = false;
				local k;
				if( touchnum > -1 ) then
					keyb:dkey(touchnum , 0 );
					k = self.kb[self.shift][touchnum+1];
					if( k == "��"  ) then 
						if( self.kmode ) then
							self.kmode =false
						else
							self.kmode=true
						end;
						k = "";
					end;
					if( k == "TAB" ) then
						spkey = k;
						k = "";
					 end;
					if( k == "SPC" ) then 
						spkey = k;
						if( self.kmode ) then k = "�@" else k = " " end;
					end
					if( k == "SFT" ) then 
						if(self.shift==1) then self.shift=2; else self.shift=1; end;
						keyb:drawKeyboardScreen();
						k = "";
					end;
					if( k == "RET" ) then
						if( kinp ) then kinp = false end;
						spkey = k;
						k = "";
					end;
					if( k == "DEL" ) then
						spkey = k;
						k = "";
					end;
					if( k == "BS" ) then
						if( #rkbuf > 0 ) then 
							ui.LOG( "BS" );
							rkbuf = string.sub( rkbuf, 1, #rkbuf-1 );
						else
							buf = string.sub( buf, 1, #buf-2 ); -- 2�o�C�g���߂���
						end
						spkey = k;
						k = "";
					end;
					if( k == "��" ) then
--						if( kinp ) then 
							--"�ϊ��ꗗ�\��"
--						end;
						spkey = k;
						k = "";
					end;
					if( k ~= "" ) then 
						if( self.kmode ) then
							kinp = true; -- ���͒� ���ȕϊ�
							rkbuf = rkbuf .. k;
							b, rkbuf = keyb:roma( string.upper(rkbuf) );
							ui.LOG( "roma buf["..b.."] rkbuf["..rkbuf.."]" );
							buf = buf .. b;
						else
							buf = k;
						end
					end;
					touchnum = -1;
				end;
			end;
			ui.waitForVBlank(1);
			if( kinp  ) then flg = true end;
			if( touch ) then flg = true end;
			if( flg ) then 
				ui.fill ( 2, 2, 2, 253, 12, 0x8000 );
				ui.drawText2( 2, 3,3, buf..rkbuf, 0xFFFF,RGB(1,15,15,31) );
			end;
		end;
		return buf..rkbuf, spkey;
	end
--[[
	drawKeyboardScreen();
	while( 1 ) do
		local buf = softKeyInput();
		if( buf ~= nil and buf ~= "" ) then ui.drawText2( 1,1,1, buf, 0xFFFF,0x8000 ); end;
		ui.waitForVBlank(1);
	end
]]--
	return keyb;
end
