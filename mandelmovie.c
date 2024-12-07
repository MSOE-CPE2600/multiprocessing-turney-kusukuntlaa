/// Author: Anish Kusukuntla
//  mandelmovie.c
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//  
//  Converted to use jpg instead of BMP and other minor changes
//  
///

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <math.h>
#include <string.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#define MAX 1000
void help();
//Removed generate_image since it was being unused and it felt easier to update the
//height and width for splitting up the regions in compute. 
void runtime(int num, int frame, int thread, int width, int height, char *dictionary);
void *compute(void *arg);
//The struct is for the values for the thread to be passed into
//creating the pthread_create
typedef struct {
    char name[100];
    double x; 
    double y; 
    double scale;
    int width; 
    int height;
} Thread_Args;
//my main would process the command line of choices using getopt. 
int main(int argc, char *argv[]){
    int num = 4;
    int frame = 50;
    int width = 1000;
    int height = 1000;
    char *directory = ".";
    char choice;
    int thread = 1;
    
    while((choice = getopt(argc, argv, "n:f:w:h:o:t:H")) != -1){
        switch(choice){
            case 'n':
                num = atoi(optarg);
                break;
            case 'f':
                frame = atoi(optarg);
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case 'o':
                directory = optarg;
                break;
            case 't':
                thread = atoi(optarg);
                if(thread < 1 || thread > 20){
                    fprintf(stderr, "The thread value needs to be in between 1 and 20\n");
                    exit(1);
                }
                break;
            case 'H':
                help(); 
                exit(1);
        }
    }
    
    printf("frames: %d\n", frame);
    printf("Number of children: %d\n", num);
    printf("Initial Image size: %d x %d\n", width, height);
    printf("treads: %d\n", thread);
    
    printf("With the amount of child processes: %d\n", num);
    runtime(num, frame, thread, width, height, directory);
    printf("Success with the frames!!\n");
    return 0;

}


//Prints out the functions you could use at the command line. 
void help(){
    printf("Use: mandelmovie [options]\n");
    printf("Where options are:\n");
    printf("-n <int>    Number of child processes\n");
    printf("-f <int>    Number of frames to generate\n");
    printf("-w <int>    Width of images in pixels\n");
    printf("-h <int>    Height of images in pixels\n");
    printf("-o <dir>    The output directory for images\n");
    printf("-t <int>    Number of threads to generate\n");
    printf("-H          Show this help text\n");

}
//would use fork and pthreads to have the amount of frames be evenly split to the processes and threads.
//The pthreads would create the new thread to have the system go faster. 
//I used sem_open for the mandel_semaphore to create the images. 
//The semaphore would close and unlink at the end. 
void runtime(int num, int frame, int thread, int width, int height, char *dictionary){
    sem_t *st = sem_open("/mandel_semaphore", O_CREAT | O_EXCL, 0644, num);
    if(st == SEM_FAILED){
        sem_unlink("/mandel_semaphore");
        st = sem_open("/mandel_semaphore", O_CREAT, 0644, num);
        if(st == SEM_FAILED){
            perror("Issues with SEM_OPEN");
            exit(1);
        }
    }
    
    double scale = (4.0)/frame;
    
    int frameschild = frame/num; 
    int remain = frame % num;
    
    for(int i = 0; i < num; i++){
        sem_wait(st);
        pid_t pid = fork();
        
        if(pid == 0){
            int framestart = i * frameschild;
            int frameend = framestart + frameschild;
            
            if(i == num - 1){
                frameend = frameend + remain;
            }
            
            
            for(int j = framestart; j < frameend; j++){
                char name[100];
                snprintf(name, 100, "%s/mandel_%d.jpg", dictionary, j);
                double scale2 = 4.0 - j * scale;
                pthread_t threading[thread];
                Thread_Args args[thread];
                
                int r_height = height/thread;
                for(int k = 0; k < thread; k++){
                    snprintf(args[k].name, 100, "%s", name);
                    args[k].x = -scale2/2;
                    args[k].y = -scale2/2 + k * (scale2/thread);
                    args[k].scale = scale2;
                    args[k].width = width;
                    if(k == thread - 1){
                        args[k].height = height - k * r_height;
                    } else {
                        args[k].height = r_height;
                    }

                    if(pthread_create(&threading[k], NULL, compute, &args[k]) != 0){
                        perror("Creating thread");
                        exit(1);
                    }
                }
                for(int k = 0; k < thread; k++) {
                    if(pthread_join(threading[k], NULL) != 0){
                        perror("Joining thread");
                        exit(1);
                    }
                    printf("thread %d to frame %d\n", k, j);
                }
                
            }
            
            sem_post(st);
            exit(0);
        } else if(pid < 0){
            perror("Issues with fork");
            sem_post(st);
            break;
        }
        
    }
    
    while(wait(NULL) > 0);
    sem_close(st);
    sem_unlink("/mandel_semaphore");
}
//compute gets passed in to the pthreads used in the runtime method
//compute would update the images height and width just so that the 
//threading would split up the region. The image would be stored once
//the height and width are updated. 
void *compute(void *arg){
    Thread_Args *args = (Thread_Args *)arg;
    imgRawImage *img = initRawImage(args->width, args->height);
    if(!img){
        fprintf(stderr, "Unable to initialize image\n");
        return NULL;
    }
    
    for(int j = 0; j < args->height; j++){
        for(int i = 0; i < args->width; i++){
            double real1 = args->x + i * (args->scale/args->width);
            double imag1 = args->y + j * (args->scale/args->height);
            double value_x = 0;
            double value_y = 0;
            int iteration = 0;

            while(value_x * value_x + value_y * value_y <= 4 && iteration < MAX){
                double total = value_x * value_x - value_y * value_y + real1;
                value_y = 2 * value_x * value_y + imag1;
                value_x = total;
                iteration++;

            }
            int color;
            if(iteration == MAX){
                color = 0;
            } else {
                color = 255 - (iteration*255/MAX);
                
            }
            setPixelRGB(img, i, j, color, color, color);
        }

    }
    if (storeJpegImageFile(img, args->name) != 0) {
        fprintf(stderr, "Unable to store image\n");
    }
    freeRawImage(img);
    return NULL;
}