//Pablo Martinez

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

sem_t sumasParciales; //numero de hilos sumando
int total_sum = 0;

typedef struct{
  pthread_t thread_id;
  int ini;
  int last;
} typeThread;//estructura para guardar hilo con su inicio y final de numeros a sumar

void * partial_sum(void * arg) {

  typeThread *data = (typeThread*)arg;

  int j = 0;
  int ni= data->ini;
  int nf= data->last;
  int partial_sum = 0;

  for (j = ni; j <= nf; j++)
    partial_sum = partial_sum + j;

  //entrada seccion critica
  sem_wait(&sumasParciales);
  total_sum = total_sum + partial_sum;
  sem_post(&sumasParciales);
  //salida seccion critica

  pthread_exit(0);
}

int main(int argc, char** argv) {

	if(argc != 3){
    		printf("Invalid arguments.\n");
    		exit(-1);
  	}
	
	sem_init(&total_sum,0,1);
	
  	int numThreads = atoi(argv[1]);//atoi convierte un char en numero
  	int lastNum = atoi(argv[2]);
	int elemsPorThread = lastNum / numThreads;
	
	typeThread ths[numThreads];

	for(int i = 0; i < numThreads; i++){
    		ths[i].ini = (i * elemsPorThread) + 1;
    		if(numThreads == i + 1)
      			ths[i].last = lastNum;
   		else
      			ths[i].last = (i * elemsPorThread) + elemsPorThread; 
  	}


	/* Threads are created */
 	for(int i = 0; i < numThreads; i++){
    		pthread_create(&ths[i].thread_id, NULL, partial_sum,(void*)(threads + i));
	}

	/* Threads are created */
 	for(int i = 0; i < numThreads; i++){
    		pthread_join(&ths[i].thread_id, NULL);
	}
 
	sem_destroy(&sumasParciales);


  	printf("total_sum = %d\n", total_sum);

  	return 0;
}
