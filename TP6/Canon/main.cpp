#include <iostream>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>

#include "fonctions.h"
#define DEBUG = 1;
using namespace std;

int main(int argc, char *argv[]) {
  int pid, nprocs;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm Comm_grille; // Nouveau communicateur pour la topologie cartésienne.

  if (argc != 3) {
    cerr << "usage: " << argv[0] << " size root" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  int root = atoi(argv[2]);

  // Création de la topologie
  int nbprocs_dims[2] = {0, 0};
  int period[2] = {1, 1};
  MPI_Dims_create(nprocs, 2, nbprocs_dims);

  if (nbprocs_dims[0] != nbprocs_dims[1]) {
    MPI_Finalize();
    cout << "impossible de créer une topologie carrée" << endl;
    return 0;
  }
  MPI_Cart_create(MPI_COMM_WORLD, 2, nbprocs_dims, period, true, &Comm_grille);

  // Les coordonnées du processus pid dans la grille logique créée
  int mycoords[2];
  MPI_Cart_coords(Comm_grille, pid, 2, mycoords);
  //rang vers coords

  int *A;
  int *B;
  int *C;

  // Le processus root génére les 2 matrices A et B et va également rassembler
  // le résultat
  if (pid == root) {
    A = generationMatriceCarre(n);
    B = generationMatriceCarre(n);
    C = new int[n * n];

#ifdef DEBUG
    cout << "*** la matrice A:" << endl;
    ecriture(n, A);
    cout << endl;

    cout << "*** la matrice B:" << endl;
    ecriture(n, B);
    cout << endl;
#endif
  }

  // Préparation des paramètres pour la gestion des blocs (un par processus)
  int l = n / nbprocs_dims[0]; //soit la racine carrée de DIM
  int *Alocal = new int[l * l];
  int *Blocal = new int[l * l];
  int *Clocal = new int[l * l];

  ///
  // Distribution des deux matrices par le processus root bloc Aij et Bij
  // sur le processus de coordonnées (i,j) dans la grille logique
  ///
  if (pid == root) {
    int Coords_dest[2];
    int dest;
    int *extract = new int[l * l];
    for (int i = 0; i < nbprocs_dims[0]; i++) {
      Coords_dest[0] = i;
      for (int j = 0; j < nbprocs_dims[1]; j++) {
        Coords_dest[1] = j;
        MPI_Cart_rank(Comm_grille, Coords_dest, &dest); //donne le rang du pid avec lequel communiquer
        if (dest != root) {
          extraction(A, extract, i, j, n, l);
          MPI_Ssend(extract, l * l, MPI_INT, dest, TAG1, MPI_COMM_WORLD);
          extraction(B, extract, i, j, n, l);
          MPI_Ssend(extract, l * l, MPI_INT, dest, TAG2, MPI_COMM_WORLD);
        } else {
          extraction(A, Alocal, i, j, n, l);
          extraction(B, Blocal, i, j, n, l);
        }
      }
    }
    delete[] extract;
  } else {
    MPI_Recv(Alocal, l * l, MPI_INT, root, TAG1, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Recv(Blocal, l * l, MPI_INT, root, TAG2, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  }
  //////////////////////////

  ///
  // Partie vue en cours : Algorithme de Canon
  ///
  int voisin_gauche, voisin_droite;

  MPI_Cart_shift(Comm_grille, 1, mycoords[0], &voisin_gauche, &voisin_droite);
  MPI_Sendrecv_replace(Alocal, l * l, MPI_INT, voisin_gauche, 10, voisin_droite,
                       10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  int voisin_haut, voisin_bas;
  MPI_Cart_shift(Comm_grille, 0, mycoords[1], &voisin_haut, &voisin_bas);
  MPI_Sendrecv_replace(Blocal, l * l, MPI_INT, voisin_haut, 10, voisin_bas, 10,
                       MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  produitMatrices(l, Alocal, Blocal, Clocal, false);

  MPI_Cart_shift(Comm_grille, 1, 1, &voisin_gauche, &voisin_droite);
  MPI_Cart_shift(Comm_grille, 0, 1, &voisin_haut, &voisin_bas);

  for (int i = 0; i < nbprocs_dims[0] - 1; i++) {
    MPI_Sendrecv_replace(Alocal, l * l, MPI_INT, voisin_gauche, 10,
                         voisin_droite, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv_replace(Blocal, l * l, MPI_INT, voisin_haut, 10, voisin_bas,
                         10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    produitMatrices(l, Alocal, Blocal, Clocal, true);
  }

  //////////////////////////

  ///
  // Rassemblement des deux matrices sur le processus root
  ///

  if (pid == root) {
    C = new int[n * n];
    int Coords_src[2];
    int src;
    int *bloc = new int[l * l];
    for (int i = 0; i < nbprocs_dims[0]; i++) {
      Coords_src[0] = i;
      for (int j = 0; j < nbprocs_dims[1]; j++) {
        Coords_src[1] = j;
        MPI_Cart_rank(Comm_grille, Coords_src, &src);
        if (src != root) {
          MPI_Recv(bloc, l * l, MPI_INT, src, TAG1, MPI_COMM_WORLD,
                   MPI_STATUS_IGNORE);
          recopie(C, bloc, i, j, n, l);
        } else {
          recopie(C, Clocal, i, j, n, l);
        }
      }
    }
    delete[] bloc;
  } else {
    MPI_Ssend(Clocal, l * l, MPI_INT, root, TAG1, MPI_COMM_WORLD);
  }
  //////////////////////////

  ///
  // Si la matrice n'est pas trop grande on calcule directement le produit sur
  // root pour vérifier le résultat
  ///
  if (pid == root) {
    int *Res = new int[n * n];
    produitMatrices(n, A, B, Res, false);
    bool erreur = false;
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++)
        if (C[i * n + j] != Res[i * n + j]) {
          erreur = true;
          break;
        }
    cout << "Erreur ? : " << erreur << endl;
    delete[] Res;
  }
  //////////////////////////

  delete[] Alocal;
  delete[] Blocal;
  delete[] Clocal;

  if (pid == root) {
    delete[] A;
    delete[] B;
    delete[] C;
  }

  MPI_Finalize();

  return 0;
}
