//lens magnification effect with 320*200*256 color background
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream.h>
#include "Ax3ddj.h"
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

main()
{
  pcxfile=fopen("c:/axiom/graphic1.pcx","r+b");
  if (pcxfile==NULL)
    exit(0);
  fseek(pcxfile,-768,SEEK_END);//the 2nd value. +ve moves forward and vice
  fread(pal,1,768,pcxfile);
  initmode();
  initvirscr();
  viradd=(word)virscr;
  initytab();
  setpal();
  initmap();
  initpath();

  fseek(pcxfile,0,SEEK_SET);//the 2nd value. +ve moves forward and vice
  fread(&pcxhead,sizeof(PCXHStruc),1,pcxfile);// get the pcx header file
  fseek(pcxfile,128,SEEK_SET);//goto the image data at 128. 127 is end of head
  width=pcxhead.xmax-pcxhead.xmin+1;
  height=pcxhead.ymax-pcxhead.ymin+1;
  pixperline=pcxhead.bytesperline;
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
  swap(viradd,VGA);
  swap(viradd,bgrndadd);
  swap(bgrndadd,VGA);
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
    lensx=130+path[degx][0];
    lensy=70+path[degx][1];
  for(i=0;i<=sqsize-1;i++)
    for(j=0;j<=sqsize-1;j++)
    {
      if(((i-sqsize/2)*(i-sqsize/2)+(j-sqsize/2)*(j-sqsize/2))>=(sqsize*sqsize/4))
	square[i+sqsize*j]=bgrndimg[(lensx+i)+(lensy+j)*320];
      else
      {
	px=(i-sqsize/2);
	py=(j-sqsize/2);
	//pz=0;
	//qx=px;
	//qy=py;
	qz=-d+sqrt((d*d)-(px*px+py*py-rc*rc));
	ix=(int)(d/(d+qz)*px);
	iy=(int)(d/(d+qz)*py);
	iz=0;
	square[i+sqsize*j]=bgrndimg[(lensx+ix+sqsize/2)+(lensy+iy+sqsize/2)*320];
      }
    }
    swap(bgrndadd,viradd);
    for(i=0;i<=sqsize-1;i++)
      for(j=0;j<=sqsize-1;j++)
	//bgrndimg[(lensx+i)+(lensy+j)*320]=square[i+20*j];
	//if(square[i+sqsize*j]!=128);
	if(((i-sqsize/2)*(i-sqsize/2)+(j-sqsize/2)*(j-sqsize/2))<(sqsize*sqsize/4))
	  plotpix(lensx+(sqsize-1-i)+k,(lensy+(sqsize-1-j))*320,square[i+sqsize*j],viradd);
    vtrace();
    vtrace();
    swap(viradd,VGA);
  }
  getch();
  inittext();
  deinitvirscr();
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
  int code;

  code=allocmem(4096,&bgrndadd);//blocks allocated with allocmem are
  bgrndimg=(byte *)MK_FP(bgrndadd,0);//paragraph aligned... ie start at ????:0000
			      //unlike calloc etc. which start at ????:0004

  if(code!=-1)
  {
    printf("Failed: maximum number of paragraphs available is \n");
    getch();
    exit(0);
  }
  clr16(0,bgrndadd);
}

void deinitmap(void)
{
  freemem(bgrndadd);
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
