//
// Created by jean-daniel on 16/09/2021.
//
#include <iostream>
#include <omp.h>
using namespace std;

int main(int, char *[])
{

    #pragma omp parallel
    {
        int nb = omp_get_num_threads();
        #pragma omp critical
        cout << "Bonjour (" << omp_get_thread_num() << "/"<<nb<<")"<< endl;
        #pragma omp barrier
        #pragma omp single
        cout<< "Nous sommes : "<<nb<< endl;
        //cout << "Nous sommes " << omp_get_num_threads() << " threads dans cette équipe" << endl;
        //cout << "Info donnée par le thread numéro : " << omp_get_thread_num() << "/"<<nb<< endl;
        #pragma omp single
        cout<< "C'est bientot la fin du TP "<< endl;
        #pragma omp barrier
        #pragma omp critical
        cout << "Au revoir(" << omp_get_thread_num() << "/"<<nb<<")"<< endl;
    }
}
