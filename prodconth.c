//File: prodconth.c
//Autor: Tyler Besecker
//Date: 11/07/2017
//Program Mutiple consumer/producer
//Programming Assignment: 4
//The problem being solved is creating a mutiple producer consumer using
//threads, mutex, and conditonal variable
//The overall flow of this program: the program declares all needed variables,
//the producer produces as long as the buffer isnt full and we are under
// 100 total numbers. The consumer consumes as long as the buffer isn't empty
//and we haven't consumed 100 total numbers
//I have tested my code and it contains no errors for output.

#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>


int buf[10]; //int buffer
int nextPro; //spot in producer
int nextCon; //spot in consumer
int value; //our current value to produce/consume
int currProduced; //number of items we currently have in buf
int numofCons; //number of times we have consumed

pthread_mutex_t lockit=PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t signalit=PTHREAD_COND_INITIALIZER;

void produce(int thread_number) { 
  printf("I am the producer and my thread number is %i\n", 
	 thread_number);
  fflush(stdout);

  while (1){
    pthread_mutex_lock(&lockit); //holds mutex
    
    while(currProduced == 10){ //if the buffer is full 
      pthread_cond_wait(&signalit,&lockit);
    }

    if(value > 99){ //if we reach max value break
      pthread_mutex_unlock(&lockit);
      break;
    }
    
    printf("Producer %i is making value:  %i\n",thread_number,value);
    fflush(stdout);
    
    buf[nextPro%10] = value; //%10 wraps the buffer
    
    nextPro++; //increment buffer position
    value++; //increment data being put produced
    currProduced++; //increment number of produced 
    
    pthread_cond_broadcast(&signalit);
    pthread_mutex_unlock(&lockit); //releases mutex
  }
  printf("producer %i exiting\n",thread_number);
  fflush(stdout);
}//end of producer

void consume(int thread_number) { 
  printf("I am the consumer and my thread number is %i\n", 
	 thread_number);
  fflush(stdout);
  
  while(1){
    pthread_mutex_lock(&lockit); //grab ME for thread

    while(currProduced == 0){ //if theres nothing to consume
      if(numofCons > 99){ //and if we have reached max consumption
	break; //end while loop
      }
      pthread_cond_wait(&signalit,&lockit); //else wait till we can consume 
    }
   
    if(numofCons > 99){ //if we reach our max consume end the loop
      pthread_mutex_unlock(&lockit);
      break;
    }
    
    printf("Consumer %i is taking value:  %i\n",thread_number,buf[nextCon%10]);
    fflush(stdout);
    
    buf[nextCon%10] = -100; //%10 wraps the buffer
    nextCon++; //increment buffer position
    currProduced--; //decrease number of produced
    numofCons++; //increment numbers we have consumed.

    pthread_mutex_unlock(&lockit);
    pthread_cond_broadcast(&signalit);
  }
  printf("consumer: %i exiting\n",thread_number);
  fflush(stdout);
}

int main() {  //most of main taken from Dr. Bracken's example code
  void *statusp; 
  pthread_t *threads;
  //set all globals to 0 before working with producer/consumer
  nextPro, nextPro, value, currProduced, numofCons = 0; 
  
  if((threads=malloc(sizeof(pthread_t)*4))==NULL){ 
    printf("error allocating\n");
    fflush(stdout);
    exit(-1); 
  }

  //creates producer1 thread 0
  if((pthread_create(&threads[0],NULL,(void *)produce,(void *)0))!=0){ 
    printf("error creating producer 1\n");
    fflush(stdout);
    exit(-1); 
  }
  
  //creates producer2 thread 1
  if((pthread_create(&threads[1],NULL,(void *)produce,(void *)1))!=0){
    printf("error creating producer 2\n");
    fflush(stdout);
    exit(-1);
  }

  //creates consumer 1 thread 2
  if(pthread_create(&threads[2],NULL,(void *)consume,(void *)2)!=0) { 
    printf("error creating consumer\n");
    fflush(stdout);
    exit(-1);
  }

  //creates consumer 2 thread 3
  if(pthread_create(&threads[3],NULL,(void *)consume,(void *)3)!=0){
    printf("error creating consumer2\n");
    fflush(stdout);
    exit(-1);
  }
  //joins thread 0
  if((pthread_join(threads[0],&statusp))!=0) { 
    printf("couldn't join with producer1\n");
    fflush(stdout);
    exit(-1);
  }
  //joins thread 1
  if((pthread_join(threads[1],&statusp))!=0){
    printf("Couldn't join with producer2\n");
    fflush(stdout);
    exit(-1);
  }
  //joins thread 2
  if((pthread_join(threads[2],&statusp))!=0) { 
    printf("couldn't join with consumer\n");
    fflush(stdout);
    exit(-1);
  }
  //joins thread 3
  if((pthread_join(threads[3],&statusp))!=0){
    printf("coulnd't join with consumer2\n");
    fflush(stdout);
    exit(-1);
  }
  //deletes mutex
  if((pthread_mutex_destroy(&lockit))!=0) { 
    printf("mutex destroy failed\n");
    fflush(stdout);
    exit(-1);
  }
  //delete condition variable
  if((pthread_cond_destroy(&signalit))!=0) { 
    printf("cond destroy failed\n");
    fflush(stdout);
    exit(-1);
  }
  //exit
}//EOF

