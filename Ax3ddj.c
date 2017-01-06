/////////////////////////////////////////////////////////////////////////////
//                     VGA Mode 13h Codes By A x I o M                     //
/////////////////////////////////////////////////////////////////////////////

//Ported to djgpp...which nearly made me pull my hair out...
//Support for 32bit registers using AT&T assembler syntax

#include <dpmi.h>         // included to use __dpmi_regs
#include <farptr.h>   // for using farpoke family of functions
#include <nearptr.h>  // for using "near ptr" like access
#include <go32.h>         // for _dos_ds
#include <stdio.h>        // for file IO
#include <stdlib.h>        // for file IO
#include <conio.h>        // for file IO
#include <math.h>         // for the sqrt function
#include <time.h>         // for the time function
#include "Ax3ddj.h"

void initmode(void)
{
  __dpmi_regs r;

  r.x.ax=0x13;
  __dpmi_int(0x10,&r);
}

void inittext(void)
{
  __dpmi_regs r;

  r.x.ax=0x03;
  __dpmi_int(0x10,&r);
}

void initytab(void)
{
  byte i;

  for(i=0 ;i<=199 ;i++)
    ytab[i]=i*320;
  return;
}

void initsinus(void)
{
  int i;

  for(i=0;i<=720;i++)
  {
    sine[i] =(double) sin( (double) pidiv180*i/2.0);
    cosine[i] =(double) cos((double) pidiv180*i/2.0);
  }
  return;
}

void initvirscr(void)
{
  virscr=(byte *)malloc(64640);

  if(virscr==0)
  {
    printf("Not Enough Memory\n");
    getch();
    exit(1);
  }
}

void initsystem(void)
{

  printf("Allocating memory for screen buffer...");
  initvirscr();
  clrbuf(0, (unsigned int) virscr);
  printf("  Ok.\n");

  printf("Creating lookup tables for y values...");
  initytab();
  printf("  Ok.\n");

  printf("Initialising lookup tables for sinus...");
  initsinus();
  printf("  Ok.\n");

  printf("Initialising random number generator...");
  srand(time(0));
  printf("  Ok.\n");

}

void freevirscr(void)
{
  free(virscr);
}



void swapint(int *a,int *b)// &x declares a ref. X is an alias for passed val
{                          // ie X=1 == x1=1
  int tem;

  tem=*a;
  *a=*b;
  *b=tem;
  return;
}

void swapfloat(float *a,float *b)// &x declares a ref. X is an alias for passed val
{                          // ie X=1 == x1=1
  float tem;

  tem=*a;
  *a=*b;
  *b=tem;
  return;
}

void vtrace(void)
{
  __asm__ __volatile__
  ("movw $0x3DA,%%dx\n\t"
"i1:\n\t"
    "inb  %%dx,%%al\n\t"   /*gets port val and perform AND, when we are in the*/
    "test $0x08,%%al\n\t"  /*middle of a retrace, port val is always 0x80 and the*/
    "jnz  i1\n\t"          /*AND returns true so we jump back and test again until*/
                     /*retrace is over...*/
"i2:\n\t"
    "inb  %%dx,%%al\n\t"   /*gets port val and perform AND, when we are in the*/
    "test $0x08,%%al\n\t"  /*midst of drawing a screen, port val is not 0x80,*/
    "jz   i2\n\t"         /*AND returns false so we jump back and test again until*/
                     /*the drawing is over so we'll be at the start of a vtrc*/
                     /*once we get out*/
    :
    :
    :"eax","edx"
  );
}

void htrace(void)
{
  __asm__ __volatile__
  ("movw $0x3DA,%%dx\n\t"
"i3:\n\t"
    "inb  %%dx,%%al\n\t"      /*gets port val and perform AND, when we are in the*/
    "test $0x01,%%al\n\t"     /*middle of a htrace, port val is always 0x80 and the*/
    "jnz  i3\n\t"          /*AND returns true so we jump back and test again until*/
                      /*retrace is over...*/
"i4:\n\t"
    "inb  %%dx,%%al\n\t"      /*gets port val and perform AND, when we are in the*/
    "test $0x01,%%al\n\t"     /*midst of drawing a line, port val is not 0x80,*/
    "jz   i4\n\t"           /*AND returns false so we jump back and test again until*/
                     /*the drawing is over so we'll be at the start of a vtrc*/
                      /*once we get out*/
    :
    :
    :"eax","edx"
  );
}

void setcol(byte ColNo, byte r, byte g, byte b)
{
  __asm__ __volatile__
  ( 
   "movw $0x03C8,%%edx\n\t" /*----------------------------*/
   "movb %0,%%eax\n\t"      /*selects col no*/
   "outb %%al,%%dx\n\t"     /*----------------------------*/
   "incw  %%edx\n\t"
   "movb %1,%%eax\n\t"
   "outb %%eax,%%dx\n\t"     /*output r value to port*/
   "movb %2,%%eax\n\t"
   "outb %%eax,%%dx\n\t"     /*output g value to port*/
   "movb %3,%%eax\n\t"
   "outb %%eax,%%dx\n\t"     /*output b value to port*/
   
   :
   :"g" (ColNo),"g" (r),"g" (g),"g" (b) //pre load regs.....
   :"eax","edx"
  );
}


void showpal(void)
{
  short i;

  for(i=0;i<=255;i++)
    //plotpix1(i,0,i,0xA000);
    plotpix(i,0,i,0xA0000);
  getch();
}


void plotpix(int x,int y,word color, int Addr)
{
  if(Addr==0xA0000)
  {
    __asm__ __volatile__
    (
                             /*CS is the selector to the code segment*/
     "movl %1,%%ebx\n\t"            /*DS ES FS is the selector for the data segment*/
     "movl $0xA0000,%%edi\n\t"      /*GS starts from the beginning of mem*/
     "movw %2,%%eax\n\t"            /*must use movw... cannot do using movb why*/
     "addl %0,%%ebx\n\t"
     "addl %%ebx,%%edi\n\t"
     "movw %%eax,%%gs:(%%edi)\n\t"
     
      :
      : "g" (x), "g" (ytab[y]), "g" (color)
      : "edi","eax","ebx"
    );
  }
  else
  {
    __asm__ __volatile__
    (
     "movl %1,%%ebx\n\t"             /* ebx = ytab[y]                  */
     "movl %3,%%edi\n\t"             /* edi = Addr                     */
     "movl %2,%%eax\n\t"             /* eax = color                    */
     "addl %0,%%ebx\n\t"             /* ebx += x                       */
     "addl %%ebx,%%edi\n\t"          /* edi += ebx                     */
     "movb %%eax,%%es:(%%edi)\n\t"   /* place color val in eax in dest */
 
//     "movl %1,%%ebx\n\t"             /* ebx = ytab[y]                  */
//     "movl %3,%%edi\n\t"             /* edi = Addr                     */
//     "movl %2,%%eax\n\t"             /* eax = color                    */
//     "addl %0,%%ebx\n\t"             /* ebx += x                       */
//     "addl %%ebx,%%edi\n\t"          /* edi += ebx                     */
//     "movb %%eax,%%es:(%%edi)\n\t"   /* place color val in eax in dest */
     //"movb %%eax,%%es:(%%edi)\n\t"   /* place color val in eax in dest */

      :
      : "g" (x), "g" (ytab[y]), "g" (color), "g" (Addr),"g" (_dos_ds)
      : "edi","eax","ebx"
    );
  }
}

void plotpix1(int x, int y, byte Col, int Addr)
{//plot pixel using farpokeb
  _farpokeb(_dos_ds,0xA0000+ytab[y]+x,Col);
}

void plotpix2(int x, int y, byte Col, int Addr)
{//plot pixel using pointer like access
  int tem;

  tem=__djgpp_conventional_base;
  if(__djgpp_nearptr_enable()==0) //enable is SLOW!
    exit(0);
  vgascr[ytab[y]+x+__djgpp_conventional_base]=Col;
  __djgpp_nearptr_disable();
}


void plotpix3(int x,int y,int color,int Addr)
{
  if(Addr==0xA0000)
  {
    __asm__ __volatile__
    (
     "push %%es\n\t"
     "movw %3,%%es\n\t"
     "movl $0xA0000,%%edi\n\t"
     "movw %1,%%eax\n\t"
     "addw %0,%%eax\n\t"
     "addw %%eax,%%edi\n\t"
     "movb %2,%%eax\n\t"
     "stosb\n\t"
     "pop  %%es\n\t"
      
      :
      : "g" (x), "g" (ytab[y]), "g" (color),"g" (_dos_ds)
      : "edi","eax"
    );
  }
  else
  {
    __asm__ __volatile__
    (
     "movw %1,%%eax\n\t"
     "movl %4,%%edi\n\t"
     "addw %0,%%eax\n\t"
     "addw %%eax,%%edi\n\t"
     "movb %2,%%eax\n\t"
     "stosb\n\t"
     
      :
      : "g" (x), "g" (ytab[y]), "g" (color),"g" (_dos_ds),"g" (Addr)
      : "edi","ax"
    );
  }
}

void swaptoscr(int Src)
{
  __asm__ __volatile__
  (
   "push %%es\n\t"
   "movl $0xA0000,%%edi\n\t" /*set the destination index*/
   "movw %1,%%es\n\t"        /*set the selector for the data segment*/
   "movl %0,%%esi\n\t"       /*place starting add of buffer in esi*/
   "movl $16000,%%ecx\n\t"   /*no of times to perform movl*/
   "rep\n\t"
   "movsl\n\t"
   "pop  %%es\n\t"
  
  :
  : "g" (Src), "g" (_dos_ds)
  : "edi","eax","esi"
  );
}

void swapbufs(word Src,word Dest)
{
  __asm__ __volatile__
  (
//   "push %%es\n\t"    // change on june 18 by Axiom
//   "movw %2,%%es\n\t"        /*set the selector for the data segment*/
   "movl %1,%%edi\n\t"
   "movl %0,%%esi\n\t"
   "movl $16000,%%ecx\n\t"
   "rep movsl\n\t"
//   "pop  %%es\n\t"
  
  :
  : "g" (Src), "g" (Dest), "g" (_dos_ds)
  : "edi","esi"
  );
}

