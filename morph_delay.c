// 3D OBJECT MORPH,
// DELAYDOT EFFECT USING PALETTE ROTATION

#include <conio.h>
#include "Ax3ddj.h"

struct coords{
	       int x,y,z;
	     };
struct step{
	       float sx,sy,sz;
	   };

void csteparr(void);
void rotateobj(int deg_ab_x, int deg_ab_y, int deg_ab_z);
void drawpts(void);
void erasepts(void);
void setupsrcobj(void);
void newobj(char objnum);
void morph1step(void);
void rotpal(void);
void initpal(void);
void incstuff(void);
void erasedelaydots(void);

#define STEPS 128
#define NOPTS 720
#define TRAILS 10
#define XDEGINC 4
#define YDEGINC 0
#define ZDEGINC 2
#define DELAYFACTOR 3

coords srcobj[NOPTS],destobj[NOPTS],rotobj[NOPTS],statobj[NOPTS];
step steparr[NOPTS];
int drawfb[TRAILS][NOPTS][2],oldpts[NOPTS][2];
int transarr[360][3];
int zdist,ptstoplot;
int i;
int xdeg=0,ydeg=0,zdeg=0;
char fbptr=TRAILS-1,erptr=0,delayflag=0;
int transptr;
byte pal[TRAILS][3];


main()
{
//  randomize();
  initytab();
  initvirscr();
  initsinus();
  clr16(0,viradd);
  initmode();
  setupsrcobj();
  initpal();
  zdist=300;
  while(!kbhit())
  {
    for(i=0;i<=90;i++)
    {
      delayflag=(delayflag+1)%DELAYFACTOR;
      if (!(delayflag==DELAYFACTOR-1))//erase trials just plotted in previous
	erasepts();                   //run
      if (delayflag==0)
	rotpal();
      drawpts();
      if (delayflag==DELAYFACTOR-1)
      {                               //erase set of pts at the end of FIFO
	erasedelaydots();             //buffer
	fbptr=(fbptr+1)%(TRAILS);     //increment saving and deletion ptrs
	erptr=(erptr+1)%(TRAILS);
      }
      vtrace();
      swap(viradd,0xA000);
      incstuff();
      rotateobj(xdeg,ydeg,zdeg);
      if (kbhit()) break;
    }
    newobj(random(7));

    csteparr();
    for(i=1;i<=STEPS;i++)
    {
      morph1step();
      delayflag=(delayflag+1)%DELAYFACTOR;
      if (!(delayflag==DELAYFACTOR-1))//erase trials just plotted in previous
	erasepts();                   //run
      if (delayflag==0)
	rotpal();
      drawpts();
      if (delayflag==DELAYFACTOR-1)
      {                               //erase set of pts at the end of FIFO
	erasedelaydots();             //buffer
	fbptr=(fbptr+1)%(TRAILS);     //increment saving and deletion ptrs
	erptr=(erptr+1)%(TRAILS);
      }

      vtrace();
      swap(viradd,0xA000);
      incstuff();
      rotateobj(xdeg,ydeg,zdeg);
      if (kbhit()) break;
    }
  }
  inittext();
  deinitvirscr();

  return 0;
}

void drawpts(void)
{
  int i,tx,ty,dist;

  for(i=0;i<=ptstoplot-1;i++)
  {
    dist=rotobj[i].z-zdist;
    tx=(int)((rotobj[i].x * 128)/dist+160);
    ty=(int)((rotobj[i].y * 128)/dist+100);
    if((tx>=0)&&(tx<320)&&(ty>=0)&&(ty<200))
      plotpix(tx,ytab[ty],TRAILS+1,viradd);
    if (delayflag==DELAYFACTOR-1)
    {
      drawfb[fbptr][i][0]=oldpts[i][0];  //save current set of delay trail
      drawfb[fbptr][i][1]=oldpts[i][1];  //pts so it could be erased later
      if((oldpts[i][0]>=0)&&(oldpts[i][0]<320)&&(oldpts[i][1]>=0)&&(oldpts[i][1]<200))
	plotpix(oldpts[i][0],ytab[oldpts[i][1]],fbptr+1,viradd);
    }  //draw the 1st delaydot trail with brightess colour in cycling palette
    oldpts[i][0]=tx;  //save pixels coords currently draw so they could
    oldpts[i][1]=ty;  //be erased in next run
  }
}

void erasepts(void) //erase trials just plotted in previous run
{
  int i;

  for(i=0;i<=ptstoplot-1;i++)
    plotpix(oldpts[i][0],ytab[oldpts[i][1]],0,viradd);
}

