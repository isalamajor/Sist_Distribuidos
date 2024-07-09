#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h> 
#include <linux/limits.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "mensaje.h"

#define MAX_CONNECTIONS 10
#define MAXSIZECHAR	256

char *directory_name = "tuplas";
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;



int crearDirectorio() {
  // Si el directorio para guardar los datos no existe, se crea

  if (access(directory_name, F_OK) == -1) {
    if (mkdir(directory_name, 0777) != 0) {
      perror("Error al crear el directorio\n");
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}


int vaciarDirectorio(const char *dirname) {

  DIR *dir;
  struct dirent *entry;
  char path[FILENAME_MAX];

  // Abrir el directorio
  dir = opendir(dirname);
  if (!dir) {
    perror("Error al abrir el directorio en vaciarDirectorio\n");
    return -1;;
  }

  // Recorrer cada entrada del directorio
  while ((entry = readdir(dir)) != NULL) {

    // Construir la ruta completa del archivo
    snprintf(path, FILENAME_MAX, "%s/%s", dirname, entry->d_name);

    // Ignorar los directorios "." y ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Eliminar el archivo
    if (unlink(path) != 0) {
      perror("Error al eliminar fichero del directorio\n");
      closedir(dir);
      return -1;
    }
  }

  // El directorio ya ha sido vaciado, lo cerramos
  closedir(dir);
  return 0;
}


int guardarDatosFichero(struct peticion p){
  
   //El nombre del fichero es la clave (key), pero hay que convertirla a string
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);

  char ruta[50] = "tuplas/";

  // Concatenar la clave convertida a string al directorio
  strcat(ruta, clave_str);

  pthread_mutex_lock(&file_mutex);

   // Crear fichero y abrirlo para escritura. 
  FILE *fichero = fopen(ruta, "wb");
  if (fichero == NULL) {
    perror("Error abriendo fichero en guardarDatosFichero\n");
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }
  // Guardar los datos de la estructura en el fichero y cerrarlo
  fwrite(&p, sizeof(struct peticion), 1, fichero);
  fclose(fichero);

  pthread_mutex_unlock(&file_mutex);
  
  return 0;
}


struct respuesta leerDatosFichero(struct peticion p){

  struct respuesta res;

  //Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);

  pthread_mutex_lock(&file_mutex);

  // Abrir el fichero
  FILE *fichero = fopen(ruta, "rb");
  if (fichero == NULL) {
    perror("Error al abrir el fichero en leerDatosFichero\n");
    res.todo_ok = -1;
    pthread_mutex_unlock(&file_mutex);
    return res;
  }

  // Leer los datos del fichero y guardarlos en la estructura petición, luego cerrarlo
  if (fread(&p, sizeof(struct peticion), 1, fichero) != 1){
    perror("Error al leer el fichero en LeerDatosFichero\n");
    fclose(fichero);
    res.todo_ok = -1;
    pthread_mutex_unlock(&file_mutex);
    return res;
  };
  fclose(fichero);

  pthread_mutex_unlock(&file_mutex);

  // Guardas datos en respuesta y devolver struct respuesta
  strcpy(res.value1, p.value1);
  res.N_value2 = p.N_value2;
  memcpy(res.V_value2, p.V_value2, p.N_value2 * sizeof(double));

  res.todo_ok = 0;

  return res;
}


int modificarValores(struct peticion p){

  // Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[50];
  snprintf(ruta, 50, "%s/%s", directory_name, clave_str);

  pthread_mutex_lock(&file_mutex);

  // Abrir el fichero
  FILE *fichero = fopen(ruta, "wb");
  if (fichero == NULL) {
    perror("Error al abrir el fichero no existe\n");
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }

  /* Escribir los nuevos datos en el fichero y guardarlos en la estructura. Esto reemplaza los antiguos */
  if (fwrite(&p, sizeof(struct peticion), 1, fichero) != 1){
    perror("Error al reescribir los datos en modificarValores\n");
    fclose(fichero);
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }; 

  fclose(fichero);
  
  pthread_mutex_unlock(&file_mutex);

  return 0;
  }


int eliminarFichero(struct peticion p){

  // Construir el  path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);
  
  pthread_mutex_lock(&file_mutex);

  // Devolver -1 si la clave no existe 
  if (access(ruta, F_OK) < 0) {
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }

  // Eliminar el fichero
  if (unlink(ruta) != 0){
    perror("Error al eliminar el fichero en eliminarFichero\n");
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }
  
  pthread_mutex_unlock(&file_mutex);

  return 0;
}


int existeFichero(struct peticion p){

  // Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);

  // Eliminar el fichero
  if (access(ruta, F_OK) < 0){
    return 0;
  }  
  return 1;
}


void *handle_client(void *arg) {

  /* Todo lo que recibamos lo guardamos en la struct para pasarselo a las funciones que ya teniamos hechas, después lo enviamos de vuelta elemento por elemento*/
  struct peticion mensaje;
  struct respuesta resultado;
  int client_socket = *((int *)arg);

  // Recepción de la petición del cliente
  if (recv(client_socket, &mensaje.op, 1, 0) < 0) {
    perror("Error al recibir el operador del cliente\n");
    close(client_socket);
    free(arg);
    pthread_exit(NULL);
  }
  if (recv(client_socket, &mensaje.clave, sizeof(int), 0) < 0) {
  perror("Error al recibir la clave del cliente\n");
  close(client_socket);
  free(arg);
  pthread_exit(NULL);
  }

  int long_valor1;
  if (strcmp(mensaje.op, "1") == 0 || strcmp(mensaje.op, "3") == 0){
    // Recibir el tamaño del valor1
    if (recv(client_socket, &long_valor1, sizeof(int), 0) < 0) {
      perror("Error al recibir el tamaño de value1 del cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
  
    // Recibir el valor1
    if (recv(client_socket, mensaje.value1, long_valor1, 0) < 0) {
      perror("Error al recibir value1 del cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
  
    // Recibir la cantidad de elementos en el arreglo V_value2
    if (recv(client_socket, &mensaje.N_value2, sizeof(int), 0) < 0) {
      perror("Error al recibir la cantidad de elementos en V_value2 del cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
  
    // Recibir el arreglo V_value2
    if (recv(client_socket, mensaje.V_value2, sizeof(double) * mensaje.N_value2, 0) < 0) {
      perror("Error al recibir V_value2 del cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
  }



  // Manejo de la petición
  switch (mensaje.op[0]) {
    case '0':
        resultado.todo_ok = vaciarDirectorio(directory_name);
        break;
    case '1':
        resultado.todo_ok = guardarDatosFichero(mensaje);
        break;
    case '2':
        resultado = leerDatosFichero(mensaje);
        break;
    case '3':
        resultado.todo_ok = modificarValores(mensaje);
        break;
    case '4':
        resultado.todo_ok = eliminarFichero(mensaje);
        break;
  pthread_mutex_lock(&file_mutex);
    case '5':
        resultado.todo_ok = existeFichero(mensaje);
        break;
    default:
        perror("Código de operación no válido\n");
        resultado.todo_ok = -1;
        break;
  }

  // Enviar el resultado al cliente
  if (send(client_socket, &resultado.todo_ok, sizeof(int), 0) < 0) {
    perror("Error al enviar la confirmación al cliente\n");
    close(client_socket);
    free(arg);
    pthread_exit(NULL);
  }

// Si se trata de un get_values, enviar todos los valores obtenidos
  if (strcmp(mensaje.op, "2") == 0){
    // Enviar value1 al cliente
    if (send(client_socket, resultado.value1, 256 * sizeof(char), 0) < 0) {
      perror("Error al enviar value1 al cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
    // Enviar N_value2 al cliente
    if (send(client_socket, &resultado.N_value2, sizeof(int), 0) < 0) {
      perror("Error al enviar N_value2 al cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
    // Enviar V_value2 al cliente
    if (send(client_socket, resultado.V_value2, 32 * sizeof(double), 0) < 0) {
      perror("Error al enviar V_value2 al cliente\n");
      close(client_socket);
      free(arg);
      pthread_exit(NULL);
    }
  }

  // Cierre del socket y liberación de recursos
  close(client_socket);
  pthread_exit(NULL);
}


        
    
int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Uso: %s <PUERTO>\n", argv[0]);
    return -1;
  }

  int puerto = atoi(argv[1]);
  if (puerto <= 0 || puerto > 65535) {
    printf("Puerto no válido: %s\n", argv[1]);
    return -1;
  }

  crearDirectorio(); // Crear directorio "tuplas" si no existe

  int server_socket, client_socket, addr_len;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);
  pthread_t thread_id;

  // Crear el socket del servidor
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("Error al crear el socket del servidor\n");
    exit(EXIT_FAILURE);
  }

  // Configurar la dirección del servidor
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(puerto);

  // Vincular el socket a la dirección del servidor
  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Error al vincular el socket a la dirección del servidor\n");
    exit(EXIT_FAILURE);
  }

  // Escuchar conexiones entrantes
  if (listen(server_socket, MAX_CONNECTIONS) < 0) {
    perror("Error al escuchar conexiones entrantes\n");
    exit(EXIT_FAILURE);
  }


  // Aceptar conexiones de clientes entrantes de manera indefinida
  while (1) {

    addr_len = sizeof(struct sockaddr_in);
    printf("Servidor en espera de conexiones...\n");
    // Aceptar la conexión entrante
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      perror("Error al aceptar la conexión entrante\n");
      exit(EXIT_FAILURE);
    }
    printf("Conexión establecida desde server\n");

    // Crear un hilo para manejar la petición del cliente
    //pthread_t client_thread;
    if (pthread_create(&thread_id, NULL, handle_client, &client_socket) != 0) {
      perror("Error en la creación del hilo \n");
      close(client_socket);
      exit(EXIT_FAILURE);
    }

    // Liberar recursos del hilo
    pthread_detach(thread_id); 
  }

  // Cerrar el socket del servidor
  close(server_socket);

  return 0;
}

