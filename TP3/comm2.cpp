#include <iostream>
#include <mpi.h>

using namespace std;
int main(int argc, char *argv[]){
    int pid,nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    int taille = stoi(argv[1]);
    int tab[taille];
    int tab2[taille];
    for(int i=0; i<taille; i++){
    	tab[i]=pid;
    }
    
    int tag = 9;
    cout<<"from "<<pid<<" s=[";
    for(int i=0; i<taille; i++){
    	cout<<tab[i]<<",";
    }
    cout<<"]"<<endl;
    MPI_Sendrecv_replace(tab, taille, MPI_INT, (pid + 1) % nprocs, tag, MPI_COMM_WORLD,&status);
    cout<<"from "<<pid<<" r=[";
    for(int i=0; i<taille; i++){
    	cout<<tab[i]<<",";
    }
    cout<<"]"<<endl;
    MPI_Finalize();
    return 0;
}
