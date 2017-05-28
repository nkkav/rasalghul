/**********************************************************************/
/*  File       : pnmgen.c
 *  Description: Testbed for image synthesis patterns.
 *  Author     : Nikolaos Kavvadias <nikos@nkavvadias.com>
 *  Copyright  : Nikolaos Kavvadias (C) 2012, 2013, 2014, 2015, 2016
 *  Website    : http://www.nkavvadias.com                            
 *
 *  This file is part of rasalghul, and is distributed under the terms 
 *  of the Modified BSD License.
 *
 *  A copy of the Modified BSD License is included with this 
 *  distribution in the file LICENSE.
 *  rasalghul is free software: you can redistribute it and/or modify it 
 *  under the terms of the Modified BSD License. 
 *  rasalghul is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 *  Modified BSD License for more details.
 * 
 *  You should have received a copy of the Modified BSD License along 
 *  with rasalghul. If not, see <http://www.gnu.org/licenses/>. 
 */
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIDTH
#define WIDTH  800
#endif
#ifndef HEIGHT
#define HEIGHT 600
#endif

#define MASK(x,y)         ((x) & (y))
#define ABS(x)            ((x) >  0   ? (x) : (-x))
#define MAX(x, y)         ((x) >  (y) ? (x) : (y))
#define MIN(x, y)         ((x) <  (y) ? (x) : (y))
#define AVG(x, y)         (((x) + (y)) >> 1)
#define CLAMP(x,L,H)      ((x)<=(L) ? (L) : (x)<=(H) ? (x) : (H))

#define GREY_AND          0x00
#define RGB_AND           0x01
#define GREY_IOR          0x02
#define RGB_IOR           0x03
#define GREY_XOR          0x04
#define RGB_XOR           0x05
#define GREY_ADD          0x06
#define RGB_ADD           0x07
#define GREY_MUL          0x08
#define RGB_MUL           0x09
#define DEKCHAOS          0x0A
#define GRAD_RG           0x0B
#define GRAD_GB           0x0C
#define GRAD_RB           0x0D
#define ADDSUBXOR         0x0E
#define MULADDSUB         0x0F
#define GREY_DIV          0x10
#define DIV_RG            0x11
#define POLY              0x12
#define RWALK             0x13
#define EDA               0x14
#define GREY_MAXSQ        0x15
#define GREY_MINSQ        0x16
#define GREY_AVGSQ        0x17
#define GREY_MAX          0x18
#define GREY_MIN          0x19
#define GREY_AVG          0x1A
#define MAXMINAVGSQ       0x1B
#define MAXMINAVG         0x1C
#define GREY_AVGCB        0x1D
#define GREY_AVGDIV       0x1E

int enable_help=0, enable_par=0, enable_random_walk=0, enable_binop=0;
int pnmgen_mode=GREY_AND, param_val=0, binop_mask=0xFF;
int xsize=WIDTH, ysize=HEIGHT;

unsigned int seed[1]={1};

#ifdef RANDUNIX        // bad!!!
#define rand genrand
unsigned int genrand(void)
{
  unsigned int t, val;
  // X_{n+1} = (a * X_{n} + c) (mod m)
  // a = 232, c = 1103515245, m = 12345 (glibc)
  t = (232 * seed[0] + 1103515245) % 12345;
  val = t & 0x7FFFFFFF;  
  seed[0] = val;
  return (val);
}
#endif
#ifdef RAND31        // looks good
#define rand rand31
unsigned int rand31(void)
{
  unsigned int hi, lo;
  unsigned int val;
  lo = 16807 * (seed[0] & 0xFFFF);
  hi = 16807 * (seed[0] >> 16);
  lo += (hi & 0x7FFF) << 16;
  lo += hi >> 15;
  if (lo > 0x7FFFFFFF) {
    lo -= 0x7FFFFFFF;
  }
  val = lo;
  seed[0] = val;
  return (val);
}
#endif
#ifdef XORSHIFT        // ok
#define rand xorshift
unsigned int xorshift(void) 
{ 
  static unsigned int x[1] = {123456789};
  static unsigned int y[1] = {362436069};
  static unsigned int z[1] = {521288629};
  static unsigned int w[1] = {88675123}; 
  unsigned int rngval;
  unsigned int t;
 
  t = x[0] ^ (x[0] << 11);
  x[0] = y[0]; 
  y[0] = z[0]; 
  z[0] = w[0];
  w[0] = w[0] ^ (w[0] >> 19) ^ t ^ (t >> 8); 
  rngval = w[0];
  return (rngval);
}
#endif
#ifdef RANDMWC
#define rand randmwc        // bad!!!
unsigned int randmwc(void)
{
  unsigned int m_z[1] = {1}, m_w[1] = {2};
  unsigned int val;
  m_z[0] = 36969 * (m_z[0] & 65535) + (m_z[0] >> 16);
  m_w[0] = 18000 * (m_w[0] & 65535) + (m_w[0] >> 16);
  val = (m_z[0] << 16) + m_w[0];  /* 32-bit result */
  return (val);
}
#endif


