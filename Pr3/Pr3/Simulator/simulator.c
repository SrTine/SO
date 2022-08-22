//Pablo Martinez, Miguel Verdaguer
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define N_PARADAS 5 // numero de paradas de la ruta
#define EN_RUTA 0 // autobus en ruta
#define EN_PARADA 1 // autobus en la parada
#define MAX_USUARIOS 40 // capacidad del autobus
#define USUARIOS 4 // numero de usuarios

// Cerrojo
pthread_mutex_t cerrojo; 

// Variables condicionales
// Array de variables para usuarios
pthread_cond_t puedenSubBaj[N_PARADAS];
pthread_cond_t busEnParada;

// estado inicial
int estado = EN_RUTA;
int parada_actual = 0; // parada en la que se encuentra el autobus
int n_ocupantes = 0; // ocupantes que tiene el autobus

// personas que desean subir en cada parada
int esperando_parada[N_PARADAS]; //= {0,0,...0};

// personas que desean bajar en cada parada
int esperando_bajar[N_PARADAS]; //= {0,0,...0};

//Declaracion de funciones
void Autobus_En_Parada();
void Conducir_Hasta_Siguiente_Parada();
void Usuario(int id_usuario, int origen, int destino);
void Subir_Autobus(int id_usuario, int origen);
void Bajar_Autobus(int id_usuario, int destino);

// Otras definiciones globales (comunicacion y sincronizacion)
void * thread_autobus(void * args) {
  while (1) {
    // esperar a que los viajeros suban y bajen
    Autobus_En_Parada();
    // conducir hasta siguiente parada
    Conducir_Hasta_Siguiente_Parada();
  }
}

void * thread_usuario(void * arg) {
  int id_usuario = 0;
  // obtener el id del usario
  while (1) {
    int a=rand() % N_PARADAS;
    int b;
    do{
      b=rand() % N_PARADAS;
    } while(a==b);
    Usuario(id_usuario,a,b);
  }
}

void Usuario(int id_usuario, int origen, int destino) {
  // Esperar a que el autobus este en parada origen para subir
  Subir_Autobus(id_usuario, origen);
  // Bajarme en estacion destino
  Bajar_Autobus(id_usuario, destino);
}

int main(int argc, char *argv[]) {
  int i;
  // Definicion de variables locales a main
  pthread_t threads[USUARIOS+1]; //usuarios + autobus
  
  // Opcional: obtener de los argumentos del programa la capacidad del
  // autobus, el numero de usuarios y el numero de paradas
  
  pthread_mutex_init(&cerrojo, NULL);
  
  // Crear el thread Autobus
  pthread_create(&threads[0], NULL, thread_autobus, NULL);
  
  for (i = 1; i < USUARIOS+1; i++){
    // Crear thread para el usuario i
    pthread_create(&threads[i], NULL, thread_usuario, NULL);
  }
  
  for (i = 0; i < USUARIOS+1; i++){
    // Esperar terminacion de los hilos
    pthread_join(threads[i], NULL);
  }
  
  pthread_mutex_destroy(&cerrojo);
  
  return 0;
}

void Autobus_En_Parada(){
  /* Ajustar el estado y bloquear al autobus hasta que no haya pasajeros que
	quieran bajar y/o subir la parada actual. Despues se pone en marcha */
  pthread_mutex_lock(&cerrojo);
  
    printf("Parando el autobus\n");

  while(esperando_bajar[parada_actual] > 0 || esperando_parada[parada_actual] > 0){
    pthread_cond_wait(&busEnParada, &cerrojo);
  }
  
  pthread_mutex_unlock(&cerrojo);
}

void Conducir_Hasta_Siguiente_Parada(){
  /* Establecer un retardo que simule el trayecto y actualizar numero de parada */
  pthread_mutex_lock(&cerrojo);
  estado = EN_RUTA;

  pthread_mutex_unlock(&cerrojo);
  
  printf("Conduciendo a la siguiente parada\n");
  sleep(3);

  pthread_mutex_lock(&cerrojo);
  
  estado = EN_PARADA;
  parada_actual += 1;
  parada_actual %= N_PARADAS;
  pthread_cond_broadcast(&puedenSubBaj[parada_actual]);
  pthread_mutex_unlock(&cerrojo);
}

void Subir_Autobus(int id_usuario, int origen){
  /* El usuario indicara que quiere subir en la parada 'origen', esperara a que
     el autobus se pare en dicha parada y subira. El id_usuario puede utilizarse para
     proporcionar informacion de depuracion */
  pthread_mutex_lock(&cerrojo);
  esperando_parada[origen] += 1;

  while(parada_actual != origen && estado != EN_PARADA) 
    pthread_cond_wait(&puedenSubBaj[origen], &cerrojo);

    printf("Subiendo al autobus\n");
    
  n_ocupantes += 1;
  esperando_parada[parada_actual] -= 1;
  if(esperando_parada[parada_actual] == 0){
    pthread_cond_signal(&busEnParada);
  }
  pthread_mutex_unlock(&cerrojo);
}
void Bajar_Autobus(int id_usuario, int destino){
  /* El usuario indicara que quiere bajar en la parada 'destino', esperara a que
     el autobus se pare en dicha parada y bajara. El id_usuario puede utilizarse para
     proporcionar informacion de depuracion */
  pthread_mutex_lock(&cerrojo);
  esperando_bajar[destino] += 1;

  while(parada_actual != destino && estado != EN_PARADA)
    pthread_cond_wait(&puedenSubBaj[destino], &cerrojo);

  printf("Bajando del autobus\n");
  
  n_ocupantes -= 1;
  esperando_bajar[parada_actual] -= 1;
  if(esperando_bajar[parada_actual] == 0) {
    pthread_cond_signal(&busEnParada);
  }
  pthread_mutex_unlock(&cerrojo);
}


