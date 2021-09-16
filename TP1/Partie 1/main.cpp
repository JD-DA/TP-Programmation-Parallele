#include <iostream>
#include <omp.h>
#include <random>
#include <chrono>
#include <ctime>

using namespace std;

int main(int argc, char* argv[]) {

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 10);


    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;

    int size = atoi(argv[1]);

    int *A = new int[size*size];
    int *B = new int[size*size];
    int *C = new int[size*size];


    for (int i=0; i<size*size; i++) {
        A[i] = dis(gen);
        B[i] = dis(gen);
        C[i] = 0;
    }

    start = chrono::system_clock::now();
    for (int i=0; i<size; i++)
        for (int j=0; j<size; j++)
            for (int k=0; k<size; k++)
                C[i*size+j] += A[i*size+k]*B[k*size+j];
    end = chrono::system_clock::now();
    elapsed_seconds = end-start;
    cout << "temps séquentiel écoulé : " << elapsed_seconds.count() << endl;


    for (int i=0; i<size*size; i++) {
        C[i] = 0;
    }

    start = chrono::system_clock::now();
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                 for (int k = 0; k < size; k++)
                    C[i * size + j] += A[i * size + k] * B[k * size + j];
    }

    end = chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "temps omp parallel for (ligne): " << elapsed_seconds.count() << endl;
for (int i=0; i<size*size; i++) {
        C[i] = 0;
    }


   for (int i=0; i<size*size; i++) {
        C[i] = 0;
    }

    start = chrono::system_clock::now();
        for (int i = 0; i < size; i++) {
#pragma omp parallel for firstprivate(i)
            for (int j = 0; j < size; j++)
                for (int k = 0; k < size; k++)
                    C[i * size + j] += A[i * size + k] * B[k * size + j];
        }


    end = chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "temps omp parallel for (point): " << elapsed_seconds.count() << endl;
for (int i=0; i<size*size; i++) {
        C[i] = 0;
    }

    start = chrono::system_clock::now();
#pragma omp parallel
    {
#pragma omp single nowait
        {
            for (int i = 0; i < size; i++) {
#pragma omp task firstprivate(i)
                {
                    for (int j = 0; j < size; j++)
                        for (int k = 0; k < size; k++)
                            C[i * size + j] += A[i * size + k] * B[k * size + j];
                }
            }
        }
    }
    end = chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "temps omp parallel task : " << elapsed_seconds.count() << endl;

    for (int i=0; i<size*size; i++)
        C[i] = 0;
start = chrono::system_clock::now();
#pragma omp parallel for
    for (int i = 0; i < size; i++)

        for (int j = 0; j < size; j++) {
            int sum = 0;
#pragma omp simd reduction(+:sum)
            for (int k = 0; k < size; k++)
                 sum+= A[i * size + k] * B[k * size + j];
            C[i * size + j] = sum;
        }

    end = chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "temps omp simd : " << elapsed_seconds.count() << endl;


 /*      cout << "la matrice A" << endl;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++)
            cout << A[i * size + j] << " ";
        cout << endl;
    }

        cout << "la matrice B" << endl;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++)
                cout << B[i * size + j] << " ";
            cout << endl;
        }
            cout << "la matrice C" << endl;
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++)
                    cout << C[i * size + j] << " ";
                cout << endl;
            }*/

    return 0;
}