//Pablo Martinez

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{

	
	int stat;
	if (argc!=2){
	  fprintf(stderr, "Usage: %s <command>\n", argv[0]);
	  exit(1);
	} else {
	  
	  pid_t child_pid = fork();
	  
	  if (child_pid == 0) {
	    
	    execlp("/bin/bash","/bin/bash", "-c", argv[1], NULL);
	    exit(-1);
	  }
	  while(wait(&stat) != -1);
	}

	return stat;

}

