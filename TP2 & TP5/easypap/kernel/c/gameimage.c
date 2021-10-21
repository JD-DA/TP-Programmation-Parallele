#include "easypap.h"

#include <math.h>
#include <omp.h>
#include <mpi.h>


static unsigned couleur = 0xFFFF00FF;



static inline int indiceplus(int a)
{
  return (a+1)%DIM;
}

static inline int indicemoins(int a)
{
  return (a-1+DIM)%DIM;
}

void do_gameimage(int x, int y)
{
  int nb_life = 0;
  int x1 = indiceplus(x);
  int x2 = indicemoins(x);
  
  int y1 = indiceplus(y);
  int y2 = indicemoins(y);
  
  nb_life = (cur_img(x1,y1)!=0)+(cur_img(x1,y)!=0)+(cur_img(x1,y2)!=0)
    +(cur_img(x2,y1)!=0)+(cur_img(x2,y)!=0)+(cur_img(x2,y2)!=0)
    +(cur_img(x,y1)!=0)+(cur_img(x,y2)!=0);

   if (cur_img(x,y)==0) {
    if (nb_life==3)
      next_img(x,y)=couleur;
    else
      next_img(x,y)=0;
  }
  else if (cur_img(x,y)!=0) {
    if (nb_life==3 || nb_life==2)
      next_img(x,y) = couleur;
    else 
      next_img(x,y) = 0;
  }
}
    

// Tile inner computation
static void do_tile_reg (int x, int y, int width, int height)
{  
  for (int i = y; i < y + height; i++)
    for (int j = x; j < x + width; j++)
      do_gameimage(i,j);
}

static void do_tile (int x, int y, int width, int height, int who)
{
  // Calling monitoring_{start|end}_tile before/after actual computation allows
  // to monitor the execution in real time (--monitoring) and/or to generate an
  // execution trace (--trace).
  // monitoring_start_tile only needs the cpu number
  monitoring_start_tile (who);
  
  do_tile_reg (x, y, width, height);
  
  // In addition to the cpu number, monitoring_end_tile also needs the tile
  // coordinates
  monitoring_end_tile (x, y, width, height, who);
}



unsigned gameimage_compute_seq (unsigned nb_iter)
{
    for (unsigned it = 1; it <= nb_iter; it++) {
        do_tile(0,0,DIM,DIM,0);
        swap_images();
    }
    return 0;
}
static int mpi_rank = -1;
static int mpi_size = -1;
static int mpi_nbLigne = -1;
static int mpi_ligne = -1;
static int mpi_rank_lef=-1;
static int mpi_rank_right=-1;
void gameimage_init_mpi(void)
{
    easypap_check_mpi();
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    mpi_nbLigne = (DIM/mpi_size);
    mpi_ligne = (DIM/mpi_size)*mpi_rank;
    mpi_rank_lef = (mpi_rank-1+mpi_size)%mpi_size;
    mpi_rank_right = (mpi_rank+1)%mpi_size;
}

unsigned gameimage_compute_mpi (unsigned nb_iter)
{
    for (unsigned it = 1; it <= nb_iter; it++) {
        int x = 0;
        int y = mpi_ligne;
        int width = DIM;
        int height = mpi_nbLigne;
        do_tile(x,y,width,height,0); //0 car on fait du mpi et pas du openMP
        swap_images();

    }
    MPI_Allgather(image+mpi_ligne*DIM, mpi_nbLigne*DIM , MPI_INT , image , mpi_nbLigne*DIM ,MPI_INT , MPI_COMM_WORLD) ;
    return 0;
}
unsigned gameimage_compute_mpiNotAll (unsigned nb_iter)
{
    for (unsigned it = 1; it <= nb_iter; it++) {
        int x = 0;
        int y = mpi_ligne;
        int width = DIM;
        int height = mpi_nbLigne;
        int thread = mpi_rank;
        do_tile(x,y,width,height,0); //0 car on fait du mpi et pas du openMP
        swap_images();
        MPI_Sendrecv(image+y*DIM,               DIM,MPI_INT,mpi_rank_lef,   1,image+y*DIM,                DIM,MPI_INT,mpi_rank,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //reicv en fonction du rank de l'emetteur
        MPI_Sendrecv(image+(y+mpi_nbLigne-1)*DIM,DIM,MPI_INT,mpi_rank_right,2,image+(y+mpi_nbLigne-1)*DIM,DIM,MPI_INT,mpi_rank,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

    }
    MPI_Gather(image+mpi_ligne*DIM, mpi_nbLigne*DIM , MPI_INT , image , mpi_nbLigne*DIM ,MPI_INT ,0, MPI_COMM_WORLD) ;
    return 0;
}