/* generate_random_walk:
 * Generate an image of a random walk.
 */
void generate_random_walk(FILE *f, int _height, int _width, int num_steps)
{
  int x, y, r;
  int i, p1_addr;
  unsigned int iseed = 7;
  int *image_data;
  
  image_data = malloc((3 * _height * _width) * sizeof(int));
   
  for (x = 0; x < _width; x++) {
    for (y = 0; y < _height; y++) {
      p1_addr = x * _height + y;
      image_data[p1_addr] = 0xFF;
    }
  }

  x = _width/2 - 1;
  y = _height/2 - 1;

  srand(iseed);
  i = 0;  
  while (i < num_steps) {
    r = rand() & 0x3; 
    switch (r) {
      case 0:
        x = x + 1;
        x = CLAMP(x, 0, _width-1);
        break;
      case 1:
        y = y - 1;
        y = CLAMP(y, 0, _height-1);
        break;
      case 2:
        x = x - 1;
        x = CLAMP(x, 0, _width-1);
        break;
      case 3:
        y = y + 1;
        y = CLAMP(y, 0, _height-1);
        break;
      default:
        break;
    }
    p1_addr = x * _height + y;
    image_data[p1_addr]--;
    i++;
  }

  for (x = 0; x < _width; x++) {
    for (y = 0; y < _height; y++) {
      p1_addr = x * _height + y;
      fprintf(f, "%d %d %d\n", MASK(image_data[p1_addr],binop_mask), 
                               MASK(image_data[p1_addr],binop_mask), 
                               MASK(image_data[p1_addr],binop_mask));
    }
  }    
  free(image_data);
}

unsigned int eda(int x0, int y0, int in1, int in2)
{
  int a, b;
  unsigned int x, y, t1, t2, t3, t4, t5, t6, t7;
  a = in1 - x0;
  b = in2 - y0;
  t1 = ABS(a);
  t2 = ABS(b);
  x = MAX(t1, t2);
  y = MIN(t1, t2);
  t3 = x >> 3;
  t4 = y >> 1;
  t5 = x - t3;
  t6 = t4 + t5;
  t7 = MAX(t6, x);
  return (t7);
}


/* print_usage:
 * Print usage instructions for the "pnmgen" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* pnmgen [options]\n");
  printf("* \n");
  printf("* Options:\n");
  printf("*   -h:              Print this help.\n");
  printf("*   -x <num>:        Read X-dimension size of input image.\n");
  printf("*   -y <num>:        Read Y-dimension size of input image.\n");
  printf("*   -rwalk <num>:    Generate an image of a random walk of <par> steps,\n");
  printf("*                    masked by <num>.\n");
  printf("*   -xy<binop> <num>:\n");
  printf("*                    Generate patterns using x binop y for pixel (x,y)\n");
  printf("*                    masked by <num>.\n");
  printf("*   -par <num>:      Additional configuration parameter, used where needed:\n");
  printf("*                    {-xydekchaos, -rwalk}.\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n\n");
}

/* pnmgen:
 * The main "pnmgen" routine.
 */
