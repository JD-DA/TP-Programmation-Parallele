//
// Created by jean-daniel on 16/09/2021.
//
#include <iostream>
#include <omp.h>
using namespace std;

#define TOTAL 2000000

int main(int, char *[])
{

    int pass=0, cur;
    double start = omp_get_wtime();
///////////////////// MODIFIER A PARTIR D'ICI UNIQUEMENT /////////////////////
#pragma omp parallel
    {
        int passres=0; //pass local que l'on ajoute à la fin des iterations pour eviter un trop grand nombre d'accès critical
        int curlocal; //curr local pour éviter colisions
        int id = omp_get_thread_num();
        int num = omp_get_num_threads();
        for (int i = id; i < TOTAL; i+=num) {
            curlocal = i;
            while (curlocal > 1)
                curlocal = curlocal % 2 ? 3 * curlocal+ 1 : curlocal / 2;

            passres++;
        }
        #pragma omp critical
        pass+=passres;

    }
///////////////////// MODIFIER JUSQU'ICI UNIQUEMENT /////////////////////
    double end = omp_get_wtime();
    cout << pass << " out of " << TOTAL << "! (delta=" << TOTAL-pass << ")" << endl;
    cout << "ellapsed time: " << (end-start)*1000 << "ms" << endl;
}
