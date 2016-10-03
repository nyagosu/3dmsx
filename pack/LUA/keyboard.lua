module( "keyboard", package.seeall )

function new()
	keyb = { 
		kmode  = false;
		KEY_MAX=78;
		shift = 1;

		kb = {
		  {"äø","","","","","","Ç–","ÉJ","","","","DEL","BS",
		   "1","2","3","4","5","6","7","8","9","0","-","^","\\",
		   "q","w","e","r","t","y","u","i","o","p","@","[","RET",
		   "a","s","d","f","g","h","j","k","l",";",":","]","RET",
		   "z","x","c","v","b","n","m",",",".","/","","","",
		   "SFT","","","","","SPC","SPC","ïœ","","","","","" },

		  {"äø","","","","","","Ç–","ÉJ","","","","DEL","BS",
		   "!","\"","#","$","%","&","\'","(",")","","=","~","|",
		   "Q","W","E","R","T","Y","U","I","O","P","`","{","RET",
		   "A","S","D","F","G","H","J","K","L","+","*","}","RET",
		   "Z","X","C","V","B","N","M","<",">","?","_","","",
		   "SFT","","","","","SPC","SPC","ïœ","","","","","" }
		};

		romadic = {
		-- 1
		{
			{"A","Ç†"},{"I","Ç¢"},{"U","Ç§"},{"E","Ç¶"},{"O","Ç®"},
			{"!", "ÅI"},{"\"", "Åh"},{"#", "Åî"},{"$", "Åê"},{"%", "Åì"},{"&", "Åï"},
			{"'", "Åf"},{"(", "Åi"},{")", "Åj"},{"-", "Å["},{"^", "ÅO"},{"-","Å["},
			{"=", "ÅÅ"},{"~", "Å`"},{"|", "Åb"},{"@", "Åó"},{"[", "Åu"},{"`", "ÅM"},
			{"{", "Åo"},{";", "ÅG"},{":", "ÅF"},{"]", "Åv"},{"+", "Å{"},{"*", "Åñ"},
			{"}", "Åp"},{",", "ÅA"},{".", "ÅB"},{"\\", "Åè"},{"<", "ÅÉ"},{">", "ÅÑ"},
			{"?", "ÅH"},{"_", "ÅQ"},{"/", "ÅE"},
		},
		-- 2
		{
			{"KA", "Ç©"},{"KI", "Ç´"},{"KU", "Ç≠"},{"KE", "ÇØ"},{"KO", "Ç±"},
			{"GA", "Ç™"},{"GI", "Ç¨"},{"GU", "ÇÆ"},{"GE", "Ç∞"},{"GO", "Ç≤"},
			{"SA", "Ç≥"},{"SI"  ,"Çµ"},{"SU"  ,"Ç∑"},{"SE"  ,"Çπ"},{"SO"  ,"Çª"},
			{"ZA", "Ç¥"},{"ZI"  ,"Ç∂"},{"JI"  ,"Ç∂"},{"ZU"  ,"Ç∏"},{"ZE"  ,"Ç∫"},{"ZO"  ,"Çº"},
			{"TA", "ÇΩ"},{"TI", "Çø"},{"TU", "Ç¬"},{"TE", "Çƒ"},{"TO", "Ç∆"},
			{"DA", "Çæ"},{"DI", "Ç¿"},{"DU", "Ç√"},{"DE", "Ç≈"},{"DO", "Ç«"},
			{"NA", "Ç»"},{"NI", "Ç…"},{"NU", "Ç "},{"NE", "ÇÀ"},{"NO", "ÇÃ"},
			{"HA", "ÇÕ"},{"HI", "Ç–"},{"HU", "Ç”"},{"FU", "Ç”"},{"HE", "Ç÷"},{"HO", "ÇŸ"},
			{"BA", "ÇŒ"},{"BI", "Ç—"},{"BU", "Ç‘"},{"BE", "Ç◊"},{"BO", "Ç⁄"},
			{"FA", "Ç”Çü"},{"FI", "Ç”Ç°"},{"FYU","Ç”Ç„"},{"FE", "Ç”Ç•"},{"FO", "Ç”Çß"},
			{"PA", "Çœ"},{"PI", "Ç“"},{"PU", "Ç’"},{"PE", "Çÿ"},{"PO", "Ç€"},
			{"MA", "Ç‹"},{"MI", "Ç›"},{"MU", "Çﬁ"},{"ME", "Çﬂ"},{"MO", "Ç‡"},
			{"YA", "Ç‚"},{"YU", "Ç‰"},{"YO", "ÇÊ"},
			{"RA", "ÇÁ"},{"RI", "ÇË"},{"RU", "ÇÈ"},{"RE", "ÇÍ"},{"RO", "ÇÎ"},
			{"WA","ÇÌ"},{"WI","ÇÓ"},{"WE","ÇÔ"},{"WO","Ç"},{"NN","ÇÒ"},{"XN","ÇÒ"},
			{"JA"  ,"Ç∂Ç·"},{"JU"  ,"Ç∂Ç„"},{"JE"  ,"Ç∂Ç•"},{"JO " ,"Ç∂ÇÂ"},
			{"LA","Çü"},{"LI","Ç°"},{"LU","Ç£"},{"LE","Ç•"},{"LO","Çß"},
			{"XA","Çü"},{"XI","Ç°"},{"XU","Ç£"},{"XE","Ç•"},{"XO","Çß"},
			{"WI","Ç§Ç°"},{"WE","Ç§Ç•"},
			{"VA","Ç§ÅJÇü"},{"VI","Ç§ÅJÇ°"},{"VU","Ç§ÅJ"},{"VE","Ç§ÅJÇ•"},{"VO","Ç§ÅJÇß"},
		},
		-- 3
		{
			{"SHI" ,"Çµ"},{"CHI","Çø"},{"TSU","Ç¬"},{"LTU","Ç¡"},
			{"KYA","Ç´Ç·"},{"KYU","Ç´Ç„"},{"KYO","Ç´ÇÂ"},
			{"SYA" ,"ÇµÇ·"},{"SYU" ,"ÇµÇ„"},{"SYE" ,"ÇµÇ•"},{"SYO" ,"ÇµÇÂ"},
			{"SHA" ,"ÇµÇ·"},{"SHU" ,"ÇµÇ„"},{"SHE" ,"ÇµÇ•"},{"SHO" ,"ÇµÇÂ"},
			{"ZYA" ,"Ç∂Ç·"},{"ZYU" ,"Ç∂Ç„"},{"ZYE" ,"Ç∂Ç•"},{"ZYO" ,"Ç∂ÇÂ"},
			{"JYA" ,"Ç∂Ç·"},{"JYU" ,"Ç∂Ç„"},{"JYE" ,"Ç∂Ç•"},{"JYO" ,"Ç∂ÇÂ"},
			{"GYA","Ç¨Ç·"},{"GYU","Ç¨Ç„"},{"GYO","Ç¨ÇÂ"},
			{"GWA","ÇÆÇü"},{"GWI","ÇÆÇ°"},{"GWE","ÇÆÇ•"},{"GWO","ÇÆÇß"},
			{"TYA","ÇøÇ·"},{"TYU","ÇøÇ„"},{"TYE","ÇøÇ•"},{"TYO","ÇøÇÂ"},
			{"CYA","ÇøÇ·"},{"CYU","ÇøÇ„"},{"CYE","ÇøÇ•"},{"CYO","ÇøÇÂ"},
			{"CHA","ÇøÇ·"},{"CHU","ÇøÇ„"},{"CHE","ÇøÇ•"},{"CHO","ÇøÇÂ"},
			{"DYA","Ç¿Ç·"},{"DYU","Ç¿Ç„"},{"DYE","Ç¿Ç•"},{"DYO","Ç¿ÇÂ"},
			{"THA","ÇƒÇ·"},{"THI","ÇƒÇ°"},{"THU","ÇƒÇ„"},{"THE","ÇƒÇ•"},{"THO","ÇƒÇÂ"},
			{"DHA","Ç≈Ç·"},{"DHI","Ç≈Ç°"},{"DHU","Ç≈Ç„"},{"DHE","Ç≈Ç•"},{"DHO","Ç≈ÇÂ"},
			{"NYA","Ç…Ç·"},{"NYU","Ç…Ç„"},{"NYE","Ç…Ç•"},{"NYO","Ç…ÇÂ"},
			{"HYA","Ç–Ç·"},{"HYU","Ç–Ç„"},{"HYE","Ç–Ç•"},{"HYO","Ç–ÇÂ"},
			{"BYA","Ç—Ç·"},{"BYU","Ç—Ç„"},{"BYE","Ç—Ç•"},{"BYO","Ç—ÇÂ"},
			{"PYA","Ç“Ç·"},{"PYU","Ç“Ç„"},{"PYE","Ç“Ç•"},{"PYO","Ç“ÇÂ"},
			{"MYA","Ç›Ç·"},{"MYU","Ç›Ç„"},{"MYE","Ç›Ç•"},{"MYO","Ç›ÇÂ"},
			{"LYA","Ç·"},{"XYA","Ç·"},{"LYU","Ç„"},{"XYU","Ç„"},{"LYO","ÇÂ"},{"XYO","ÇÂ"},
			{"RYA","ÇËÇ·"},{"RYU","ÇËÇ„"},{"RYE","ÇËÇ•"},{"RYO","ÇËÇÂ"},
		},
		-- 4
		{
			{"LTSU","Ç¡"}
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
					if( k == "äø"  ) then 
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
						if( self.kmode ) then k = "Å@" else k = " " end;
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
							buf = string.sub( buf, 1, #buf-2 ); -- 2ÉoÉCÉgåàÇﬂÇ§Çø
						end
						spkey = k;
						k = "";
					end;
					if( k == "ïœ" ) then
--						if( kinp ) then 
							--"ïœä∑àÍóóï\é¶"
--						end;
						spkey = k;
						k = "";
					end;
					if( k ~= "" ) then 
						if( self.kmode ) then
							kinp = true; -- ì¸óÕíÜ Ç©Ç»ïœä∑
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