void clrvidscr(byte Col)
{
  __asm__ __volatile__
  (
   "push %%es\n\t"
   "movl %0,%%eax\n\t"
   "movl $0xA0000,%%edi\n\t"
   "movb %%al,%%ah\n\t"
   "movl %%eax,%%ebx\n\t"
   "movw %1,%%es\n\t"
   "shl  $16,%%eax\n\t"
   "movw %%ebx,%%eax\n\t"
   "movl $16000,%%ecx\n\t"
   "rep\n\t"
   "stosl\n\t"
   "pop  %%es\n\t"
  
  :
  : "g" (Col), "g" (_dos_ds)
  : "edi","eax","ecx","ebx"
  );
}

void clrbuf(int Col,word Addr)
{
  __asm__ __volatile__
  (
   "movl %0,%%eax\n\t"
   "movl %1,%%edi\n\t"
   "movb %%al,%%ah\n\t"
   "movl %%eax,%%ebx\n\t"
   "shl  $16,%%eax\n\t"
   "movw %%ebx,%%eax\n\t"
   "movl $16160,%%ecx\n\t"
   "rep\n\t"
   "stosl\n\t"
  
  :
  : "g" (Col), "g" (Addr)
  : "edi","eax","ecx","ebx"
  );
}

void line(short x1, short y1, short x2, short y2, byte Col, word Addr)
{
  int dy,dx,xstepup,xstepdown,ystepup,ystepdown,
      dincup,dincdown,PtsToPlot,decision,x,y,i;

  dy=abs(y1-y2);             // y length of the line
  dx=abs(x1-x2);             // x length of the line
  if(dx>=dy)                 // xlength >= ylength ie. gradient < 1
  {
    PtsToPlot=dx+1;          // calculate the no. of pts to plot
    decision=(dy << 1) - dx; // calculate the decision variable for 1st midpt
    dincup=(dy-dx) << 1;     // amount to inc decision by if top pixel is chos
    dincdown=dy << 1;        // amount to inc decision by if bottom pixel is c
    xstepup=1;               // amount to inc x by if top pixel is chosen
    xstepdown=1;             // amount to inc x by if bottom pixel is chosen
    ystepup=1;               // amount to inc y by if top pixel is chosen
    ystepdown=0;             // amount to inc y by if bottom pixel is chosen
  }
  else
  {
    PtsToPlot=dy+1;          // calculate the no. of pts to plot
    decision=(dx << 1) - dy; // calculate the decision variable for 1st midpt
    dincup=(dx-dy) << 1;     // amount to inc decision by if top pixel is chos
    dincdown=dx << 1;        // amount to inc decision by if bottom pixel is c
    xstepup=1;               // amount to inc x by if top pixel is chosen
    xstepdown=0;             // amount to inc x by if bottom pixel is chosen
    ystepup=1;               // amount to inc y by if top pixel is chosen
    ystepdown=1;             // amount to inc y by if bottom pixel is chosen
  }
  if(x1>x2)
  {
    xstepup=-xstepup;
    xstepdown=-xstepdown;
  }
  if(y1>y2)
  {
    ystepup=-ystepup;
    ystepdown=-ystepdown;
  }
  x=x1;
  y=y1;
  for(i=1; i<=PtsToPlot; i++)
  {
    plotpix(x,y,Col,Addr);
    if(decision<=0)
    {
      decision=decision+dincdown;
      x=x+xstepdown;
      y=y+ystepdown;
    }
    else
    {
      decision=decision+dincup;
      x=x+xstepup;
      y=y+ystepup;
    }

  }
}

