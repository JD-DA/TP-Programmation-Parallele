#ifndef FONCTIONS_H
#define FONCTIONS_H

const int TAG1 = 10;
const int TAG2 = 20;

int *generationMatriceCarre(int taille);
void produitMatrices(int n, int *A, int *B, int *C, bool accumule);
void ecriture(int n, int *A);
void extraction(int *A, int *extract, int ligne, int colonne, int n, int l);
void recopie(int *C, int *bloc, int ligne, int colonne, int n, int l);
#endif
