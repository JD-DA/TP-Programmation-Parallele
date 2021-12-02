#include <iostream>
#include <math.h>
#include <mpi.h>

using namespace std;

const int TAG = 10;


/*******************************************************************************************************/
/* Une petite fonction d'affichage...                                                                  */
/*******************************************************************************************************/
void afficher(double *A, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            cout << A[i * n + j] << " ";
        cout << endl;
    }
}

/*******************************************************************************************************/
/* Partie pour générer la matrice à partir d'un résultat                                               */
/*******************************************************************************************************/
void generation(double* A, int n)
{
  double *L = new double[n*n];
  double *U = new double[n*n];

  srand(time(NULL));
  for (int i=0; i<n; i++) {
    for (int j=0; j<i+1; j++)
      L[i*n+j] = (float)rand()/(float)rand();
    L[i*n+i]=1.0;
  }
  for (int i=0; i<n; i++)
    for (int j=i; j<n; j++)
      U[i*n+j] = (float)rand()/(float)rand();

  for (int i=0; i<n; i++)
    for (int j=0; j<n; j++)
      for (int k=0; k<n; k++)
	A[i*n+j] += L[i*n+k]*U[k*n+j];

  /*  afficher(A,n);
  afficher(L,n);
  afficher(U,n);*/
  delete[] L;
  delete[] U;
}


/*******************************************************************************************************/
/* Partie avec les fonctions utiles pour les communications et la mise en place de la factorisation LU */
/*******************************************************************************************************/
/* Un processus va avoir une liste de blocs par rapport à la distribution en Round-Robin */
/* On aura un tableau qui va permettre de stocker les blocs les uns après les autres     */
/* cette fonction permet de calculer le numéro du bloc pour le retrouver dans ce tableau */
/* Si un bloc a les coordonnées (i,j) dans la matrice globale alors il aura les          */
/* coordonnées (i/p, j/p) sur le processus et le processus a bien b/p x b/p blocs        */
/* d'où le numéro calculé.                                                               */
int num_bloc_local(int i, int j, int b, int p) {
    return (i / p) * (b / p) + (j / p);
}

/* Lorsqu'on a une matrice représentée par bloc on peut avoir besoin d'extraire un bloc  */
/* de taille l x l de coordonnées (kl,kc) dans la matrice globale de taille n x n        */
void extraction(double *matrice, double *bloc, int n, int l, int kl, int kc) {
    for (int i = 0; i < l; i++)
        for (int j = 0; j < l; j++)
            bloc[i * l + j] = matrice[(i + kl * l) * n + kc * l + j];
}