void asmline2(int x1, int y1, int x2, int y2, int col, word Location)
{
  int temp=ytab[y1];

  if(Location==0xA0000)
  {
    __asm__ __volatile__
    (                        /*initialization of variables*/
     "movl  $1,_xmove_if_up\n\t"
     "movl  %6,%%edi\n\t"
     "movl  $320,_ymove_if_up\n\t"  /*sets es to selector for flat mem*/
     "addl  %1,%%edi\n\t"
     "movl  $1,_xmove_if_down\n\t"
     "addl  $0xA0000,%%edi\n\t"     /*sets edi to point to the first pixel to plot*/
     "movl  $320,_ymove_if_down\n\t"
      /*movw  %0,%%es*/
                            /*initialization of variables*/

		       /*deltax:= abs(x1-x2); calculates delta x*/
     "movl  %1,%%eax\n\t"   /*ax:=x1*/
     "subl  %3,%%eax\n\t"   /*ax:=ax-x2 ie ax:=x1-x2*/
     "jns   skip1\n\t"      /*if result is not -ve then jump*/
     "negl    %%eax\n\t"      /*else if result i -ve, then neg ax for absolute value*/
             	       /*ax == deltax*/
     "jmp   calcdy\n\t"
		    /*deltax:= abs(x1-x2); */

"skip1:\n\t"
      "negl  _xmove_if_up\n\t"   /*xmove_if_up:=-xmove_if_up,since x1>x2*/
      "negl  _xmove_if_down\n\t" /*xmove_if_down:=-xmove_if_down,since x1>x2*/

"calcdy:\n\t"             /*deltay:= abs(y1-y2);* calculates delta y*/

      "movl  %2,%%ebx\n\t"    /*ax:=y1*/
      "subl  %4,%%ebx\n\t"    /*ax:=ax-y2 ie ax:=y1-y2*/
      "jns   skip2\n\t"       /*if result is not -ve then jump*/
      "negl   %%ebx\n\t"       /*else if result i -ve, then neg bx for absolute value*/
  		     /*bx == deltay*/
      "jmp   cmpdxndy\n\t"
     		    /*deltay:= abs(y1-y2);*/

"skip2:\n\t"
      "negl  _ymove_if_up\n\t"      /*ymove_if_up:=-ymove_if_up,since y1>y2*/
      "negl  _ymove_if_down\n\t"    /*ymove_if_down:=-ymove_if_down,since y1>y2*/

"cmpdxndy:\n\t"

      "cmpl  %%ebx,%%eax\n\t"            /*compare ax and bx*/
      "jae   dxgdy\n\t"                  /*jump if ax >= bx ie. deltax>=deltay*/
		                 /*else if ax<bx then*/
      "movl   $0,_xmove_if_down\n\t"      /*xmove_if_down:=0*/
     /* testl  $0x80000000,_ymove_if_down test if ymove_if_down is -ve test 0x8000*/
      "testl  $-1,_xmove_if_down\n\t" /*test if xmove_if_down is -ve test 0x8000*/
      "jz     postve\n\t"                 /*if +ve then jmp, no change*/
      "movl   $-320,_ymove_if_down\n\t"   /*if -ve then := -ve value*/


"postve:\n\t"
      "movl  %%ebx,%%ecx\n\t"    /*cx:=bx ie. points_to_plot:=deltay*/
      "movl  %%eax,%%edx\n\t"    /*dx := ax        ie. decide:=deltax*/
      "shll   $1,%%edx\n\t"       /*dx := dx*2      ie. decide:=deltax shl 1*/
      "subl  %%ebx,%%edx\n\t"    /*dx := dx-bx     ie. decide:=(deltax shl 1)-deltay*/
  		         /*bx=inc_up:=(deltax - deltay) shl 1;*/
      "movl  %%eax,%%ebx\n\t"    /*bx:=deltax*/
      "subl  %%ecx,%%ebx\n\t"    /*bx:=deltax-deltay*/
      "shll   $1,%%ebx\n\t"       /*bx:=(deltax-deltay) shl 1*/
	   	         /*bx=inc_up:=(deltax - deltay) shl 1;*/
      "shll   $1,%%eax\n\t"       /*ax:=ax*2 ie. ax==inc_down:=deltax shl 1*/
      "incl   %%ecx\n\t"          /*cx:=cx+1 ie. cx=points_to_plot:=deltay+1*/
      "xchgl %%eax,%%ebx\n\t"    /*swap ax and bx*/

		         /*ax=inc_up*/
		         /*bx=inc_down*/
		         /*dx=decide*/
		         /*cx=points_to_plot*/
      "jmp  blast_to_scr\n\t"

"dxgdy:\n\t"                          /*when deltax>=deltay*/
      "movl  $0,_ymove_if_down\n\t"      /*ymove_if_down:=0*/
/*      testl  $0x80000000,_ymove_if_down test if ymove_if_down is -ve test 0x8000*/
      "testl  $-1,_ymove_if_down\n\t" /*test if ymove_if_down is -ve test 0x8000*/
      "jz    postve2\n\t"                 /*if +ve then jmp, no change*/
      "movl  $-1,_xmove_if_down\n\t"     /*if -ve then := -ve value*/
                                /*when deltax>=deltay*/
"postve2:\n\t"
      "movl  %%eax,%%ecx\n\t"  /*cx:=ax ie. points_to_plot:=deltax*/
      "movl  %%ebx,%%edx\n\t"  /*dx:=bx ie. decide:=deltay*/
      "shll   $1,%%edx\n\t"     /*dx:=dx*2 ie. decide:=deltay shl 1*/
      "subl  %%eax,%%edx\n\t"  /*dx:=dx-ax ie. decide:=(deltay shl 1)-deltax*/
	      	          /*ax=inc_up:=(deltay - deltax) shl 1;*/
      "movl  %%ebx,%%eax\n\t"  /*ax:=deltay*/
      "subl  %%ecx,%%eax\n\t"  /*ax:=deltay-deltax*/
      "shll   $1,%%eax\n\t"     /*ax:=(deltay-deltax) shl 1*/
          	          /*ax=inc_up:=(deltay - deltax) shl 1;*/
      "shll   $1,%%ebx\n\t"     /*bx:=bx*2 ie. bx==inc_down:=deltay shl 1*/
      "incl   %%ecx\n\t"        /*cx:=cx+1 ie. cx=points_to_plot:=deltax+1*/
		          /*ax=inc_up*/
		          /*bx=inc_down*/
		          /*dx=decide*/
		          /*cx=points_to_plot*/
		          /*deltax>=deltay*/

"blast_to_scr:\n\t"
      "movl  %%eax,_inc_up\n\t"
      "movl  %5,%%eax\n\t"           /*mov al,[col]*/
"comp:\n\t"
      "movb  %%eax,%%gs:(%%edi)\n\t" /*mov es:[di],al...possible prob*/
      "cmpl   $0,%%edx\n\t"          /*gs contains by default the selector for flat*/
      "jle   dn\n\t"
"up:\n\t"
      "addl  _inc_up,%%edx\n\t"
      "addl   _xmove_if_up,%%edi\n\t"
      "addl   _ymove_if_up,%%edi\n\t"
      "jmp   rpt\n\t"
"dn:\n\t"
      "addl  %%ebx,%%edx\n\t"
      "addl  _xmove_if_down,%%edi\n\t"
      "addl  _ymove_if_down,%%edi\n\t"

"rpt:\n\t"
      "decl  %%ecx\n\t"
      "jnz  comp\n\t"

      
    :
    : "g" (_dos_ds),"g" (x1), "g" (y1), "g" (x2), "g" (y2), "g" (col), "g" (ytab[y1])
    : "edi"/*,"eax"*/,"ebx","ecx","edx","memory"
    );
  }
  else
  {
    __asm__ __volatile__
    (                        /*initialization of variables*/
      "movl  $1,_xmove_if_up\n\t"
      "movl  %6,%%edi\n\t"
      "movl  $320,_ymove_if_up\n\t"  /*sets es to selector for flat mem*/
      "addl  %1,%%edi\n\t"
      "movl  $1,_xmove_if_down\n\t"
      "addl  %0,%%edi\n\t"     /*sets edi to point to the first pixel to plot*/
      "movl  $320,_ymove_if_down\n\t"
      /*movw  %0,%%es*/
                            /*initialization of variables*/

		    /*deltax:= abs(x1-x2); calculates delta x*/
      "movl  %1,%%eax\n\t"   /*ax:=x1*/
      "subl  %3,%%eax\n\t"   /*ax:=ax-x2 ie ax:=x1-x2*/
      "jns   skip1_2\n\t"      /*if result is not -ve then jump*/
      "negl    %%eax\n\t"      /*else if result i -ve, then neg ax for absolute value*/
             	     /*ax == deltax*/
      "jmp   calcdy_2\n\t"
		    /*deltax:= abs(x1-x2); */

"skip1_2:\n\t"
      "negl  _xmove_if_up\n\t"   /*xmove_if_up:=-xmove_if_up,since x1>x2*/
      "negl  _xmove_if_down\n\t" /*xmove_if_down:=-xmove_if_down,since x1>x2*/

"calcdy_2:\n\t"             /*deltay:= abs(y1-y2);* calculates delta y*/

      "movl  %2,%%ebx\n\t"    /*ax:=y1*/
      "subl  %4,%%ebx\n\t"    /*ax:=ax-y2 ie ax:=y1-y2*/
      "jns   skip2_2\n\t"       /*if result is not -ve then jump*/
      "negl   %%ebx\n\t"       /*else if result i -ve, then neg bx for absolute value*/
  		     /*bx == deltay*/
      "jmp   cmpdxndy_2\n\t"
     		    /*deltay:= abs(y1-y2);*/

"skip2_2:\n\t"
      "negl  _ymove_if_up\n\t"      /*ymove_if_up:=-ymove_if_up,since y1>y2*/
      "negl  _ymove_if_down\n\t"    /*ymove_if_down:=-ymove_if_down,since y1>y2*/

"cmpdxndy_2:\n\t"

      "cmpl  %%ebx,%%eax\n\t"            /*compare ax and bx*/
      "jae   dxgdy_2\n\t"                  /*jump if ax >= bx ie. deltax>=deltay*/
		                 /*else if ax<bx then*/
      "movl   $0,_xmove_if_down\n\t"      /*xmove_if_down:=0*/
      "testl  $0x80000000,_ymove_if_down\n\t" /*test if ymove_if_down is -ve test 0x8000*/
      "jz     postve_2\n\t"                 /*if +ve then jmp, no change*/
      "movl   $-320,_ymove_if_down\n\t"   /*if -ve then := -ve value*/


"postve_2:\n\t"
      "movl  %%ebx,%%ecx\n\t"    /*cx:=bx ie. points_to_plot:=deltay*/
      "movl  %%eax,%%edx\n\t"    /*dx := ax        ie. decide:=deltax*/
      "shll   $1,%%edx\n\t"       /*dx := dx*2      ie. decide:=deltax shl 1*/
      "subl  %%ebx,%%edx\n\t"    /*dx := dx-bx     ie. decide:=(deltax shl 1)-deltay*/
  		         /*bx=inc_up:=(deltax - deltay) shl 1;*/
      "movl  %%eax,%%ebx\n\t"    /*bx:=deltax*/
      "subl  %%ecx,%%ebx\n\t"    /*bx:=deltax-deltay*/
      "shll   $1,%%ebx\n\t"       /*bx:=(deltax-deltay) shl 1*/
	   	         /*bx=inc_up:=(deltax - deltay) shl 1;*/
      "shll   $1,%%eax\n\t"       /*ax:=ax*2 ie. ax==inc_down:=deltax shl 1*/
      "incl   %%ecx\n\t"          /*cx:=cx+1 ie. cx=points_to_plot:=deltay+1*/
      "xchgl %%eax,%%ebx\n\t"    /*swap ax and bx*/

		         /*ax=inc_up*/
		         /*bx=inc_down*/
		         /*dx=decide*/
		         /*cx=points_to_plot*/
      "jmp  blast_to_scr_2\n\t"

"dxgdy_2:\n\t"                          /*when deltax>=deltay*/
      "movl  $0,_ymove_if_down\n\t"      /*ymove_if_down:=0*/
      "testl $0x80000000,_xmove_if_down\n\t" /*test if xmove_if_down is -ve*test 0x8000*/
      "jz    postve2_2\n\t"                 /*if +ve then jmp, no change*/
      "movl  $-1,_xmove_if_down\n\t"     /*if -ve then := -ve value*/
                                /*when deltax>=deltay*/
"postve2_2:\n\t"
      "movl  %%eax,%%ecx\n\t"  /*cx:=ax ie. points_to_plot:=deltax*/
      "movl  %%ebx,%%edx\n\t"  /*dx:=bx ie. decide:=deltay*/
      "shll   $1,%%edx\n\t"     /*dx:=dx*2 ie. decide:=deltay shl 1*/
      "subl  %%eax,%%edx\n\t"  /*dx:=dx-ax ie. decide:=(deltay shl 1)-deltax*/
	      	          /*ax=inc_up:=(deltay - deltax) shl 1;*/
      "movl  %%ebx,%%eax\n\t"  /*ax:=deltay*/
      "subl  %%ecx,%%eax\n\t"  /*ax:=deltay-deltax*/
      "shll   $1,%%eax\n\t"     /*ax:=(deltay-deltax) shl 1*/
          	          /*ax=inc_up:=(deltay - deltax) shl 1;*/
      "shll   $1,%%ebx\n\t"     /*bx:=bx*2 ie. bx==inc_down:=deltay shl 1*/
      "incl   %%ecx\n\t"        /*cx:=cx+1 ie. cx=points_to_plot:=deltax+1*/
		          /*ax=inc_up*/
		          /*bx=inc_down*/
		          /*dx=decide*/
		          /*cx=points_to_plot*/
		          /*deltax>=deltay*/

"blast_to_scr_2:\n\t"
      "movl  %%eax,_inc_up\n\t"
      "movl  %5,%%eax\n\t"           /*mov al,[col]*/
"comp_2:\n\t"
      "movb  %%eax,%%ds:(%%edi)\n\t" /*mov es:[di],al*/
      "cmpl   $0,%%edx\n\t"          /*gs contains by default the selector for flat*/
      "jle   dn_2\n\t"
"up_2:\n\t"
      "addl  _inc_up,%%edx\n\t"
      "addl   _xmove_if_up,%%edi\n\t"
      "addl   _ymove_if_up,%%edi\n\t"
      "jmp   rpt_2\n\t"
"dn_2:\n\t"
      "addl  %%ebx,%%edx\n\t"
      "addl  _xmove_if_down,%%edi\n\t"
      "addl  _ymove_if_down,%%edi\n\t"

"rpt_2:\n\t"
      "decl  %%ecx\n\t"
      "jnz  comp_2\n\t"
    
    :
    : "g" (Location),"g" (x1), "g" (y1), "g" (x2), "g" (y2), "g" (col), "g" (ytab[y1])
    : "edi",/*"eax",*/"ebx","ecx","edx","memory"
    );
  }
}

