//////////////////////////////////////////////////////////////////////////////
// bmp2bin.cpp                                                              //
//////////////////////////////////////////////////////////////////////////////
/*
        Bitmap to binary converter
        by Rafael Vuijk (aka Dark Fader)

        History:
				V1.07 - fixed for proper endian detection
                V1.06 - added support for MirkoSDK sprite format
                v1.05 - palette output code for gp32 by mr.spiv
                v1.04+ - bigendian support by mr.spiv
                v1.04 - .act & MS palette loading support, palette quantization method 1
                v1.03 - added 8 bit bmp support
                v1.02 - fixed rotation bug, copied structures from WinGDI.h
                v1.01 - combined
                v1.00 - seperate versions for each platform
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
//#include <sys/param.h>
//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
#define VER                                     "1.04"
#define VERF                                    "1.05"
#define ALIGN4(n)                       (((n)+3) &~ 3)


//////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                 //
//////////////////////////////////////////////////////////////////////////////
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef int LONG;

//
// Convert Little Endian to Big Endian..
//

/*static BYTE endiaB( BYTE b ) {
        return b;
}
*/
static WORD endiaW( WORD w ) {
        return w;
}

static DWORD endiaDW( DWORD w ) {
        return w;
}

static LONG endiaL( LONG w ) {
        return w;
}

//
//
//

#pragma pack(1)

