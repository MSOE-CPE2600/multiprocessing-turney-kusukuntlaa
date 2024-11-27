# System Programming Lab 11 Multiprocessing
#5
a. The implementation of my mandelmovie.c is meant to generate a series of Mandelbrot images that are set by using multiprocessing and then combining them into an animation. The program has the getopt function to parse command-line arguments that allows the user to specify parameters such as the number of processes, frames, image dimensions, and output directory. The generate_image function would calculate the pixel colors based on the iterations of the Mandelbrot set formula such as the scaling and positioning of the set dynamically for each frame. The runtime function incorporates the semaphores and process forking to manage the generation of frames by child processes. The workload is as close to even among them to enhance performance. Each child process would generate an even amount of frames, write them as JPG files, and then exits. The processes would need to be completed, so I used a wait(NULL) at the end. 
b. 
Child Processes	  Runtime (s)
1	               112.6
2	               115.6
5	               197.7
10	               220.8
20	               216.8

![Screenshot](CPE2600RuntimesLab11.png)

c. My results for my runtimes were from using the time ./mandelmovie -n (# of Child Processes). I used the user time to get as accurate results possible. As seen by the table and graph, I see that as the number of child processes increases, the runtime is larger since the the fork would split the frames up for each process. The rate would be by the frames/child process. An example would be if there are 10 processes and 50 frames, the runtime is larger since the amount of processes at each frame is 50/10 which is a total of 5 child processes with each one of them being at 10 frames. 
