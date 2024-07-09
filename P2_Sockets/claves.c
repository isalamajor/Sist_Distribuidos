#include "claves.h"
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "mensaje.h"
#define MAXSIZE 25
#define MAXSIZECHAR 256

// Función para obtener las variables de entorno
char *get_env_variable(const char *env_var) {
  char *value = getenv(env_var);
  if (value == NULL) {
      fprintf(stderr, "Error: %s variable de entorno no definida\n", env_var);
      exit(EXIT_FAILURE);
  }
  if (strcmp(value, "localhost") == 0){
      value = "127.0.0.1";
  }
  return value;
}

struct respuesta mandar_peticion(const char *op, int clave, char *value1, int N_value2, double V_value2[]) {
  
  struct respuesta res; // Resultado de la petición

  // Comprobamos la validez del formato los valores, que deben estar dentro de cierto rango 
  if (strcmp(op, "1") == 0 || strcmp(op, "3") == 0) {
    int result = strlen(value1) - 1;
      if (result > 255) {
        res.todo_ok = -1;
        perror("value1 no puede exceder de 255 carateres");
        return res;
      }

      if (N_value2 < 1) {
        perror("Numero de N_value2 no puede ser < 1");
        res.todo_ok = -1;
        return res;
      }
      if (N_value2 > 32) {
        perror("Numero de N_value2 no puede ser > 32");
        res.todo_ok = -1;
        return res;
      }
  }

  // Obtenemos el puerto y dirección ip del servidor mediante variables de entorno
  const char *ip_tuplas = get_env_variable("IP_TUPLAS");
  const char *port_tuplas = get_env_variable("PORT_TUPLAS");

  // Crear socket
  int sockfd; // File descriptor del socket
  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error al crear el socket en cliente\n");
    res.todo_ok = -1;
    return res;
  }

  // Estructura de dirección
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(port_tuplas)); // Puerto desde el cual el cliente se conecta al servidor

  if (inet_pton(AF_INET, ip_tuplas, &serv_addr.sin_addr.s_addr) <= 0) { 
    perror("Dirección IP del servidor no válida\n");
    close(sockfd);
    res.todo_ok = -1;
    return res;
  }

  // Conectar el cliente al socket
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Error conectando con el servidor desde el cliente\n");
    close(sockfd);
    res.todo_ok = -1;
    return res;
  }   

  // Enviar los datos al servidor
  if (send(sockfd, op, 1 , 0) < 0){
    perror("Error al enviar op code al servidor desde el cliente\n");
    close(sockfd);
    res.todo_ok = -1;
    return res;
  }
  
  if (send(sockfd, &clave, sizeof(int), 0) < 0 ) {
    perror("Error al enviar clave al servidor desde el cliente\n");
    close(sockfd);
    res.todo_ok = -1;
    return res;
  }
  
  // Enviar los valores a guardar para set_value y modify_value
  if (strcmp(op, "1") == 0 || strcmp(op, "3") == 0) {
    int s = strlen(value1);
    if (send(sockfd, &s, sizeof(int), 0) < 0) {
      perror("Error al enviar el tamaño de value1 desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
    if (send(sockfd, value1, strlen(value1), 0) < 0) {
      perror("Error al enviar value1 desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
    if (send(sockfd, &N_value2, sizeof(int), 0) < 0) {
      perror("Error al enviar N_value2 desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
    if ( send(sockfd, V_value2, sizeof(double) * N_value2, 0) < 0) {
      perror("Error al enviar V_value2 desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
  }

  // Recibir respuesta del servidor, recibir los valores para get_value (op "GET")
  if (recv(sockfd, &res.todo_ok, sizeof(int), 0) < 0){
    perror("Error al recibir la respuesta del servidor desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
  };
  if (strcmp(op, "2") == 0){
    if (recv(sockfd, &res.value1, sizeof(res.value1), 0) < 0) {
      perror("Error al recibir value1 del servidor desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
    if (recv(sockfd, &res.N_value2, sizeof(int), 0) < 0) {
      perror("Error al recibir N_value2 del servidor desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
    if (recv(sockfd, res.V_value2, sizeof(double) * res.N_value2, 0) < 0) {
      perror("Error al recibir V_value2 del servidor desde el cliente\n");
      close(sockfd);
      res.todo_ok = -1;
      return res;
    }
  }

  // Cerrar el socket
  close(sockfd);
  return res;
}


/* Para cada función, se crea un struct respuesta con la información necesaria correspondiente, y se llama a mandar_peticion,
  que devuelve un struct respuesta, de la que solo se devuelve el 0 o 1 de la variable todo_ok.*/
int init(){ 
  double V_Nulo[] = {0.0};
  struct respuesta r;
  r = mandar_peticion("0", 0, NULL, 0, V_Nulo); 
  return r.todo_ok;
}

int set_value(int key, char *value1, int N_value2, double *V_value2){
  struct respuesta r;
  r = mandar_peticion("1", key, value1, N_value2, V_value2);
  return r.todo_ok;
}

// Aquí es necesario guardar los datos extraidos en las variables pasadas por parámetros
int get_value(int key, char *value1, int *N_value2, double *V_value2){
  struct respuesta r;
  r = mandar_peticion("2", key, value1, *N_value2, V_value2);
  strcpy(value1, r.value1);
  *N_value2 = r.N_value2;
  for (int i = 0; i < r.N_value2; i++) { 
    V_value2[i] = r.V_value2[i];
  }
  return r.todo_ok;
}


int modify_value(int key, char *value1, int N_value2, double *V_value2){
  struct respuesta r;
  r = mandar_peticion("3", key, value1, N_value2, V_value2);
  return r.todo_ok;
}


int delete_key(int key){
  double V_Nulo[] = {0.0};
  struct respuesta r;
  r = mandar_peticion("4", key, NULL, 0, V_Nulo);
  return r.todo_ok;
}


int exist(int key){
  double V_Nulo[] = {0.0};
  struct respuesta r;
  r = mandar_peticion("5", key, NULL, 0, V_Nulo);
  return r.todo_ok;
}
