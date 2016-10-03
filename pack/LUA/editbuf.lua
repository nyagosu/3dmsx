module( "editbuf", package.seeall )

function new()
	textbuf = { 
		TABWIDTH  = 4;
		SCRWIDTH  = 42;
		SCRHEIGHT = 18;
		buf  = {},
		sjis = {},
		cursor = { x=1, y=1 }, 
		dsppos = { x=1, y=1 },
		scrpos = { x=1, y=1 },
	}

	function textbuf:clear ()
		self.buf = {};
		self.sjis= {};
		self.cursor.x=1;
		self.cursor.y=1;
		self.scrpos.x=1;
		self.scrpos.y=1;
	end

	-- 行数
	function textbuf:length()
		return #self.buf;
	end

	-- 2バイト文字数
	function textbuf:count( b )
		b = textbuf:getsjisflg( b );
		return #( string.gsub(b,"[2]","") );
	end


	function textbuf:getExpand(y, s, e)
		local i = 1;
		local b = "";
		local c = "";
		local cnt = 1;
		local ret = "";
		local tcnt = 0;
		if( self.buf[y] == nil ) then
			return string.rep( " ",self.SCRWIDTH );
		end
		while( cnt <= e ) do
			if( tcnt == 0 ) then
				c  = string.sub(self.buf[y],i,i);
				sf = string.sub(self.sjis[y],i,i);
				i = i + 1;
				tcnt = 1;
				if( c == "\t" ) then
					tcnt = textbuf.TABWIDTH - ((cnt-1)%textbuf.TABWIDTH);
					b = " ";
				else 
					if( c == "" ) then
						b = " ";
					else
						b = c;
					end
				end
			end
			if( cnt == s ) then
				if( sf == "2" ) then b = " "; end
			end
			if( cnt == e ) then
				if( sf == "1" ) then b = " "; end
			end
			if( cnt >= s ) then ret = ret .. b; end
			tcnt=tcnt-1;
			cnt =cnt +1;
		end
		return ret;
	end

	function textbuf:getsjisflg( val )
		local s="";
		local flg=false;
		for i=1,#val,1 do
			local c = string.sub(val,i,i);
			c = string.byte(c);
			if( flg ) then
				flg = false;
				s = s .. "2";
			else
				if((c>0x80 and c<0xa0) or (c>0xdf and c<0xfd)) then 
					flg = true;
					s = s .. "1"
				else
					s = s .. "0"
				end
			end
		end
		return s;
	end
	function textbuf:get( t1, t2 )
		return  string.sub( self.buf[self.cursor.y] , t1, t2 ), 
				string.sub( self.sjis[self.cursor.y], t1, t2 );
	end
	function textbuf:set( val )
		self.buf [self.cursor.y] = val;
		self.sjis[self.cursor.y] = textbuf:getsjisflg(val);
	end
	function textbuf:insertline( val )
		table.insert( self.buf , self.cursor.y, val );
		table.insert( self.sjis, self.cursor.y, textbuf:getsjisflg(val) );
	end
	function textbuf:deleteline()
		table.remove( self.buf , self.cursor.y );
		table.remove( self.sjis, self.cursor.y );
	end
	function textbuf:separate(buf,x)
		if( buf == nil ) then
			return "", "";
		else
			if( x > 1 ) then
				return string.sub( buf ,1, x-1 ), string.sub( buf ,x );
			else
				return "", string.sub( buf ,x );
			end
		end
	end
	function textbuf:chkKanji(off_x)
		if( off_x == nil) then off_x = 0; end;
		return string.sub( self.sjis[self.cursor.y],self.cursor.x + off_x , self.cursor.x + off_x);
	end
	function textbuf:insert( val )
		local b1,b2 = textbuf:separate( self.buf[self.cursor.y], self.cursor.x );
		self.buf [self.cursor.y] = b1 .. val .. b2;

		b1,b2 = textbuf:separate( self.sjis[self.cursor.y], self.cursor.x );
		val = textbuf:getsjisflg(val);
		self.sjis[self.cursor.y] = b1 .. val .. b2;
		return val;
	end

	function textbuf:input( val )
		val = textbuf:insert( val );
		fori=0,#( string.gsub(val,"[2]","") ),1 do
			textbuf:right();
		end
	end

	function textbuf:separateLine()
		local b1,b2 = textbuf:separate( self.buf[self.cursor.y] , self.cursor.x );
		self.buf [self.cursor.y] = b1;
		table.insert( self.buf , self.cursor.y+1, b2 );

		b1,b2 = textbuf:separate( self.sjis[self.cursor.y] , self.cursor.x );
		self.sjis[self.cursor.y] = b1;
		table.insert( self.sjis, self.cursor.y+1, b2 );
	end
	function textbuf:delete()
		if( self.buf[self.cursor.y] ~= nil ) then
			if( self.cursor.x > #self.buf[self.cursor.y] ) then
				if( self.buf[self.cursor.y+1] ~= nil )then 
					self.buf[self.cursor.y] = self.buf[self.cursor.y] .. textbuf[self.cursor.y+1];
					self.sjis[self.cursor.y] = self.sjis[self.cursor.y] .. self.sjis[self.cursor.y+1];
					textbuf:down();
					textbuf:deleteline();
					chg = 2;
				end
			else
				local dcnt = 2;
				if( textbuf:chkKanji() == "1" ) then dcnt = 3; end

				local b1 = string.sub(self.buf[self.cursor.y],1,self.cursor.x);
				local b2 = string.sub(self.buf[self.cursor.y],self.cursor.x+dcnt);
				self.buf[self.cursor.y] = b1 .. b2;

				b1 = string.sub(self.sjis[self.cursor.y],1,self.cursor.x)
				b2 = string.sub(self.buf[self.cursor.y],self.cursor.x+dcnt);
				self.sjis[self.cursor.y] = b1 .. b2;
				chg = 1;
			end
		end
	end
	function textbuf:load( fn )
		self:clear();
		local i = 1;
		for line in io.lines(fn) do
			self:insertline( string.gsub(line,"[\r]","") );
			self:down();
			i = i + 1;
		end
		self:setcursor(1,1);
	end
	function textbuf:save( fn )
		local fp = io.open(fn ,"w");
		for i,v in ipairs(self.buf) do
			fp:write( v .."\r\n" );
		end
		fp:close();
	end

	function textbuf:toBufPos(b, x)
		local i = 1;
		local cnt = 1;
		local ret = "";
		if( b == nil ) then return 1,1; end
		if( x == 1 ) then return 1,1; end
		local c = string.sub(b,i,i);
		while( c ~= "" ) do
			local tcnt = 1;
			if( c == "\t" ) then
				tcnt = textbuf.TABWIDTH - ((cnt-1)%textbuf.TABWIDTH);
			end
			if( x < cnt + tcnt ) then break; end
			cnt = cnt + tcnt;
			i=i+1;
			c = string.sub(b,i,i);
		end
		return i, cnt;
	end

	function textbuf:toDspPos(b, x)
		local i = 1;
		local cnt = 1;
		if( b == nil ) then return 1; end
		while( i < x ) do
			local tcnt = 1;
			local c = string.sub(b,i,i);
			if( c== "" ) then break; end;
			if( c=="\t" ) then tcnt = textbuf.TABWIDTH - ((cnt-1)%textbuf.TABWIDTH); end
			cnt = cnt + tcnt;
			i = i + 1;
		end
		return cnt;
	end

-- cursor 関連
	function textbuf:left()
		local chg = 0;
		if( self.buf[self.cursor.y] ~= nil ) then 
			if( self.cursor.x == 1 ) then
				if( self.cursor.y > 1 ) then
					self.cursor.y = self.cursor.y - 1;
					self.cursor.x = #self.buf[self.cursor.y] + 1;
					chg = 2;
				end;
			else
				local z = 1;
				if( textbuf:chkKanji(-1) == "2" ) then
					z = 2;
				end
				self.cursor.x=self.cursor.x - z;
				chg = 1;
			end;
		end;
		chg = textbuf:fitscreen(chg)
		return chg;
	end
	function textbuf:right()
		local chg = 0;
		if( self.buf[self.cursor.y] ~= nil ) then 
			if( self.cursor.x+1 > #self.buf[self.cursor.y] ) then
				self.cursor.x = 1;
				self.cursor.y = self.cursor.y + 1;
				chg = 2;
			else
				local z = 1;
				if( textbuf:chkKanji() =="1" ) then
					z = 2;
				end
				self.cursor.x=self.cursor.x + z;
				chg = 1;
			end;
		end;
		chg = textbuf:fitscreen(chg)
		return chg;
	end
	function textbuf:up()
		local chg = 0;
		if( self.cursor.y > 1 ) then
			local dx = textbuf:toDspPos(self.buf[self.cursor.y],self.cursor.x)
			self.cursor.y=self.cursor.y-1;
			self.cursor.x = textbuf:toBufPos(self.buf[self.cursor.y],dx);
			if( textbuf:chkKanji()=="2" ) then
				self.cursor.x = self.cursor.x - 1;
			end
			chg=2;
		end
		chg = textbuf:fitscreen(chg)
		return chg;
	end
	function textbuf:down()
		local chg = 0;
		if( self.buf[self.cursor.y] ~= nil ) then 
			local dx = textbuf:toDspPos(self.buf[self.cursor.y],self.cursor.x)
			self.cursor.y=self.cursor.y+1;
			if( self.buf[self.cursor.y] == nil ) then
				self.cursor.x = 1;
			else
				self.cursor.x = textbuf:toBufPos(self.buf[self.cursor.y],dx);
				if( textbuf:chkKanji()=="2" ) then
					self.cursor.x = self.cursor.x - 1;
				end
			end
			chg=1;
		end
		chg = textbuf:fitscreen(chg)
		return chg;
	end
	function textbuf:setcursor( x, y )
		self.cursor.y = y;
		if( self.buf[self.cursor.y] == nil ) then
			self.cursor.x = 1;
		else
			local l = #self.buf[self.cursor.y] + 1;
			if(  l < x ) then
				self.cursor.x = l;
			else
				self.cursor.x = x;
				if( textbuf:chkKanji() == "2" ) then self.cursor.x = self.cursor.x - 1; end
			end
		end
		textbuf:fitscreen(1)
		return 2;
	end

	function textbuf:fitscreen(chg)
		if( chg>0 ) then
			local dx = textbuf:toDspPos(self.buf[self.cursor.y],self.cursor.x);
			if( dx < self.scrpos.x ) then self.scrpos.x = dx; chg = 2; end
			if( dx > self.scrpos.x+self.SCRWIDTH-1 ) then self.scrpos.x = dx - self.SCRWIDTH; chg = 2; end
			if( self.cursor.y < self.scrpos.y ) then self.scrpos.y = self.cursor.y; chg = 2; end
			if( self.cursor.y > self.scrpos.y+self.SCRHEIGHT-1 ) then self.scrpos.y = self.cursor.y - (self.SCRHEIGHT-1); chg = 2; end
		end
		return chg;
	end
	
	function textbuf:getscreenbuf(y)
		return textbuf:getExpand(self.scrpos.y + y - 1, self.scrpos.x, self.scrpos.x + self.SCRWIDTH - 1 );
	end
	function textbuf:getscreenpos()
		local  dx = textbuf:toDspPos(self.buf[self.cursor.y],self.cursor.x) - self.scrpos.x + 1;
		local  dy = self.cursor.y - self.scrpos.y + 1;
		return dx,dy;
	end

	return textbuf;
end
