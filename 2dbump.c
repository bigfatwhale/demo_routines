//bitmap file decoding for 320*200*256 color bitmaps

#include<conio.h>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include"Ax3ddj.h"
#include"pic_format.h"
#define Sqr(X) ((X) * (X))

byte onepix;
byte *bitmap,*hmap,*envmap;
word bitmapadd,hmapadd,emapadd;
int i,j,color,xcolcmp,ycolcmp;
int xgrad,ygrad,hmapx,hmapy;
float normx,normy,normz;
int lightx,lighty,lightxdiff,lightydiff ;
int deg;

void loadbmp(void);
void initmap(void);
void deinitmap(void);
void loadhmap(void);
void setpal(void);
void dobump(void);
void genenv(void);

word viradd;

main()
{
  initmode();
  initvirscr();
  viradd =(word) virscr;
  clrbuf(0, viradd);
  initmap();
  initytab();
  loadbmp();
  setpal();
  loadhmap();
  genenv();
//  swaptoscr(viradd);
//  getch();
//  exit(0);
  lightx=50;
  lighty=50;
  deg=0;
  while(!kbhit())
  {
    lightx=ceil(80*sin(1*pidiv180*deg))+160;
    lighty=ceil(80*sin(2*pidiv180*deg))+100;;
    deg+=2;

  for(hmapx=0;hmapx<=319;hmapx++)
    for(hmapy=1;hmapy<=199;hmapy++)
    {
      xgrad=hmap[(hmapx+1)+ytab[hmapy]]-hmap[(hmapx+1)+ytab[hmapy]];
      ygrad=hmap[hmapx+ytab[hmapy+1]]-hmap[hmapx+ytab[hmapy-1]];
      //normx=xgrad;
      //normy=ygrad;
      lightxdiff=abs(hmapx-lightx);
      lightydiff=abs(hmapy-lighty);

      if(Sqr(lightxdiff) + Sqr(lightydiff) <= Sqr(128))
      {
      xcolcmp=((xgrad+lightxdiff));
      if(xcolcmp<=0) xcolcmp=0;
      if(xcolcmp>=127) xcolcmp=127;
      if(lightxdiff>=127) xcolcmp=0;
      ycolcmp=((ygrad+lightydiff));
      if(ycolcmp<=0) ycolcmp=0;
      if(ycolcmp>=127) ycolcmp=127;
      if(lightydiff>=127) ycolcmp=0;
      color=xcolcmp+ycolcmp;
      //if (color>=127) color=127;
      //if (color<=0) color=0;
      //if (lightxdiff>158) color=200;
      //if (lightydiff>158) color=200;

//      printf("%d %d\n", xcolcmp, ycolcmp);
      char bgcol = bitmap[hmapx + hmapy *320];
      //char emapval = envmap[(128 - xcolcmp)+256*(128 - ycolcmp)];
      char emapval = envmap[xcolcmp+256*ycolcmp];
      plotpix(hmapx,hmapy,((float)emapval / 256.0)*128 + bgcol ,viradd);
      }
      else
        plotpix(hmapx,hmapy,bitmap[hmapx + hmapy *320],viradd);
	      
    }
  vtrace();
  swaptoscr(viradd);
  }



  inittext();
  freevirscr();
  deinitmap();
}

void loadbmp(void)
{
  FILE *bmpfile;
  tBITMAPINFO bfinfo;
  tBMPFILEHEADER bfhead;

  bmpfile=fopen("axbump3.bmp","r+b");
  fseek(bmpfile,0,SEEK_SET);//the 2nd value. +ve moves forward and vice
  fread(&bfhead,sizeof(bfhead),1,bmpfile);// get the bmp header
  fseek(bmpfile,14,SEEK_SET);//the 2nd value. +ve moves forward and vice
  fread(&bfinfo.bmiheader,sizeof(bfinfo),1,bmpfile);// get the bmp info header
  fseek(bmpfile,54,SEEK_SET);//the 2nd value. +ve moves forward and vice
  fread(&bfinfo.bmicolors,sizeof(bfinfo.bmicolors),1,bmpfile);// get the pal
  for(i=0;i<=255;i++) ///set the pal
    setcol(i,bfinfo.bmicolors[i].rgbred>>2,
	     bfinfo.bmicolors[i].rgbgreen>>2,
	     bfinfo.bmicolors[i].rgbblue>>2);
  for(i=0;i<=199;i++)
    for(j=0;j<=319;j++)
    {
      fread(&onepix,sizeof(onepix),1,bmpfile);//read 1 line
      bitmap[j+ytab[199-i]]=onepix;
    }
  fclose(bmpfile);
}

