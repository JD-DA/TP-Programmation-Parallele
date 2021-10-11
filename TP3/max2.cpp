#include <iostream>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

using namespace std;
int main(int argc, char *argv[]) {
    int pid, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    int taille = stoi(argv[1]);
    int root = stoi(argv[2]);
    int chunk = taille / nprocs;

    if (pid == root) {
        int tab[taille];
        srand(time(NULL));

        for (int i = 0; i < taille; i++) {
            tab[i] = rand() % 1000 + 1;
        }
        cout << "Tab : [";
        for (int i = 0; i < taille; i++) {
            cout << tab[i] << ",";
        }
        cout << "]" << endl;
        for (int i = 0; i < nprocs; i++) {
            if(i==root)
                continue;
            int tabToSend[chunk];
            for (int j = 0; j < chunk; j++) {
                tabToSend[j] = tab[i * chunk + j];
            }
            MPI_Ssend(tabToSend, chunk, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        cout << "from " << pid << " r=[";
        int maximum = 0;
        for (int j = chunk*root; j < chunk*(root+1); j++) {
            maximum = max(maximum,tab[j]);
            cout << tab[j] << ",";
        }
        cout << "]" << endl;
        cout << "Max found  by "<<pid<<" : " <<maximum<< endl;

        int res = 0;
        for (int i=0;i<nprocs;i++){
            if(i==root)
                continue;
            MPI_Recv(&res, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            maximum = max(maximum,res);
        }
        cout << "Max found : " <<maximum<< endl;
    } else {
        int tab[chunk];
        MPI_Recv(tab, chunk, MPI_INT, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "from " << pid << " r=[";
        for (int i = 0; i < chunk; i++) {
            cout << tab[i] << ",";
        }
        cout << "]" << endl;
        int maximum = 0;
        for (int j = 0; j < chunk; j++) {
            maximum = max(maximum,tab[j]);
        }
        cout << "Max found  by "<<pid<<" : " <<maximum<< endl;
        MPI_Ssend(&maximum, 1, MPI_INT, root, 1, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}