void erasedelaydots(void) //erase delaydot trials at end of buffer
{
  int i;

  for(i=0;i<=ptstoplot-1;i++)
    plotpix(drawfb[erptr][i][0],ytab[drawfb[erptr][i][1]],0,viradd);
}

void csteparr(void) //create step used to morph object
{
  int i;

  for(i=0;i<=NOPTS-1;i++)
    {
      statobj[i].x=srcobj[i].x;
      statobj[i].y=srcobj[i].y;
      statobj[i].z=srcobj[i].z;
      steparr[i].sx=(float)(destobj[i].x-srcobj[i].x)/STEPS;
      steparr[i].sy=(float)(destobj[i].y-srcobj[i].y)/STEPS;
      steparr[i].sz=(float)(destobj[i].z-srcobj[i].z)/STEPS;
    }
  return;
}

void rotateobj(int deg_ab_x, int deg_ab_y, int deg_ab_z) //object rotation
{
  int tempx,tempy,tempz,i;

  for(i=0;i<=NOPTS-1;i++)
  {
    // X-axis rotation //
    rotobj[i].y=srcobj[i].y*cosine[deg_ab_x]+srcobj[i].z*sine[deg_ab_x];
    rotobj[i].z=srcobj[i].z*cosine[deg_ab_x]-srcobj[i].y*sine[deg_ab_x];
    // X-axis rotation //

    // Y-axis rotation //
    tempx=srcobj[i].x*cosine[deg_ab_y]-rotobj[i].z*sine[deg_ab_y];
    rotobj[i].z=rotobj[i].z*cosine[deg_ab_y]+srcobj[i].x*sine[deg_ab_y];
    // Y-axis rotation //

    // Z-axis rotation //
    rotobj[i].x=tempx*cosine[deg_ab_z]-rotobj[i].y*sine[deg_ab_z];
    rotobj[i].y=rotobj[i].y*cosine[deg_ab_z]+tempx*sine[deg_ab_z];
    // Z-axis rotation //

    // TRANSLATION //
    rotobj[i].x+=transarr[transptr][0];
    rotobj[i].y+=transarr[transptr][1];
    rotobj[i].z+=transarr[transptr][2];
    // TRANSLATION //
  }
  return;
}

void setupsrcobj(void) //initialte objects the first time round
{                      //and create translation array
  int i, j;
  int wx, wy, wz;

  for(i=0;i<=24;i++)
    for(j=0;j<=24;j++)
    {
      srcobj[i+j*25].x=(i-12)*12;
      srcobj[i+j*25].y=(j-12)*12;
      srcobj[i+j*25].z=-70;
      statobj[i+j*25].x=srcobj[i+j*25].x;
      statobj[i+j*25].y=srcobj[i+j*25].y;
      statobj[i+j*25].z=srcobj[i+j*25].z;
      ptstoplot++;
    }
  wx=3;
  wy=3;
  wz=2;
  for(i=0;i<=359;i++)
  {
     transarr[i][0]=75*sin(wx*i*pidiv180);
     transarr[i][1]=75*sin(wy*i*pidiv180);
     transarr[i][2]=75*sin(wz*i*pidiv180);
  }
}


