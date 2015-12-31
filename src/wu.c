/**********************************************************************/
/*  File       : wu.c
 *  Description: Testbed for line drawing algorithms with 
 *               antialiasing:
 *               - Xiaolin Wu's algorithm
 *  Author     : Nikolaos Kavvadias <nikos@nkavvadias.com>
 *  Copyright  : Nikolaos Kavvadias (C) 2014, 2015, 2016
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
#include <math.h>

#define XSIZE_DEFAULT    256
#define YSIZE_DEFAULT    256
#define MAX_COLOR_LEVELS 256

#define PFM_RGB          16 /* F */
#define PFM_GREYSCALE    17 /* f */

/* Swap two ints using a temporary. */
#define SWAP(x, y)        (x ^= y ^= x ^= y)
/* Absolute value. */
#define ABS(x)            ((x) > 0 ? (x) : (-x))
/* Maximum of two values. */
#define MAX(x,y)          ((x) > (y) ? (x) : (y))
/* Macros for even and odd. */
#define EVEN(x)           !(x&1)
#define ODD(x)            (x&1)
/* Plot pixel at (x,y) with color c. */
#define PLOT(x,y,c)       image_data[(y)*x_dim+(x)] = c
/* Definition of TRUE and FALSE. */
#define FALSE             0
#define TRUE              1

FILE *outfile;
float *image_data, *rgb_image_data;
int   *temp_image_data;
char outfile_name[96];
/**/
int enable_pgm=1,enable_pfm=0;
int enable_wu=1;
int x_dim=XSIZE_DEFAULT, y_dim=YSIZE_DEFAULT;


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

/* WriteFloat:
 * Write a possibly byte-swapped floating-point number.
 * NOTE: Assume IEEE format.
 */
int WriteFloat(FILE *fptr, float *f, int swap)
{
  unsigned char *cptr, tmp;

  if (swap) {
    cptr    = (unsigned char*)f;
    tmp     = cptr[0];
    cptr[0] = cptr[3];
    cptr[3] = tmp;
    tmp     = cptr[1];
    cptr[1] = cptr[2];
    cptr[2] = tmp;
  }
  if (fwrite(f, sizeof(float), 1, fptr) != 1) {
    return (FALSE);
  }  
  return (TRUE); 
}

/* write_pfm_file:
 * Write the contents of a PFM (portable float map) file.
 */
void write_pfm_file(FILE *f, float *img_out, char *img_out_fname, 
  int x_size, int y_size, 
  int img_type, int endianess)
{
  int i, j, x_scaled_size, y_scaled_size;
  int swap = (endianess == 1) ? 0 : 1;
  float fendian = (endianess == 1) ? +1.0 : -1.0;
  
  x_scaled_size = x_size;
  y_scaled_size = y_size;
  /* Write the magic number string. */
  if (img_type == PFM_RGB) {
    fprintf(f, "PF\n");
  } else if (img_type == PFM_GREYSCALE) {
    fprintf(f, "Pf\n");
  } else {
    fprintf(stderr, "Error: Image type invalid for PFM format!\n");
    exit(1);    
  }
  /* Write the image dimensions. */
  fprintf(f, "%d %d\n", x_scaled_size, y_scaled_size);
  /* Write the endianess/scale factor as float. */
  fprintf(f, "%f\n", fendian);
  
  /* Write the image data. */
  for (i = 0; i < y_scaled_size; i++) {
    for (j = 0; j < x_scaled_size; j++) {
      if (img_type == PFM_RGB) {
        WriteFloat(f, &img_out[3*(i*x_scaled_size+j)+0], swap);
        WriteFloat(f, &img_out[3*(i*x_scaled_size+j)+1], swap);
        WriteFloat(f, &img_out[3*(i*x_scaled_size+j)+2], swap);
      } else if (img_type == PFM_GREYSCALE) {
        WriteFloat(f, &img_out[i*x_scaled_size+j], swap);
      }
    }
  }  
  fclose(f);
}

/* ipart:
 * Return integer part of x.
 */    
