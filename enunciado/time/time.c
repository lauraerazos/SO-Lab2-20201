#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {

  int rc = fork();
  struct timeval current_time;
  int tiempoInicial;
  int tiempoFinal;
  
  if (rc < 0) {
    // fork failed; exit
    fprintf(stderr, "fork failed\n");
    exit(1);

  } else if (rc == 0) {
    // child (new process)
    
    gettimeofday(&current_time, NULL);
    tiempoInicial = current_time.tv_usec;
    char *myargs[argc];
    
    for (int i=1; i<argc; i++){
        myargs[i-1] = strdup(argv[i]);
    }
    
    myargs[argc-1] = NULL;
    
    execvp(myargs[0], myargs);  // run

  } else {
    // parent goes down this path (original process)
    wait(NULL);
    gettimeofday(&current_time, NULL);
    tiempoFinal = current_time.tv_usec;

    printf("Tiempo: %d", tiempoFinal-tiempoInicial);
  }
  return 0;
}