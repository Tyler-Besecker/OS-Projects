//File: wrestler.c
//Autor: Tyler Besecker
//Date: 10/22/2017
//Program Arm Wrestler

#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <wait.h>

#define SEMPERM 0600 //permissions for semaphores
#define SHMPERM 0600 //permissions for shared_memory
#define max_matches 20
#define max_waiting 3
#define numForks 5

//used for shared memory
#define PATRONMONEY 0
#define WRESTLERMONEY 1
#define MATCHES 2
#define CLOSEDSTATUS 3
#define WAITING 4

int *buf;
int sleepW, wrestler, mutex, sharedMem; //semaphores need to be global
//so all functions can access them.
int statloc;

void Wrestler();
void patron();
void checkSig(int semChk);
void checkWait(int semChk);

int waitt(int semid){ //wait code taken from Dr. Bracken's website example
  struct sembuf p_buf;
  p_buf.sem_num = 0;
  p_buf.sem_op = -1;
  p_buf.sem_flg = 0;

  return semop(semid,&p_buf,1); //used to check system calls
} 

int signall(int semid){ //signal code taken from Dr. Bracken's website example

  struct sembuf v_buf;
  v_buf.sem_num = 0;
  v_buf.sem_op = 1;
  v_buf.sem_flg = 0;

  return semop(semid,&v_buf,1); //used to check system calls
}

int main(){
  int pM;
  int forkValue; //used to see if in child or parent
  int id1, id2, id3, id4, id5; //5 fork ids

  sleepW = semget(IPC_PRIVATE,2,SEMPERM|IPC_CREAT|IPC_EXCL);
  if(sleepW < 0){
    printf("Error on admit semaphore\n");
    fflush(stdout);
    exit(-1);
  }
  
  wrestler = semget(IPC_PRIVATE,2,SEMPERM|IPC_CREAT|IPC_EXCL);
  if(wrestler < 0){
    printf("Error on wrestler semaphore\n");
    fflush(stdout);
    exit(-1);
  }
    
  mutex = semget(IPC_PRIVATE,2,SEMPERM|IPC_CREAT|IPC_EXCL);
  if(mutex < 0){
    printf("Error on mutex semaphore\n");
    fflush(stdout);
    exit(-1);
  }
     
  sharedMem = semget(IPC_PRIVATE,2,SEMPERM|IPC_CREAT|IPC_EXCL);
  if(sharedMem < 0){
    printf("Error on sharedMem semaphore\n");
    fflush(stdout);
    exit(-1);
  }
      
  union {
    int val;
    struct semid_ds *buffer;
    ushort *array;
  }semnum, semnum2;

  semnum.val = 0;
  semnum2.val = 1;
 
  if(semctl(sleepW,0,SETVAL,semnum) < 0){
    printf("Error on semctl sleepW\n");
    fflush(stdout);
    exit(-1);
  }
  
  if(semctl(wrestler,0,SETVAL, semnum) < 0){
    printf("Error on semctl wrestler\n");
    fflush(stdout);
    exit(-1);
  }
   
  if(semctl(mutex,0,SETVAL, semnum) < 0){
    printf("Error on semctl mutex\n");
    fflush(stdout);
    exit(-1);
  }
  
  if(semctl(sharedMem,0,SETVAL,semnum2) < 0){
    printf("Error on semctl sharedMem\n");
    fflush(stdout);
    exit(-1);
  }
  
  srand(getpid()); //seed the #generator
  
  //obtain shared memory
  pM = shmget(IPC_PRIVATE,sizeof(int)*10,IPC_CREAT|IPC_EXCL|SHMPERM);
  if(pM < 0){
    printf("Error on shared memory\n");
    fflush(stdout);
    exit(-1);
  }
  
  
  forkValue = fork(); //creates child and parent

  buf = (int *)shmat(pM,(int*)0,0);

  if(forkValue < 0){
    return(-1);
  } // Failed fork

  if(forkValue == 0){//wrestler/child
    Wrestler();
  } //end of wrestler
  
  else{ //check every fork
    id1 = fork();
    if(id1 < 0){
      return(-1);
    } // Failed fork

    if(id1 == 0){
      patron();
    }
    else{
      id2 = fork();
      if(id2 < 0){
	return(-1);
      } // Failed fork
      if(id2 == 0){
      	patron();
      }
      else{
	id3 = fork();
	if(id3 < 0){
	  return(-1);
	} // Failed fork
	if(id3 == 0){
	  patron();
	}
	else{
	  id4 = fork();
	  if(id4 < 0){
	    return(-1);
	  } // Failed fork
	  if(id4 == 0){
	    patron();
	  }
	  else{
	    if(id5 < 0){
	      return(-1);
	    } // Failed fork
	    id5 = fork();
	    if(id5 == 0){
	      patron();
	    }
	  }
	}
      }
    }
  }
  //wait for all processes to end  
  wait(&id1);
  wait(&id2);
  wait(&id3);
  wait(&id4);
  wait(&id5);
  wait(&statloc);

  checkWait(sharedMem);
  printf("Patron's Winnings: %d\n",buf[PATRONMONEY]);
  fflush(stdout);
  checkSig(sharedMem);

  checkWait(sharedMem);
  printf("Wrestler's Winnings: %d\n",buf[WRESTLERMONEY]);
  fflush(stdout);
  checkSig(sharedMem);
  
  //returns shared memory and semaphores 
  if(shmctl(pM,IPC_RMID,NULL) < 0){
    printf("Failed at returning shared memory.\n");
    fflush(stdout);
    exit(-1);
  }

  if(semctl(sleepW,0,IPC_RMID,NULL) < 0){
    printf("Failed at returning sleepW semaphore\n");
    fflush(stdout);
    exit(-1);
  }
  if(semctl(wrestler,0,IPC_RMID,NULL) < 0){
    printf("Failed at returning wrestler semaphore\n");
    fflush(stdout);
    exit(-1);
  }
  if(semctl(mutex,0,IPC_RMID,NULL) < 0){
    printf("Failed at returning mutex semaphore\n");
    fflush(stdout);
    exit(-1);
  }
  if(semctl(sharedMem,0,IPC_RMID,NULL) < 0){
    printf("Failed at returning sharedMem semaphore\n");
    fflush(stdout);
    exit(-1);
  }
  
}

