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
          int id = omp_get_thread_num();
          int num = omp_get_num_threads();
          unsigned short locAmin=0;
          unsigned short locAmax=0;
#pragma omp critical
          std::cout<<"le thread n°"<<id<<" est en cours d'execution"<<std::endl;
          for (j = id; j < astro.height(); j+=num)
              for (i = 0; i < astro.width(); i++) {
                  locAmin = min(locAmin, astro(i, j));
                  locAmax = max(locAmax, astro(i, j));
              }

#pragma omp critical
          amin = min(locAmin, amin);
          amax = max(locAmax, amax);
      }

      unsigned short arange = amax - amin;
      long len=0;
#pragma omp parallel private i,j
      {
          int id = omp_get_thread_num();
          int num = omp_get_num_threads();

#pragma omp critical
          std::cout << "le thread n°" << id << " est en cours d'execution" << std::endl;
          int lenLocal = 0;
          for (j = id; j < astro.height(); j += num) {
              lenLocal += 1;
              for (i = 0; i < astro.width(); i++)
                  resca(i, j) = (astro(i, j) - amin) * 255 / arange;
          }
#pragma omp critical
          {
              len += lenLocal;
              std::cout << "le thread n°" << id << " a parcouru "<<lenLocal<<" lignes" << std::endl;
          }

      }
      std::cout<<"le nombre de ligne parcouru est"<<len<<" sur "<<astro.height()<<std::endl;
      ///////////////////// MODIFIER JUSQU'ICI UNIQUEMENT /////////////////////
  }
  double end = omp_get_wtime();
  cout << (end - start) / iterations << endl;
  resca.save_pnm("/tmp/resca.pgm");
  return 0;
}