void hline(int x1, int y, int x2, int Col, word Addr)
{
  if(Addr==VGA)
  {
    __asm__ __volatile__
    (
      "push  %%es\n\t"
      "movl  $0xA0000,%%edi\n\t" /* di=y*320+x*/
      "movw  %0,%%es\n\t"      /*load es with selector*/
      "movl  %1,%%eax\n\t"     /* ax=x1*/
      "movl  %3,%%ebx\n\t"     /* bx=x2*/
      "cmpl  %%ebx,%%eax\n\t"  /* ax-bx, flags set indicating if ax<bx or ax>bx*/
      "jb    x1lx2\n\t"        /* jmp if below, jmp if ax<bx ie. x1<x2*/

"x1gx2:\n\t"                  /* x1 > x2*/
      "addl   %5,%%edi\n\t"      /* initialise di*/
      "movl   %%eax,%%ecx\n\t"   /* cx=ax==x1*/
      "addl   %3,%%edi\n\t"      /* di=y*320+x*/
      "subl   %%ebx,%%ecx\n\t"   /* cx=cx-bx ie. cx=x1-x2*/
      "incl   %%ecx\n\t"         /* cx=cx+1, to get the no of pts to plot*/
      "movl   %4,%%eax\n\t"      /* al=col*/
      "movb   %%al,%%ah\n\t"     /* ah=al==col (for use with stosw}*/
      "shrl   $1,%%ecx\n\t"      /* divide by 2 since we're using rep stosw*/
      "jnc    blast\n\t"         /* even no of points*/
      "jmp    plot1\n\t"         /* odd no of pts, so plot 1 pt, then rep plot 2*/
                         /* using rep stosw*/
"x1lx2:\n\t"                      /* x1 < x2*/
      "addl   %5,%%edi\n\t"      /* initialise di*/
      "movl   %%ebx,%%ecx\n\t"   /* cx=ax==x2*/
      "addl   %1,%%edi\n\t"      /* di=y*320+x*/
      "subl   %%eax,%%ecx\n\t"   /* cx=cx-ax ie. cx=x2-x1*/
      "incl   %%ecx\n\t"         /* cx=cx+1, to get the no of pts to plot*/
      "movl   %4,%%eax\n\t"      /* al=col*/
      "movb   %%al,%%ah\n\t"     /* ah=al==col (for use with stosw}*/
      "shrl   $1,%%ecx\n\t"      /* divide by 2 since we're using rep stosw*/
      "jnc    blast\n\t"         /* even no of points*/
      "jmp    plot1\n\t"         /* odd no of pts, so plot 1 pt, then rep plot 2*/
                            /* using rep stosw*/
"plot1:\n\t"
      "stosb\n\t"

"blast:\n\t"
      "rep\n\t"
      "stosw\n\t"
      "pop  %%es\n\t"
  
  :
  : "g" (_dos_ds), "g" (x1), "g" (y), "g" (x2), "g" (Col), "g" (ytab[y])
  : "edi","eax","ebx","ecx","memory"
  );
  }
  else
  {
    __asm__ __volatile__
    (
      "push  %%es\n\t"
      "movl  %0,%%edi\n\t"
      "movw  %%ds,%%ax\n\t"
      "movw  %%ax,%%es\n\t"
      "movl  %1,%%eax\n\t"     /* ax=x1*/
      "movl  %3,%%ebx\n\t"     /* bx=x2*/
      "cmpl  %%ebx,%%eax\n\t"  /* ax-bx, flags set indicating if ax<bx or ax>bx*/
      "jb    x1lx2_2\n\t"        /* jmp if below, jmp if ax<bx ie. x1<x2*/

"x1gx2_2:\n\t"                  /* x1 > x2*/
      "addl   %5,%%edi\n\t"      /* initialise di*/
      "movl   %%eax,%%ecx\n\t"   /* cx=ax==x1*/
      "addl   %3,%%edi\n\t"      /* di=y*320+x*/
      "subl   %%ebx,%%ecx\n\t"   /* cx=cx-bx ie. cx=x1-x2*/
      "incl   %%ecx\n\t"         /* cx=cx+1, to get the no of pts to plot*/
      "movl   %4,%%eax\n\t"      /* al=col*/
      "movb   %%al,%%ah\n\t"     /* ah=al==col (for use with stosw}*/
      "shrl   $1,%%ecx\n\t"      /* divide by 2 since we're using rep stosw*/
      "jnc    blast_2\n\t"         /* even no of points*/
      "jmp    plot1_2\n\t"         /* odd no of pts, so plot 1 pt, then rep plot 2*/
                         /* using rep stosw*/
"x1lx2_2:\n\t"                      /* x1 < x2*/
      "addl   %5,%%edi\n\t"      /* initialise di*/
      "movl   %%ebx,%%ecx\n\t"   /* cx=ax==x2*/
      "addl   %1,%%edi\n\t"      /* di=y*320+x*/
      "subl   %%eax,%%ecx\n\t"   /* cx=cx-ax ie. cx=x2-x1*/
      "incl   %%ecx\n\t"         /* cx=cx+1, to get the no of pts to plot*/
      "movl   %4,%%eax\n\t"      /* al=col*/
      "movb   %%al,%%ah\n\t"     /* ah=al==col (for use with stosw}*/
      "shrl   $1,%%ecx\n\t"      /* divide by 2 since we're using rep stosw*/
      "jnc    blast_2\n\t"         /* even no of points*/
      "jmp    plot1_2\n\t"         /* odd no of pts, so plot 1 pt, then rep plot 2*/
                            /* using rep stosw*/
"plot1_2:\n\t"
      "stosb\n\t"

"blast_2:\n\t"
      "rep\n\t"
      "stosw\n\t"
      "pop  %%es\n\t"
  
  :
  : "g" (Addr), "g" (x1), "g" (y), "g" (x2), "g" (Col), "g" (ytab[y])
  : "edi","eax","ebx","ecx","memory"
  );
  }
}




void hglenzline(int x1,int y,int x2,int col, int location)
{
  if(location==VGA)
  {
    __asm__ __volatile__
    (
      "movl  %0,%%es\n\t"
      "movl  $0xA0000,%%edi\n\t"
      "movl  %1,%%eax\n\t"
      "movl  %3,%%ebx\n\t"
      "cmpl  %%ebx,%%eax\n\t"
      "jb    x1lwx2\n\t"

"x1grx2:\n\t"                   /*  x1 > x2*/

      "addl  %5,%%edi\n\t"
      "movl  %4,%%eax\n\t"
      "addl  %3,%%edi\n\t"
      "movl  %1,%%ecx\n\t"
      "subl  %3,%%ecx\n\t"
      "addl  $1,%%ecx\n\t"
      "jmp   draw\n\t"

"x1lwx2:\n\t"                   /*  x1 < x2*/
      "addl  %5,%%edi\n\t"
      "movl  %4,%%eax\n\t"
      "addl  %1,%%edi\n\t"
      "movl  %3,%%ecx\n\t"
      "subl  %1,%%ecx\n\t"
                       /*add  cx,1*/
"draw:\n\t"
      "movl  %%es:(%%edi),%%eax\n\t"
      "addl  %4,%%eax\n\t"
      "stosb\n\t"
      "loop  draw\n\t"

    
    :
    : "g" (_dos_ds), "g" (x1), "g" (y), "g" (x2), "g" (col), "g" (ytab[y])
    : "edi","eax","ebx","ecx","memory"
    );
  }
  else
  {
    __asm__ __volatile__
    (
      "movl  %0,%%edi\n\t"
      "movl  %1,%%eax\n\t"
      "movl  %3,%%ebx\n\t"
      "cmpl  %%ebx,%%eax\n\t"
      "jb    x1lwx2_2\n\t"

"x1grx2_2:\n\t"                   /*  x1 > x2*/

      "addl  %5,%%edi\n\t"
      "movl  %4,%%eax\n\t"
      "addl  %3,%%edi\n\t"
      "movl  %1,%%ecx\n\t"
      "subl  %3,%%ecx\n\t"
      "addl  $1,%%ecx\n\t"
      "jmp   draw_2\n\t"

"x1lwx2_2:\n\t"                   /*  x1 < x2*/
      "addl  %5,%%edi\n\t"
      "movl  %4,%%eax\n\t"
      "addl  %1,%%edi\n\t"
      "movl  %3,%%ecx\n\t"
      "subl  %1,%%ecx\n\t"
                       /*add  cx,1*/
"draw_2:\n\t"
      "movl  %%ds:(%%edi),%%eax\n\t"
      "addl  %4,%%eax\n\t"
      "stosb\n\t"
      "loop  draw_2\n\t"

    
    :
    : "g" (location), "g" (x1), "g" (y), "g" (x2), "g" (col), "g" (ytab[y])
    : "edi","eax","ebx","ecx","memory"
    );
  }

}

void hline2(int XStart, int Y,int XEnd, int Col, word Addr)
{ //assume start value < end value
  if(Addr==VGA)
  {
    __asm__ __volatile__
    (


                         /* optimisation of the above*/
       "movl  $0xA0000,%%edi\n\t"
       "movw  %0,%%es\n\t"      /*load es with selector*/
       "addl  %5,%%edi\n\t"
       "movl  %2,%%ecx\n\t"
       "addl  %1,%%edi\n\t"
       "movl  %4,%%eax\n\t"
       "subl  %1,%%ecx\n\t"
       "movb  %%al,%%ah\n\t"
      /* incl  %%ecx   */
       "shrl  $1,%%ecx\n\t"
       "jnc    blast2\n\t"
       "stosb\n\t"
"blast2:\n\t"
       "rep\n\t"
       "stosw\n\t"
  
  :
  : "g" (_dos_ds), "g" (XStart), "g" (XEnd), "g" (Y), "g" (Col), "g" (ytab[Y])
  : "edi","eax","ecx"
  );
  }
  else
  {
    __asm__ __volatile__
    (
       "movw  %%ds,%%ax\n\t"      /*load es with selector*/
       "movl  %0,%%edi\n\t"
       "movw  %%ax,%%es\n\t"
       "addl  %5,%%edi\n\t"
       "movl  %2,%%ecx\n\t"
       "addl  %1,%%edi\n\t"
       "movl  %4,%%eax\n\t"
       "subl  %1,%%ecx\n\t"
       "movb  %%al,%%ah\n\t"
       "incl  %%ecx\n\t"
       "shrl  $1,%%ecx\n\t"
       "jnc    blast2_2\n\t"
       "stosb\n\t"
"blast2_2:\n\t"
       "rep\n\t"
       "stosw\n\t"
  
  :
  : "g" (Addr), "g" (XStart), "g" (XEnd), "g" (Y), "g" (Col), "g" (ytab[Y])
  : "edi","eax","ecx"
  );
  }
}

void hglenzline2(int x1,int y,int x2,int col,word location)
{/*assume x1<x2*/
  if(location==VGA)
  {
    __asm__ __volatile__
    (
                    /*  x1 < x2*/
      "movl  %0,%%es\n\t"
      "movl  $0xA0000,%%edi\n\t"
      "movl  %4,%%eax\n\t"
      "addl  %5,%%edi\n\t"
      "movb  %%al,%%ah\n\t"
      "addl  %1,%%edi\n\t"
      "movl  %3,%%ecx\n\t"
      "subl  %1,%%ecx\n\t"
                   /*add  cx,1*/
"draw2:\n\t"
      "movb  %%es:(%%edi),%%eax\n\t"
      "addl  %4,%%eax\n\t"
      "stosb\n\t"
      "loop draw2\n\t"
    
    :
    : "g" (_dos_ds), "g" (x1), "g" (y), "g" (x2), "g" (col), "g" (ytab[y])
    : "edi","eax","ebx","ecx","memory"
    );
  }
  else
  {
    __asm__ __volatile__
    (
                    /*  x1 < x2*/
      "movl  %0,%%edi\n\t"
      "movl  %4,%%eax\n\t"
      "addl  %5,%%edi\n\t"
      "movb  %%al,%%ah\n\t"
      "addl  %1,%%edi\n\t"
      "movl  %3,%%ecx\n\t"
      "subl  %1,%%ecx\n\t"
                   /*add  cx,1*/
"draw2_2:\n\t"
      "movb  %%ds:(%%edi),%%eax\n\t"
      "addl  %4,%%eax\n\t"
      "stosb\n\t"
      "loop draw2_2\n\t"
    
    :
    : "g" (location), "g" (x1), "g" (y), "g" (x2), "g" (col), "g" (ytab[y])
    : "edi","eax","ebx","ecx","memory"
    );
  }
}