void newobj(char objnum)
{
  int i,j,k,theta,phi,px,py,pz;;

  switch (objnum)
  {

    case 0:   // CYLINDER

      ptstoplot=0;
      for(i=0;i<=24;i++)
	for(theta=0;theta<=24;theta++)
	{
	  destobj[theta+i*25].x=(int)(40*cosine[theta*15]);
	  destobj[theta+i*25].y=15*(i-12);
	  destobj[theta+i*25].z=(int)(40*sine[theta*15]);
	  ptstoplot+=1;
	}
      for(i=625;i<=719;i++)
      {
	destobj[i].x=0;
	destobj[i].y=0;
	destobj[i].z=0;
	ptstoplot+=1;
      }
      break;  // CYLINDER

    case 1:   //SPHERE

      ptstoplot=0;
      for(phi=0;phi<=24;phi++)
	for(theta=0;theta<=24;theta++)
	{
	  destobj[theta+phi*25].x=(int)(130*cosine[phi*15]*cosine[theta*15]);
	  destobj[theta+phi*25].y=(int)(130*sine[phi*15]);
	  destobj[theta+phi*25].z=(int)(130*cosine[phi*15]*sine[theta*15]);
	  ptstoplot+=1;
	}
      for(i=625;i<=719;i++)
      {
	destobj[i].x=0;
	destobj[i].y=0;
	destobj[i].z=0;
	ptstoplot+=1;
      }
      break;  // SPHERE

    case 2:   //CONE

      ptstoplot=0;
      for(i=0;i<=24;i++)
	for(theta=0;theta<=24;theta++)
	{
	  destobj[theta+i*25].x=(int)(3*(24-i)*cosine[theta*15]);
	  destobj[theta+i*25].y=14*(i-12);
	  destobj[theta+i*25].z=(int)(3*(24-i)*sine[theta*15]);
	  ptstoplot+=1;
	}
      for(i=625;i<=719;i++)
      {
	destobj[i].x=0;
	destobj[i].y=0;
	destobj[i].z=0;
	ptstoplot+=1;
      }
      break;  //CONE

    case 3:   //HOUR-GLASS

      ptstoplot=0;
      for(i=0;i<=24;i++)
	for(theta=0;theta<=24;theta++)
	{
	  destobj[theta+i*25].x=(int)(5*abs(12-i)*cosine[theta*15]);
	  destobj[theta+i*25].y=10*(i-12);
	  destobj[theta+i*25].z=(int)(5*abs(12-i)*sine[theta*15]);
	  ptstoplot+=1;
	}     //HOUR-GLASS
      for(i=625;i<=719;i++)
      {
	destobj[i].x=0;
	destobj[i].y=0;
	destobj[i].z=0;
	ptstoplot+=1;
      }
      break;

    case 4:   //TORUS

      ptstoplot=0;
      for(phi=0;phi<=24;phi++)
	for(theta=0;theta<=24;theta++)
	{
	  destobj[theta+phi*25].x=(int)(45*cosine[theta*15]);
	  destobj[theta+phi*25].y=(int)((45*sine[theta*15]+90)*cosine[phi*15]);
	  destobj[theta+phi*25].z=(int)((45*sine[theta*15]+90)*sine[phi*15]);
	  ptstoplot+=1;
	}
      for(i=625;i<=719;i++)
      {
	destobj[i].x=0;
	destobj[i].y=0;
	destobj[i].z=0;
	ptstoplot+=1;
      }
      break;  //TORUS

    case 5:   //CROWN
      ptstoplot=0;
      phi=0;
      for(theta=0;theta<=719;theta++)
      {
	destobj[theta].x=(int)(100*cosine[theta%360]);
	destobj[theta].y=(int)(20*sin(3*360*(float)phi/360*pidiv180));
	destobj[theta].z=(int)(100*sine[theta%360]);
	ptstoplot+=1;
	phi+=3;
      }
      break;  //CROWN


    case 6:  //lissajous figure
      px=random(15)+2;py=random(15)+2;pz=random(15)+2;
      ptstoplot=0;
      for(i=0;i<=719;i++)
      {
	destobj[i].x=(int)(80*sin(px*pidiv180*i/2));
	destobj[i].y=(int)(80*sin(py*pidiv180*i/2));
	destobj[i].z=(int)(80*sin(pz*pidiv180*i/2));
	ptstoplot++;
      }
      break; //lissajous figure
  }
}


void morph1step(void) //morph one step closer to destination object
{
  int index;

  for(index=0;index<=NOPTS-1;index++)
  {
    srcobj[index].x=(int)(statobj[index].x+steparr[index].sx*i);
    srcobj[index].y=(int)(statobj[index].y+steparr[index].sy*i);
    srcobj[index].z=(int)(statobj[index].z+steparr[index].sz*i);
  }
}


void rotpal(void)     //rotates palette
{
  unsigned char temp[3],i;

  temp[0]=pal[TRAILS-1][0];
  temp[1]=pal[TRAILS-1][1];
  temp[2]=pal[TRAILS-1][2];
  memmove(pal[1],pal[0],TRAILS*3);
  pal[0][0]=temp[0];
  pal[0][1]=temp[1];
  pal[0][2]=temp[2];
  for(i=1;i<=TRAILS;i++)
    setcol(i,pal[i-1][0],pal[i-1][1],pal[i-1][2]);


}

void initpal(void)    //sets up the palette
{
  int i;

  for(i=1;i<=TRAILS;i++)
  {
    pal[i][0]=0;//i*63/TRAILS/2;
    pal[i][1]=0;//i*63/TRAILS;
    pal[i][2]=i*63/TRAILS;
  }
  for(i=1;i<=TRAILS;i++)
    setcol(i,pal[i][0],pal[i][1],pal[i][2]);
  setcol(TRAILS+1,32,63,63);
}

void incstuff(void)   //increments stuff needed in every run
{
  xdeg=(xdeg+XDEGINC)%360;
  ydeg=(ydeg+YDEGINC)%360;
  zdeg=(zdeg+ZDEGINC)%360;
  transptr=(transptr+1)%360;
}
