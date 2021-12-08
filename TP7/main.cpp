#include <iostream>


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