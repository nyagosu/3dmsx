
def jis2sjis( s )

	c1 = s[0,2].hex
	c2 = s[2,2].hex

#   /* 第２バイトの変換 */
   
   if( ( c1 % 2 ) == 0 )
      c2 += 0x7d
   else
      c2 += 0x1f
   end
   
   if( c2 > 0x7e )
      c2 = c2 + 1
   end
#   /* 第１バイトの変換 */
   c1 = c1 + 1;
   c1 = c1 / 2;
   if( c1 < 0x5f )
      c1 = c1 + 0x70;
   else
      c1 = c1 + 0xb0;
   end

	return c1.to_s(16) + c2.to_s(16)
end


flg = 0
buf = Array.new(256, '00000000000000000000' )

num = 0

file = open("font5x6.font.bmp","rb")

file.binmode

bfType    = file.read(2);
bfSize    = file.read(4).unpack("V")[0];
reserve   = file.read(4);
bfOffBits = file.read(4).unpack("V")[0];

#infoheader
biSize    = file.read(4).unpack("V")[0];
biWidth   = file.read(4).unpack("V")[0];	# bmp 幅
biHeight  = file.read(4).unpack("V")[0];	# bmp 高さ
biPlanes  = file.read(2).unpack("v")[0];
biBitCount= file.read(2).unpack("v")[0];
biCompress= file.read(4).unpack("V")[0];
biSizeImg = file.read(4).unpack("V")[0];
biXPixPerM= file.read(4).unpack("V")[0];
biYPixPerM= file.read(4).unpack("V")[0];
biClrUsed = file.read(4).unpack("V")[0];
biClrImp  = file.read(4).unpack("V")[0];

# pallette
bpPal = file.read(1024).unpack("V*");

#image
buf = Array.new(6,"");
(biSizeImg/48).times{ |n|
	6.times { |a|
		buf2 = "";
		file.read(8).each_byte{|c| buf2 += c==0xFF?'0':'1'; }
		buf[a] = buf2 + ' ' + buf[a];
	}
}
buf = buf.reverse;
6.times { |a| print buf[a] + "\n"; }

file.close

printf( "BITMAPFILEHEADER\n" );
printf( "TYPE     :%s\n", bfType );
printf( "FILE SIZE:%d\n", bfSize );
printf( "OFFSET   :%d\n", bfOffBits );

printf( "\nBITMAPINFOHEADER\n" );
printf( "SIZE     :%d\n", biSize     );
printf( "WIDTH    :%d\n", biWidth    );
printf( "HEIGHT   :%d\n", biHeight   );
printf( "PLANES   :%d\n", biPlanes   );
printf( "BITCOUNT :%d\n", biBitCount );
printf( "IMG SIZE :%d\n", biSizeImg  );

printf( "color:0  :%d\n", bpPal[0]   );
printf( "color:255:%x\n", bpPal[255] );


