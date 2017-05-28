/**********************************************************************/
/*  File       : ellipsedraw.c
 *  Description: Testbed for ellipse drawing algorithms:
 *               - Basic algorithm as attested by Alois Zingl [4Q]
 *               - Optimized algorithm as attested by Alois Zingl [4Q]
 *
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
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define XSIZE_DEFAULT    256
#define YSIZE_DEFAULT    256
#define MAX_COLOR_LEVELS 256

/* Swap two ints without using a temporary. */
#define SWAP(x, y)        do { x ^= y; y ^= x; x ^= y; } while (0)
/* Absolute value. */
#define ABS(x)            ((x) > 0 ? (x) : (-x))
/* Maximum of two values. */
#define MAX(x,y)          ((x) > (y) ? (x) : (y))
/* Minimum of two values. */
#define MIN(x,y)          ((x) < (y) ? (x) : (y))
/* Macros for even and odd. */
#define EVEN(x)           !(x&1)
#define ODD(x)            (x&1)


FILE *outfile;
char *outfile_name = NULL;
int *image_data;
unsigned int current_color=0x0;
//
int enable_pbm=1,enable_pgm=0;
int enable_azbasic=1, enable_azopt=0;
int x_dim=XSIZE_DEFAULT, y_dim=YSIZE_DEFAULT;


/* write_pbm_file:
 * Write the contents of a PBM (portable bit map) file.
 */
void write_pbm_file(FILE *f, int *img_out, char *img_out_fname, 
  int x_size, int y_size, int x_scale_val, int y_scale_val, int linevals,
  int is_ascii)
{
  int i, j, x_scaled_size, y_scaled_size;
  int k, v, temp, step;
 
  x_scaled_size = x_size * x_scale_val;
  y_scaled_size = y_size * y_scale_val; 
  /* Write the magic number string. */
  if (is_ascii == 1) {
    fprintf(f, "P1\n");
  step = 1;
  } else {
    fprintf(f, "P4\n");
  step = 8;
  }
  /* Write a comment containing the file name. */
  fprintf(f, "# %s\n", img_out_fname);
  /* Write the image dimensions. */
  fprintf(f, "%d %d\n", x_scaled_size, y_scaled_size);
  
  /* Write the image data. */
  for (i = 0; i < y_scaled_size; i++) {
    for (j = 0; j < x_scaled_size; j+=step) {
      if (is_ascii == 1) {
        fprintf(f, "%d ", img_out[i*x_scaled_size+j]);
      } else {
        temp = 0;
        for (k = 0; k < 8; k++) {
          v = img_out[i*x_scaled_size+j+k];
          temp |= (v << (7-k));
        }
        fprintf(f, "%c", temp);
      }
      if (((i*x_scaled_size+j) % linevals) == (linevals-1)) {
        fprintf(f, "\n");
      }
    }
  }   
  fclose(f);
}

/* write_pgm_file:
 * Write the contents of a PGM (portable grey map) file.
 */
void write_pgm_file(FILE *f, int *img_out, char *img_out_fname, 
  int x_size, int y_size, int x_scale_val, int y_scale_val, 
  int img_colors, int linevals, int is_ascii)
{
  int i, j, x_scaled_size, y_scaled_size;
 
  x_scaled_size = x_size * x_scale_val;
  y_scaled_size = y_size * y_scale_val; 
  /* Write the magic number string. */
  if (is_ascii == 1) {
    fprintf(f, "P2\n");
  } else {
    fprintf(f, "P5\n");
  }
  /* Write a comment containing the file name. */
  fprintf(f, "# %s\n", img_out_fname);
  /* Write the image dimensions. */
  fprintf(f, "%d %d\n", x_scaled_size, y_scaled_size);
  /* Write the maximum color/grey level allowed. */
  fprintf(f, "%d\n", img_colors);
  
  /* Write the image data. */
  for (i = 0; i < y_scaled_size; i++) {
    for (j = 0; j < x_scaled_size; j++) {
      if (is_ascii == 1) {
        fprintf(f, "%d ", img_out[i*x_scaled_size+j]);
        if (((i*x_scaled_size+j) % linevals) == (linevals-1)) {
          fprintf(f, "\n");
        }
      } else {
        fprintf(f, "%c", img_out[i*x_scaled_size+j]);
      }
    }
  } 
  fclose(f);
}

/* azplotellipsebasic:
 * C implementation for an ellipse generation algorithm. 
 * Original author: Alois Zingl
 * Source: http://free.pages.at/easyfilter/bresenham.html
 */ 
