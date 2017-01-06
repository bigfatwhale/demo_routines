//lens magnification effect with 320*200*256 color background
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include<conio.h>
#include <dos.h>
#include "Ax3ddj.c"
#include "pic_format.h"

#define sqsize 60
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

  initmap();

  pcxfile=fopen("graphic1.pcx","r+b");
  if (pcxfile==NULL)
    exit(0);
  fseek(pcxfile,-769,SEEK_END);//the 2nd value. +ve moves forward and vice
  fread(&indicatorval,1,768,pcxfile);
  printf("Indicator value %d\n", indicatorval);
  
  fseek(pcxfile,-768,SEEK_END);//the 2nd value. +ve moves forward and vice
  fread(pal,1,768,pcxfile);
  fseek(pcxfile,0,SEEK_SET);//the 2nd value. +ve moves forward and vice
  fread(&pcxhead,sizeof(PCXHStruc),1,pcxfile);// get the pcx header file
  fseek(pcxfile,128,SEEK_SET);//goto the image data at 128. 127 is end of head
  width=pcxhead.xmax-pcxhead.xmin+1;
  height=pcxhead.ymax-pcxhead.ymin+1;
  pixperline=pcxhead.bytesperline;
  printf("%d \n", pixperline);




  initmode();
  initvirscr();
  plotpix(500,3000, 50, (int)virscr);
  showpal();
//  getch();
  freevirscr();
  inittext();
  exit(0);
  viradd=(word)virscr;
  setpal();
 
  for(i=0;i<height;i++)
  {
    pixcount=0;
    while(pixcount<pixperline)
    {
      fread(&checkbyte,1,1,pcxfile);  //read the next byte from the file
      if((checkbyte&192)==192) // check if it is repeated (11000000==192)
      {
	runlength=checkbyte&63;//if so calculate the runlength 00111111==63
	fread(&color,1,1,pcxfile); //read the next byte as the color
	for(j=0;j<=runlength-1;j++) //plot the whole runlength
	{
	  plotpix(pixcount,ytab[i],color,viradd);
	  pixcount+=1; //pixcount keeps track of the x value of current pix
	}
      }
      else
      {
	plotpix(pixcount,ytab[i],checkbyte,viradd);
	// since the first 2 bits are not set, the checkbyte is the color of
	// the current and there is no need to mask out the color
	pixcount+=1;
      }
    }
  }
  printf("here\n");
  swapbufs((word)virscr,VGA);
  swapbufs((word)virscr,bgrndadd);
  swapbufs(bgrndadd,VGA);
  if(pcxfile!=NULL)
    fclose(pcxfile);
  d=-15;
  rs=35;
  rc=sqrt(rs*rs-d*d);

  lensx=130;
  lensy=70;
  while(!kbhit())
  {
    degx=(degx+1)%360;
//    swapbufs(bgrndadd,viradd);

    clrbuf(0, viradd);
    for(int x = 0; x < 320; x++)
      for(int y = 0; y < 200; y++)
      {
	int srcx = x * cosine[degx] + y * sine[degx];
	int srcy = - x * sine[degx] + y * cosine[degx];
	unsigned char col = bgrndimg[srcx + ytab[srcy]];
	if((x >=0) && (x < 320) && (y >= 0) && (y < 200))
	virscr[x + ytab[y]] = col;
	      
      }

    
    swapbufs(viradd,VGA);
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

  bgrndimg = (unsigned char *) calloc(65536, sizeof(char));
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

  px=2;py=7;
  for(i=0;i<=359;i++)
  {
    path[i][0]=ceil(80*sin(px*pidiv180*i));
    path[i][1]=ceil(60*sin(py*pidiv180*i));
  }
}
