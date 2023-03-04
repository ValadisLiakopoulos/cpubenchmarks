#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <sys/sysctl.h>

#define scorebase 15000

int threadsC=0;
int sigleCoreScore=0;
int multiCoreScore=0;
int sigleThreadScore=0;
int multiThreadScore=0;

double get_wtime(void){
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec*1.0e-6;
}

double intergral(double x){
       return log(x)*sqrt(x);
}

void *thread_function(void *z){

    double a=1,b=4;
    unsigned long const n = 1e9;
    const double dx=(b-a)/n;
    double S=0;

    for(unsigned long i=*(int *)z; i<n; i+=threadsC){
         double xi=a+(i + 0.5)*dx;
            S+=intergral(xi);
    }
            S*=dx;
            return NULL;
}

void process_calc(int k,int processNumber){
    double a=1,b=10;
    unsigned long const n = 1e9;
    const double dx=(b-a)/n;
    double S=0;

    for(unsigned long i=k; i<n; i+=processNumber) {
         double xi=a+(i+0.5)*dx;
            S+=intergral(xi);
    }
            S*=dx;
}


int main(int argc, char *argv[]){

    system("clear");

    printf("Parallel Benchmarks System: \nCore Characteristics: \n");

    int NumOfCores=sysconf(_SC_NPROCESSORS_ONLN),finalscore;
    int NumOfThreads=NumOfCores*2;
    char cpumodel[100];
    size_t size = sizeof(cpumodel);
    sysctlbyname("machdep.cpu.brand_string", cpumodel, &size, NULL, 0);
    printf("CPU model: %s\n", cpumodel);


    printf("Cores: %d\n", NumOfCores);

    pthread_t threads[NumOfThreads];
    pid_t pid;

    int thread_args[NumOfThreads];
    for(int i=0; i<NumOfThreads; i++){
        thread_args[i]=i+1;
    }

    double t0,t1,tstart,tend,result;
    double times[10];
    fflush(stdout);

    tstart=get_wtime();
    //sigle core
    t0=get_wtime();
    process_calc(0, 1);
    t1=get_wtime();

    times[0]=t1-t0;
    sigleCoreScore=scorebase/times[0];

    //multi core: dimiourgw mia process gia kathe core kai ypologizoyn to olokliroma
    t0=get_wtime();
    for(int ifork=0; ifork<NumOfCores; ifork++){

        pid=fork();
        if(pid==0){
            process_calc(ifork,NumOfCores);
            exit(0);
        }
    }
    waitpid(-1, NULL, 0);
    t1=get_wtime();

    times[1]=t1-t0;
    multiCoreScore=scorebase/times[1];

    //sigle thread
    threadsC=1;
    t0=get_wtime();
    if(pthread_create(&threads[0], NULL, thread_function, &thread_args[0])!=0){
            fprintf(stderr, "Failed to create thread\n");
            exit(EXIT_FAILURE);
    }
    if (pthread_join(threads[0], (void **)&result)!=0){
            fprintf(stderr, "Failed to join thread\n");
            exit(EXIT_FAILURE);
    }

    t1=get_wtime();
    times[2]=t1-t0;
    sigleThreadScore=scorebase/times[2];

    //multi thread
    threadsC=NumOfCores*2;
    t0=get_wtime();
    for(int i=0; i<threadsC; i++){
    if (pthread_create(&threads[i], NULL, thread_function, &thread_args[i])!=0) {
            fprintf(stderr, "Failed to create thread\n");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0; i<threadsC; i++){
        if (pthread_join(threads[i], (void **)&result)!=0) {
            fprintf(stderr, "Failed to join thread\n");
            exit(EXIT_FAILURE);
        }
    }
    t1=get_wtime();
    times[3]=t1-t0;
    multiThreadScore=scorebase/times[3];
    tend=get_wtime();
    //telika
    finalscore=(sigleCoreScore+multiCoreScore+sigleThreadScore+multiThreadScore)/3;
    printf("\nSingle core score: %d\nMulti core score: %d \nSingle thread score: %d \nMulti thread score: %d\nFinal score: %d", sigleCoreScore, multiCoreScore, sigleThreadScore, multiThreadScore, finalscore);
    printf("\nTime taken for measures: %.5f\n", tend-tstart);
    fflush(stdout);
    sleep(2);

    return 0;

}
