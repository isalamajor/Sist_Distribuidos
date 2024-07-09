#include "claves_rpc.h"
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

#define MAX_CONNECTIONS 10
#define MAXSIZECHAR 256

//hacemos un struct datos para guardar los datos que se pasan por RCP en el formato de nuestras funciones
struct datos
{
  char value1[256];
  int N_value2;
  double V_value2[32];
};

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


int vaciarDirectorio(const char *dirname)
{
  crearDirectorio();
  DIR *dir;
  struct dirent *entry;
  char path[FILENAME_MAX];

  // Abrir el directorio
  dir = opendir(dirname);
  if (!dir)
  {
    perror("Error al abrir el directorio en vaciarDirectorio\n");
    return -1;
    ;
  }

  // Recorrer cada entrada del directorio
  while ((entry = readdir(dir)) != NULL)
  {

    // Construir la ruta completa del archivo
    snprintf(path, FILENAME_MAX, "%s/%s", dirname, entry->d_name);

    // Ignorar los directorios "." y ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    // Eliminar el archivo
    if (unlink(path) != 0)
    {
      perror("Error al eliminar fichero del directorio\n");
      closedir(dir);
      return -1;
    }
  }

  // El directorio ya ha sido vaciado, lo cerramos
  closedir(dir);
  return 0;
}

int guardarDatosFichero(int key, char *value1, double_vector V_value2)
{

  struct datos elem;
  strcpy(elem.value1, value1);
  memcpy(elem.V_value2, V_value2.double_vector_val, V_value2.double_vector_len * sizeof(double));
  elem.N_value2 = V_value2.double_vector_len;

  // El nombre del fichero es la clave (key), pero hay que convertirla a string
  char clave_str[20];
  sprintf(clave_str, "%d", key);

  char ruta[50] = "tuplas/";

  // Concatenar la clave convertida a string al directorio
  strcat(ruta, clave_str);

  pthread_mutex_lock(&file_mutex);

  // Crear fichero y abrirlo para escritura.
  FILE *fichero = fopen(ruta, "wb");
  if (fichero == NULL)
  {
    perror("Error abriendo fichero en guardarDatosFichero\n");
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }
  // Guardar los datos de la estructura en el fichero y cerrarlo
  fwrite(&elem, sizeof(struct datos), 1, fichero);
  fclose(fichero);

  pthread_mutex_unlock(&file_mutex);
  return 0;
}

struct respuesta leerDatosFichero(int key)
{

  struct respuesta res;
  // Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", key);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);

  pthread_mutex_lock(&file_mutex);

  // Verificar si el archivo existe
  if (access(ruta, F_OK) < 0)
  {
    perror("El archivo no existe\n");
    res.todo_ok = -1;
    pthread_mutex_unlock(&file_mutex);
    return res;
  }

  // Abrir el fichero
  FILE *fichero = fopen(ruta, "rb");
  if (fichero == NULL)
  {
    perror("Error al abrir el fichero en leerDatosFichero\n");
    res.todo_ok = -1;
    pthread_mutex_unlock(&file_mutex);
    return res;
  }
  struct datos elem;
  // Leer los datos del fichero y guardarlos en la estructura datos, luego cerrarlo
  if (fread(&elem, sizeof(struct datos), 1, fichero) != 1)
  {
    perror("Error al leer el fichero en LeerDatosFichero\n");
    fclose(fichero);
    res.todo_ok = -1;
    pthread_mutex_unlock(&file_mutex);
    return res;
  };
  fclose(fichero);

  pthread_mutex_unlock(&file_mutex);

  // Copiar los datos le�dos en la estructura de respuesta
  res.todo_ok = 0;
  res.value1 = strdup(elem.value1);
  res.V_value2.double_vector_val = malloc(elem.N_value2 * sizeof(double));
  if (res.V_value2.double_vector_val == NULL)
  {
    perror("Error al asignar memoria para V_value2.double_vector_val\n");
    // Liberar memoria de value1 si se asign� antes
    free(res.value1);
    res.todo_ok = -1;
    return res;
  }
  memcpy(res.V_value2.double_vector_val, elem.V_value2, elem.N_value2 * sizeof(double));
  res.V_value2.double_vector_len = elem.N_value2;

