//lens magnification effect with 320*200*256 color background
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include "Ax3ddj.c"
#include "pic_format.h"

#define sqsize 66
//size of square used to simulate lens


FILE *pcxfile;
PCXHStruc pcxhead;
unsigned char pal[768];
unsigned char *pcximage,*bgrndimg;
unsigned int pcxadd,bgrndadd;
unsigned char checkbyte,runlength;
int pixcount,height,width,pixperline,color;
int i,j,k;
int square[sqsize*sqsize];
float rc,d,rs,px,py,pz,qx,qy,qz,ix,iy,iz;
int lensx,lensy;
int path[360][2],degx;

word viradd;

void setpal(void);
void initmap(void);
void initpath(void);
void deinitmap(void);

unsigned char indicatorval;

main()
{
  initytab();
  initpath();
  initsinus();
  initmap();
  bgrndadd = (word)bgrndimg;
  pcxfile=fopen("car.pcx","rb");
  if (pcxfile==NULL)
    exit(0);
  fread(&pcxhead,128,1,pcxfile);// get the pcx header file
  width=pcxhead.xmax-pcxhead.xmin+1;
  height=pcxhead.ymax-pcxhead.ymin+1;
  pixperline=pcxhead.bytesperline;
  printf("width %d height %d ppl %d \n", width, height, pixperline);

  fseek(pcxfile,-769,SEEK_END);//the 2nd value. +ve moves forward and vice
  fread(&indicatorval,1,768,pcxfile);
  printf("Indicator value %d\n", indicatorval);
  
  fseek(pcxfile,-768,SEEK_END);//the 2nd value. +ve moves forward and vice
  fread(pal,1,768,pcxfile);
  fseek(pcxfile,128,SEEK_SET);//goto the image data at 128. 127 is end of head


//getch();
  initmode();
  initvirscr();
//  plotpix(500,3000, 50, (int)virscr);
  viradd=(word)virscr;
  setpal();
//  showpal();

/*
  for(int xx = 0; xx < 320; xx++)
    for(int yy = 0; yy < 200; yy++)
    {
	plotpix(xx,yy,50,viradd);
//	printf("yval = %d\n", ytab[yy]);
    }
*/
  

  int currenty = 0;  
  while(currenty < height)
  {
      fread(&checkbyte,1,1,pcxfile);  //read the next byte from the file
      if((checkbyte&192)==192) // check if it is repeated (11000000==192)
      {
	runlength=checkbyte&63;//if so calculate the runlength 00111111==63
	fread(&color,1,1,pcxfile); //read the next byte as the color
	for(j=0;j<=runlength-1;j++) //plot the whole runlength
	{
	  plotpix(pixcount,currenty,color,viradd);
	  pixcount+=1; //pixcount keeps track of the x value of current pix
	}
      }
      else
      {
	plotpix(pixcount,currenty,checkbyte,viradd);
	// since the first 2 bits are not set, the checkbyte is the color of
	// the current and there is no need to mask out the color
	pixcount+=1;
      }
     if(pixcount==width)
     { 
	pixcount = 0;
        currenty++;
     }
  }
  
    swapbufs(viradd,bgrndadd);
    swaptoscr(bgrndadd);
//  freevirscr();
//  inittext();
//  exit(0);


  if(pcxfile!=NULL)
    fclose(pcxfile);
  d=-15;
  rs=35;
  rc=sqrt(rs*rs-d*d);

  lensx=130;
  lensy=70;
  float scalex = 1.0;
  float scaley = 1.0;
  
  while(!kbhit())
  {
    degx=(degx+1)%720;
    scalex = 2.0 + 2 * sine[degx];
    scaley = 2.0 + 2 * sine[degx];
    
    int cx = 50;
    int cy = 100;
    int M = 15;
    float k = 0.01;
    clrbuf(0, viradd);
    for(int x = 0; x < 320; x++)
      for(int y = 0; y < 200; y++)
      {
	int thetax = x - cx;
	int thetay = y - cy;
	      
	float h = M * cosine[(int) (0.2 * thetax)];
	float d = h * M * k * cosine[(int)(k * 0.2 * thetax)];
	
	int srcx;

	if( (x % 360 < 180) && (x % 360 >=0) )
        srcx = x - d;
	while(srcx < 0) 
	  srcx += 320;
	while(srcx >= 320)
	  srcx -= 320;
	
	int srcy;

	h = M * cosine[(int) (0.2 * thetay)];
	d = h * M * k * cosine[(int)(k * 0.2 * thetay)];

	if( (y % 360 < 180) && (y % 360 >=0) )
        srcy = y - d;
	while(srcy < 0) 
	  srcy += 200;
	while(srcy >= 200)
	  srcy -= 200;


	
	if(!((srcx >=0) && (srcx < 320) && (srcy >= 0) && (srcy < 200)))
	  continue;
	unsigned char col = bgrndimg[srcx + ytab[srcy]];
	if((x >=0) && (x < 320) && (y >= 0) && (y < 200))
	virscr[x + ytab[y]] = col;
	      
      }

    swaptoscr(viradd);
  }
  getch();
  inittext();
  freevirscr();
  deinitmap();
}

void setpal(void)
{
  int i;

  for(i=0;i<=255;i++)
    setcol(i,pal[i*3]>>2,pal[i*3+1]>>2,pal[i*3+2]>>2);
}

void initmap(void)
{
  int code = -1;

  bgrndimg = (unsigned char *) calloc(64640, sizeof(char));
//  code=allocmem(4096,&bgrndadd);//blocks allocated with allocmem are
//  bgrndimg=(byte *)MK_FP(bgrndadd,0);//paragraph aligned... ie start at ????:0000
			      //unlike calloc etc. which start at ????:0004

  if(code!=-1)
  {
    printf("Failed: maximum number of paragraphs available is \n");
    getch();
    exit(0);
  }
//  clr16(0,bgrndadd);
}

void deinitmap(void)
{
//  freemem(bgrndadd);
	free(bgrndimg);
}

void initpath(void)
{
  int i,py,px;

  px=3;py=7;
  for(i=0;i<=359;i++)
  {
    path[i][0]=ceil(60*sin(px*pidiv180*i));
    path[i][1]=ceil(40*sin(py*pidiv180*i));
  }
}
