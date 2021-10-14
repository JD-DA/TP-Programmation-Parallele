#include <iostream>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

using namespace std;
int main(int argc, char *argv[]) {
    int pid, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    int taille = stoi(argv[1]);
    int root = stoi(argv[2]);
    int chunk = taille / nprocs;
    int reste = taille % nprocs;

    int* sendcounts;
    int* displ;


    float tab[taille];
    if (pid == root) {

        srand(time(NULL));

        for (int i = 0; i < taille; i++) {
            tab[i] = rand() % 1000 + 1;
        }

//        int smallChunk = taille % nprocs;
        int start = 0;
        int startRoot;      //start du tab que root va devoir analyser
        int endRoot;
        cout << "Tab : [";
        for (int n = 0; n < taille; n++) {
            cout << tab[n] << ",";
        }
        cout << "]" << endl;

        sendcounts = new int[nprocs];
        displ = new int[nprocs];


        for (int i = 0; i < nprocs; i++) {

            int sizeToSend = chunk;
            /*if(smallChunk>0){       //calcul de la taille de ce chunk, on ajoute une case à chaque chunk si taill%nprocs!=0
                sizeToSend++;       //ainsi pour n tel que taille%nprocs==n les n premiers pid auront une case en plus a analyser
                smallChunk--;
            }*/
            if (i < reste)
                sizeToSend++;

            sendcounts[i] = sizeToSend;
            displ[i] = start;

            /*if(i==root) {           //dans le cas où on arrive au process root on incremente la taille et on continue
                startRoot = start;
                start+=sizeToSend;
                endRoot=start;
                continue;
            }*/
            //MPI_Ssend(&sizeToSend, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //on envoie la taille du chunk

            /*float tabToSend[sizeToSend];
            int index = 0;
            for (int j = start; j < start + sizeToSend; j++) {      //on part de start et on va jusqu'à start+sizetosend
                tabToSend[index++] = tab[j];
            }*/

            start += sizeToSend;
            //MPI_Ssend(tabToSend, sizeToSend, MPI_FLOAT, i, 9, MPI_COMM_WORLD);

        }
    }
    int n_local = chunk;
    if (pid<reste)
        n_local++;

    float* tab_local = new float[n_local];
    MPI_Scatterv(tab,sendcounts,displ,MPI_FLOAT,tab_local,n_local,MPI_FLOAT,root,MPI_COMM_WORLD);
    /*cout << "from " << pid << " r=[";
    for (int i = 0; i < n_local; i++) {
        cout << tab_local[i] << ",";
    }
    cout << "]" << endl;*/
    float norm = 0;
    for (int j = 0; j < n_local; j++) {
        norm += pow(tab_local[j],2);
    }
    //cout<<"from "<<pid<<" norm = "<<norm<<endl;
    float normToSend = norm;
    MPI_Allreduce(&normToSend,&norm,1,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);
    norm = pow(norm, 0.5);
    //cout<<"from "<<pid<<" norm after reduce = "<<norm<<endl;

    for (int j = 0; j < n_local; j++) {
        tab_local[j] /= norm;
    }
    /*cout <<"from "<<pid<< "Tablocal : [";
    for (int n = 0; n < n_local; n++) {
        cout << tab_local[n] << ",";
    }
    cout << "]" << endl;*/


    MPI_Gather(tab_local, n_local , MPI_FLOAT , tab , n_local ,
                MPI_FLOAT , root , MPI_COMM_WORLD) ;
if(pid==root) {
    cout << "Tab " << pid << " : [";
    for (int n = 0; n < taille; n++) {
        cout << tab[n] << ",";
    }
    cout << "]" << endl;
}
    /*
    if(pid==root){
        cout << "from " << pid << " r=[";
        float norm = 0;
        for (int j = startRoot; j < endRoot; j++) {
            norm += pow(tab[j],2);
            cout << tab[j] << ",";
        }
        cout << "]" << endl;

        cout << "Norm found  by "<<pid<<" : " <<norm<< endl;

        float res = 0;

        for (int i=0;i<nprocs;i++){
            if(i==root)
                continue;
            MPI_Recv(&res, 1, MPI_FLOAT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout<<"on a recu "<<res<<" from "<<i<<" norm = "<<norm<<" et norm + res = "<<norm+res<<endl;
            norm = norm + res;
        }
        cout << "Norm before root : " <<norm<< endl;

        norm = pow(norm, 0.5);
        cout << "Norm found : " <<norm<< endl;
        //On a trouvé la norme, maintenant on la partage ainsi que le vecteur pour faire la multiplication
        for (int i=0;i<nprocs;i++){
            if(i==root)
                continue;
            cout<<"on envoie "<<norm<<" a "<<i<<endl;
            float test = norm;
            MPI_Ssend(&test, 1, MPI_FLOAT, i, 5, MPI_COMM_WORLD);
        }

        int index = 0;
        int sizeToReceive;
        for (int i=0;i<nprocs;i++){
            if(i<reste){
                sizeToReceive = chunk +1;
            }else{
                sizeToReceive = chunk;
            }
            if(i==root) {
                for (int n = startRoot; n < endRoot; n++) {
                    tab[n] /= norm;
                }
                cout << "Tab "<<i<<" : [";
                for (int n = 0; n < taille; n++) {
                    cout << tab[n] << ",";
                }
                cout << "]" << endl;
            }else {

                MPI_Recv(tab + index, sizeToReceive, MPI_LONG, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                cout << "Tab "<<i<<" : [";
                for (int n = 0; n < taille; n++) {
                    cout << tab[n] << ",";
                }
                cout << "]" << endl;
            }
            index+=sizeToReceive;
        }

    } else {
        int sizeChunk;
        MPI_Recv(&sizeChunk, 1, MPI_INT, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        float tab[sizeChunk];
        MPI_Recv(tab, sizeChunk, MPI_FLOAT, root, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "from " << pid << " r=[";
        for (int i = 0; i < sizeChunk; i++) {
            cout << tab[i] << ",";
        }
        cout << "]" << endl;
        float norm = 0;
        for (int j = 0; j < sizeChunk; j++) {
            norm += pow(tab[j],2);
        }
        cout << "Norm found  from "<<pid<<" : " <<norm<< endl;
        MPI_Ssend(&norm, 1, MPI_FLOAT, root, 1, MPI_COMM_WORLD);
        //on reçoit la nouvelle norme
        float test;
        MPI_Recv(&norm, 1, MPI_FLOAT, root, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout<<pid<<" a recu a nouvelle norme : "<<norm<<endl;
        for (int j = 0; j < sizeChunk; j++) {
            tab[j] /= norm;
        }
        cout << "after norm received from " << pid << " r=[";
        for (int i = 0; i < sizeChunk; i++) {
            cout << tab[i] << ",";
        }
        cout << "]" << endl;
        MPI_Ssend(tab, sizeChunk, MPI_FLOAT, root, 3, MPI_COMM_WORLD);


    }*/
    MPI_Finalize();
    return 0;
}