int ipart(float x) 
{
  float f;
  if (x > 0.0) {
    f = floor(x);
  } else {
    f = ceil(x);
  }
  return (int)f;
}

/* fround:
 * Round to closest integer.
 */
int fround(float x)
{
  return ipart(x + 0.5);
}

/* fpart:
 * Return the fractional part of x.
 */
float fpart(float x)
{
  float f;
  f = x - ipart(x);
  return f;
}

/* rfpart:
 * Return 1.0-fpart(x).
 */
float rfpart(float x)
{
  float f;
  f = 1.0 - fpart(x);
  return f;
}

/* wu:
 * Xiaolin Wu's line drawing algorithm for antialiased lines.
 * Reference: http://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
 * Reference: Wu, Xiaolin (July 1991). "An efficient antialiasing technique". 
 * Computer Graphics 25 (4): 143–152. doi:10.1145/127719.122734.
 * Reference: Wu, Xiaolin (1991). "Fast Anti-Aliased Circle Generation". In 
 * James Arvo (Ed.). Graphics Gems II. San Francisco: Morgan Kaufmann. pp. 
 * 446–450. ISBN 0-12-064480-0.
 */
void wu(int x0, int y0, int x1, int y1) 
{
  int steep = (ABS(y1-y0) > ABS(x1-x0));
  int x, dx, dy, xend, xpxl1, ypxl1, xpxl2, ypxl2;
  int xx0, xx1, yy0, yy1;
  float f1, f2;
  float gradient;
  float xgap;
  float yend, intery;

  xx0 = x0;
  xx1 = x1;
  yy0 = y0;
  yy1 = y1;
  if (steep != 0) {
    SWAP(xx0, yy0);
    SWAP(xx1, yy1);
  }
  if (xx0 > xx1) {
    SWAP(xx0, xx1);
    SWAP(yy0, yy1);
  }
  dx = xx1 - xx0;
  dy = yy1 - yy0;
  gradient = (float)dy / dx;

  // handle first endpoint
  xend  = fround(xx0);
  yend  = yy0 + gradient * (xend - xx0);
  xgap  = rfpart(xx0 + 0.5);
  xpxl1 = xend;   // this will be used in the main loop
  ypxl1 = ipart(yend);
  f1    = rfpart(yend) * xgap;
  f2    = fpart(yend) * xgap;   
  if (steep != 0) {
    PLOT(ypxl1,   xpxl1, f1);
    PLOT(ypxl1+1, xpxl1, f2);
  } else {
    PLOT(xpxl1, ypxl1,   f1);
    PLOT(xpxl1, ypxl1+1, f2);
  }
  intery = yend + gradient; // first y-intersection for the main loop
 
  // handle second endpoint
  xend  = fround(xx1);
  yend  = yy1 + gradient * (xend - x1);
  xgap  = fpart(xx1 + 0.5);
  xpxl2 = xend; // this will be used in the main loop
  ypxl2 = ipart(yend);
  f1    = rfpart(yend) * xgap;
  f2    = fpart(yend) * xgap;
  if (steep != 0) {
    PLOT(ypxl2,   xpxl2, f1);
    PLOT(ypxl2+1, xpxl2, f2);
  } else {
    PLOT(xpxl2, ypxl2,   f1);
    PLOT(xpxl2, ypxl2+1, f2);
  }
 
  // main loop
  for (x = xpxl1+1; x <= xpxl2-1; x++) {
    f1    = rfpart(intery);
    f2    = fpart(intery);   
    if (steep != 0) {
      PLOT(ipart(intery),   x, f1);
      PLOT(ipart(intery)+1, x, f2);
    } else {
      PLOT(x, ipart (intery),   f1);
      PLOT(x, ipart (intery)+1, f2);
    }
    intery = intery + gradient;
  }
}

/* linedraw_antialias:
 * Selecting anti-aliasing line drawing algorithm.
 */
void linedraw_antialias(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  if (enable_wu == 1) {
    wu(x1, y1, x2, y2);
  } else {
    fprintf(stderr, "Error: Line drawing algorithm not yet implemented!\n");
    exit(1);
  }
}

