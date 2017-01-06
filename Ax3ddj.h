/////////////////////////////////////////////////////////////////////////////
//                     VGA Mode 13h Codes By A x I o M                     //
/////////////////////////////////////////////////////////////////////////////

//Ported to djgpp...which nearly made me pull my hair out...
//Supports 32-bit registers using AT&T assembler syntax

//--------------------- Data Structures n Constants ------------------------//

#define VGA 0xA0000
#define maxvert 10000
#define maxface 10000

double pidiv180 = 0.017453292;

typedef unsigned char byte;
typedef unsigned int word; //word value is 4 bytes in djgpp, short is 2 bytes


typedef struct  //lightsource vector
{
          float x, y, z;
} lightvect;

typedef struct    //structure used to store the vertices of polygons in 2D
{
          int x, y;
} Coord;

typedef struct  //structure used to store the vertices of polygons in 3D
{
          float x, y, z;
            float nx, ny, nz;
              int col;
} Coord3d;

typedef struct // polygon... which points to ths vertices list for the 3 pts
{
          int n;
            float mz;
} psortuple;

typedef struct // polygon... which points to ths vertices list for the 3 pts
{
          int v1,v2,v3;
            float nx,ny,nz;
              float mx,my,mz;
} polygon;
                   //no of vertices
typedef struct // base 3d object
{
          int numfaces; //no of faces of the object
            int numvertices;
              Coord3d vertices[maxvert]; // array of 3d pts that form the object
                polygon polys[maxface]; //array of polygon that forms the object
                  psortuple sarr[maxface];
} object;

typedef struct  //structure that contains info about the number of vertices
{                //of a polygon and a pointer to the list of vertices
          int NoOfPts;   //number of pts
            Coord *CoordPtr; //pointer to a list of pts
} PointList;

typedef struct  //structure that contains the starting and ending x-coords
{              //for drawing horizontal lines
          int xstart;
            int xend;
} Horline;

typedef struct  //structure that contains a pointer the list lines of to
{                  //be drawn and info used in drawing
          int NoOfLines;   //number of pts
            int Ystart;      // y value of first line
              Horline *HorlinePtr; //pointer to a list of horizontal lines
} HorlineList;

//------------------------ Variable Declarations ---------------------------//

//variables for asmline routine
int inc_up,inc_down,xmove_if_up,xmove_if_down,ymove_if_up,ymove_if_down;

int ytab[200];           // lookup table for y values, for faster rendering
double sine[721];        // sine table
double cosine[721];      // cosine table
short edgelist[200][2];  // list of start and end points for polygon rendering
int cedgelist[200][2];   // list of start and end colors for gouraud shading
float nedgelist[200][2][3]; // list of start and end normals for phong shading
lightvect light;         // the light vector, coordinates of the lightsource.

byte *virscr;            // pointer to screen buffer
unsigned char *vgascr=(unsigned char *)0xA0000; //pointer to VGA screen

//--------------------- initialization routines ----------------------------//

void initmode(void);     // switch to mode 13h
void inittext(void);     // switch to textmode
void initytab(void);     // setup lookup table for y
void initsinus(void);    // setup sinus lookup
void initvirscr(void);   // allocates memory for the virtual screen
void initpal(float ra, float rd, float rs,  //sets up the palette using the
                             float ga, float gd, float gs,  //phong illumination model
                                          float ba, float bd, float bs, int n);
void initsystem(void);   // call the basic initialisation routines
void freevirscr(void);   // deallocates memory for the virtual screen

//--------------------- screen manipulation routines -----------------------//

void swapint(int *a,int *b); // swaps two ints... pass address using &
void swapfloat(float *a,float *b);// swaps two floats... pass address using &
void vtrace(void);       // wait for vertical retrace
void htrace(void);       // wait for vertical retrace
void showpal(void);      // shows palette on scr
void setcol(byte ColNo, byte r, byte g, byte b); // sets the col for ColNo
void getcol(int ColNo, int *r, int *g, int *b);// gets the col for ColNo
void plotpix(int x,int y,word color, int Addr);//plotpix using inline asm
void plotpix1(int x, int y, byte Col, int Addr);//plotpix using farpokeb
void plotpix2(int x, int y, byte Col, int Addr);//plotpix using nearptr
void plotpix3(int x,int y,int  color,int Addr);  //2nd plotpix in inline asm
void swaptoscr(int Src);// swap screen buffer to video memory
void swapbufs(word Src,word Dest);//swap from source to dest buffer
void clrvidscr(byte Col);// clrs screen to col
void clrbuf(int Col,word Addr);// clrs screen buffers to col

//--------------------- Polygon Fill and Line Routines ---------------------//

void line(short x1, short y1, short x2, short y2, byte Col, word Addr);// draws line
void asmline2(int x1, int y1, int x2, int y2, int col, word Location);
void hline(int x1, int y, int x2, int Col, word Addr);
void hline2(int XStart, int Y,int XEnd, int Col, word Addr);
void ghline(int XStart, int Y, int XEnd, int Cstart, int Cend, word Addr);
void hglenzline(int x1,int y,int x2,int col, int location);
void hglenzline2(int x1,int y,int x2,int col,word location);
void scanedge(int xx1,int yy1,int xx2,int yy2);
void scanedge2(int x1,int y1,int x2,int y2);
void cscanedge2(int x1, int y1, int c1, int x2, int y2, int c2);
void nscanedge2(int x1, int y1, float nx1, float ny1, float nz1,
                                int x2, int y2, float nx2, float ny2, float nz2);
void glenzpoly(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,
                           int col, int location);
void glenztri(int x1,int y1,int x2,int y2,int x3,int y3,int col, int location);
void FillPoly(PointList *CoordList,unsigned char col, unsigned int Addr);
void filltri(int x1,int y1,int x2, int y2,int x3,int y3
                         ,int col,word add);
void gfilltri(int x1, int y1, int x2, int y2, int x3, int y3,
                             int c1, int c2, int c3, word add);
void phongtri(int x1, int y1, int x2, int y2, int x3, int y3,
                              float nx1, float ny1, float nz1, float nx2, float ny2, float nz2,
                                            float nx3, float ny3, float nz3, word add);
void ScanEdge(int x1, int y1, int x2, int y2, int LeftEdge,
                          int SkipTop, Horline **ScanlinePtr);
void DrawHList(HorlineList *Hptr,unsigned char col, unsigned int Addr);
void loadobj(char filename[255], object *obj);//loads object
int psortcmpto(const void *e1, const void *e2);
void blurmotion(int Addr);

//------------------- 3d object translation routines -----------------------//

void rotateobj(int deg_ab_x, int deg_ab_y, int deg_ab_z,
                               object *src, object *dest);
void rotatenorm(int deg_ab_x, int deg_ab_y, int deg_ab_z,
                                object *src, object *dest);
void genmidpt(object *obj);   //generate polygon normals of the object.
void genvertcol(object *obj); //generate vertex color for goraud shading.
void genpolynorm(object *obj);
void genvertnorm(object *obj);