void scanedge(int xx1,int yy1,int xx2,int yy2)
{
  int tem,m,i,xv;
  float xstep;
  float temp;

  if(yy2<yy1)
  {
    swapint(&yy1,&yy2);
    swapint(&xx1,&xx2);
  }
  xv=xx1;
  temp=xx1;
  xstep=((float)(xx2-xx1))/((float)(yy2-yy1));
  for(i=yy1;i<=yy2;i++)
  {
    if(xv<edgelist[i][0]) edgelist[i][0]=xv;
    if(xv>edgelist[i][1]) edgelist[i][1]=xv;
    temp=temp+xstep;
    xv=temp;
  }
}

void scanedge2(int x1, int y1, int x2, int y2)
{
  int dx, dy, i;
  float curx, xstepval;

  if(y2 < y1)
  {
    swapint(&x1, &x2);
    swapint(&y1, &y2);
  }
  xstepval = (float) (x2 - x1) / (float) (y2 - y1);
  curx = x1;

  for(i=y1;i<=y2;i++)
  {
    if(curx < edgelist[i][0])
      edgelist[i][0] = (int) curx;

    if(curx > edgelist[i][1])
      edgelist[i][1] = (int) curx;

    curx += xstepval;
  }
}

void cscanedge2(int x1, int y1, int c1, int x2, int y2, int c2)
{
  int dx, dy, i, j;
  float curx, xstepval, cstepval, curcol;

  if(y2 < y1)
  {
    swapint(&c1, &c2);
    swapint(&x1, &x2);
    swapint(&y1, &y2);
  }
  xstepval = (float) (x2 - x1) / (float) (y2 - y1);
  cstepval = (float) (c2 - c1) / (float) (y2 - y1);
  curx = x1;
  curcol = c1;

  for(i = y1; i <= y2; i++)
  {
    if(curx < edgelist[i][0])
    {
      edgelist[i][0] = (int) curx;
      cedgelist[i][0] = (int) curcol;
    }
    if(curx > edgelist[i][1])
    {
      edgelist[i][1] = (int) curx;
      cedgelist[i][1] = (int) curcol;
    }
    curx += xstepval;
    curcol += cstepval;
  }
}

void nscanedge2(int x1, int y1, float nx1, float ny1, float nz1,
                int x2, int y2, float nx2, float ny2, float nz2)
{
  int dy, i, j;
  float stepval, nxstepval, nystepval, nzstepval;
  float curx, curnx, curny, curnz;

  if(y2 < y1)
  {
    swapfloat(&nx1, &nx2);
    swapfloat(&ny1, &ny2);
    swapfloat(&nz1, &nz2);
    swapint(&x1, &x2);
    swapint(&y1, &y2);
  }

  dy = y2 - y1;
  stepval = (float) (x2 - x1) / (float) dy;
  nxstepval = (float) (nx2 - nx1) / (float) dy;
  nystepval = (float) (ny2 - ny1) / (float) dy;
  nzstepval = (float) (nz2 - nz1) / (float) dy;

  curx = x1;
  curnx = nx1;
  curny = ny1;
  curnz = nz1;

  for(i = y1; i <= y2; i++)
  {
    if(curx < edgelist[i][0])
    {
      edgelist[i][0] = (int)curx;
      nedgelist[i][0][0] = curnx;
      nedgelist[i][0][1] = curny;
      nedgelist[i][0][2] = curnz;
    }
    if(curx > edgelist[i][1])
    {
      edgelist[i][1] = (int)curx;
      nedgelist[i][1][0] = curnx;
      nedgelist[i][1][1] = curny;
      nedgelist[i][1][2] = curnz;
    }
    curx+=stepval;
    curnx+=nxstepval;
    curny+=nystepval;
    curnz+=nzstepval;
  }
}

void glenzpoly(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,
	       int col, int location)
{
  int i,miny,maxy;

  for(i=0;i<=199;i++)
  {
    edgelist[i][0]=32767;
    edgelist[i][1]=-32768;
  }

/*{  asm
    mov  di,offset [EdgeList]
    mov  cx,200
@iter:
    mov  ax,32767
    "stosw
    mov  ax,-32768
    "stosw
    "loop @iter
  end;}  */

  if(y1!=y2)
  scanedge2(x1,y1,x2,y2);
  if(y2!=y3)
  scanedge2(x2,y2,x3,y3);
  if(y3!=y4)
  scanedge2(x3,y3,x4,y4);
  if(y1!=y4)
  scanedge2(x1,y1,x4,y4);

  miny=y1;maxy=y1;
  if(miny>y2) miny=y2;
  if(miny>y3) miny=y3;
  if(miny>y4) miny=y4;
  if(maxy<y2) maxy=y2;
  if(maxy<y3) maxy=y3;
  if(maxy<y4) maxy=y4;

  for(i=miny;i<=maxy;i++)
    if(edgelist[i][0]<edgelist[i][1]) //affects polygon fitting together
      hglenzline(edgelist[i][0],i,edgelist[i][1],col,location);
}

void glenztri(int x1,int y1,int x2,int y2,int x3,int y3,int col, int location)
{
  int i,miny,maxy;

  for(i=0;i<=199;i++)
  {
    edgelist[i][0]=32767;
    edgelist[i][1]=-32768;
  }

/*{  asm
    mov  di,offset [EdgeList]
    mov  cx,200
@iter:
    mov  ax,32767
    "stosw
    mov  ax,-32768
    "stosw
    "loop @iter
  end;}  */

  if(y1!=y2)
  scanedge2(x1,y1,x2,y2);
  if(y2!=y3)
  scanedge2(x2,y2,x3,y3);
  if(y1!=y3)
  scanedge2(x1,y1,x3,y3);

  miny=y1;maxy=y1;
  if(miny>y2) miny=y2;
  if(miny>y3) miny=y3;
  if(maxy<y2) maxy=y2;
  if(maxy<y3) maxy=y3;

  for(i=miny;i<=maxy;i++)
    if(edgelist[i][0]<edgelist[i][1]) //affects polygon fitting together
      hglenzline(edgelist[i][0],i,edgelist[i][1],col,location);
}


void filltri(int x1,int y1,int x2, int y2,int x3,int y3
	     ,int col,word add)
{
  int i,miny,maxy;

  for(i=0;i<=199;i++) //initialize edgelist
  {
    edgelist[i][0]=32767;
    edgelist[i][1]=-32768;
  }

  if(y1!=y2)
    scanedge2(x1,y1,x2,y2);
  if(y2!=y3)
    scanedge2(x2,y2,x3,y3);
  if(y1!=y3)
    scanedge2(x1,y1,x3,y3);
  miny=y1;
  maxy=y1;
  if(y2<miny) miny=y2;
  if(y3<miny) miny=y3;
  if(y2>maxy) maxy=y2;
  if(y3>maxy) maxy=y3;

  for(i=miny;i<maxy;i++)
    if(edgelist[i][0]<=edgelist[i][1]) //affects polygon fitting together
      hline2(edgelist[i][0],i,edgelist[i][1],col,add);//adjust when needed
//    else
//      plotpix(edgelist[i][0],ytab[i],col,viradd);

}