void initmap(void)
{
  int code = -1;

  bitmapadd = (word) malloc(64640); 
  bitmap = (byte *) bitmapadd;
//  code=allocmem(4096,&bitmapadd);//blocks allocated with allocmem are
//  bitmap=(byte *)MK_FP(bitmapadd,0);//paragraph aligned... ie start at ????:0000
			      //unlike calloc etc. which start at ????:0004
  if(code!=-1)
  {
    printf("Failed: maximum number of paragraphs available is \n");
    getch();
    exit(0);
  }
  clrbuf(0,bitmapadd);
  
  hmapadd = (word) malloc(64640);
  hmap = (byte *) hmapadd;
//  code=allocmem(4096,&hmapadd);//blocks allocated with allocmem are
//  hmap=(byte *)MK_FP(hmapadd,0);//paragraph aligned... ie start at ????:0000
			      //unlike calloc etc. which start at ????:0004
  if(code!=-1)
  {
    printf("Failed: maximum number of paragraphs available is \n");
    getch();
    exit(0);
  }
  clrbuf(0,hmapadd);

  emapadd = (word) malloc(256*256);
  envmap = (byte *) emapadd;
	  
//  code=allocmem(4096,&emapadd);//blocks allocated with allocmem are
//  envmap=(byte *)MK_FP(emapadd,0);//paragraph aligned... ie start at ????:0000
			      //unlike calloc etc. which start at ????:0004
  if(code!=-1)
  {
    printf("Failed: maximum number of paragraphs available is \n");
    getch();
    exit(0);
  }
  clrbuf(0,emapadd);

}

void deinitmap(void)
{
  free(bitmap);
  free(hmap);
  free(envmap);
}

void loadhmap(void)
{
  int i,j;

  for(i=0;i<=199;i++)
    for(j=0;j<=319;j++)
    {
      hmap[j+ytab[i]]=(256-bitmap[j+ytab[i]]);
    }
}

void setpal(void)
{
  int i;

  for(i=0;i<=255;i++)
    setcol(i,i>>2,i>>2,i>>2);
}

void dobump(void)
{
  lightx=50;
  lighty=50;
  for(hmapx=0;hmapx<=319;hmapx++)
    for(hmapy=0;hmapy<=199;hmapy++)
    {
      xgrad=hmap[(hmapx+1)+ytab[hmapy]]-hmap[(hmapx+1)+ytab[hmapy]];
      ygrad=hmap[hmapx+ytab[hmapy+1]]-hmap[hmapx+ytab[hmapy-1]];
      normx=xgrad;
      normy=ygrad;
      normz=sqrt(1-(normx*normx+normy*normy));






    }

}

void genenv(void)
{
  int i,j;

  float radius = sqrt(Sqr(128) + Sqr(128));
  for(i=0;i<=255;i++)
    for(j=0;j<=255;j++)
    {
      int curlength = (int)sqrt((double)((i-128)*(i-128))+(double)((j-128)*(j-128)));
      int color = ((float)curlength / radius) * 256;
      color=255-color;
      envmap[i+j*256]=color;
      //if (color>=255) color=255;
      //if (color<=0) color=0;
      //if (lightxdiff>158) color=200;
      //if (lightydiff>158) color=200;

      plotpix(i,j,color,viradd);
    }
}


