#include "claves.h"
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "mensaje.h"
#define MAXSIZE 25


struct respuesta mandar_peticion(int op, int clave, char * value1, int N_value2, double V_value2[]) {
  
  

  // Definir colas del servidor y del cliente
  mqd_t q_servidor;       
  mqd_t q_cliente;        

  struct peticion pet; // Petición
  struct respuesta res; // Resultado de la petición
  struct mq_attr attr; // Atributos de la cola
  char queuename[MAXSIZE]; // Nombre de la cola cliente, porque habrá varias


  // El máximo de mensajes en la cola cliente es 1, del tamaño del struct petición
  attr.mq_maxmsg = 1;     
  attr.mq_msgsize = sizeof(struct respuesta); // En la cola cliente recibimos una respuesta


  

  // Abrir cola del cliente
  sprintf(queuename, "/Cola-%d", getpid());
  if ((q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr)) == -1) {
    perror("Error en mq_open 1 del cliente\n");
    res.todo_ok = -1;

    // Cerrar colas
    mq_close(q_servidor); 
    mq_close(q_cliente); 
    if (mq_unlink(queuename) == -1) {
      perror("Error en el mq_unlink 3 del cliente\n");
    }
    return res;
  }

  // Abrir cola del servidor
  if ((q_servidor = mq_open("/Cola_Servidor", O_WRONLY)) == -1){
    perror("Error en el mq_open 2 del cliente\n");
    res.todo_ok = -1;

    mq_close(q_servidor); 
    mq_close(q_cliente); 
    if (mq_unlink(queuename) == -1) {
      perror("Error en el mq_unlink 3 del cliente\n");
    }
    return res;
  }
  
  // Comprobamos que el rango de los parametros para set_value y modify_value
  if (op == 1 || op == 3) {
    int result = strlen(value1) - 1;
    if (result > 255) {
      perror("Value1 fuera de rango\n");
      res.todo_ok = -1;

      mq_close(q_servidor); 
      mq_close(q_cliente); 
      if (mq_unlink(queuename) == -1) {
        perror("Error en el mq_unlink 3 del cliente\n");
      }
      return res;
    }
  
    if (N_value2 < 1 || N_value2 > 32) {
      perror("N_value2 fuera de rango\n");
      res.todo_ok = -1;

      mq_close(q_servidor); 
      mq_close(q_cliente); 
      if (mq_unlink(queuename) == -1) {
        perror("Error en el mq_unlink 3 del cliente\n");
      }
      return res;
    }
  }
  

  // Crear petición, añadiendo los parámetros al struct, y el nombre de su cola cliente
  if (value1 == NULL) {
    pet.value1[0] = '\0'; 
  } else {
    strcpy(pet.value1, value1);
  }

  pet.N_value2 = N_value2;
  for (int i = 0; i < pet.N_value2; i++) { 
    pet.V_value2[i] = V_value2[i];
  }

  strcpy(pet.q_name, queuename);

  pet.op = op; 
  pet.clave = clave;

  // Enviar petición al servidor
  if (mq_send(q_servidor, (const char *)&pet, sizeof(pet), 0) < 0){
    perror("Error del el mq_send del cliente\n");
    mq_close(q_servidor);
    mq_close(q_cliente);
    if (mq_unlink(queuename) == -1) {
        perror("Error en el mq_unlink 1 del cliente\n");
    }
    res.todo_ok = -1;
    return res;
  }	

  // Recibir respuesta del servidor
  if (mq_receive(q_cliente, (char *) &res, sizeof(struct respuesta), 0) < 0){
    perror("Error en el mq_recv del cliente\n");
    mq_close(q_servidor);
    mq_close(q_cliente);
    if (mq_unlink(queuename) == -1) {
      perror("Error en el mq_unlink2 del cliente\n");
    }
    res.todo_ok = -1;
    return res;
  }	

  // Cerrar colas
  mq_close(q_servidor); 
  mq_close(q_cliente); 
  if (mq_unlink(queuename) == -1) {
    perror("Error en el mq_unlink 3 del cliente");
  }
  return res;

}


/* Para cada función, se crea un struct respuesta con la información necesaria 
correspondiente, y se llama a mandar_peticion, que devuelve un struct respuesta,
de la que solo se devuelve el 0 o 1 de la variable todo_ok.*/
int init(){ 
  double V_Nulo[] = {0.0};
  struct respuesta r;
  r = mandar_peticion(0, 0, NULL, 0, V_Nulo); 
  return r.todo_ok;

}

int set_value(int key, char *value1, int N_value2, double *V_value2){
  struct respuesta r;
  r = mandar_peticion(1, key, value1, N_value2, V_value2);
  return r.todo_ok;
}

// Aquí es necesario guardar los datos extraidos en las variables pasadas por parámetros
int get_value(int key, char *value1, int *N_value2, double *V_value2){
  struct respuesta r;
  r = mandar_peticion(2, key, value1, *N_value2, V_value2);
  strcpy(value1, r.value1);
  *N_value2 = r.N_value2;
  for (int i = 0; i < r.N_value2; i++) { 
    V_value2[i] = r.V_value2[i];
  }
  return r.todo_ok;
}


int modify_value(int key, char *value1, int N_value2, double *V_value2){
  struct respuesta r;
  r = mandar_peticion(3, key, value1, N_value2, V_value2);
  return r.todo_ok;
}


int delete_key(int key){
  double V_Nulo[] = {0.0};
  struct respuesta r;
  r = mandar_peticion(4, key, NULL, 0, V_Nulo);
  return r.todo_ok;
}


int exist(int key){
  double V_Nulo[] = {0.0};
  struct respuesta r;
  r = mandar_peticion(5, key, NULL, 0, V_Nulo);
  return r.todo_ok;
}
