//
// Created by jean-daniel on 14/10/2021.
//

#include <mpi.h>
#include <random>
#include <iostream>

using namespace std;

const int tag = 34;


int main(int argc, char **argv) {
    int pid, nprocs;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int n = atoi(argv[1]); // taille du tableau total

    if (n%nprocs!=0) {
        MPI_Finalize();
        cout << "Impossible d'exécuter " << n << " n'est pas divisible par nprocs=" << nprocs << endl;
        return 0;
    }

    int root = atoi(argv[2]);

    int* tab;

    if (pid==root) {
        tab = new int [n];
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 1000);
        for (int i = 0; i < n; i++)
            tab[i] = dis(gen);
        cout << "pid=" << pid << " : ";
        for (int i = 0; i < n; i++)
            cout << tab[i] << " ";
        cout << endl;
    }


    int n_local = n/nprocs;

    int* tab_recu = new int[n_local];

    if (pid==root)
        for (int i=0; i<nprocs; i++)
            if (i!=root)
                MPI_Ssend(tab+i*n_local,n_local,MPI_INT,i,tag,MPI_COMM_WORLD);
            else
                for (int j=0; j<n_local; j++)
                    tab_recu[j] = tab[n_local*root+j];
    else
        MPI_Recv(tab_recu,n_local,MPI_INT,root,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

    cout << "tab_recu de " << pid << " : ";
    for (int i=0; i<n_local; i++)
        cout << tab_recu[i] << " ";
    cout << endl;


    MPI_Barrier(MPI_COMM_WORLD);
    int* tab_recu2 = new int[n_local];
    MPI_Scatter(tab,n_local,MPI_INT,tab_recu2,n_local,MPI_INT,root,MPI_COMM_WORLD);

    cout << "tab_recu de " << pid << " : ";
    for (int i=0; i<n_local; i++)
        cout << tab_recu2[i] << " ";
    cout << endl;




     int* sendcounts;
    int* displ;

    int n_local = n/nprocs;

    int reste = n%nprocs;

    if (pid==root) {
      sendcounts = new int[nprocs];
      displ = new int[nprocs];

      int ptr = 0;
      for (int i=0; i<reste; i++) {
	sendcounts[i] = n_local+1;
	displ[i] = ptr;
	ptr+=(n_local+1);
      }
      for (int i=reste; i<nprocs; i++) {
	sendcounts[i] = n_local;
	displ[i] = ptr;
	ptr+=n_local;
      }
    }

    if (pid<reste)
      n_local++;

    int* tab_local = new int[n_local];

    MPI_Scatterv(tab,sendcounts,displ,MPI_INT,tab_local,n_local,MPI_INT,root,MPI_COMM_WORLD);

    int max[2];
    int res[2];

    max[0] = tab_local[0];
    max[1] = 0;

    for (int i = 1; i < n_local; i++)
        if (max[0] < tab_local[i]) {
            max[0] = tab_local[i];
            max[1] = i;
        }


    if (pid<reste)
      max[1] += pid*n_local;
    else
      max[1] += (reste*(n_local+1) + (pid-reste)*n_local);

    MPI_Reduce(max, res, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

    if (pid == 0)
        cout << "max=" << res[0] << " loc=" << res[1] << endl;


    if (pid==root) {
      delete[] tab;
      delete[] sendcounts;
      delete[] displ;
    }

    MPI_Finalize();
    return 0;
}

