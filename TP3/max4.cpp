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

        int smallChunk = taille % nprocs;
        int start = 0;
        int startRoot;      //start du tab que root va devoir analyser
        int endRoot;
        cout << "Tab : [";
        for (int n = 0; n < taille; n++) {
            cout << tab[n] << ",";
        }
        cout << "]" << endl;
        for (int i = 0; i < nprocs; i++) {

            int sizeToSend = chunk;
            if(smallChunk>0){       //calcul de la taille de ce chunk, on ajoute une case à chaque chunk si taill%nprocs!=0
                sizeToSend++;       //ainsi pour n tel que taille%nprocs==n les n premiers pid auront une case en plus a analyser
                smallChunk--;
            }

            if(i==root) {           //dans le cas où on arrive au process root on incremente la taille et on continue
                startRoot = start;
                start+=sizeToSend;
                endRoot=start;
                continue;
            }
            MPI_Ssend(&sizeToSend, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //on envoie la taille du chunk

            int tabToSend[sizeToSend];
            int index = 0;
            for (int j = start; j < start + sizeToSend; j++) {      //on part de start et on va jusqu'à start+sizetosend
                tabToSend[index++] = tab[j];
            }

            start+=sizeToSend;
            MPI_Request req;
            MPI_Issend(tabToSend, sizeToSend, MPI_INT, i, 0, MPI_COMM_WORLD,&req);
        }
        cout << "from " << pid << " r=[";
        int maximum = 0;
        for (int j = startRoot; j < endRoot; j++) {
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
        int sizeChunk;
        MPI_Recv(&sizeChunk, chunk, MPI_INT, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int tab[sizeChunk];
        MPI_Recv(tab, sizeChunk, MPI_INT, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "from " << pid << " r=[";
        for (int i = 0; i < sizeChunk; i++) {
            cout << tab[i] << ",";
        }
        cout << "]" << endl;
        int maximum = 0;
        for (int j = 0; j < sizeChunk; j++) {
            maximum = max(maximum,tab[j]);
        }
        cout << "Max found  by "<<pid<<" : " <<maximum<< endl;
        MPI_Ssend(&maximum, 1, MPI_INT, root, 1, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}

