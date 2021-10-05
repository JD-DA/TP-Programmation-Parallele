//
// Created by jean-daniel on 30/09/2021.
//


#include "easypap.h"

#include <math.h>
#include <omp.h>

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

unsigned baker_compute_corners (unsigned nb_iter){
    int w = DIM/2;
    int h = DIM/2;
    for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp parallel for collapse(2)
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < 1; j++) {
                int originX = i;
                int originY = j;
                //printf("origin%d:%d\n",originX,originY);
                int cycle = 1;
                float x1 = j/2;
                float x2 = j%2;
                int curX,curY;
                if(i<DIM/2){
                    curX=2*i+x2;
                    curY=x1;
                    //next_img(i,j)=cur_img(2*i+x2,x1);
                }else{
                    curX=4*h-2*i-x2-1;
                    curY=2*w-x1-1;
                    //next_img(i,j)=cur_img(4*h-2*i-x2-1,2*w-x1-1);
                }
                //printf("c1 : %d:%d\n",curX,curY);
                while(originX!=curX || originY!=curY){
                    i = curX;
                    j = curY;
                    cycle++;
                    float x1 = j/2;
                    float x2 = j%2;
                    if(i<h){
                        curX=2*i+x2;
                        curY=x1;
                        //next_img(i,j)=cur_img(2*i+x2,x1);
                    }else{
                        curX=4*h-2*i-x2-1;
                        curY=2*w-x1-1;
                        //next_img(i,j)=cur_img(4*h-2*i-x2-1,2*w-x1-1);
                    }
                    //printf("c%d : %d:%d\n",cycle,curX,curY);
                }
                printf("Pour le pixel de coor : %d:%d (i:j) on a un cycle de :%d\n",originX,originY,cycle);
            }
        }

    }
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


