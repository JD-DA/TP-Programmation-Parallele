//
// Created by jean-daniel on 30/09/2021.
//


#include "easypap.h"

#include <math.h>
#include <omp.h>
//#include <stdbool.h>

static void rotate (void);
static unsigned compute_color (int i, int j);

// If defined, the initialization hook function is called quite early in the
// initialization process, after the size (DIM variable) of images is known.
// This function can typically spawn a team of threads, or allocated additionnal
// OpenCL buffers.
// A function named <kernel>_init_<variant> is search first. If not found, a
// function <kernel>_init is searched in turn.
void baker_init (void)
{
    // check tile size's conformity with respect to CPU vector width
    // easypap_check_vectorization (VEC_TYPE_FLOAT, DIR_HORIZONTAL);

    PRINT_DEBUG ('u', "Image size is %dx%d\n", DIM, DIM);
    PRINT_DEBUG ('u', "Block size is %dx%d\n", TILE_W, TILE_H);
    PRINT_DEBUG ('u', "Press <SPACE> to pause/unpause, <ESC> to quit.\n");
}

// The image is a two-dimension array of size of DIM x DIM. Each pixel is of
// type 'unsigned' and store the color information following a RGBA layout (4
// bytes). Pixel at line 'l' and column 'c' in the current image can be accessed
// using cur_img (l, c).

// The kernel returns 0, or the iteration step at which computation has
// completed (e.g. stabilized).

unsigned baker_compute_seq (unsigned nb_iter)
{
    for (unsigned it = 1; it <= nb_iter; it++) {
        int w = DIM/2;
        int h = DIM/2;
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                float x1 = j/2;
                float x2 = j%2;
                if(i<h){
                    next_img(i,j)=cur_img(2*i+x2,x1);
                }else{
                    next_img(i,j)=cur_img(4*h-2*i-x2-1,2*w-x1-1);
                }
            }
        }


        swap_images (); // fonction de EasyPAP permettant de faire le swap de cur_img et next_img
    }
    return 0;
}

unsigned baker_compute_omp (unsigned nb_iter)
{
    for (unsigned it = 1; it <= nb_iter; it++) {
        int w = DIM/2;
        int h = DIM/2;

#pragma omp parallel for collapse(2)
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                float x1 = j/2;
                float x2 = j%2;
                if(i<h){
                    next_img(i,j)=cur_img(2*i+x2,x1);
                }else{
                    next_img(i,j)=cur_img(4*h-2*i-x2-1,2*w-x1-1);
                }
            }
        }


        swap_images (); // fonction de EasyPAP permettant de faire le swap de cur_img et next_img
    }
    return 0;
}

// Tile inner computation
static void do_tile_reg (int x, int y, int width, int height)
{

    int w = DIM/2;
    int h = DIM/2;
    for (int i = x; i < height+x; i++) {
        for (int j = y; j < width+y; j++) {
            float x1 = j/2;
            float x2 = j%2;
            if(i<h){
                next_img(i,j)=cur_img(2*i+x2,x1);
            }else{
                next_img(i,j)=cur_img(4*h-2*i-x2-1,2*w-x1-1);
            }
        }
    }
    /*printf("x : %d\n",x);
    printf("%d\n",y);
    printf("%d\n",width);
    printf("%d\n",height);
    int nbPix=0;
    printf("on est là !\n");
    for (int i = 0; i < height; i++) {
        //printf("%d,",i);

        for (int j = 0; j < width; j++) {
            nbPix++;
            next_img(x+i, y+j) = cur_img(x+i, y+j);

        }

    }*/
    //printf("nbPix : %d\n",nbPix);
}

static void do_tile (int x, int y, int width, int height, int who)
{
    // Calling monitoring_{start|end}_tile before/after actual computation allows
    // to monitor the execution in real time (--monitoring) and/or to generate an
    // execution trace (--trace).
    // monitoring_start_tile only needs the cpu number
    printf("ici\n");
    monitoring_start_tile (who);
    printf("la\n");
    do_tile_reg (x, y, width, height);
    printf("par la\n");
    // In addition to the cpu number, monitoring_end_tile also needs the tile
    // coordinates
    monitoring_end_tile (x, y, width, height, who);
    printf("par ici\n");
}


///////////////////////////// OMP version TP tiled
unsigned baker_compute_omp_tiled (unsigned nb_iter)
{

#pragma omp parallel for collapse(2)
    for (int y = 0; y < DIM; y += TILE_H){
        for (int x = 0; x < DIM; x += TILE_W) {
            do_tile(x, y, TILE_W, TILE_H,  omp_get_thread_num()/* CPU id */);
        }
    }

    swap_images();

    return 0;
}

int pgcd(int a, int b) {
    if (b == 0)
        return a;
    else
        return pgcd(b, a % b);
}