void azplotellipsebasic(int xm, int ym, int a, int b)
{
  int x = -a, y = 0;   /* II. quadrant from bottom left to top right */
  int e2 = b*b, err = x*(2*e2+x)+e2;              /* error of 1.step */
 
  do {
    // SETPIXEL(xm-x, ym+y);            /*   I. Quadrant */
    image_data[(xm-x) + x_dim*(ym+y)] = current_color; 
    // SETPIXEL(xm+x, ym+y);            /*  II. Quadrant */
    image_data[(xm+x) + x_dim*(ym+y)] = current_color; 
    // SETPIXEL(xm+x, ym-y);            /* III. Quadrant */   
    image_data[(xm+x) + x_dim*(ym-y)] = current_color; 
    // SETPIXEL(xm-x, ym-y);            /*  IV. Quadrant */
    image_data[(xm-x) + x_dim*(ym-y)] = current_color; 
    // error accumulation        
    e2 = 2*err;
    if (e2 >= (x*2+1)*(int)b*b) {       /* e_xy+e_x > 0 */
      err += (++x*2+1)*(int)b*b; 
    }
    if (e2 <= (y*2+1)*(int)a*a) {       /* e_xy+e_y < 0 */ 
      err += (++y*2+1)*(int)a*a;
    }
  } while (x <= 0);
 
  while (y++ < b) {          /* too early stop of flat ellipses a=1, */  
    // SETPIXEL(xm, ym+y);               /* -> finish tip of ellipse */
    image_data[(xm) + x_dim*(ym+y)] = current_color;
    // SETPIXEL(xm, ym-y);
    image_data[(xm) + x_dim*(ym-y)] = current_color; 
  }
}

/* azplotellipseopt:
 * C implementation for an optimized ellipse generation algorithm. 
 * Original author: Alois Zingl
 * Source: http://free.pages.at/easyfilter/bresenham.html
 */ 
void azplotellipseopt(int xm, int ym, int a, int b)
{
  int x = -a, y = 0; /* II. quadrant from bottom left to top right */
  int e2 = b, dx = (1+2*x)*e2*e2;              /* error increment  */
  int dy = x*x, err = dx+dy;                    /* error of 1.step */
 
  do {
    // SETPIXEL(xm-x, ym+y);            /*   I. Quadrant */
    image_data[(xm-x) + x_dim*(ym+y)] = current_color;     
    // SETPIXEL(xm+x, ym+y);            /*  II. Quadrant */ 
    image_data[(xm+x) + x_dim*(ym+y)] = current_color;     
    // SETPIXEL(xm+x, ym-y);            /* III. Quadrant */
    image_data[(xm+x) + x_dim*(ym-y)] = current_color;     
    // SETPIXEL(xm-x, ym-y);            /*  IV. Quadrant */
    image_data[(xm-x) + x_dim*(ym-y)] = current_color;     
    // error accumulation    
    e2 = 2*err;
    if (e2 >= dx) { 
      x++; 
      err += dx += 2*(int)b*b; 
    }      /* x step */
    if (e2 <= dy) { 
      y++; 
      err += dy += 2*(int)a*a; 
    }      /* y step */
  } while (x <= 0);
 
  while (y++ < b) {   /* too early stop for flat ellipses with a=1, */    
    // SETPIXEL(xm, ym+y);              /* -> finish tip of ellipse */
    image_data[(xm) + x_dim*(ym+y)] = current_color;          
    // SETPIXEL(xm, ym-y);
    image_data[(xm) + x_dim*(ym-y)] = current_color;     
  }
}

/* ellipsedraw:
 * Select ellipse drawing algorithm.
 */
void ellipsedraw(int xm, int ym, int a, int b)
{
  if (enable_azbasic == 1) {
    azplotellipsebasic(xm, ym, a, b);
  } 
  else if (enable_azopt == 1) {
    azplotellipseopt(xm, ym, a, b);
  }
}

/* print_usage:
 * Print usage instructions for the "ellipsedraw" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* ./ellipsedraw [options] <outfile>\n");
  printf("* \n");
  printf("* Options:\n");
  printf("*   -h:              Print this help.\n");
  printf("*   -b:              Use the basic algorithm by Alois Zingl (default).\n");
  printf("*   -o:              Use the optimized algorithm by Alois Zingl.\n");
  printf("*   -pbm:            Generate a PBM image (default).\n");
  printf("*   -pgm:            Generate a PGM image.\n");
  printf("*   -x <num>:        Value for the x-dimension of the image (default:256).\n");
  printf("*   -y <num>:        Value for the y-dimension of the image (default:256).\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n\n");
}

/* main:
 * The main "ellipsedraw" routine.
 */