/* print_usage:
 * Print usage instructions for the "wu" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* ./wu [options] <outfile>\n");
  printf("* \n");
  printf("* Options:\n");
  printf("*   -h:              Print this help.\n");
  printf("*   -w:              Use Xiaolin Wu's anti-aliasing line drawing algorithm.\n");
  printf("*   -pgm:            Generate a PGM image (default).\n");
  printf("*   -pfm:            Generate a PFM (Portable Float Map) image.\n");
  printf("*   -x <num>:        Value for the x-dimension of the image (default:256).\n");
  printf("*   -y <num>:        Value for the y-dimension of the image (default:256).\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n\n");
}

/* main:
 * The main "wu" routine.
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
    else if (strcmp("-w", argv[i]) == 0) {
      enable_wu        = 1;
    }
    else if (strcmp("-pgm", argv[i]) == 0) {
      enable_pgm       = 1;
      enable_pfm       = 0;
    }
    else if (strcmp("-pfm", argv[i]) == 0) {
      enable_pgm       = 0;
      enable_pfm       = 1;
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
  image_data = malloc(x_dim * y_dim * sizeof(float));
  // Initialize image data.
  for (i = 0; i < x_dim*y_dim; i++) {
    image_data[i] = 0x00;
  }

  strcpy(outfile_name,"");
  if (enable_wu == 1) {
    strcpy(outfile_name, "wu");
  } 

  if (enable_pgm == 1) {
    strcat(outfile_name, ".pgm");
  } else if (enable_pfm == 1) {
    strcat(outfile_name, ".pfm");
  }

  if (enable_pgm == 1) {
    outfile = fopen(outfile_name, "w");
  } else if (enable_pfm == 1) {
    outfile = fopen(outfile_name, "wb");
  } else {
    fprintf(stderr, "Error: The specified output file has not been created.\n");
    exit(1);
  } 

  // 0 <= tan(.) <= 1
  for (k = 0; k <= (x_dim>>1); k += 8) {
    linedraw_antialias(16, 16, (x_dim>>1)+16, k+16);
    fprintf(stderr, "(%d,%d)--(%d,%d)\n", 16, 16, (x_dim>>1)+16, k+16);
  }
  // 1 < tan(.) < +INF.
  for (k = 0; k <= (x_dim>>1); k += 8) {
    linedraw_antialias(16, 16, k+16, (x_dim>>1)+16);
    fprintf(stderr, "(%d,%d)--(%d,%d)\n", 16, 16, k+16, (x_dim>>1)+16);
  }

  /* Copy to temporary image array in case PGM generation is enabled. */
  if (enable_pgm == 1) {
    temp_image_data = malloc(x_dim * y_dim * sizeof(int));
    for (i = 0; i < x_dim*y_dim; i++) {
      temp_image_data[i] = 255 - (int)(255.0 * image_data[i]);
    }
  } else if (enable_pfm == 1) {
    rgb_image_data = malloc(3 * x_dim * y_dim * sizeof(float));
    for (i = 0; i < x_dim*y_dim; i++) {
      rgb_image_data[3*i+0] = image_data[i];
      rgb_image_data[3*i+1] = image_data[i];
      rgb_image_data[3*i+2] = image_data[i];
    }    
  }

  /* Write output file. */
  if (enable_pgm == 1) {
    write_pgm_file(outfile, temp_image_data, outfile_name, 
    x_dim, y_dim, 1, 1, MAX_COLOR_LEVELS-1, 16, 1);
  } else if (enable_pfm == 1) {
#ifdef WRITE_GREYSCALE
    write_pfm_file(outfile, image_data, outfile_name,
      x_dim, y_dim, PFM_GREYSCALE, -1
#else
    write_pfm_file(outfile, rgb_image_data, outfile_name,
      x_dim, y_dim, PFM_RGB, -1
    );
#endif
  }

  /* Deallocate space. */
  free(image_data);
  if (enable_pgm == 1) {
    free(temp_image_data);
  }
  if (enable_pfm == 1) {
    free(rgb_image_data);
  }
  fclose(outfile);

  return 0;
}