typedef struct tagRGBTRIPLE {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {    // size 14 bytes
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{             // size 40 bytes
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

#pragma pack()

typedef void WritePixel(const RGBTRIPLE *p);

//////////////////////////////////////////////////////////////////////////////
// Variables                                                                //
//////////////////////////////////////////////////////////////////////////////
static char *inputFile;
static char *outputFile;
static char *paletteFile;
static char *outPaletteFile ;
static RGBTRIPLE palette[256];
static RGBTRIPLE *imageData;
static BITMAPFILEHEADER bfh;
static BITMAPINFOHEADER bih;
static char flags[256];
static WritePixel *writePixel;          // pixel write function
static FILE *fi;
static FILE *fo;
static FILE *fp = NULL;

//
//
//

static int writePaletteGP( RGBQUAD* p, int fmt, FILE *fp ) {
        int m,n;
        RGBQUAD q;

        for (n = 0; n < 256; n += 16 ) {
                for (m = 0; m < 16; m++) {
                        q = p[n+m];
                //      printf("%02x %02x %02x - %04x %04x %04x\n",
                //                      q.rgbRed,q.rgbGreen,q.rgbBlue,
                //                      q.rgbRed>>3<<11,q.rgbGreen>>2<<5,q.rgbBlue>>3);

                        fprintf(fp,"0x%04x",(q.rgbBlue>>3<<1) | (q.rgbGreen>>3<<6) | (q.rgbRed>>3<<11));
                        if (m < 15) {
                                fprintf(fp,",");
                        } else {
                                fprintf(fp,"\n");
                        }

                }
        }
        return 0;
}




//////////////////////////////////////////////////////////////////////////////
// Sqr                                                                      //
//////////////////////////////////////////////////////////////////////////////
inline unsigned long Sqr(unsigned int n)
{
        return n*n;
}

//////////////////////////////////////////////////////////////////////////////
// Dist1                                                                    //
//////////////////////////////////////////////////////////////////////////////
unsigned long Dist1(const RGBTRIPLE *a, const RGBTRIPLE *b)
{
        return
        (
                Sqr((int)a->rgbtRed - (int)b->rgbtRed)*28 +
                Sqr((int)a->rgbtGreen - (int)b->rgbtGreen)*91 +
                Sqr((int)a->rgbtBlue - (int)b->rgbtBlue)*9
        );
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelP1                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelP1(const RGBTRIPLE *p)           // '1': 8 bits palette (method 1)
{
        unsigned char out;
        unsigned long bestDist = (unsigned long)-1;
        
        for (int i=0; i<256; i++)
        {
                unsigned long dist = Dist1(p, &palette[i]);
                if (dist < bestDist) { bestDist = dist; out = i; }
        }
        
        fwrite(&out, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixel24                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixel24(const RGBTRIPLE *p)   // 't': 24 bits
{
        fwrite(&p->rgbtRed, 1, 1, fo);
        fwrite(&p->rgbtGreen, 1, 1, fo);
        fwrite(&p->rgbtBlue, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixel8                                                              //
//////////////////////////////////////////////////////////////////////////////
void WritePixel8(const RGBTRIPLE *p)            // 'e': 8 bits (b2g3r3)
{
        unsigned char out = (p->rgbtBlue>>6<<6) | (p->rgbtGreen>>5<<3) | (p->rgbtRed>>5<<0);
        fwrite(&out, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGP8                                                            //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGP8(const RGBTRIPLE *p)          // 'i': 8 bits (LUT, GamePark)
{
        unsigned char out = p->rgbtRed;
 // hack by Mr.Spiv
        fwrite(&out, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGP32                                                           //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGP32(const RGBTRIPLE *p)         // 'p': 16 bits (r5g5b5x1, GamePark)
{
        unsigned short out = endiaW((p->rgbtBlue>>3<<1) | (p->rgbtGreen>>3<<6) | (p->rgbtRed>>3<<11));
        fwrite(&out, 2, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGB                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGB(const RGBTRIPLE *p)           // 'g': 16 bits (x1b5g5r5, GameBoy)
{
        unsigned short out = (p->rgbtBlue>>3<<10) | (p->rgbtGreen>>3<<5) | (p->rgbtRed>>3<<0);
        fwrite(&out, 2, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelNDS                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelNDS(const RGBTRIPLE *p)           // 'g': 16 bits (x1b5g5r5, NDS)( x1 is allways 1 )
{
        unsigned short out = (p->rgbtBlue>>3<<10) | (p->rgbtGreen>>3<<5) | (p->rgbtRed>>3<<0) | 0x8000;
        fwrite(&out, 2, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelBit                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelFont(const RGBTRIPLE *p)           // 'g': 16 bits (x1b5g5r5, GameBoy)
{
	static int cnt = 7;
	static unsigned char out = 0;
	
    out |= (((p->rgbtBlue>>3<<10) | (p->rgbtGreen>>3<<5) | (p->rgbtRed>>3<<0))==0x7FFF?0:1)<<cnt ;
	cnt --;
	if( cnt < 0 ){
        fwrite(&out, 1, 1, fo);
		cnt = 7;
		out = 0;
	}
}
//////////////////////////////////////////////////////////////////////////////
// main                                                                     //
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
        // parse parameters
        for (int a=1; a<argc; a++)
        {
                if (argv[a][0] == '-')
                {
                        for (int i=1; argv[a][i]; i++)
                        {
                                flags[argv[a][i]]++;
                                if ((argv[a][i] >= '0') && (argv[a][i] <= '9')) paletteFile = argv[++a];
                        }
                }
                else
                {
                        if (!inputFile) inputFile = argv[a];
                        else if (!outputFile) outputFile = argv[a];
                        else if (!outPaletteFile) outPaletteFile = argv[a];
                        else
                        {
                                fprintf(stderr, "Error: Too many filesnames given!\n");
                        }
                }
        }

        // show help
        if (flags['?'] || flags['h'] || !inputFile || !outputFile)
        {
                fprintf(stderr, "bmp2bin "VER" by Rafael Vuijk (aka Dark Fader)\n");
                fprintf(stderr, "bmp2bin "VERF" by Jouni Korhonen (aka Mr.Spiv)\n");
                fprintf(stderr, "\n");
                fprintf(stderr, "Syntax: bmp2bin [-flags] <input.bmp> <output.raw> [<palette.txt>]\n");
                fprintf(stderr, "\n");
                fprintf(stderr, "Flags/parameters:\n");
                fprintf(stderr, "  -i                  8 bits output, LUT, GP32 256 colors palette\n");
                fprintf(stderr, "  -e                  8 bits output, b2g3r3)\n");
                fprintf(stderr, "  -1 palette.act      8 bits output, palette quantization method 1\n");
                fprintf(stderr, "  -g                  16 bits output, x1b5g5r5, GameBoy\n");
                fprintf(stderr, "  -n                  16 bits output, x1b5g5r5, NDS (x1 allways '1')\n");
                fprintf(stderr, "  -p                  16 bits output, r5g5b5x1, GP32 (default)\n");
                fprintf(stderr, "  -t                  24 bits output, b8g8r8\n");
                fprintf(stderr, "  -r                  rotate 90 degrees clockwise\n");
                fprintf(stderr, "  -x                  write sprite header, Mr.Mirko SDK\n");
                fprintf(stderr, "  -f                  write 2 color bitimage for font (white=0 other=1)\n");
                return -1;
        }

        // read palette
        if (paletteFile)
        {
                // open bitmap file
                FILE *f = fopen(paletteFile, "rb");
                if (!f) { fprintf(stderr, "Error opening palette file!\n"); return -1; }
                
                fseek(f, 0, SEEK_END);
                int size = ftell(f);
                fseek(f, 0, SEEK_SET);
        
                // read data
                //if (strstr(paletteFile, ".act"))
                if (size == 3*256)
                {
                        // assume r,g,b...
                        unsigned char paletteData[256][3];
                        fread(paletteData, sizeof(paletteData), 1, f);

                        for (int i=0; i<256; i++)
                        {
                                palette[i].rgbtRed = paletteData[i][0];
                                palette[i].rgbtGreen = paletteData[i][1];
                                palette[i].rgbtBlue = paletteData[i][2];
                        }
                }
                else if (size == 4*256)
                {
                        // assume r,g,b,x...
                        unsigned char paletteData[256][4];
                        fread(paletteData, sizeof(paletteData), 1, f);

                        for (int i=0; i<256; i++)
                        {
                                palette[i].rgbtRed = paletteData[i][0];
                                palette[i].rgbtGreen = paletteData[i][1];
                                palette[i].rgbtBlue = paletteData[i][2];
                        }
                }
                else
                {
                        fprintf(stderr, "Unknown palette format!\n");
                        return -1;
                }
                
                // close file
                fclose(f);
        }

        // outpalette

        if (outPaletteFile && (flags['i'])) {
                if ((fp = fopen(outPaletteFile,"wb")) == NULL) {
                        fprintf(stderr,"Error opening output palette file!\n");
                        return -1;      // let the compiler do the cleanup :/
                }
        }

        // read
        {
                // open bitmap file
                fi = fopen(inputFile, "rb");
                if (!fi) { fprintf(stderr, "Error opening bitmap file!\n"); return -1; }

                // read headers
                fread(&bfh, sizeof(bfh), 1, fi);
                fread(&bih, sizeof(bih), 1, fi);

                // checks
                if (endiaW(bih.biPlanes) != 1) { fprintf(stderr, "Unsupported number of planes!\n"); return -1; }
                if (endiaDW(bih.biCompression) != 0) { fprintf(stderr, "Unsupported compression type!\n"); return -1; }

                // load to RGB format
                imageData = new RGBTRIPLE[endiaL(bih.biWidth) * endiaL(bih.biHeight) + 1/*dummy*/];

                // load data with correct bit depth
                if (endiaW(bih.biBitCount) == 8)
                {
                        fprintf(stderr,"  The BMP is a 8bits image..\n");
                        // read palette (quads -> triples)
                        RGBQUAD paletteQ[256];
                        RGBTRIPLE palette[256];
                        fread(paletteQ, sizeof(paletteQ), 1, fi);
                        for (int i=0; i<256; i++)
                        {
                                palette[i].rgbtRed = paletteQ[i].rgbRed;
                                palette[i].rgbtGreen = paletteQ[i].rgbGreen;
                                palette[i].rgbtBlue = paletteQ[i].rgbBlue;
                        }
                        
                        if (fp && flags['i']) {
                                writePaletteGP(paletteQ,0,fp);
                        }


                        // go to pixel data
                        fseek(fi, endiaDW(bfh.bfOffBits), SEEK_SET);
                        
                        int lineSize = endiaL(bih.biWidth) * 1;
                        int dummysize = ALIGN4(lineSize) - lineSize;
                        int dummy;
                        
                        unsigned char *lineData = new unsigned char [lineSize];
        
                        // read & convert pixel data
                        for (int y=endiaL(bih.biHeight)-1; y>=0; y--)
                        {
                                RGBTRIPLE *pImageData = imageData + (endiaL(bih.biWidth)*y);            // imageData is up-side down!
                                fread(lineData, lineSize, 1, fi);
                                fread(&dummy, dummysize, 1, fi);
                                if (flags['i']) {
                                        for (int x=0; x<endiaL(bih.biWidth); x++) pImageData[x].rgbtRed = lineData[x];
                                } else {
                                        for (int x=0; x<endiaL(bih.biWidth); x++) pImageData[x] = palette[lineData[x]];
                                }
                        }
                }
                else if (endiaW(bih.biBitCount) == 24)
                {
                        fprintf(stderr,"  The BMP is a 24bits image..\n");
                        // go to pixel data
                        fseek(fi, endiaDW(bfh.bfOffBits), SEEK_SET);

                        int lineSize = endiaL(bih.biWidth) * 3;
                        int dummysize = ALIGN4(lineSize) - lineSize;
                        int dummy;
        
                        // read pixel data
                        for (int y=endiaL(bih.biHeight)-1; y>=0; y--)
                        {
                                RGBTRIPLE *pImageData = imageData + endiaL(bih.biWidth)*y;              // imageData is up-side down!
                                fread(pImageData, lineSize, 1, fi);
                                fread(&dummy, dummysize, 1, fi);
                        }
                }
                else
                {
                        fprintf(stderr, "Unsupported bit depth!\n");
                        return -1;
                }
        
                // close file
                fclose(fi);
        }

        // Close output palette file..
        if (fp) fclose(fp);

        // select pixel writer
        writePixel = WritePixelGP32;
        if (flags['p']) writePixel = WritePixelGP32;
        if (flags['g']) writePixel = WritePixelGB;
        if (flags['n']) writePixel = WritePixelNDS;
        if (flags['i']) writePixel = WritePixelGP8;
        if (flags['e']) writePixel = WritePixel8;
        if (flags['t']) writePixel = WritePixel24;
        if (flags['1']) writePixel = WritePixelP1;
        if (flags['f']) writePixel = WritePixelFont;

        // write
        {
                fo = fopen(outputFile, "wb");
                if (!fo) { fprintf(stderr, "Error opening output file!\n"); return -1; }

                // Mr.Mirko 2004
                // write Header
                /*
                typedef struct {
                  char magic[4];
                  u16 size_x;
                  u16 size_y;
                  u16 reserved1;
                  u16 reserved2;
                } SHEADER; */

                if (flags['x'])
                {
                  char sdk[]="Mr.M";
                  short reserved=0;
                  short x = endiaL(bih.biWidth);
                  short y = endiaL(bih.biHeight);

                  if (flags['r']) { printf ("Please dont rotate Sprite...\n"); }

                  fprintf(stderr, "X: %d\n",x);
                  fprintf(stderr, "Y: %d\n",y);
                  fwrite(&sdk, 1, 4, fo); // 4 bytes
                  fwrite(&x, 2, 1, fo);   // 1 short
                  fwrite(&y, 2, 1, fo);   // 1 short
                  fwrite(&reserved, 2, 1, fo);// 1 short
                  fwrite(&reserved, 2, 1, fo);// 1 short
                }

                // rotated?
                if (flags['r'])
                {
                        for (int x=0; x<endiaL(bih.biWidth); x++)
                        {
                                for (int y=endiaL(bih.biHeight)-1; y>=0; y--)
                                {
                                        RGBTRIPLE *p = imageData + (y*endiaL(bih.biWidth)) + x;
                                        writePixel(p);
                                }
                        }
                }
                else
                {
//                        RGBTRIPLE *p = imageData;
                        for (int y=0; y<endiaL(bih.biHeight); y++)
                        {
                                for (int x=0; x<endiaL(bih.biWidth); x++)
                                {
                                        RGBTRIPLE *p = imageData + (y*endiaL(bih.biWidth)) + x;
                                        writePixel(p);
                                }
                        }
                }

                // close files
                fclose(fo);
        }

        return 0;
}

