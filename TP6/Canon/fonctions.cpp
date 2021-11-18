#include <iomanip>
#include <iostream>
#include <random>
#include <stdlib.h>

#include "fonctions.h"

using namespace std;

int *generationMatriceCarre(int taille) {
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, 10);
  int *mat = new int[taille * taille];
  for (int i = 0; i < taille * taille; i++)
    mat[i] = dis(gen);
  return mat;
}

void produitMatrices(int n, int *A, int *B, int *C, bool accumule) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (!accumule)
        C[i * n + j] = 0;
      for (int k = 0; k < n; k++)
        C[i * n + j] += A[i * n + k] * B[k * n + j];
    }
  }
}

void ecriture(int n, int *A) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++)
      cout << right << setw(3) << A[i * n + j] << " ";
    cout << endl;
  }
}

void extraction(int *A, int *extract, int ligne, int colonne, int n, int l) {
  for (int i = 0; i < l; i++)
    for (int j = 0; j < l; j++)
      extract[i * l + j] = A[(ligne * l + i) * n + colonne * l + j];
}

void recopie(int *C, int *bloc, int ligne, int colonne, int n, int l) {
  for (int i = 0; i < l; i++)
    for (int j = 0; j < l; j++)
      C[(ligne * l + i) * n + colonne * l + j] = bloc[i * l + j];
}