void Wrestler(){
  printf("I am the WRESTLER!! My ID is: %i\n",getpid());
  fflush(stdout);
  
  while(1){
    checkWait(sharedMem); //Mutual Exclusion for sharedMem
    if(buf[MATCHES] == max_matches){
      checkSig(wrestler);
      checkSig(sharedMem); //releases ME for SM
      exit(0);
    }
    
    checkSig(sharedMem); //releases ME for SM
    checkWait(sleepW); //goes to sleep if no one in room
    
    checkWait(sharedMem); //waits for ME for SM
    buf[WAITING] = buf[WAITING] - 1; //take someone out of the waiting room
    checkSig(sharedMem); //Releases SM
    
    checkSig(wrestler); //wrestler is now ready to fight
    checkWait(mutex); //used for ME
    
    printf("Wrestler: %i Is fighting!!\n", getpid());
    fflush(stdout);
    
    if(rand() % 4 == 0){ 
      printf("Patron wins!\n");
      fflush(stdout);
      checkWait(sharedMem);
      printf("Match Number: %d \n\n", buf[MATCHES] + 1);
      fflush(stdout);
      buf[PATRONMONEY] = buf[PATRONMONEY] + 25;
      checkSig(sharedMem);
      //add 10 to patron money
    }
    else{
      printf("wrestler wins!\n");
      fflush( stdout );
      checkWait(sharedMem);
      printf("Match Number: %d \n\n", buf[MATCHES] + 1);
      fflush(stdout);
      buf[WRESTLERMONEY] = buf[WRESTLERMONEY] + 10;
      checkSig(sharedMem);
      //add 25 to wrestler money
    }
    
    checkWait(sharedMem); //waits for ME for SM
    buf[MATCHES] = buf[MATCHES] + 1; //increment matches
    checkSig(sharedMem); //Releases SM
  
    checkWait(sharedMem);
    if(buf[MATCHES] == max_matches){
      buf[CLOSEDSTATUS] = 1;
      checkSig(sharedMem);

      checkWait(sharedMem);
      while(buf[WAITING] > 0){
	buf[WAITING] = buf[WAITING] - 1;
	checkSig(sharedMem);
      }
    }
    else{
      checkSig(sharedMem);
    }
  }
}

void patron(){
  printf("I am a patron! My ID is: %i\n",getpid());
  fflush(stdout);
  
  while(1){
    
    checkWait(sharedMem); //wait for ME for SM
    if(buf[CLOSEDSTATUS] == 1){
      checkSig(sharedMem); //Release SM
      exit(0);
   
    }

    if(buf[WAITING] < max_waiting){ //waiting room
      buf[WAITING] = buf[WAITING] + 1; //increase num of waiting
      checkSig(sharedMem); //release SM
      checkSig(sleepW); //wake wrestler

      checkWait(wrestler); //waits for the wrestler
      if(buf[MATCHES] == max_matches){ //checks again to see if wrestler is done
	checkSig(wrestler);
      	exit(0);
      }
      else{
	checkSig(sharedMem); //if we skip the if we need to release shared mem
      }
      
      printf("Patron: %i Is waiting!\n", getpid());
      fflush(stdout);
             
      checkSig(mutex); //used to sync with wrestler
      printf("Patron: %i Is Fighting!\n", getpid());
      fflush(stdout);
    }
    else{
      checkSig(sharedMem);
    }
  }
}

void checkWait(int semChk){

  if(waitt(semChk) < 0){
    printf("wait failed.\n");
    fflush(stdout);
    exit(-1);
  }
  else{
    //printf("wait worked.\n");
    //fflush(stdout);
  }
    
}

void checkSig(int semChk){

  if(signall(semChk) < 0){
    printf("Signal failed.\n");
    fflush(stdout);
    exit(-1);
  }
  else{
    //printf("signal worked.\n");
    //fflush(stdout);
  }
  
}
