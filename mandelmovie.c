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
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>

#define MAX 1000
void help();
void generate_image(char *name, double x, double y, double scale, int width, int height);
void runtime(int num, int frame, int width, int height, char *dictionary);
//my main would process the command line of choices using getopt. 
int main(int argc, char *argv[]){
    int num = 4;
    int frame = 50;
    int width = 1000;
    int height = 1000;
    char *directory = ".";
    char choice;
    while((choice = getopt(argc, argv, "n:f:w:h:o:H")) != -1){
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
            case 'H':
                help();
                exit(1);
        }

    }

    printf("frames: %d\n", frame);
    printf("Number of children: %d\n", num);
    printf("Image size: %d x %d\n", width, height);
    
    
    printf("With the amount of child processes: %d\n", num);
    runtime(num, frame, width, height, directory);
    printf("Success with the frames!!\n");
    return 0;

}
//The image would be generated through using the pixels to move around 
//on creating the image. 
//The size of the image would be created by incorporating the width and height at
//certain scales. 
void generate_image(char *name, double x, double y, double scale, int width, int height){
    imgRawImage *img = initRawImage(width, height);
    if(!img) {
        fprintf(stderr, "Unable to initiate image\n");
        exit(1);
    }

    double min1 = x - scale/2;
    double max1 = x + scale/2;
    double min2 = y - (scale * height / width)/2;
    double max2 = y + (scale * height / width)/2;
    
    for(int j = 0; j < height; j++){
        for(int i = 0; i < width; i++){
            double real1 = min1 + i * (max1 - min1)/width;
            double imag1 = min2 + j * (max2 - min2)/height;

            double real2 = 0;
            double imag2 = 0;
            int iteration = 0;

            while(real2 * real2 + imag2 * imag2 <= 4 && iteration < MAX){
                double t = real2 * real2 - imag2 * imag2 + real1;
                imag2 = 2 * real2 * imag2 + imag1;
                real2 = t;
                iteration++;
            }
            int color;
            if(iteration == MAX){
                color = 0;
            } else {
                color = 255 - (iteration * 255/MAX);
            }
            setPixelRGB(img, i, j, color, color, color);
        }
    }
    
    if(storeJpegImageFile(img, name) != 0){
        fprintf(stderr, "Unable to store %s\n", name);

    }
    freeRawImage(img);

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
    printf("-H          Show this help text\n");

}
//would do a fork to have the amount of frames be evenly split to the processes
//The fork would create the new process to have the system go faster. 
//I used sem_open for the mandel_semaphore to create the images. 
//The semaphore would close and unlink at the end. 
void runtime(int num, int frame, int width, int height, char *dictionary){
    
    
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
                generate_image(name, 0, 0, scale2, width, height);
                
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