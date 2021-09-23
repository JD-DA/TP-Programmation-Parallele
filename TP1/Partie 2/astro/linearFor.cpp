#include <iostream>
#include <omp.h>
#include <stdlib.h>
#define cimg_display 0
#define cimg_plugin "fits.h"
#include "CImg.h"
using namespace cimg_library;
using namespace std;

int main(int, char *argv[]) {
  CImg<unsigned short> astro(argv[1]);
  CImg<unsigned char> resca(astro.width(), astro.height(), 1, 1, 0);
  unsigned short amin = 65535, amax = 0;
  int i, j;
  int iterations = atoi(argv[2]);

  double start = omp_get_wtime();

  for (int k = 0; k < iterations; k++) {
      ///////////////////// MODIFIER A PARTIR D'ICI UNIQUEMENT /////////////////
#pragma omp parallel
      {
          unsigned short locAmin=0;
          unsigned short locAmax=0;
#pragma omp for collapse(2)
          for (j = 0; j < astro.height(); j++)
              for (int i = 0; i < astro.width(); i++) {
                  locAmin = min(locAmin, astro(i, j));
                  locAmax = max(locAmax, astro(i, j));
              }

#pragma omp critical
          amin = min(locAmin, amin);
          amax = max(locAmax, amax);
      }

      unsigned short arange = amax - amin;
#pragma omp parallel
      {
#pragma omp for collapse(2)
          for (j = 0; j < astro.height(); j++) {
              for (int i = 0; i < astro.width(); i++)
                  resca(i, j) = (astro(i, j) - amin) * 255 / arange;
          }

      }
      ///////////////////// MODIFIER JUSQU'ICI UNIQUEMENT /////////////////////
  }
  double end = omp_get_wtime();
  cout << (end - start) / iterations << endl;
  resca.save_pnm("/tmp/resca.pgm");
  return 0;
}
