def jis2sjis( s )

	c1 = s[0,2].hex
	c2 = s[2,2].hex

   if( ( c1 % 2 ) == 0 )
      c2 += 0x7d
   else
      c2 += 0x1f
   end
   
   if( c2 > 0x7e ) then c2 = c2 + 1 end

   c1 = (c1 - 0x21)/2 + 0x81 ;
   if( c1 >= 0x9f ) then c1 = c1 + 0x40; end;

	return c1.to_s(16) + c2.to_s(16)
end

flg = 0
buf = []
w = 0
h = 0

open("5x10rk.bdf") {|file|
  a = []
  while l = file.gets
    b = l.split(' ')

	if b[0] == "BBX"
		w = b[1].to_i
		h = b[2].to_i
		next
	end
	if b[0] == "STARTCHAR"
		num = b[1].hex
		next
	end
	if b[0] == "ENDCHAR"
		flg = 0 
		buf[num] = a
		a = []
		next
	end
    if b[0] == "BITMAP"
	    flg = 1 
	    next
	end
	if flg == 1
		a << l[0,2].hex
	end
  end
}

open( "maru10.bin","wb") {|ofile|
	for i in  0 .. 255 
		a = buf[i]
		for j in 0 .. h-1
			ofile.putc(a.shift)
		end
	end
}

a = []
b = {}
buf=[]
num = 0
open("knj10.bdf") {|file|
  while l = file.gets
    b = l.split(' ')

	if b[0] == "BBX"
		w = b[1].to_i
		h = b[2].to_i
		next
	end
	if b[0] == "STARTCHAR"
		num = jis2sjis( b[1] )
		next
	end
	if b[0] == "ENDCHAR"
		flg = 0 
		buf[num.hex] = a
		a = []
		next
	end
    if b[0] == "BITMAP"
	    flg = 1 
	    next
	end
	if flg == 1 then 
		a << l[0,2].hex
		a << l[2,4].hex
	end
  end
}

def outbuf( fp, b, h )
	for i in 0..h-1
		if( b == nil )
			fp.putc(0)
			fp.putc(0)
		else
			fp.putc(b.shift)
			fp.putc(b.shift)
		end
	end
end

open( "maru10.bin","a+b") {|ofile|
	for i in  0x81 .. 0x9F
		for j in  0x40 .. 0x7E
			outbuf( ofile, buf[i*0x100+j], h )
		end
		for j in  0x80 .. 0xFC
			outbuf( ofile, buf[i*0x100+j], h )
		end
	end

	for i in  0xE0 .. 0xFC
		for j in  0x40 .. 0x7E
			outbuf( ofile, buf[i*0x100+j], h )
		end
		for j in  0x80 .. 0xFC
			outbuf( ofile, buf[i*0x100+j], h )
		end
	end	
}