void FillPoly(PointList *CoordList,unsigned char col, unsigned int Addr)
{
  //CoordList is a pointer to a list of points that make up a polygon
  //caller would pass the ADDRESS of polygon structure to function
  Coord *Vertlist; //used as an array that contains the list of points
  HorlineList Hlist; //contains the list of horizontal lines to be drawn
  Horline *Edgelist; //addresses the start of edgelist
  int TLVIndex,TRVIndex,LPIndex,Next,Prev,Current;
  int i,MaxY,MinY;
  int FlatTop,LEDir,SkipTopLine;
  int Temp;
  int DxN,DyN,DxP,DyP;

  Vertlist=CoordList->CoordPtr; //Vertlist now address the list of pts
				//that make up the polygon
  TLVIndex=0;
  LPIndex=0;
  MaxY=Vertlist[0].y;
  MinY=Vertlist[0].y;

  for(i=1;i<CoordList->NoOfPts;i++)
  {
    if(Vertlist[i].y<MinY)
    {
      MinY=Vertlist[i].y; //new top
      TLVIndex=i;         //new index of top polygon vertex
    }
    if(Vertlist[i].y>MaxY)
    {
      MaxY=Vertlist[i].y; //new top
      LPIndex=i;         //index of top polygon vertex
    }
  }

  TRVIndex=TLVIndex;
  //scan in ascending order through vertex list to find index of last top
  //vertex
  while(Vertlist[TRVIndex].y == MinY)
    TRVIndex=(TRVIndex+1) % CoordList->NoOfPts;
  //get back to previous index if current point is not a top vertex
  TRVIndex=(TRVIndex-1+CoordList->NoOfPts)%CoordList->NoOfPts;

  //decrement index to test next point in array
  while(Vertlist[TLVIndex].y == MinY)
    TLVIndex=(TLVIndex-1+CoordList->NoOfPts)%CoordList->NoOfPts;
  //get back to previous index if current point is not a top vertex
  TLVIndex=(TLVIndex+1) % CoordList->NoOfPts;

  if(Vertlist[TRVIndex].x!=Vertlist[TLVIndex].x)
    FlatTop=1; //top is flat
  else
    FlatTop=0; //top is pointed


  LEDir=-1;
  if(FlatTop)
  {
    if(Vertlist[TLVIndex].x>Vertlist[TRVIndex].x)
    {
      LEDir=1;            /*  swaps indices so that:             */
      Temp=TLVIndex;      /*  TLVIndex -> __________ <- TRVIndex */
      TLVIndex=TRVIndex;  /*             /          \            */
      TRVIndex=Temp;      /*            /            \           */
			  /*           /              \          */
    }
  }
  else
  { //top of polygon is not flat ie. pointed top
    Next=(TLVIndex+1) % CoordList->NoOfPts;
    Prev=(TLVIndex-1+CoordList->NoOfPts)%CoordList->NoOfPts;
    DxN=Vertlist[Next].x-Vertlist[TLVIndex].x;
    DyN=Vertlist[Next].y-Vertlist[TLVIndex].y;
    DxP=Vertlist[Prev].x-Vertlist[TLVIndex].x;
    DyP=Vertlist[Prev].y-Vertlist[TLVIndex].y;
    if((long)DyP*DxN-(long)DyN*DxP<0)  // (DyN/DxN) > (DyP/DxP)
      LEDir=1;
  }
  Hlist.NoOfLines=MaxY-MinY-1+FlatTop;//omit top scanline if top is pointed
  Hlist.Ystart=MinY+1-FlatTop; //starting y coord is the second scan line
			       //unless top is flat
  Hlist.HorlinePtr=(Horline *) malloc(sizeof(Horline)*Hlist.NoOfLines);
  Edgelist=Hlist.HorlinePtr;//another pointer to list of horizontal lines
			    //to be used in edge tracing
  Prev=TLVIndex; //initialise indices to start from the left edge
  Current=TLVIndex;
  if(FlatTop)     //if top of polygon is flat then do not skip the topmost
    SkipTopLine=0;//scanline
  else            //else omit first scanline
    SkipTopLine=1;

  /* scan conversion of left edges */
  do
  {
    if(LEDir>0)
      Current=(Current+1)%CoordList->NoOfPts;// advances index incrementally
    else                                     // with wrapping
      Current=(Current-1+CoordList->NoOfPts)%CoordList->NoOfPts;
    // advances index decrementally, with wrapping

    ScanEdge(Vertlist[Prev].x,Vertlist[Prev].y,
	     Vertlist[Current].x,Vertlist[Current].y
	     ,1,SkipTopLine,&Edgelist);
    Prev=Current;
    SkipTopLine=0;// scan conversion will not omit the top scanline now,
		  // we only want to skip for the first time round when we
		  // are scanning the top vertex
  } while(Current!=LPIndex);
  /* scan conversion of left edges */

  Edgelist=Hlist.HorlinePtr;//restart from top of list again,
			    //tracing values for xend this time round

  /*re-initialise all indexes to start from top of poly again*/
  Prev=TLVIndex; //initialise indices to start from the left edge
  Current=TLVIndex;
  if(FlatTop)     //if top of polygon is flat then do not skip the topmost
    SkipTopLine=0;//scanline
  else            //else omit first scanline
    SkipTopLine=1;
  /*re-initialise all indexes to start from top of poly again*/

  /* scan conversion of right edges */
  do
  {
    if(LEDir>0)
      Current=(Current-1+CoordList->NoOfPts)%CoordList->NoOfPts;
    else    // advances index decrementally, with wrapping
      Current=(Current+1)%CoordList->NoOfPts;
      // advances index incrementally with wrapping

    ScanEdge(Vertlist[Prev].x-1,Vertlist[Prev].y,
	     Vertlist[Current].x-1,Vertlist[Current].y
	     ,0,SkipTopLine,&Edgelist);
    Prev=Current;
    SkipTopLine=0;// scan conversion will not omit the top scanline now,
		  // we only want to skip for the first time round when we
		  // are scanning the top vertex
  } while(Current!=LPIndex);
  /* scan conversion of right edges */

  DrawHList(&Hlist,col,Addr); //draws list of horizontal lines to screen
  free(Hlist.HorlinePtr);// free up memory allocated to edgelist

}

void ScanEdge(int x1, int y1, int x2, int y2, int LeftEdge, int SkipTop,
	      Horline **ScanlinePtr)
{ //ScanlinePtr is a pointer that points to a pointer to
  //list of horizontal lines
  int Dx,Dy;
  int width, height;
  int i,Xdir;
  int numerator,denominator,errorterm;
  int MinXstep;
  Horline *LoopPtr;

  LoopPtr=*ScanlinePtr; //LoopPtr points to the start of list of horizontal
			//lines
  Dx=x2-x1;             //calculate deltax
  Dy=y2-y1;             //calculate deltay
  width=abs(Dx);
  height=Dy;
  if(Dx>0)
    Xdir=1;
  else
    Xdir=-1;

  if(Dy==0) //horizontal edge
    return;

  if(width==0) //edge is vertical
    for(i=y1+SkipTop;i<y2;i++, LoopPtr++)
    {
      if(LeftEdge)
	LoopPtr->xstart=x1;
      else
	LoopPtr->xend=x1;
    }       //edge is vertical

  if(width==height) //diagonal lines
  {                 //x increases by 1 unit for a unit increase in y
    if(SkipTop)
      x1=x1+Xdir;
    for(i=y1+SkipTop;i<y2;i++, LoopPtr++)
    {
      if(LeftEdge)
	LoopPtr->xstart=x1;
      else
	LoopPtr->xend=x1;
      x1+=Xdir;
    }
  }                 //diagonal lines

  if(height>width) //y-major lines
  {
    //numerator=width;
    //denominator=height;

    //initialize errorterm differently
    if(Xdir>0) //x increases as y increases
      errorterm=height;
    else       //x decreases as y increases
      errorterm=1;
    //initialize errorterm differently

    if(SkipTop) // skip top scanline as indicated
    {
      errorterm+=width; // updates all values for use with next point
      if(errorterm>height)
      {
	x1+=Xdir;
	errorterm-=height;
      }
    }

    if(LeftEdge) //scan converts for left edge
    {
      for(i=y1+SkipTop;i<y2;i++,LoopPtr++)
      {
	LoopPtr->xstart=x1;
	errorterm+=width; // updates all values for use with next point
	if(errorterm>height)
	{
	  x1+=Xdir;
	  errorterm-=height;
	}
      }
     }           //scan converts for left edge
     else
     {           //scan converts for right edge
      for(i=y1+SkipTop;i<y2;i++,LoopPtr++)
      {
	LoopPtr->xend=x1;
	errorterm+=width; // updates all values for use with next point
	if(errorterm>height) //if fractional term overflows, then it is
	{                         //time to increment x
	  x1+=Xdir;
	  errorterm-=height;
	}
      }
     }           //scan converts for right edge
  }        //y-major lines

  if(height<width) //x-major lines
  {
    MinXstep=(width/height)*Xdir; // amount to increment x each time,
				  // multiply by Xdir to set direction
    numerator=width%height;
    //denominator=height;
    //initialize errorterm differently
    if(Xdir>0) //x increases as y increases
      errorterm=height;
    else       //x decreases as y increases
      errorterm=1;
    //initialize errorterm differently
    if(SkipTop) // skip top scanline as indicated
    {
      x1+=MinXstep; // min value to increment x with each time
      errorterm+=numerator; // updates all values for use with next point
      if(errorterm>height)
      {
	x1+=Xdir;
	errorterm-=height;
      }
    }

    if(LeftEdge) //scan converts for left edge
    {
      for(i=y1+SkipTop;i<y2;i++,LoopPtr++)
      {
	LoopPtr->xstart=x1;
	x1+=MinXstep; // min value to increment x with each time
	errorterm+=numerator; // updates all values for use with next point
	if(errorterm>height)
	{
	  x1+=Xdir;
	  errorterm-=height;
	}
      }
     }
     else  //scan converts for right edge
     {
      for(i=y1+SkipTop;i<y2;i++,LoopPtr++)
      {
	LoopPtr->xend=x1;
	x1+=MinXstep; // min value to increment x with each time
	errorterm+=numerator; // updates all values for use with next point
	if(errorterm>height) //if fractional term overflows, then it is
	{                         //time to increment x
	  x1+=Xdir;
	  errorterm-=height;
	}
      }
     }             //x-major lines
  }
  *ScanlinePtr=LoopPtr;
  //equivalent to updating Edgelist so that next call to ScanEdge would start
  //tracing from current y value left from the previous scan
  //Edgelist=LoopPtr;
}

void DrawHList(HorlineList *Hptr,unsigned char col, unsigned int Addr)
{
  int i,j;
  Horline *LoopPtr;

  LoopPtr=Hptr->HorlinePtr; //LoopPtr points to first entryin edgelist
  for(i=Hptr->Ystart;i<Hptr->Ystart+Hptr->NoOfLines;i++,LoopPtr++)
  {
    hline2(LoopPtr->xstart,LoopPtr->xend,i,col,Addr);
  }
}

void loadobj(char filename[255] , object *obj)
{
  FILE *ascfile;
  char str[80];
  float x, y, z;
  int vertnum, facenum, i;
  int ix, iy, iz;

  printf("Loading 3D Object from %s...",filename);

  ascfile = fopen( filename,"r+t");
  if(ascfile == NULL)
  {
    printf("Error loading file\n");
    getch();
  }
//  fgets(str,80,ascfile);//get 1st line 'Ambient ...'
//  fgets(str,80,ascfile);//get 2nd line 'space'

  fgets(str, 80, ascfile);//get 3nd line 'Object named'
//Tri-mesh, Vertices: 128 Faces: 244

  fscanf(ascfile,"%*s %*s %d %*s %d \n", &vertnum, &facenum);//get no of vertices
//  printf("%d\n",vertnum);                               //get no of faces
//  printf("%d\n",facenum);
  obj->numfaces = facenum;
  obj->numvertices = vertnum;
  fgets(str, 80, ascfile);//get next line which says vertex list

  for(i = 0; i < vertnum; i++)//get and stores vertex list into obj structure
  {                      //vertnum specifies the number of vertex entries
//  Vertex 0:  X:-3.113875    Y:0.350000    Z:-0.250000
//     s   dc  cc  f           ccf            ccf
//  fscanf(ascfile,"%*s %*d %*c %*c %*c %f %*c %*c %f %*c %*c %f \n"
//       ,obj->vertices[i].x,obj->vertices[i].y,obj->vertices[i].z);
  fscanf(ascfile,"%*s %*d %*c %*c %*c %f %*c %*c %f %*c %*c %f \n"
       , &x, &y, &z);
  obj->vertices[i].x = x;
  obj->vertices[i].y = y;
  obj->vertices[i].z = z;
  obj->vertices[i].nx = 0;
  obj->vertices[i].ny = 0;
  obj->vertices[i].nz = 0;
  }

  fgets(str, 80, ascfile);//get next line which says Face List


  for(i = 0; i < facenum; i++)//get and stores vertex list into obj structure
  {                      //vertnum specifies the number of vertex entries
  //Face 0: A:2 B:1 C:0 AB:1 BC:1 CA:1
  //  s  dc ccd ccd ccd
  fscanf(ascfile,"%*s %*d %*c %*c %*c %d %*c %*c %d %*c %*c %d \n"
        ,&ix,&iy,&iz);
  obj->polys[i].v1=ix;
  obj->polys[i].v2=iy;
  obj->polys[i].v3=iz;

  fgets(str,80,ascfile);//get next line which says AB:1 BC:1 CD:1 ...
  fgets(str,80,ascfile);//get next line which says Smoothing :...
//  fgets(str,80,ascfile);//get next line which says Material ...
  }
//  rotobj=obj3d;
  fclose(ascfile);

  genmidpt(obj);     //generate midpoints
  genpolynorm(obj);  //generate polygon normals
  genvertnorm(obj);  //generate vertex normals

  printf("Done!\n");

}