  return res;
}

int modificarValores(int key, char *value1, double_vector V_value2)
{

  struct datos elem;
  strcpy(elem.value1, value1);
  memcpy(elem.V_value2, V_value2.double_vector_val, V_value2.double_vector_len * sizeof(double));
  elem.N_value2 = V_value2.double_vector_len;

  // Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", key);
  char ruta[50];
  snprintf(ruta, 50, "%s/%s", directory_name, clave_str);

  pthread_mutex_lock(&file_mutex);

  // Abrir el fichero
  FILE *fichero = fopen(ruta, "wb");
  if (fichero == NULL)
  {
    perror("Error al abrir el fichero no existe\n");
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }

  // Buscar la posici�n del archivo
  if (fseek(fichero, 0, SEEK_SET) != 0) {
      perror("Error al buscar la posici�n del archivo\n");
      fclose(fichero);
      pthread_mutex_unlock(&file_mutex);
      return -1;
  }

  // Escribir los nuevos datos en el fichero y guardarlos en la estructura. Esto reemplaza los antiguos 
  if (fwrite(&elem, sizeof(elem), 1, fichero) != 1)
  {
    perror("Error al reescribir los datos en modificarValores\n");
    fclose(fichero);
    pthread_mutex_unlock(&file_mutex);
    return -1;
  };

  fclose(fichero);

  pthread_mutex_unlock(&file_mutex);

  return 0;
}

int eliminarFichero(int key)
{

  // Construir el  path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", key);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);

  pthread_mutex_lock(&file_mutex);

  // Devolver -1 si la clave no existe
  if (access(ruta, F_OK) < 0)
  {
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }

  // Eliminar el fichero
  if (unlink(ruta) != 0)
  {
    perror("Error al eliminar el fichero en eliminarFichero\n");
    pthread_mutex_unlock(&file_mutex);
    return -1;
  }

  pthread_mutex_unlock(&file_mutex);

  return 0;
}

int existeFichero(int key)
{

  // Construir el path del fichero a leer
  char clave_str[20];
  sprintf(clave_str, "%d", key);
  char ruta[PATH_MAX];
  snprintf(ruta, PATH_MAX, "%s/%s", directory_name, clave_str);

  // Eliminar el fichero
  if (access(ruta, F_OK) < 0)
  {
    return 0;
  }
  return 1;
}

bool_t
init_rpc_1_svc(int *result, struct svc_req *rqstp)
{
  bool_t retval = TRUE;

  *result = vaciarDirectorio(directory_name);

  return retval;
}

bool_t
set_value_rpc_1_svc(int key, char *value1, double_vector V_value2, int *result, struct svc_req *rqstp)
{
  bool_t retval = TRUE;

  *result = guardarDatosFichero(key, value1, V_value2);

  return retval;
}

bool_t
get_value_rpc_1_svc(int key, respuesta *result, struct svc_req *rqstp)
{
  bool_t retval = TRUE;
  printf("%d", key);
  *result = leerDatosFichero(key);

  return retval;
}

bool_t
modify_value_rpc_1_svc(int key, char *value1, double_vector V_value2, int *result, struct svc_req *rqstp)
{
  bool_t retval = TRUE;

  *result = modificarValores(key, value1, V_value2);

  return retval;
}

bool_t
delete_key_rpc_1_svc(int key, int *result, struct svc_req *rqstp)
{
  bool_t retval = TRUE;

  *result = eliminarFichero(key);

  return retval;
}

bool_t
exist_rpc_1_svc(int key, int *result, struct svc_req *rqstp)
{
  bool_t retval = TRUE;

  *result = existeFichero(key);

  return retval;
}

int claves_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
  xdr_free(xdr_result, result);

  /*
   * Insert additional freeing code here, if needed
   */

  return 1;
}