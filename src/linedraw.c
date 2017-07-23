/**********************************************************************/
/*  File       : linedraw.c
 *  Description: Testbed for line drawing algorithms:
 *               - Direct scan conversion [O]
 *               - Bresenham [Q]
 *               - Midpoint [O]
 *               - DDA (Direct Differential Analyzer) [Q]
 *  Author     : Nikolaos Kavvadias <nikos@nkavvadias.com>
 *  Copyright  : Nikolaos Kavvadias (C) 2009, 2010, 2011, 2012, 
 *                                      2013, 2014, 2015, 2016
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
/* Macros for even and odd. */
#define EVEN(x)           !(x&1)
#define ODD(x)            (x&1)


FILE *outfile;
char *outfile_name;
int *image_data;
unsigned int current_color=0x0;
//
int enable_pbm=1,enable_pgm=0;
int enable_dsc=1,enable_bresenham=0,enable_midpoint=0,enable_dda=0;
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

/* dsc:
 * Direct scan conversion algorithm for line drawing.
 * Reference: http://ocw.unican.es/ensenanzas-tecnicas/visualizacion-e-interaccion-grafica/material-de-clase-2/03-LineAlgorithms.pdf
 */
void dsc(int x1, int y1, int x2, int y2) 
{
  // 1st octant.
  int x, y;
  float ytrue, m;

  x = x1;
  m = (float)(y2-y1)/(float)(x2-x1);
  while (x <= x2) {
    ytrue = m*x + y1;
//    y = ceil(ytrue);
    y = (int)ytrue;
    // plot(x, y); 
    image_data[y*x_dim + x] = current_color;
    x = x + 1;
  }
}

/* bresenham:
 * Bresenham's algorithm for line drawing.
 * Reference: http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
 */
void bresenham(int x0, int y0, int x1, int y1) 
{
  // 1st quadrant.
  int Dx = x1 - x0; 
  int Dy = y1 - y0;
	int x;
  int steep = (ABS(Dy) >= ABS(Dx));
  int xstep, ystep;
  int TwoDy, TwoDyTwoDx;
  int E;
  int y;
  int xDraw, yDraw;  
	 
  if (steep) {
    SWAP(x0, y0);
    SWAP(x1, y1);
    // recompute Dx, Dy after swap
    Dx = x1 - x0;
    Dy = y1 - y0;
  }
  xstep = 1;
  if (Dx < 0) {
    xstep = -1;
    Dx = -Dx;
  }
  ystep = 1;
  if (Dy < 0) {
    ystep = -1;		
    Dy = -Dy; 
  }
  TwoDy = 2*Dy; 
  TwoDyTwoDx = TwoDy - 2*Dx; // 2*Dy - 2*Dx
  E = TwoDy - Dx; // 2*Dy - Dx
  y = y0;
  for (x = x0; x != x1; x += xstep) {		
    if (steep) {			
      xDraw = y;
      yDraw = x;
    } else {			
      xDraw = x;
      yDraw = y;
    }
    // plot(xDraw, yDraw);
    image_data[yDraw*x_dim + xDraw] = current_color;
    // next
    if (E > 0) {
      E += TwoDyTwoDx; // E += 2*Dy - 2*Dx;
      y = y + ystep;
    } else {
      E += TwoDy; // E += 2*Dy;
    }
  }
}

/* midpoint:
 * Midpoint algorithm for line drawing.
 */
void midpoint(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  // 1st octant.
  int inc_E, inc_NE;
  int dx, dy, d;
  unsigned int x, y;
	
  dx = x2 - x1; 
  dy = y2 - y1; 
  d = 2*dy - dx;	
  inc_E = 2 * dy; 
  inc_NE = 2 * (dy - dx);
  y = y1;
	
  for (x = x1; x <= x2; x++) {
    // plot(x,y)
    image_data[y*x_dim + x] = current_color;
    if (d > 0) {
      y = y + 1;
      d = d + inc_NE;
    } else {
      d = d + inc_E;
    }
  }
}	

/* dda:
 * Digital differential analyzer algorithm.
 * Reference: http://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
 */