/* La matrice initiale est générée sur le processus root                                 */
/* Il faut ensuite que ce processus root la distribue par bloc. Pour celain on peut      */
/* utiliser une topologie cartésienne afin de faciliter le calcul du rang du processus   */
/* à qui envoyer un bloc                                                                 */
/* cette fonction est capable de faire une distribution en round-robin dans les 2        */
/* dimensions de la topologie cartésienne                                                */
void distributionBlocs(double *matrice, double *blocs, int n, int l, int b, int root, int p, MPI_Comm grille) {
    int pid, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int nb_blocs = (b / p) * (b / p);

    if (pid == root) {
        double *tmp = new double[l * l];
        for (int i = 0; i < b; i++)
            for (int j = 0; j < b; j++) {
                extraction(matrice, tmp, n, l, i, j);
                int to[2];
                to[0] = i % p;
                to[1] = j % p;
                int to_rank;
                MPI_Cart_rank(grille, to, &to_rank);
                if (to_rank != pid)
                    MPI_Ssend(tmp, l * l, MPI_DOUBLE, to_rank, TAG, MPI_COMM_WORLD);
                else {
                    int num_bloc = num_bloc_local(i, j, b, p);
                    for (int k = 0; k < l * l; k++)
                        blocs[num_bloc * l * l + k] = tmp[k];
                }
            }
        delete[] tmp;
    }
    if (pid != root) {
        for (int i = 0; i < nb_blocs; i++)
            MPI_Recv(blocs + i * l * l, l * l, MPI_DOUBLE, root, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

/* le rassemblement des données associé à la distribution précédente                     */
void rassemblementBlocs(double *matrice, double *blocs, int n, int l, int b, int root, int p, MPI_Comm grille) {
    int pid, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int nb_blocs = (b / p) * (b / p);

    if (pid == root) {
        double *tmp = new double[l * l];
        int from[2];
        int from_rank;
        for (int i = 0; i < b; i++) {
            for (int j = 0; j < b; j++) {
                from[0] = i % p;
                from[1] = j % p;
                MPI_Cart_rank(grille, from, &from_rank);
                if (from_rank != root) {
                    MPI_Recv(tmp, l * l, MPI_DOUBLE, from_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    for (int kl = 0; kl < l; kl++)
                        for (int kc = 0; kc < l; kc++)
                            matrice[(kl + i * l) * n + j * l + kc] = tmp[kl * l + kc];
                } else {
                    for (int kl = 0; kl < l; kl++) {
                        for (int kc = 0; kc < l; kc++) {
                            int num_bloc = num_bloc_local(i, j, b, p);
                            matrice[(kl + i * l) * n + j * l + kc] = blocs[num_bloc * l * l + kl * l + kc];
                        }
                    }
                }
            }
        }
        delete[] tmp;
    } else {
        for (int i = 0; i < nb_blocs; i++) {
            MPI_Ssend(blocs + i * l * l, l * l, MPI_DOUBLE, root, TAG, MPI_COMM_WORLD);
        }
    }
}
/*******************************************************************************************************/

/*******************************************************************************************************/
/* Partie avec les fonctions pour les opérations sur les matrices                                      */
/* Factorisation LU séquentielle                                                                       */
/* Résolution de systèmes triangulaires inférieurs/supérieurs                                          */
/*******************************************************************************************************/

/* On va utiliser une seule matrice pour stocker la partie U et la partie L de la factorisation        */
/* sachant que les 1 de la diagonale de L ne seront pas conservés                                      */
/* La fonction extraitL permet d'extraire la partie L d'une telle matrice                              */
void extraitL(double *ABlocs, double *blocL, int l) {
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < i; j++)
            blocL[i * l + j] = ABlocs[i * l + j];
        blocL[i * l + i] = 1.0;
        for (int j = i + 1; j < l; j++)
            blocL[i * l + j] = 0.0;
    }
}
/* Même chose pour extraire la partie U                                                                */
void extraitU(double *ABlocs, double *blocL, int l) {
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < i; j++)
            blocL[i * l + j] = 0.0;
        for (int j = i; j < l; j++)
            blocL[i * l + j] = ABlocs[(i * l + j)];
    }
}
/* Lorsqu'on a plusieurs blocs à gérer sur un processus on a un tableau unique pour les stocker        */
/* les uns à la suite des autres. Cette fonction permet simplement d'extraire un bloc à partir de son  */
/* numéro local (cf la fonction num_bloc_local)                                                        */
void extrait(double *ABlocs, double *blocL, int num_bloc, int l) {
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < l; j++)
            blocL[num_bloc * l * l + (i * l + j)] = ABlocs[num_bloc * l * l + (i * l + j)];
    }
}
/* La fonction pour le pivot de Gauss                                                                  */
/* la ligne i1 est la ligne de référence et elle n'est pas modifiée                                    */
/* On veut faire apparaître un 0 sur la ligne i2 à la position "diagonale"                             */
void gaussEliminationLigne(double *A, int i1, int i2, int n) {
    double pivot = A[i2 * n + i1] / A[i1 * n + i1];
    for (int j = i1; j < n; j++)
        A[i2 * n + j] = A[i2 * n + j] - pivot * A[i1 * n + j];
    A[i2 * n + i1] = pivot;
}
/* La décomposition LU à partir de la fonction précédente d'un pivot de Gauss entre 2 lignes           */
/* A est mise à jour directement avec L dans la partie triangulaire inférieure et U dans la partie     */
/* triangulaire supérieure (on ne représente pas les 1 de la diagonale de L                            */
void decompLU(double *A, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++) {
            gaussEliminationLigne(A, i, j, n);
        }
}
/* Résolution du système triangulaire supérieur     :                                                  */
/* On résout M x U = bloc et en sortie on copie M (le résultat) dans bloc                              */
void resolutionSup(double *bloc, double *U, int l) {
    double *M = new double[l * l];
    double tmp;
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < l; j++) {
            tmp = 0;
            for (int k = 0; k <= j - 1; k++) {
                tmp += M[i * l + k] * U[k * l + j];
            }
            M[i * l + j] = (bloc[i * l + j] - tmp) / U[j * l + j];
        }
    }
    for (int i = 0; i < l; i++)
        for (int j = 0; j < l; j++)
            bloc[i * l + j] = M[i * l + j];
    delete[] M;
}
/* Résolution du système triangulaire inférieur     :                                                  */
/* On résout L x M = bloc et en sortie on copie M (le résultat) dans bloc                              */
void resolutionInf(double *bloc, double *L, int l) {
    double *M = new double[l * l];
    double tmp;
    for (int j = 0; j < l; j++) {
        for (int i = 0; i < l; i++) {
            tmp = 0;
            for (int k = 0; k <= i - 1; k++) {
                tmp += M[k * l + j] * L[i * l + k];
            }
            M[i * l + j] = (bloc[i * l + j] - tmp);
        }
    }
    for (int i = 0; i < l; i++)
        for (int j = 0; j < l; j++)
            bloc[i * l + j] = M[i * l + j];
    delete[] M;
}


