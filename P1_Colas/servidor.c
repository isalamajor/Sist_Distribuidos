#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "mensaje.h"
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h> 
#include <linux/limits.h>

char *directory_name = "tuplas";

// Vamos a necesitar mutex para que no haya concurrencia al copiar el mensaje
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
mqd_t  q_servidor;


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


   // Crear fichero y abrirlo para escritura. 
  FILE *fichero = fopen(ruta, "wb");
  if (fichero == NULL) {
    perror("Error abriendo fichero en guardarDatosFichero\n");
    return -1;
  }
  // Guardar los datos de la estructura en el fichero y cerrarlo
  fwrite(&p, sizeof(struct peticion), 1, fichero);
  fclose(fichero);
  return 0;
}


void copiarPeticionARespuesta(struct peticion p, struct respuesta *r) {
    
    // Función auxiliar a leerDatosFichero, copias los valores en común que necesitamos
    strcpy(r->value1, p.value1);
    r->N_value2 = p.N_value2;
    memcpy(r->V_value2, p.V_value2, p.N_value2 * sizeof(double));
}


struct respuesta leerDatosFichero(struct peticion p){

  struct respuesta res;

  //Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);


  // Abrir el fichero
  FILE *fichero = fopen(ruta, "rb");
  if (fichero == NULL) {
    perror("Error al abrir el fichero en leerDatosFichero\n");
    res.todo_ok = -1;
    return res;
  }

  // Leer los datos del fichero y guardarlos en la estructura petición, luego cerrarlo
  if (fread(&p, sizeof(struct peticion), 1, fichero) != 1){
    perror("Error al leer el fichero en LeerDatosFichero\n");
    fclose(fichero);
    res.todo_ok = -1;
    return res;
  };
  fclose(fichero);

  // Guardas datos en respuesta y devolver struct respuesta
  copiarPeticionARespuesta(p, &res);
  res.todo_ok = 0;
  return res;
}


int modificarValores(struct peticion p){

  // Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[50];
  snprintf(ruta, 50, "%s/%s", directory_name, clave_str);

  // Abrir el fichero
  FILE *fichero = fopen(ruta, "wb");
  if (fichero == NULL) {
    perror("Error al abrir el fichero no existe\n");
    return -1;
  }

  /* Escribir los nuevos datos en el fichero y guardarlos en la estructura.
   Esto debería reemplazar los antiguos */
  if (fwrite(&p, sizeof(struct peticion), 1, fichero) != 1){
    perror("Error al reescribir los datos en modificarValores\n");
    fclose(fichero);
    return -1;
  }; 

  fclose(fichero);
  return 0;
  }


int eliminarFichero(struct peticion p){
  
  // Construir el  path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", p.clave);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);
   
  // Devolver -1 si la clave no existe 
  if (access(ruta, F_OK) < 0) {
    return -1;
  }
         
  // Eliminar el fichero
  if (unlink(ruta) != 0){
    perror("Error al eliminar el fichero en eliminarFichero\n");
    return -1;
  }

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



void tratar_mensaje(void  *mess){
  
  struct peticion mensaje;	
  mqd_t q_cliente;		
  struct respuesta resultado;		

  // Copiar el mensaje a la estructura local
  pthread_mutex_lock(&mutex_mensaje);

  mensaje = (*(struct peticion *) mess);
  mensaje_no_copiado = false; // Despertar al servidor
  pthread_cond_signal(&cond_mensaje);

  pthread_mutex_unlock(&mutex_mensaje);

  // Ejecutar petición
  switch (mensaje.op) {
    case 0:
        resultado.todo_ok = vaciarDirectorio(directory_name);
        break;
    case 1:
        resultado.todo_ok = guardarDatosFichero(mensaje);
        break;
    case 2:
        resultado = leerDatosFichero(mensaje);
        break;
    case 3:
        resultado.todo_ok = modificarValores(mensaje);
        break;
    case 4:
        resultado.todo_ok = eliminarFichero(mensaje);
        break;
    case 5:
        resultado.todo_ok = existeFichero(mensaje);
        break;
    default:
        perror("Código de operación no válido\n");
        resultado.todo_ok = -1;
        break;
  }

  // Abrir cola cliente 
  if ((q_cliente = mq_open(mensaje.q_name, O_WRONLY)) == -1){
    perror("Error al abrir la cola del cliente\n");
    mq_close(q_servidor);
    mq_unlink("/Cola_Servidor");
  }
  
  // Enviar el resultado
  if (mq_send(q_cliente, (char *)&resultado, sizeof(struct respuesta), 0) <0) {
    perror("Error en el mq_send del servidor al cliente");
    mq_close(q_servidor);
    mq_unlink("/Cola_Servidor");
    mq_close(q_cliente);
    pthread_exit(NULL);
  }

  mq_close(q_cliente);
  pthread_exit(NULL);
}


int main(void) {
  crearDirectorio(); // Crear directorio "tuplas" si no existe
  struct peticion mess;     
  struct mq_attr attr;
  pthread_attr_t t_attr;
  pthread_t thid;


  attr.mq_maxmsg = 4;  
  attr.mq_msgsize = sizeof(struct peticion);
  attr.mq_flags = 0;

  // Crear la cola servidor
  if ((q_servidor = mq_open("/Cola_Servidor", O_CREAT|O_RDONLY, 0700, &attr)) == -1) {
    perror("Error en el mq_open de la cola servidor\n");
    return -1;
  }

  pthread_mutex_init(&mutex_mensaje, NULL);
  pthread_cond_init(&cond_mensaje, NULL);
  pthread_attr_init(&t_attr);

  // Atributos de los threads, threads independientes
  pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

  while(1) {

    // Esperar a recibir mensaje del cliente
    if (mq_receive(q_servidor, (char *) &mess, sizeof(mess), 0) < 0 ){
      perror("Error en el mq_receive del servidor");
      return -1;
    }
    
    // Ya recibido el mensaje, creamos un hilo
    if (pthread_create(&thid, &t_attr, (void *)tratar_mensaje, (void *)&mess) != 0) {
      perror("Error al crear el hilo");
      break; 
    }
    else {
      // Esperar a que el hilo actual copie el mensaje
      pthread_mutex_lock(&mutex_mensaje);
      while (mensaje_no_copiado){
        pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
      }

      mensaje_no_copiado = true;     
      pthread_mutex_unlock(&mutex_mensaje);
    }
  }

  // Cerrar cola servidor y destuir los recursos mutex
  mq_close(q_servidor);
  mq_unlink("/Cola_Servidor");
  pthread_mutex_destroy(&mutex_mensaje);
  pthread_cond_destroy(&cond_mensaje);
  return 0;
}