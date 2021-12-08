#include <iostream>
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace std;

void swap(int* a, int* b){
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void QuickSort(int* A, int q, int r) {
    if (q<r) {
        int x = A[q];
        int s = q;
        for (int i=q+1; i<r; i++) {
            if (A[i]<=x) {
                s++;
                swap(A+s,A+i);
            }
        }
        swap(A+q,A+s);
        QuickSort(A,q,s);
        QuickSort(A,s+1,r);
    }
}

/*
    * tab est le tableau de données
    * n sa longueur
    * pivot est le pivot que vous choisirez avec la méthode de votre choix
    * nb_threads est le nombre de threads souhaité
    * s est un tableau tel que s[i] est le nombre d’éléments plus petits ou égaux au pivot pour
        le ième morceau du tableau 0 ≤ i < nb threads
    * r est un tableau tel que r[i] est le nombre d’éléments plus grand que le pivot pour
        le ième morceau du tableau 0 ≤ i < nb threads
 */
void Partitionnement(int* tab, int n, int pivot, int nb_threads, int* s, int* r){
    //srand (time(NULL));
    //rand()%100;
    int chunk = n/nb_threads;
    int reste = n%nb_threads;
#pragma omp parallel num_threads(nb_threads) shared(s,r,tab,pivot,n)
    {
        int pid = omp_get_thread_num();
        int debut = chunk*pid + min(pid,reste);
        int fin = debut+chunk;
        if(pid<reste){
            fin++;
        }
        for(int i = debut; i<fin;i++){
            if(tab[i]<=pivot){
                swap(tab[i],tab[s[pid]]);
                s[pid]++;
            }

        }
        r[pid]=chunk-s[pid];
    }

}
/*
* s est le tableau calculé précedemment avec le nombre des éléments plus petits ou égaux au pivot
* r idem pour le nombre des éléments pls grands que le pivot
*/
//num threads : puissance de 2
void SommePrefixe(int* s, int* r, int* somme_left, int* somme_right, int nb_threads){
#pragma omp parallel num_threads(nb_threads)
    {
        int pid = omp_get_thread_num();
//init 0
        if(pid==0){
            somme_left[0]=0;
            somme_right[0]=0;
        }else{
            somme_right[pid]=r[pid-1];
            somme_left[pid]=s[pid-1];
        }
#pragma omp barrier
        for (double i = 1; i < log2(nb_threads);i*=2) {
            if(pid>=i){
                somme_left[pid]+=somme_left[pid-i];
                somme_right[pid]+=somme_right[pid-i];
            }
#pragma omp barrier
        }
    }
}

/*
    * res est le tableau résultat du réarrangement.
    * les autres paramètres sont similaires aux fonctions précédentes.
 */
void Rearrangement(int* somme_left, int* somme_right, int* tab, int* res, int n, int nb_threads){
    int chunk = n/nb_threads;
    int reste = n%nb_threads;
#pragma omp parallel num_threads(nb_threads)
    {
        int pid = omp_get_thread_num();
        int debut = chunk*pid + min(pid,reste);
        int fin = debut+chunk;
        int pivot = somme_left[pid+1];
        int fin_left = somme_left[nb_threads];
        if(pid<reste){
            fin++;
        }
        for(int i = debut; i<fin;i++){
            if(i<=pivot){
                res[somme_left[pid]++]=tab[i];
            }else{
                res[fin_left+somme_right[pid]++]=tab[i]
            }
        }
}
}



int main(int argc, char** argv) {

    if (argc<2) {
        cout << "Mauvaise utilisation du programme :" << endl;
        cout << "./Tri [taille du tableau] " << endl;
        return 1;
    }

    int n = atoi(argv[1]);

    int* tab = new int[n];

    srand(time(NULL));
    for (int i=0; i<n; i++)
        tab[i] = rand()%100;

    QuickSort(tab,0,n);

    for (int i=0; i<n; i++)
        cout << tab[i] << " ";
    cout << endl;
    return 0;
}