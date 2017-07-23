#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef XDIM
#define XDIM 160
#endif
#ifndef YDIM
#define YDIM 120
#endif

#ifdef PRINT  
char imgout_file_name[20];
FILE *imgout_file;


/* decode:
 * Decode the RGB encoding of the specified color.
 * NOTE: This scheme can only allow for up to 256 distinct colors
 * (essentially: R3G3B2).
 */
void decode(int c, int *red, int *green, int *blue)
{
  int t = c;
  *red = ((t >> 5) & 0x7) << 5;
  *green = ((t >> 2) & 0x7) << 5;
  *blue = ((t ) & 0x3) << 6;
}
#endif

/* rugca:
 * Generic implementation of the rug rule automaton.
 */
void rugca(int s, int inc, int g)
{
  static int img_temp[XDIM*YDIM], img_work[XDIM*YDIM];  
  int i, k, x, y;
  int taddr, u, uaddr;
  int cs;
  int sum=0;
  int x_offset[8] = {-1, 0, 1, 1, 1, 0,-1,-1};
  int y_offset[8] = {-1,-1,-1, 0, 1, 1, 1, 0};
#ifdef PRINT
  int red, green, blue;
#endif

  i = 0;
  while (i < g) {

    printf("### GENERATION %09d ###\n", i);

#ifdef PRINT      
    // Print current generation.
    if ((i % s) == 0) {
      sprintf(imgout_file_name, "rugca-%09d.ppm", i);
      imgout_file = fopen(imgout_file_name, "w");

      /* ASCII PPM image header */
      fprintf(imgout_file, "P3\n");
      fprintf(imgout_file, "# %s\n", imgout_file_name);
      fprintf(imgout_file, "%d %d\n255\n", XDIM, YDIM);     
      
      for (y = 0; y < YDIM; y++) {
        for (x = 0; x < XDIM; x++) {
          /* Address current pixel for visualization */
          taddr = y*XDIM+x;
          /* Decode pixel color */
          decode(img_temp[taddr], &red, &green, &blue);

          /* Write the pixel */
          fprintf(imgout_file, "%d %d %d ", red, green, blue);
          if ((x & 0x3) == 0) {
            fprintf(imgout_file, "\n");
          }          
        }
      }
      fclose(imgout_file); 
    }
#endif          

    // Calculate next grid state.
    for (y = 1; y < YDIM-1; y++) {
      for (x = 1; x < XDIM-1; x++) {      
        sum = 0;
        taddr = y*XDIM + x;
        for (k = 0; k < 8; k++) {
          uaddr = taddr + y_offset[k]*XDIM + x_offset[k];
          u = img_temp[uaddr];
          sum += u;
        }
        // Averaging sum.
        sum = sum >> 3;
        // Increment cs, modulo 256.
        cs = (sum + inc) & 0xFF;
        img_work[taddr] = cs;
      }
    }

    // Copy back current generation.
    for (x = 0; x < XDIM*YDIM; x++) {
      img_temp[x] = img_work[x];
    }

    // Advance generation.
    i++;
  }
}

/* print_usage:
 * Print usage instructions for the "rugca" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* ./rugca [options]\n");
  printf("* \n");
  printf("* Options:\n");
  printf("* -h           : Print this help.\n");
  printf("* -step <num>  : Generate a PPM image every <num> generations (Default: 1).\n");
  printf("* -gens <num>  : Total number of CA generations (Default: 1).\n");
  printf("* -incr <num>  : Cell increment (Default: 1).\n");
  printf("* \n");
}

/* main:
 * The main routine.
 */
int main(int argc, char **argv)
{
  int step=1, incr=1, gens=1;
  int i;

  // Read input arguments
  for (i = 1; i < argc; i++) {
    if (strcmp("-h", argv[i]) == 0) {
      print_usage();
      exit(1);
    } else if (strcmp("-step", argv[i]) == 0) {
      if ((i+1) < argc) {
        i++;
        step = atoi(argv[i]);
      }
    } else if (strcmp("-gens", argv[i]) == 0) {
      if ((i+1) < argc) {
        i++;
        gens = atoi(argv[i]);
      }
    } else if (strcmp("-incr", argv[i]) == 0) {
      if ((i+1) < argc) {
        i++;
        incr = atoi(argv[i]);
      } 
    }
  }

  /* Perform operations. */
  rugca(step, incr, gens);

  return 0;
}