unsigned baker_compute_corners (unsigned nb_iter){
    //
    int w = DIM/2;
    int h = DIM/2;
    int cycleList[DIM*DIM];
    printf("taille : %d\n",DIM);
    int largeur = DIM;
    int hauteur = DIM;
    int*** cyclelistcoor = (int***) malloc(sizeof (int**)*DIM); //cyclelistcoor[x][y] = {x1,y1,x2,y2,x3,y3...xcyclelength,ycyclelength}
    int** cycleLength = (int**)malloc(sizeof(int*)*DIM); //cyclelength[x][y] = taille du cycle

    for (int i=0;i<largeur;i++){
        int **tab1= (int**)malloc(sizeof(int*)*DIM);
        int *tabLen = (int*)malloc(sizeof(int)*DIM);
        cyclelistcoor[i]=tab1;
        cycleLength[i]=tabLen;
    }

#pragma omp parallel for collapse(2)
        for (int i = 0; i < largeur; i++) {
            for (int j = 0; j < hauteur; j++) {
                int *tab2 = (int *) malloc(sizeof(int) * 5000); //tableau qui va contenir chaque coordonnees des pixels du cycle
                tab2[0] = i;
                tab2[1] = j;
                int originX = i;
                int originY = j;
                int cycle = 1;
                float x1 = j/2;
                float x2 = j%2;
                int curX,curY;
                if(i<DIM/2){
                    curX=2*i+x2;
                    curY=x1;
                }else{
                    curX=4*h-2*i-x2-1;
                    curY=2*w-x1-1;
                }
                int compteur=2;
                while((originX!=curX || originY!=curY) && compteur<5000){
                    tab2[compteur++] = curX;
                    tab2[compteur++] = curY;
                    cycle++;
                    float x1 = curY/2;
                    float x2 = curY%2;
                    if(curX<h){
                        curX=2*curX+x2;
                        curY=x1;
                    }else{
                        curX=4*h-2*curX-x2-1;
                        curY=2*w-x1-1;
                    }
                }
#pragma omp critical
                {
                    //cycleList[originY * DIM + originX] = cycle;
                    cyclelistcoor[i][j]=tab2;
                    cycleLength[i][j]=cycle;
                }
                //printf("Pour le pixel de coor : %d:%d (i:j) on a un cycle de :%d\t%d\n",originX,originY,cycle,cycleList[originY*DIM+originX]);
            }

        }
    //reduction du taleau qui contient les longueur des cycles afin de voir le nombre de cycle
        printf("\n");
        int cycleListUniq[DIM*DIM];
        int index=0;
        for (int i = 0; i < largeur; i++) {
            for (int j = 0; j < hauteur; j++) {
                int num = cycleLength[i][j];
                int present = 0;
                for(int m = 0; m<=index;m++){
                    if(num==cycleListUniq[m]){
                        present=1;
                        break;
                    }
                }
                if(0==present){
                    cycleListUniq[index++]=num;
                }
            }
        }
        for(int m = 1; m<index;m++) {
            printf("%d ",cycleListUniq[m]);
        }
    /*for (int i=0;i<largeur;i++){
        for (int j=0;j<hauteur;j++){
            int length = cycleLength[i][j];
            printf("Taille du cycle : %d [",length);
            for(int x = 0; x<length*2;x+=2){
                printf("| %d/%d ",cyclelistcoor[i][j][x],cyclelistcoor[i][j][x+1]);
            }
            printf("\n");
        }
    }*/
//#pragma omp parallel for collapse(2)
    for (int i = 0; i < largeur; i++) {
        for (int j = 0; j < hauteur; j++) {
            int length = cycleLength[i][j];
            int reste = 126661429%length;
            /*printf("Pour %d/%d on a un cycle de %d et le reste est %d soit la coor %d/%d \n",i,j,length,reste,cyclelistcoor[i][j][reste*2],cyclelistcoor[i][j][reste*2+1]);
            printf("Taille du cycle : %d [",length);
            for(int x = 0; x<length*2;x+=2){
                printf("| %d/%d ",cyclelistcoor[i][j][x],cyclelistcoor[i][j][x+1]);
            }
            printf("\n");
             */
            //printf("From %d/%d to %d/%d",i,j,cyclelistcoor[i][j][reste*2],cyclelistcoor[i][j][reste*2+1]);
            next_img(cyclelistcoor[i][j][reste*2],cyclelistcoor[i][j][reste*2+1])=cur_img(i,j);

        }
    }
    swap_images();


    for (int i=0;i<largeur;i++){
        for (int j=0;j<hauteur;j++) {
            free(cyclelistcoor[i][j]);
        }
        free(cyclelistcoor[i]);
        free(cycleLength[i]);
    }
    free(cyclelistcoor);
    free(cycleLength);

    return 0;
}





//////////////////////////////////////////////////////////////////////////

static float base_angle = 0.0;
static int color_a_r = 255, color_a_g = 255, color_a_b = 0, color_a_a = 255;
static int color_b_r = 0, color_b_g = 0, color_b_b = 255, color_b_a = 255;

static float atanf_approx (float x)
{
    float a = fabsf (x);

    return x * M_PI / 4 + 0.273 * x * (1 - a);
}

static float atan2f_approx (float y, float x)
{
    float ay   = fabsf (y);
    float ax   = fabsf (x);
    int invert = ay > ax;
    float z    = invert ? ax / ay : ay / ax; // [0,1]
    float th   = atanf_approx (z);           // [0,π/4]
    if (invert)
        th = M_PI_2 - th; // [0,π/2]
    if (x < 0)
        th = M_PI - th; // [0,π]
    if (y < 0)
        th = -th;

    return th;
}

// Computation of one pixel
static unsigned compute_color (int i, int j)
{
    float angle =
            atan2f_approx ((int)DIM / 2 - i, j - (int)DIM / 2) + M_PI + base_angle;

    float ratio = fabsf ((fmodf (angle, M_PI / 4.0) - (float)(M_PI / 8.0)) /
                         (float)(M_PI / 8.0));

    int r = color_a_r * ratio + color_b_r * (1.0 - ratio);
    int g = color_a_g * ratio + color_b_g * (1.0 - ratio);
    int b = color_a_b * ratio + color_b_b * (1.0 - ratio);
    int a = color_a_a * ratio + color_b_a * (1.0 - ratio);

    return rgba (r, g, b, a);
}

static void rotate (void)
{
    base_angle = fmodf (base_angle + (1.0 / 180.0) * M_PI, M_PI);
}


