// Imports
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int main() {
  int ShmID;
  int *ShmPTR;
  pid_t pid;
  int bankAcc = 0;
  
  sem_t* user_curr;
  if ((user_curr = sem_open("/user_curr", O_CREAT, 0644, 1)) == SEM_FAILED) {
      perror("Could not initialize/open semaphore for user_curr. Status: FAILED");
      exit(-1);
   }
  
  
  // Seed random for sleeping calls.
  srand(time(NULL)); 
  // Create ShmId for two integers. 
  ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
  
  if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
  }
  
  ShmPTR = (int *) shmat(ShmID, NULL, 0);
  if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
  }
  
  // Assign pointers for each of the integer variables.
  ShmPTR[0] = bankAcc;
  
  pid = fork();
  if (pid < 0) {
    printf("*** fork error (server) ***\n");
    exit(1);
  } else if (pid > 0) {
    while (1) {
      printf("Pid %d parent process wants in.\n", pid);
      fflush(stdout);
      sem_wait(user_curr);
    
      printf("Pid %d parent process is in.\n", pid);
      
      sleep(rand() % 6); // sleep for 0-5 seconds inclusive.
      int acc = ShmPTR[0];

      if (acc <= 100) {
        int balance = rand() % 101; // Randomly generate money to deposit between 0 and 100 inclusive.
        if ((balance % 2) == 0) {
          acc += balance;
          printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, acc);
          ShmPTR[0] = acc;
        } else {
          printf("Dear old Dad: Doesn't have any money to give\n");
        }
      } else {
        printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", acc);
      }
      printf("Pid %d parent process is out.\n",pid);
      sem_post(user_curr);
    }
    exit(0);
  } else {
    while (1) {
      printf("Pid %d child process wants in.\n",pid);
      fflush(stdout);
      sem_wait(user_curr);
    
      printf("Pid %d child user is in.\n",pid);
      
      sleep(rand() % 5);
      int acc = ShmPTR[0];
      
      int balance = rand() % 51; // generate a balance between 0 and 50 inclusive.
      printf("Poor Student needs $%d\n", balance);
      if (balance <= acc) {
        acc -= balance;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, acc);
        ShmPTR[0] = acc;
      } else {
        printf("Poor Student: Not Enough Cash ($%d)\n", acc );
      }
      printf("Child process with pid %d is out.\n",pid);
      sem_post(user_curr);
    }
    exit(0);
  }
}