int main(int argc, char **argv)
{
  int i;
  int x,y;
  char *binop_str = NULL, *fname = NULL;
  FILE *f;

  // Read input arguments
  if (argc < 2)
  {
    print_usage();
    exit(1);
  }

  for (i=1; i < argc; i++)
  {
    if (strcmp("-h",argv[i]) == 0)
    {
      print_usage();
      exit(1);
    }
    else if (strcmp("-par", argv[i]) == 0)
    {
      enable_par = 1;
      if ((i+1) < argc)
      {
        i++;
        param_val = atoi(argv[i]);
      }
    }
    else if (strcmp("-rwalk", argv[i]) == 0)
    {
      enable_random_walk = 1;
      binop_str = malloc((strlen(argv[i])+1) * sizeof(char));
      strcpy(binop_str, "rwalk");
      pnmgen_mode = RWALK;
      if ((i+1) < argc)
      {
        i++;
        binop_mask = atoi(argv[i]);
      }
    }
    else if (strncmp("-xy", argv[i], 3) == 0)
    {
      enable_binop = 1;
      binop_str = malloc((strlen(argv[i])+1) * sizeof(char));
      strcpy(binop_str, argv[i]+3);
      if (strcmp(binop_str, "greyand") == 0)
      {
        pnmgen_mode = GREY_AND;
      }
      else if (strcmp(binop_str, "rgband") == 0)
      {
        pnmgen_mode = RGB_AND;
      }
      else if (strcmp(binop_str, "greyior") == 0)
      {
        pnmgen_mode = GREY_IOR;
      }
      else if (strcmp(binop_str, "rgbior") == 0)
      {
        pnmgen_mode = RGB_IOR;
      }
      else if (strcmp(binop_str, "greyxor") == 0)
      {
        pnmgen_mode = GREY_XOR;
      }
      else if (strcmp(binop_str, "rgbxor") == 0)
      {
        pnmgen_mode = RGB_XOR;
      }
      else if (strcmp(binop_str, "greyadd") == 0)
      {
        pnmgen_mode = GREY_ADD;
      }
      else if (strcmp(binop_str, "rgbadd") == 0)
      {
        pnmgen_mode = RGB_ADD;
      }
      else if (strcmp(binop_str, "greymul") == 0)
      {
        pnmgen_mode = GREY_MUL;
      }
      else if (strcmp(binop_str, "rgbmul") == 0)
      {
        pnmgen_mode = RGB_MUL;
      }
      else if (strcmp(binop_str, "dekchaos") == 0)
      {
        pnmgen_mode = DEKCHAOS;
      }
      else if (strcmp(binop_str, "gradrg") == 0)
      {
        pnmgen_mode = GRAD_RG;
      }
      else if (strcmp(binop_str, "gradgb") == 0)
      {
        pnmgen_mode = GRAD_GB;
      }
      else if (strcmp(binop_str, "gradrb") == 0)
      {
        pnmgen_mode = GRAD_RB;
      }
      else if (strcmp(binop_str, "addsubxor") == 0)
      {
        pnmgen_mode = ADDSUBXOR;
      }
      else if (strcmp(binop_str, "muladdsub") == 0)
      {
        pnmgen_mode = MULADDSUB;
      }
      else if (strcmp(binop_str, "greydiv") == 0)
      {
        pnmgen_mode = GREY_DIV;
      }
      else if (strcmp(binop_str, "divrg") == 0)
      {
        pnmgen_mode = DIV_RG;
      }
      else if (strcmp(binop_str, "poly") == 0)
      {
        pnmgen_mode = POLY;
      }
      else if (strcmp(binop_str, "eda") == 0)
      {
        pnmgen_mode = EDA;
      }
      else if (strcmp(binop_str, "greymaxsq") == 0)
      {
        pnmgen_mode = GREY_MAXSQ;
      }
      else if (strcmp(binop_str, "greyminsq") == 0)
      {
        pnmgen_mode = GREY_MINSQ;
      }
      else if (strcmp(binop_str, "greyavgsq") == 0)
      {
        pnmgen_mode = GREY_AVGSQ;
      }
      else if (strcmp(binop_str, "greymax") == 0)
      {
        pnmgen_mode = GREY_MAX;
      }
      else if (strcmp(binop_str, "greymin") == 0)
      {
        pnmgen_mode = GREY_MIN;
      }
      else if (strcmp(binop_str, "greyavg") == 0)
      {
        pnmgen_mode = GREY_AVG;
      }
      else if (strcmp(binop_str, "maxminavgsq") == 0)
      {
        pnmgen_mode = MAXMINAVGSQ;
      }
      else if (strcmp(binop_str, "maxminavg") == 0)
      {
        pnmgen_mode = MAXMINAVG;
      }
      else if (strcmp(binop_str, "greyavgcb") == 0)
      {
        pnmgen_mode = GREY_AVGCB;
      }
      else if (strcmp(binop_str, "greyavgdiv") == 0)
      {
        pnmgen_mode = GREY_AVGDIV;
      }
      else
      {
        fprintf(stderr, "Error: Unknown operation for image generation.\n");
        exit(1);
      }
      if ((i+1) < argc)
      {
        i++;
        binop_mask = atoi(argv[i]);
      }
    }
    else if (strcmp("-x",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        xsize = atoi(argv[i]);
      }
    }
    else if (strcmp("-y",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        ysize = atoi(argv[i]);
      }
    }
  }

  fname = malloc((strlen(binop_str)+25) * sizeof(char));
  if ((pnmgen_mode == DEKCHAOS) ||
      (pnmgen_mode == RWALK)) {
    sprintf(fname, "xy%s-%d.ppm", binop_str, param_val);
  } else {
    sprintf(fname, "xy%s.ppm", binop_str);
  }
  if ((f = fopen(fname, "w")) == NULL)
  {
    fprintf(stderr, "Error: Can't create the specified PPM file for export.\n");
    exit (1);
  }
 
  fprintf(f, "P3\n");
  fprintf(f, "%i %i\n", xsize, ysize);
  fprintf(f, "255\n");
  
  if (pnmgen_mode == RWALK) {
    generate_random_walk(f, ysize, xsize, param_val);  
  } else {
  for (x = 0; x < xsize; x++)
  {
    for (y = 0; y < ysize; y++)
    {
       switch (pnmgen_mode) {
         case GREY_AND:
           fprintf(f, "%d %d %d\n", MASK(x&y,binop_mask), MASK(x&y,binop_mask), MASK(x&y,binop_mask));
           break;
         case RGB_AND:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(y,binop_mask), MASK(x&y,binop_mask));
           break;
         case GREY_IOR:
           fprintf(f, "%d %d %d\n", MASK(x|y,binop_mask), MASK(x|y,binop_mask), MASK(x|y,binop_mask));
           break;
         case RGB_IOR:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(y,binop_mask), MASK(x|y,binop_mask));
           break;
         case GREY_XOR:
           fprintf(f, "%d %d %d\n", MASK(x^y,binop_mask), MASK(x^y,binop_mask), MASK(x^y,binop_mask));
           break;
         case RGB_XOR:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(y,binop_mask), MASK(x^y,binop_mask));
           break;
         case GREY_ADD:
           fprintf(f, "%d %d %d\n", MASK(x+y,binop_mask), MASK(x+y,binop_mask), MASK(x+y,binop_mask));
           break;
         case RGB_ADD:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(x+y,binop_mask), MASK(x+y,binop_mask));
           break;
         case GREY_MUL:
           fprintf(f, "%d %d %d\n", MASK(x*y,binop_mask), MASK(x*y,binop_mask), MASK(x*y,binop_mask));
           break;
         case RGB_MUL:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(x*y,binop_mask), MASK(x*y,binop_mask));
           break;
         case DEKCHAOS:
           fprintf(f, "%d %d %d\n", MASK(((x*x*y)>>param_val),1) == 1 ? 0x00 : 0xFF, 
                                    MASK(((x*x*y)>>param_val),1) == 1 ? 0x00 : 0xFF, 
                                    MASK(((x*x*y)>>param_val),1) == 1 ? 0x00 : 0xFF);
           break;
         case GRAD_RG:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(y,binop_mask), MASK(0,binop_mask));
           break;
         case GRAD_GB:
           fprintf(f, "%d %d %d\n", MASK(0,binop_mask), MASK(x,binop_mask), MASK(y,binop_mask));
           break;
         case GRAD_RB:
           fprintf(f, "%d %d %d\n", MASK(x,binop_mask), MASK(0,binop_mask), MASK(y,binop_mask));
           break;
         case ADDSUBXOR:
           fprintf(f, "%d %d %d\n", MASK(x+y,binop_mask), MASK(x-y,binop_mask), MASK(x^y,binop_mask));
           break;
         case MULADDSUB:
           fprintf(f, "%d %d %d\n", MASK(x*y,binop_mask), MASK(x+y,binop_mask), MASK(x-y,binop_mask));
           break;
         case GREY_DIV:
           fprintf(f, "%d %d %d\n", MASK((x*x/(y+1)),binop_mask), MASK((x*x/(y+1)),binop_mask), MASK((x*x/(y+1)),binop_mask));
           break;
         case DIV_RG:
           fprintf(f, "%d %d %d\n", MASK((x*x/(y+1)),binop_mask), MASK((y*y/(x+1)),binop_mask), MASK(128,binop_mask));
           break;
         case POLY:
           fprintf(f, "%d %d %d\n", MASK(x*x+y*y-x*y,binop_mask), MASK(x*x+y*y-x*y,binop_mask), MASK(x*x+y*y-x*y,binop_mask));
           break;
         case EDA:
           fprintf(f, "%d %d %d\n", MASK(eda(xsize/2-1,ysize/2-1,x,y),binop_mask), 
                                    MASK(eda(xsize/2-1,ysize/2-1,x,y),binop_mask), 
                                    MASK(eda(xsize/2-1,ysize/2-1,x,y),binop_mask));
           break;
         case GREY_MAXSQ:
           fprintf(f, "%d %d %d\n", MASK(MAX(x*x,y*y),binop_mask), MASK(MAX(x*x,y*y),binop_mask), MASK(MAX(x*x,y*y),binop_mask));
           break;
         case GREY_MINSQ:
           fprintf(f, "%d %d %d\n", MASK(MIN(x*x,y*y),binop_mask), MASK(MIN(x*x,y*y),binop_mask), MASK(MIN(x*x,y*y),binop_mask));
           break;
         case GREY_AVGSQ:
           fprintf(f, "%d %d %d\n", MASK(AVG(x*x,y*y),binop_mask), MASK(AVG(x*x,y*y),binop_mask), MASK(AVG(x*x,y*y),binop_mask));
           break;
         case GREY_MAX:
           fprintf(f, "%d %d %d\n", MASK(MAX(x,y),binop_mask), MASK(MAX(x,y),binop_mask), MASK(MAX(x,y),binop_mask));
           break;
         case GREY_MIN:
           fprintf(f, "%d %d %d\n", MASK(MIN(x,y),binop_mask), MASK(MIN(x,y),binop_mask), MASK(MIN(x,y),binop_mask));
           break;
         case GREY_AVG:
           fprintf(f, "%d %d %d\n", MASK(AVG(x,y),binop_mask), MASK(AVG(x,y),binop_mask), MASK(AVG(x,y),binop_mask));
           break;
         case MAXMINAVGSQ:
           fprintf(f, "%d %d %d\n", MASK(MAX(x*x,y*y),binop_mask), MASK(MIN(x*x,y*y),binop_mask), MASK(AVG(x*x,y*y),binop_mask));
           break;
         case MAXMINAVG:
           fprintf(f, "%d %d %d\n", MASK(MAX(x,y),binop_mask), MASK(MIN(x,y),binop_mask), MASK(AVG(x,y),binop_mask));
           break;
         case GREY_AVGCB:
           fprintf(f, "%d %d %d\n", MASK(AVG(x*x*x,y*y*y),binop_mask), MASK(AVG(x*x*x,y*y*y),binop_mask), MASK(AVG(x*x*x,y*y*y),binop_mask));
           break;
         case GREY_AVGDIV:
           fprintf(f, "%d %d %d\n", MASK(AVG(x*x/(y+1),y*y/(x+1)),binop_mask), MASK(AVG(x*x/(y+1),y*y/(x+1)),binop_mask), MASK(AVG(x*x/(y+1),y*y/(x+1)),binop_mask));
           break;
         default:
           break;
      }
    }
  }
  }
  free(binop_str);
  free(fname);
  fclose(f);
  return 0;
}
