#include <mpi.h>
#include <iostream>
#include <iomanip>

using namespace std;

double fx(double x)
{
    double res =(double)4.0/(1+x*x);
    return res;
}


int main ( int argc , char **argv )
{
    int pid, nprocs;
    MPI_Init (&argc , &argv) ;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid ) ;
    MPI_Comm_size (MPI_COMM_WORLD, &nprocs ) ;

    int n = atoi(argv[1]);
    int root = atoi(argv[2]);

    double Pi;
    double delta = (double)1/n;

    int chunk = n/nprocs;
    int reste = n%nprocs;

    int nlocal = chunk;

    if(pid<reste){
        nlocal++;
    }
    int start = pid *chunk + (min(pid,reste));

    double res = 0;

    for(int i= start;i<nlocal + start;i++){
        res+= fx(((double)1/(2*n))+i*((double)1/n))*delta;
    }
    cout<<"From "<<pid<<" on a pi = "<<res<<endl;

    MPI_Reduce(&res,&Pi,1,MPI_DOUBLE,MPI_SUM,root,MPI_COMM_WORLD);
    if (pid==root) {
        cout << "PI=" << std::setprecision(15) << Pi << endl;
    }
    MPI_Finalize() ;
    return 0 ;
}