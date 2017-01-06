/////////////////////////////////////////////////////////////////////////////
//               Graphic Files Format Header By A x I o M                 //
/////////////////////////////////////////////////////////////////////////////

typedef struct
{
  int  bftype;     // Specifies the type of file. It must be BM.
  long bfsize;     // Specifies the size in DWORDs of the file.
  int  bfreserved1;//Is reserved and must be set to zero.
  int  bfreserved2;//Is reserved and must be set to zero.
  long bfoffbits;  //Specifies in bytes the offset from the BITMAPFILEHEADER
		   //of the actual bitmap in the file.
} tBMPFILEHEADER;

typedef struct
{
  long bisize;
  long biwidth;
  long biheight;
  int  biplanes;
  int  bibitcount;
  long bicompression;
  long bisizeimage;
  long bixpelspermeter;
  long biypelspermeter;
  long biclrused;
  long biclrimportant;

} tBITMAPINFOHEADER;

typedef struct
{
  unsigned char rgbblue;
  unsigned char rgbgreen;
  unsigned char rgbred;
  unsigned char rgbreserved;
} tRGBQUAD;

typedef struct
{
  tBITMAPINFOHEADER bmiheader;
  tRGBQUAD   bmicolors[256];// there are 256elements since we are
				   // loading 256 color bitmaps
} tBITMAPINFO;

typedef struct  // structure for pcx file header
{
  char manufacturer;
  char version;
  char encoding; // 1=pcx RLE
  char bitsperpix; //no of bits representing 1 pix on the screen
  short int  xmin,ymin,xmax,ymax; //image dimension in the order they are stored
  short int  vdpi; //hor and vert resolution of image in dpi
  //int  hdpi,vdpi; //hor and vert resolution of image in dpi
  char  colormap[48];
  char reserved; //should be set to 0
  char noplanes; // no of planes
  short int  bytesperline;// bytes per scanline. must be EVEN. is != xmax-xmin
  short int  palinfo; //how to interpret palette. 1=color/BW 2=greyscale
  short int  Vscrsize,Hscrsize;// vert and hor scr size
  char filler[54]; //empty. used to fill out 54 bytes to make 128 byte header
} PCXHStruc;