void blurmotion(int Addr)
{
  if(Addr==0xA0000)
  {
    __asm__ __volatile__
    (
      "push  %%es\n\t"
      "movl  $0xA0000,%%edi\n\t"
     // "movw  %0,%%es
      "movl  $64000,%%ecx\n\t"
      "addl  $320,%%edi\n\t"

"loop1:\n\t"

      "movl  $0,%%eax\n\t"
      "movl  $0,%%ebx\n\t"
      "movb  (%%edi),%%eax\n\t"
      "movb  1(%%edi),%%ebx\n\t"
      "addl  %%ebx,%%eax\n\t"
	 /*mov  bl,es:[di-1]*/
	 /*add  ax,bx*/
	 /*mov  bl,es:[di+320]*/
	 /*add  ax,bx*/
         /*"shrl  $1,%%eax*/
//      jz   "loop2
      "decl  %%eax\n\t"

"loop2:\n\t"
      "movb  %%eax,-320(%%edi)\n\t"
      "incl  %%edi\n\t"
      "decl  %%ecx\n\t"
      "jnz  loop1\n\t"
      "pop  %%es\n\t"
      
      :
      :"g" (_dos_ds)
      :"ecx","edi","eax","ebx"
      );
  }
  else
  {
    __asm__ __volatile__
    (
      "movl  %0,%%edi\n\t"
      "movl  $64000,%%ecx\n\t"
      "addl  $320,%%edi\n\t"

"loop1_2:\n\t"

      "movl  $0,%%eax\n\t"
      "movl  $0,%%ebx\n\t"
      "movb  (%%edi),%%eax\n\t"
      "movb  1(%%edi),%%ebx\n\t"
      "addl  %%ebx,%%eax\n\t"
	 /*mov  bl,es:[di-1]*/
	 /*add  ax,bx*/
	 /*mov  bl,es:[di+320]*/
	 /*add  ax,bx*/
      "shrl  $1,%%eax\n\t"
      "jz   loop2_2\n\t"
      "decl  %%eax\n\t"


"loop2_2:\n\t"
      "movb  %%eax,-320(%%edi)\n\t"
      "incl  %%edi\n\t"
      "decl  %%ecx\n\t"
      "jnz  loop1_2\n\t"
      
      :
      :"g" (Addr)
      :"ecx","edi","eax","ebx"
      );
  }
}

void initpal(float ra, float rd, float rs,
             float ga, float gd, float gs,
             float ba, float bd, float bs, int n)    //sets up the palette
{
  int i;
  int Ired,Iblue,Igreen;
  float ambient,diffuse,specular;

//  n=10;
  for(i=1;i<=91;i++) // phong illumination model
  {


    ambient=ra;
    diffuse=rd*cosine[90-i];
    specular=rs*pow(cosine[90-i],n);
    Ired=ambient+diffuse+specular;

    ambient=ga;
    diffuse=gd*cosine[90-i];
    specular=gs*pow(cosine[90-i],n);
    Igreen=ambient+diffuse+specular;

    ambient=ba;
    diffuse=bd*cosine[90-i];
    specular=bs*pow(cosine[90-i],n);
    Iblue=ambient+diffuse+specular;

    if(Ired>63)Ired=63;
    if(Igreen>63)Igreen=63;
    if(Iblue>63)Iblue=63;
    setcol(i, Ired,Igreen,Iblue);


  }

/*  for(i=1;i<=32;i++)//red color fire palette
  {
    setcol(i, (i << 1)-1 ,0, 0 );
    setcol(i+32, 63, (i << 1)-1,0 );
    setcol(i+64, 63,63, (i << 1)  );
    setcol(i+96, 63, 63, 63 );
  }   */
/*  for(i=1;i<=32;i++)//blue color fire palette
  {
    setcol(i, 0, 0, (i << 1)-1 );
    setcol(i+32, 0, (i << 1)-1,63 );
    setcol(i+64, (i << 1), 63,63  );
    setcol(i+96, 63, 63, 63 );
  }*/
/*  for(i=1;i<=32;i++)//green color fire palette
  {
    setcol(i, 0,  (i << 1)-1,0 );
    setcol(i+32,  (i << 1)-1,63,0);
    setcol(i+64, 63,63,(i << 1)  );
    setcol(i+96, 63, 63, 63 );
  }*/

}

int psortcmpto(const void *e1, const void *e2)
{
  if( ((const psortuple *)e2)->mz > ((const psortuple *)e1)->mz)
    return -1;
  if( ((const psortuple *)e2)->mz == ((const psortuple *)e1)->mz)
    return 0;
  if( ((const psortuple *)e2)->mz < ((const psortuple *)e1)->mz)
    return 1;

}

void rotateobj(int deg_ab_x, int deg_ab_y, int deg_ab_z,
               object *src, object *dest)
{
  //object rotation

  float a,b,c,d,e,f,g,h,i;
  float x1,y1,z1,t1,t2;
  int j;

  a=cosine[deg_ab_y]*cosine[deg_ab_z];//precalculated stuff to get 9 muls
  b=-cosine[deg_ab_y]*sine[deg_ab_z]; //which are the respective x,y,z coeffs
  c=-sine[deg_ab_y];
  t1=sine[deg_ab_x]*sine[deg_ab_y];
  d=t1*cosine[deg_ab_z]+cosine[deg_ab_x]*sine[deg_ab_z];
  e=-t1*sine[deg_ab_z]+cosine[deg_ab_x]*cosine[deg_ab_z];
  f=sine[deg_ab_x]*cosine[deg_ab_y];
  t2=cosine[deg_ab_x]*sine[deg_ab_y];
  g=t2*cosine[deg_ab_z]-sine[deg_ab_x]*sine[deg_ab_z];
  h=-t2*sine[deg_ab_z]-sine[deg_ab_x]*cosine[deg_ab_z];
  i=cosine[deg_ab_x]*cosine[deg_ab_y];//precalculated stuff to get 9 muls

  for(j = 0; j < src->numvertices; j++)//rotates about X, then Y, then Z axis resp.
  {
    // rotate the vertices...
    x1=src->vertices[j].x;
    y1=src->vertices[j].y;
    z1=src->vertices[j].z;

    dest->vertices[j].x = x1 * a + y1 * b + z1 * c; //final rotated x coord
    dest->vertices[j].y = x1 * d + y1 * e + z1 * f; //final rotated y coord
    dest->vertices[j].z = x1 * g + y1 * h + z1 * i; //final rotated z coord

    //rotate the vertex normal
    x1=src->vertices[j].nx;
    y1=src->vertices[j].ny;
    z1=src->vertices[j].nz;

    dest->vertices[j].nx = x1 * a + y1 * b + z1 * c; //final rotated x coord
    dest->vertices[j].ny = x1 * d + y1 * e + z1 * f; //final rotated y coord
    dest->vertices[j].nz = x1 * g + y1 * h + z1 * i; //final rotated z coord
  }
}

void rotatenorm(int deg_ab_x, int deg_ab_y, int deg_ab_z,
                object *src, object *dest)
{
  //object normal rotation
  float a,b,c,d,e,f,g,h,i;
  float x1,y1,z1,t1,t2;
  int j;

  a=cosine[deg_ab_y]*cosine[deg_ab_z];//precalculated stuff to get 9 muls
  b=-cosine[deg_ab_y]*sine[deg_ab_z]; //which are the respective x,y,z coeffs
  c=-sine[deg_ab_y];
  t1=sine[deg_ab_x]*sine[deg_ab_y];
  d=t1*cosine[deg_ab_z]+cosine[deg_ab_x]*sine[deg_ab_z];
  e=-t1*sine[deg_ab_z]+cosine[deg_ab_x]*cosine[deg_ab_z];
  f=sine[deg_ab_x]*cosine[deg_ab_y];
  t2=cosine[deg_ab_x]*sine[deg_ab_y];
  g=t2*cosine[deg_ab_z]-sine[deg_ab_x]*sine[deg_ab_z];
  h=-t2*sine[deg_ab_z]-sine[deg_ab_x]*cosine[deg_ab_z];
  i=cosine[deg_ab_x]*cosine[deg_ab_y];//precalculated stuff to get 9 muls

  for(j = 0; j < src->numfaces; j++)//rotates about X, then Y, then Z axis resp.
  {
    x1=src->polys[j].nx;
    y1=src->polys[j].ny;
    z1=src->polys[j].nz;

    dest->polys[j].nx = x1 * a + y1 * b + z1 * c; //final rotated x coord
    dest->polys[j].ny = x1 * d + y1 * e + z1 * f; //final rotated y coord
    dest->polys[j].nz = x1 * g + y1 * h + z1 * i; //final rotated z coord
  }
}

