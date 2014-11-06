/**********************************************************************/
/*  File       : circledraw.c
 *  Description: Testbed for circle drawing algorithms:
 *               - Pseudo-Bresenham quadrant-based algorithm by Alois 
 *                 Zingl [4Q]
 *               - Relicarium website algorithm [4Q]
 *
 *  Author     : Nikolaos Kavvadias <nikos@nkavvadias.com>
 *  Copyright  : Nikolaos Kavvadias (C) 2011, 2012, 2013, 2014
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

/* Swap two ints using a temporary. */
#define SWAP(x, y)        (x ^= y ^= x ^= y)
/* Absolute value. */
#define ABS(x)            ((x) > 0 ? (x) : (-x))
/* Maximum of two values. */
#define MAX(x,y)          ((x) > (y) ? (x) : (y))
/* Macros for even and odd. */
#define EVEN(x)           !(x&1)
#define ODD(x)            (x&1)


FILE *outfile;
char outfile_name[96];
int *image_data;
unsigned int current_color=0x0;
//
int enable_pbm=1,enable_pgm=0;
int enable_bresenham=1,enable_relicarium=0;
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

/* bresenham:
 * Pseudo-Bresenham's algorithm for circle drawing.
 * Reference: http://free.pages.at/easyfilter/bresenham.html
 *
 * This quadrant-based circle drawing algorithm determines which points should 
 * be plotted in order to form a close approximation of a circle with center 
 * coordinates (xm,ym) and radius r. It can be used for drawing circles on raster 
 * displays as well as fundamental functionality in numerous software graphics 
 * libraries.
 * The algorithm uses only integer operations such as addition, subtraction, bit 
 * shifting, comparison, absolute value and conditional moves. It consists of a 
 * single loop that keeps track of the propagated error over the x- and y-axis, 
 * which is then used for calculating four pixels (one per quadrant), during each 
 * iteration. The loop terminates when all the possible steps over the x- or y- 
 * axes are exhausted. 
 */
void bresenham(int xm, int ym, int r)
{
  int x = -r, y = 0, err = 2-2*r, r0 = r; /* bottom left to top right */ 
  int addr;
  int temp=0, t;

  do {
    // SETPIXEL(xm - x, ym + y);           /*   I. Quadrant +x +y */
    image_data[(xm-x) + x_dim*(ym+y)] = current_color;
    // SETPIXEL(xm - y, ym - x);           /*  II. Quadrant -x +y */
    image_data[(xm-y) + x_dim*(ym-x)] = current_color;
    // SETPIXEL(xm + x, ym - y);           /* III. Quadrant -x -y */
    image_data[(xm+x) + x_dim*(ym-y)] = current_color; 
    // SETPIXEL(xm + y, ym + x);           /*  IV. Quadrant +x -y */
    image_data[(xm+y) + x_dim*(ym+x)] = current_color;
    // error accumulation      
    r0 = err;
    if (r0 <= y) { 
      err += ++y*2+1;           /* e_xy+e_y < 0 */
    }      
    if (r0 > x || err > y) {    /* e_xy+e_x > 0 or no 2nd y-step */
      err += ++x*2+1;           /* -> x-step now */
    }
  } while (x < 0);
}

/* relicarium:
 * C implementation for a circle generation algorithm. 
 * Reference: http://www.relicarium.org [site down]
 */
void relicarium(int xm, int ym, int r)
{
  int x = r;
  int y = 0;
  int e = 0;
  for (;;) {
    // SETPIXEL(xm + x, ym + y);
    image_data[(xm+x) + x_dim*(ym+y)] = current_color;
    // SETPIXEL(xm + x, ym - y);
    image_data[(xm+x) + x_dim*(ym-y)] = current_color;
    // SETPIXEL(xm - x, ym - y);
    image_data[(xm-x) + x_dim*(ym-y)] = current_color;
    // SETPIXEL(xm - x, ym + y);
    image_data[(xm-x) + x_dim*(ym+y)] = current_color;
    // SETPIXEL(xm + y, ym + x);
    image_data[(xm+y) + x_dim*(ym+x)] = current_color;
    // SETPIXEL(xm + y, ym - x);
    image_data[(xm+y) + x_dim*(ym-x)] = current_color;
    // SETPIXEL(xm - y, ym - x);
    image_data[(xm-y) + x_dim*(ym-x)] = current_color;
    // SETPIXEL(xm - y, ym + x);     
    image_data[(xm-y) + x_dim*(ym+x)] = current_color;
    if (x <= y) {
      break;
    }
    e += 2*y + 1;
    y++;
    if (e > x) {
      e += 1 - 2*x;
      x--;
    } 
  }
}

/* circledraw:
 * Selecting circle drawing algorithm.
 */
void circledraw(int xm, int ym, int r)
{
	if (enable_bresenham == 1) {
    bresenham(xm, ym, r);
  } else if (enable_relicarium == 1) {
		relicarium(xm, ym, r);
	}
}

/* print_usage:
 * Print usage instructions for the "circledraw" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* ./circledraw [options] <outfile>\n");
  printf("* \n");
  printf("* Options:\n");
  printf("*   -h:              Print this help.\n");
  printf("*   -b:              Use the pseudo-Bresenham circle drawing algorithm.\n");
  printf("*   -r:              Use the circle drawing algorithm from relicarium.org [site down].\n");
  printf("*   -pbm:            Generate a PBM image (default).\n");
  printf("*   -pgm:            Generate a PGM image.\n");
  printf("*   -x <num>:        Value for the x-dimension of the image (default:256).\n");
  printf("*   -y <num>:        Value for the y-dimension of the image (default:256).\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n\n");
}

/* main:
 * The main "circledraw" routine.
 */
int main(int argc, char **argv)
{
	int i,j,k;
  int res;
  int xm=0, xm_offset=0, ym=0, r=0, t0, t1, t2, t3;  

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
      enable_bresenham  = 1;
      enable_relicarium = 0;
    }
    else if (strcmp("-r", argv[i]) == 0) {
      enable_bresenham  = 0;
      enable_relicarium = 1;
    }
    else if (strcmp("-pbm", argv[i]) == 0) {
      enable_pbm        = 1;
      enable_pgm        = 0;
    }
    else if (strcmp("-pgm", argv[i]) == 0) {
      enable_pbm        = 0;
      enable_pgm        = 1;
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

  strcpy(outfile_name,"");
  if (enable_bresenham == 1) {
    strcpy(outfile_name, "circle-bresenham");
  } else if (enable_relicarium == 1) {
		strcpy(outfile_name, "circle-relicarium");
  }

  if (enable_pbm == 1) {
    strcat(outfile_name, ".pbm");
    current_color = 0x1;
  } else if (enable_pgm == 1) {
    strcat(outfile_name, ".pgm");
    current_color = MAX_COLOR_LEVELS-1;
  }
  outfile = fopen(outfile_name, "w");

#define NTESTS            16
#define XM_OFFSET_STEP    64
#define R_STEP             8
#define YM_INIT           16
  for (i = 0; i < NTESTS; i++) {
    xm = xm_offset + XM_OFFSET_STEP;
    ym = YM_INIT;
S_001_002:    
    r = 2;
S_001_003:    
    t0 = x_dim - xm;
    t1 = y_dim - ym;
    t2 = xm;
    t3 = ym;
    if ((r >= t0) || (r >= t1) || (r >= t2) || (r >= t3)) {
      goto S_004_001;
    }
    circledraw(xm, ym, r); 
    r = r + R_STEP;
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
	fclose(outfile);

  return 0;
}