int main(int argc, char **argv)
{
  int i;
  int xm=0, xm_offset=0, ym=0, a=0, b=0, t0, t1, t2, t3;
  int iseed = 7;

  // Read input arguments.
  if (argc < 2) {
    print_usage();
    exit(1);
  }

  for (i = 1; i < argc; i++) {
    if (strcmp("-h", argv[i]) == 0) {
      print_usage();
      exit(1);
    }
    else if (strcmp("-b", argv[i]) == 0) {
      enable_azbasic  = 1;
      enable_azopt    = 0;
    }
    else if (strcmp("-o", argv[i]) == 0) {
      enable_azbasic  = 0;
      enable_azopt    = 1;
    }
    else if (strcmp("-pbm", argv[i]) == 0) {
      enable_pbm      = 1;
      enable_pgm      = 0;
    }
    else if (strcmp("-pgm", argv[i]) == 0) {
      enable_pbm      = 0;
      enable_pgm      = 1;
    } 
    else if (strcmp("-x", argv[i]) == 0) {
      if ((i+1) < argc) {
        i++;
        x_dim = atoi(argv[i]);
      }
    }
    else if (strcmp("-y", argv[i]) == 0) {
      if ((i+1) < argc) {
        i++;
        y_dim = atoi(argv[i]);
      }
    }
    else {
      fprintf(stderr, "Error: Unknown command-line option: %s.\n", argv[i]);
      exit(1);      
    }
  }
  
  // Allocate space for image_data.
  image_data = malloc(x_dim * y_dim * sizeof(int));
  // Initialize image data.
  for (i = 0; i < x_dim*y_dim; i++) {
    image_data[i] = 0x00;
  }

  outfile_name = malloc((strlen("azellipse-basic")+
                   strlen(".pxm")+1) * sizeof(char));

  if (enable_azbasic == 1) {
    strcpy(outfile_name, "azellipse-basic");
  } 
  else if (enable_azopt == 1) {
    strcpy(outfile_name, "azellipse-opt");
  } else {
    fprintf(stderr, "Error: No ellipse generation algorithm specified.\n");
    exit (1);
  }

  if (enable_pbm == 1) {
    strcat(outfile_name, ".pbm");
    current_color = 0x1;
  } else if (enable_pgm == 1) {
    strcat(outfile_name, ".pgm");
    current_color = MAX_COLOR_LEVELS-1;
  }
  outfile = fopen(outfile_name, "w");

  srand(iseed);

#define NTESTS            16
#define XM_OFFSET_STEP    64
#define A_STEP             4
#define B_STEP             8
#define YM_INIT           16

  for (i = 0; i < NTESTS; i++) {
    xm = xm_offset + XM_OFFSET_STEP;
    ym = YM_INIT;
S_001_002:    
    a = 2;
    b = 2;
S_001_003:    
    t0 = x_dim - xm;
    t1 = y_dim - ym;
    t2 = xm;
    t3 = ym;
    if ((a >= t0) || (a >= t1) || (a >= t2) || (a >= t3) ||
        (b >= t0) || (b >= t1) || (b >= t2) || (b >= t3)) {
      goto S_004_001;
    }
    ellipsedraw(xm, ym, a, b); 
    a = a + A_STEP;
    b = b + B_STEP;
    while (a == b && b < t0 && b < t1 && b < t2 && b < t3) {
      b = rand()%(y_dim/4);
    }    
    if ((i % NTESTS/4) == 0) {
      SWAP(a, b);
    }
    goto S_001_003;
S_004_001:
    xm = xm - XM_OFFSET_STEP/2;
    ym = ym + XM_OFFSET_STEP/2;
    if (xm != 0) {
      goto S_001_002;
    }
    if (xm_offset >= 3*x_dim/4) {
      break;
    }
    xm_offset = xm_offset + XM_OFFSET_STEP; 
  }

  /* Write output file. */
  if (enable_pbm == 1) {
    write_pbm_file(outfile, image_data, outfile_name, 
    x_dim, y_dim, 1, 1, 32, 1);
  } else if (enable_pgm == 1) {
    write_pgm_file(outfile, image_data, outfile_name, 
    x_dim, y_dim, 1, 1, MAX_COLOR_LEVELS-1, 16, 1);
  }

  /* Deallocate space. */
  free(image_data);
  free(outfile_name);

  return 0;
}