void genmidpt(object *obj)    //generate polygon normals of the object
{
  int i,v1,v2,v3;

  for(i = 0; i < obj->numfaces; i++)
  {
    v1 = obj->polys[i].v1;
    v2 = obj->polys[i].v2;
    v3 = obj->polys[i].v3;

    obj->polys[i].mx = (obj->vertices[v1].x
		        +obj->vertices[v2].x
		        +obj->vertices[v3].x) / 3;
    obj->polys[i].my = (obj->vertices[v1].y
		        +obj->vertices[v2].y
		        +obj->vertices[v3].y)/3;
    obj->polys[i].mz = (obj->vertices[v1].z
		        +obj->vertices[v2].z
		        +obj->vertices[v3].z)/3;

    obj->sarr[i].mz = obj->polys[i].mz;
    obj->sarr[i].n = i;
  }
}

void genvertcol(object *obj)
{
  int i,c;
  float nx, ny, nz, modn, cost;

  //using cross product to generate polygon normal
  for(i = 0; i < obj->numvertices; i++)
  {

    nx = light.x /*- obj->vertices[i].x*/; //light source at infinity
    ny = light.y /*- obj->vertices[i].y*/; //for point light source uncomment
    nz = light.z /*- obj->vertices[i].z*/;
    modn = (float) sqrt(nx * nx + ny * ny + nz * nz);

    cost = (obj->vertices[i].nx * nx
           +obj->vertices[i].ny * ny
           +obj->vertices[i].nz * nz)/modn;

    c = (cost * 90);
    if(c > 90) c = 90; if(c < 0) c = 0;
    obj->vertices[i].col = c;
  }
}

void genpolynorm(object *obj)
{
  int i;
  float mod,a1,a2,a3,b1,b2,b3;

  //using cross product to generate polygon normal
  for(i = 0; i < obj->numfaces; i++)
  {
    a1 = obj->vertices[obj->polys[i].v2].x
         -obj->vertices[obj->polys[i].v1].x;
    a2 = obj->vertices[obj->polys[i].v2].y
         -obj->vertices[obj->polys[i].v1].y;
    a3 = obj->vertices[obj->polys[i].v2].z
         -obj->vertices[obj->polys[i].v1].z;
    b1 = obj->vertices[obj->polys[i].v3].x
         -obj->vertices[obj->polys[i].v1].x;
    b2 = obj->vertices[obj->polys[i].v3].y
         -obj->vertices[obj->polys[i].v1].y;
    b3 = obj->vertices[obj->polys[i].v3].z
         -obj->vertices[obj->polys[i].v1].z;

    obj->polys[i].nx = (a2 * b3) - (a3 * b2); //cross product

    obj->polys[i].ny = (a3 * b1) - (a1 * b3);

    obj->polys[i].nz = (a1 * b2) - (a2 * b1);

    //calculate magnitude
    mod = (float) sqrt( (obj->polys[i].nx * obj->polys[i].nx)
	              + (obj->polys[i].ny * obj->polys[i].ny)
	              + (obj->polys[i].nz * obj->polys[i].nz) );

    obj->polys[i].nx /= mod; //normalize into unit vector
    obj->polys[i].ny /= mod;
    obj->polys[i].nz /= mod;
  }
}

void genvertnorm(object *obj)
{
  int i,j,num;
  float abs,sum;
  float sx, sy, sz;

  //using cross product to generate polygon normal
  for(i = 0; i < obj->numvertices; i++)
  {
    sx = 0; sy = 0; sz = 0; num = 0;
    for(j = 0; j < obj->numfaces; j++)
    {
      if( obj->polys[j].v1 == i )
      {
        sx += obj->polys[j].nx;
        sy += obj->polys[j].ny;
        sz += obj->polys[j].nz;
        num++;
      }
      if(obj->polys[j].v2 == i)
      {
        sx += obj->polys[j].nx;
        sy += obj->polys[j].ny;
        sz += obj->polys[j].nz;
        num++;
      }
      if (obj->polys[j].v3 == i)
      {
        sx += obj->polys[j].nx;
        sy += obj->polys[j].ny;
        sz += obj->polys[j].nz;
        num++;
      }
    }
    sx /=(float) num;
    sy /=(float) num;
    sz /=(float) num;
    sum = (sx * sx) + (sy * sy) + (sz * sz);
    abs = (float) sqrt(sum );
    obj->vertices[i].nx = sx/abs;
    obj->vertices[i].ny = sy/abs;
    obj->vertices[i].nz = sz/abs;
  }
}

void gfilltri(int x1, int y1, int x2, int y2, int x3, int y3,
             int c1, int c2, int c3, word add)
{
  int i, j, miny, maxy;
  float temp,cstepval;

  for(i = 0; i <= 199; i++) //initialize edgelist
  {
    edgelist[i][0] =  32767;
    edgelist[i][1] =  -32768;
  }

  if(y1 != y2)
    cscanedge2(x1, y1, c1, x2, y2, c2);
  if(y2 != y3)
    cscanedge2(x2, y2, c2, x3, y3, c3);
  if(y1 != y3)
    cscanedge2(x1, y1, c1, x3, y3, c3);
  miny = y1;
  maxy = y1;
  if(y2 < miny) miny = y2;
  if(y3 < miny) miny = y3;
  if(y2 > maxy) maxy = y2;
  if(y3 > maxy) maxy = y3;

  for(i = miny; i < maxy; i++)
  {
    cstepval = (float)(cedgelist[i][0] - cedgelist[i][1]) /
               (float)(edgelist[i][0] - edgelist[i][1]);
    temp = cedgelist[i][0];
    if(edgelist[i][0] <= edgelist[i][1]) //affects polygon fitting together
    {
      for(j = edgelist[i][0]; j < edgelist[i][1]; j++)
      {
        plotpix(j,i,(int) temp,(int)virscr);
        temp+=cstepval;
      }
//    hline2(, i, edgelist[i][1], col, add);//adjust when needed

    }
  }
}

void phongtri(int x1, int y1, int x2, int y2, int x3, int y3,
              float nx1, float ny1, float nz1, float nx2, float ny2, float nz2,
              float nx3, float ny3, float nz3, word add)
{
  int i, j, miny, maxy, dx, col;
  float temp, curnx, curny, curnz;
  float stepval, nxstepval, nystepval, nzstepval;
  float a1, a2, a3;
  float costheta, modlight;

  for(i = 0; i <= 199; i++) //initialize edgelist
  {
    edgelist[i][0] =  32767;
    edgelist[i][1] =  -32768;
  }

  if(y1 != y2)
    nscanedge2(x1, y1, nx1, ny1, nz1, x2, y2, nx2, ny2, nz2);
  if(y2 != y3)
    nscanedge2(x2, y2, nx2, ny2, nz2, x3, y3, nx3, ny3, nz3);
  if(y1 != y3)
    nscanedge2(x1, y1, nx1, ny1, nz1, x3, y3, nx3, ny3, nz3);

  miny = y1;
  maxy = y1;
  if(y2 < miny) miny = y2;
  if(y3 < miny) miny = y3;
  if(y2 > maxy) maxy = y2;
  if(y3 > maxy) maxy = y3;

  modlight = sqrt( light.x * light.x + light.y * light.y + light.z * light.z);

  for(i = miny; i < maxy; i++)
  {
    dx = (edgelist[i][0] - edgelist[i][1]);
    nxstepval = (float)(nedgelist[i][0][0] - nedgelist[i][1][0]) / (float) dx;
    nystepval = (float)(nedgelist[i][0][1] - nedgelist[i][1][1]) / (float) dx;
    nzstepval = (float)(nedgelist[i][0][2] - nedgelist[i][1][2]) / (float) dx;

    curnx = nedgelist[i][0][0];
    curny = nedgelist[i][0][1];
    curnz = nedgelist[i][0][2];

    if(edgelist[i][0] <= edgelist[i][1]) //affects polygon fitting together
    {
      for(j = edgelist[i][0]; j < edgelist[i][1]; j++)
      {
        costheta = ((light.x) * curnx + (light.y) * curny +
                    (light.z) * curnz) / (
                    sqrt( curnx * curnx + curny * curny + curnz * curnz) *
                    modlight);

        col = costheta * 90;
        if(col > 90) col = 90;
        if(col < 0) col = 0;

        plotpix(j, i, col, (int) virscr);

        curnx += nxstepval;
        curny += nystepval;
        curnz += nzstepval;
      }

    }
  }
}

void ghline(int XStart, int Y, int XEnd, int Cstart, int Cend, word Addr)
{ //assume start value < end value
  int colstep;

  if(Addr==VGA)
  {
    __asm__ __volatile__
    (


                         /* optimisation of the above*/
       "movl  $0xA0000,%%edi\n\t"
       "movw  %0,%%es\n\t"      /*load es with selector*/
       "addl  %5,%%edi\n\t"
       "movl  %2,%%ecx\n\t"
       "addl  %1,%%edi\n\t"
       "movl  %4,%%eax\n\t"
       "subl  %1,%%ecx\n\t"
       "movb  %%al,%%ah\n\t"
      /* "incl  %%ecx   */
       "shrl  $1,%%ecx\n\t"
       "jnc    blast2\n\t"
       "stosb\n\t"

"blast3:\n\t"
       "rep\n\t"
       "stosw\n\t"
  
  :
  : "g" (_dos_ds), "g" (XStart), "g" (XEnd), "g" (Y),"g" (Cstart), "g" (ytab[Y])
  : "edi","eax","ecx"
  );
  }
  else
  {
    __asm__ __volatile__
    (
       "movw  %%ds,%%ax\n\t"      /*load es with selector*/
       "movl  %0,%%edi\n\t"
       "movw  %%ax,%%es\n\t"
       "addl  %5,%%edi\n\t"
       "movl  %2,%%ecx\n\t"
       "addl  %1,%%edi\n\t"
       "movl  %4,%%eax\n\t"
       "subl  %1,%%ecx\n\t"
       "movb  %%al,%%ah\n\t"
       "incl  %%ecx\n\t"
       "shrl  $1,%%ecx\n\t"
       "jnc    blast2_2\n\t"
       "stosb\n\t"
"blast3_2:\n\t"
       "rep\n\t"
       "stosw\n\t"
  
  :
  : "g" (Addr), "g" (XStart), "g" (XEnd), "g" (Y), "g" (Cstart),"g" (ytab[Y])
  : "edi","eax","ecx"
  );
  }
}