/* Résolution du système triangulaire inférieur     :                                                  */
/* Lorsqu'on veut calculer Atilde                                                                      */
/* on part du bloc de A à mettre à jour et on sait que bloc = bloc-L*U                                 */
void calculAtilde(double *bloc, double *L, double *U, int n) {

    double *tmp = new double[n * n];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            tmp[i * n + j] = 0;
            for (int k = 0; k < n; k++)
                tmp[i * n + j] += L[i * n + k] * U[k * n + j];
        }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            bloc[i * n + j] = bloc[i * n + j] - tmp[i * n + j];

    delete[] tmp;
}
/*******************************************************************************************************/
/*******************************************************************************************************/
/* Partie juste pour tester si le résutat est correct                                                  */
/* Le résultat est supposé être une seule matrice avec en partie triangulaire inférieure stricte la    */
/* matrice L et la partie triangulaire supérieure la matrice U de la factorisation                     */
/*******************************************************************************************************/
unsigned int testResultat(double *A, double *B, int n) {
    unsigned int erreur = 0;
    double *L = new double[n * n];
    double *U = new double[n * n];
    extraitL(B, L, n);
    extraitU(B, U, n);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double tmp = 0;
            for (int k = 0; k < n; k++)
                tmp += L[i * n + k] * U[k * n + j];
            if (A[i * n + j] != tmp) {
                erreur = 1;
                break;
            }
        }
    delete[] L;
    delete[] U;
    return erreur;
}

/*******************************************************************************************************/
int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);
    int pid, nbprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nbprocs);

    int root = atoi(argv[3]);

    int n; // la matrice est de taille n x n
    int l; // le bloc est de taille l x l
    int b; // la matrice en blocs est de taille b x b blocs de taille l x l
    int b_per_rank; // combien de blocs par processus
    int p; // le nombre de processus dans une dimension et on suppose la grille de taille p x p

    double *A; // Pour la matrice initiale
    double *B; // Pour le résultat fusion de L et U
    double *ABlocs; // Pour recevoir le ou les blocs de la matrice qu'on aura en charge

    // Pour la gestion des processus et la création de la topologie et des groupes dont
    // on aura besoin pour les communications

    MPI_Comm Comm_grille;
    int nbprocs_dims[2] = {0, 0};
    int period[2] = {0, 0};
    int mycoords[2];

    MPI_Dims_create(nbprocs, 2, nbprocs_dims);
    MPI_Cart_create(MPI_COMM_WORLD, 2, nbprocs_dims, period, true, &Comm_grille);
    MPI_Cart_coords(Comm_grille, pid, 2, mycoords);

    p = nbprocs_dims[0];
    n = atoi(argv[1]);
    l = atoi(argv[2]);
    b = n / l; // On peut rajouter un test pour bien vérifier que n est divisible par l
    b_per_rank = (b / p) * (b / p); // Dans un premier temps on pourra supposer un bloc par processus de la grille
                                    // On choisira l et b dans ce sens et cette valeur sera donc égale à 1.

    if (pid == root) {
        A = new double[n * n];
        B = new double[n * n];
        generation(A, n);
    }
    ABlocs = new double[b_per_rank * l * l];

    distributionBlocs(A, ABlocs, n, l, b, root, nbprocs_dims[0], Comm_grille);
    /////
    /// A compléter ici pour mettre en place la boucle sur le nombre de blocs diagonaux
    /// pour réaliser la factorisation LU.
    double temp = new double[l*l];
    double temp2 = new double[l*l];
    MPI_Comm Comm_ligne;
    MPI_Comm Comm_colonne;
    int periodLigne[2] = {0, 0};
    int remains[2];
    remains[0] = false;
    remains[1] = true;
    MPI_Cart_sub( Comm_grille, remains, &Comm_ligne );
    remains[0] = true;
    remains[1] = false;
    MPI_Cart_sub( Comm_grille, remains, &Comm_colonne );
    for (int e = 0; e < 1/*b_per_rank*/; e++) {
        if (mycoords[0] == e and mycoords[1] == e) { //en gros e est root
            //on est dans A00
            decompLU(ABlocs, l);

            extraitL(double *ABlocs, temp, l);
            //L00*U0j=A0j
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_ligne);

            extraitU(double *ABlocs, temp, l);
            //Lj0*U00=Ai0
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_colonne);

            //? recevoir le resultat de la colonne/ligne ?  Non ! il est exclu du reste :
            // Il partage a une ligne puis chaque blic de ligne va partager a sa colonne, l'excluant alors

        } else if (mycoords[0] == e) {
            //on est dans A0j, La premiere colonne
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_colonne);
            resolutionSup(ABlocs, temp, l); // renvoie L
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_ligne);
        }else if (mycoords[1] == e) {
            //on est dans Ai0, la premiere ligne
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_ligne);
            resolutionInf(ABlocs, temp, l); // renvoie U
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_colonne);

        }else{
            MPI_Bcast(temp, l*l,MPI_DOUBLE,e,Comm_ligne); // L
            MPI_Bcast(temp2, l*l,MPI_DOUBLE,e,Comm_colonne); // U
            calculAtilde(ABlocs, temp, temp2, l)
        }

    }



    rassemblementBlocs(B, ABlocs, n, l, b, root, nbprocs_dims[0], Comm_grille);

    if (pid == root) {
        for (int i=0; i<n*n; i++)
            if (A[i]!=B[i]) {
                cout << "erreur !! " << endl;
                break;
            }
    }
    if (pid == root) {
        delete[] A;
        delete[] B;
    }
    delete[] ABlocs;


    MPI_Finalize();
    return 0;
}