void dda(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
#ifdef DDA_USE_FLOAT
  // 1st quadrant.
  // Source: http://www.cglabprograms.com/2008/10/dda-line-drawing-algorithm.html
  int s, dx, dy, m;
  float xi, yi, x, y;
 
  dx = x2 - x1;
  dy = y2 - y1;

  s = MAX(ABS(dx), ABS(dy));
  
  xi = dx / (float) s;
  yi = dy / (float) s;
 
  x = x1;
  y = y1;
  // plot(x,y)
  image_data[(int)y*x_dim + (int)x] = current_color;  
 
  for (m = 0; m < s; m++) {
    x = x + xi;
    y = y + yi;
    // plot(x,y)
    image_data[(int)y*x_dim + (int)x] = current_color;  
  }
#else // DDA_USE_INTEGER_1
  // Source: http://programmingvilla.com/programs/graphics/program-to-draw-a-line-using-integer-dda-method/
  int x, y, dy, dx, c=0;

  dx = x2-x1;
  dy = y2-y1;
  x  = x1;
  y = y1;
  if (ABS(dx) >= ABS(dy)) {
    while (x <= x2) {
      // plot(x,y)
      image_data[y*x_dim + x] = current_color;  
      c = c + dy;     // update the fractional part
      if (c >= dx) {  // that is, the fractional part is greater than 1 now
        c = c - dx;   // update the fractional part
        y = y + 1;    // carry the overflowed integer over
      }
      x = x + 1;
    }
  } else {
    while (y <= y2) {
      // plot(x,y)
      image_data[y*x_dim + x] = current_color;  
      c = c + dx;     // update the fractional part
      if (c >= dy) {  // that is, the fractional part is greater than 1 now
        c = c - dy;   // update the fractional part
        x = x + 1;    // carry the overflowed integer over
      }
      y = y + 1;
    }
  }
#endif
}

/* linedraw:
 * Selecting line drawing algorithm.
 */
void linedraw(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  if (enable_dsc == 1) {
    dsc(x1, y1, x2, y2);
  } else if (enable_bresenham == 1) {
    bresenham(x1, y1, x2, y2);
  } else if (enable_midpoint == 1) {
    midpoint(x1, y1, x2, y2);
  } else if (enable_dda == 1) {
    dda(x1, y1, x2, y2);
  }
}

/* print_usage:
 * Print usage instructions for the "linedraw" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* ./linedraw [options] <outfile>\n");
  printf("* \n");
  printf("* Options:\n");
  printf("*   -h:              Print this help.\n");
  printf("*   -s:              Use direct scan conversion line drawing algorithm (default).\n");
  printf("*   -b:              Use Bresenham's line drawing algorithm.\n");
  printf("*   -m:              Use midpoint line drawing algorithm.\n");
  printf("*   -d:              Use DDA (Digital Differential Analyzer) line drawing algorithm.\n");
  printf("*   -pbm:            Generate a PBM image (default).\n");
  printf("*   -pgm:            Generate a PGM image.\n");
  printf("*   -x <num>:        Value for the x-dimension of the image (default:256).\n");
  printf("*   -y <num>:        Value for the y-dimension of the image (default:256).\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n\n");
}

/* main:
 * The main "linedraw" routine.
 */
int main(int argc, char **argv)
{
  int i,k;

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
    else if (strcmp("-s", argv[i]) == 0) {
      enable_dsc       = 1;      
      enable_bresenham = 0;
      enable_midpoint  = 0;
      enable_dda       = 0;
    }
    else if (strcmp("-b", argv[i]) == 0) {
      enable_dsc       = 0;      
      enable_bresenham = 1;
      enable_midpoint  = 0;
      enable_dda       = 0;
    }
    else if (strcmp("-m", argv[i]) == 0) {
      enable_dsc       = 0;      
      enable_bresenham = 0;
      enable_midpoint  = 1;
      enable_dda       = 0;
    }
    else if (strcmp("-d", argv[i]) == 0) {
      enable_dsc       = 0;      
      enable_bresenham = 0;
      enable_midpoint  = 0;
      enable_dda       = 1;
    }
    else if (strcmp("-pbm", argv[i]) == 0) {
      enable_pbm       = 1;
      enable_pgm       = 0;
    }
    else if (strcmp("-pgm", argv[i]) == 0) {
      enable_pbm       = 0;
      enable_pgm       = 1;
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

  outfile_name = malloc((strlen("line-bresenham")+
                   strlen(".pxm")+1) * sizeof(char));

  if (enable_dsc == 1) {
    strcpy(outfile_name, "line-dsc");
  } else if (enable_bresenham == 1) {
    strcpy(outfile_name, "line-bresenham");
  } else if (enable_midpoint == 1) {
    strcpy(outfile_name, "line-midpoint");
  } else if (enable_dda == 1) {
    strcpy(outfile_name, "line-dda");
  } else {
    fprintf(stderr, "Error: Unspecified line generation algorithm\n");
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

  // 0 <= tan(.) <= 1
  for (k = 0; k <= (x_dim>>1); k += 8) {
    linedraw(16, 16, (x_dim>>1)+16, k+16);
#ifdef DIAG  
    fprintf(stderr, "(%d,%d)--(%d,%d)\n", 16, 16, (x_dim>>1)+16, k+16);
#endif    
  }
  // 1 < tan(.) < +INF.
  if ((enable_bresenham == 1) || (enable_dda == 1))
  for (k = 0; k <= (x_dim>>1); k += 8) {
    linedraw(16, 16, k+16, (x_dim>>1)+16);
#ifdef DIAG    
    fprintf(stderr, "(%d,%d)--(%d,%d)\n", 16, 16, k+16, (x_dim>>1)+16);
#endif    
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
