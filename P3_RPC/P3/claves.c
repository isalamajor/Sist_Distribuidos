#include <stdio.h>
#include <string.h>
#include "claves.h"
#include "claves_rpc.h"

#define MAXSIZEVECTOR 32


// para obtener las variables de entorno
char *get_env_variable(char *env_var)
{
  char *value = getenv(env_var);
  if (value == NULL)
  {
    fprintf(stderr, "Error: %s variable de entorno no definida\n", env_var);
    exit(EXIT_FAILURE);
  }
  if (strcmp(value, "localhost") == 0)
  {
    value = "127.0.0.1";
  }
  return value;
}

int init()
{
  //obtenemos la dirección der servidor por variables de entorno
  char *host = get_env_variable("IP_TUPLAS");
  CLIENT *clnt;
  enum clnt_stat retval_1;
  int result_1;
  //creamos el cliente para la conexión tcp
  clnt = clnt_create(host, CLAVES_PROG, CLAVES_VERSION, "tcp");
  if (clnt == NULL)
  {
    clnt_pcreateerror(host);
    exit(1);
  }

  retval_1 = init_rpc_1(&result_1, clnt);
  if (retval_1 != RPC_SUCCESS)
  {
    clnt_perror(clnt, "call failed");
  }
  clnt_destroy(clnt);
  return result_1;
}

int set_value(int key, char *value1, int N_value2, double *V_value2)
{
  if (N_value2 < 1 || N_value2 > 32) {
    perror("N_value2 out of range\n");
    return -1;
  }
  char *host = get_env_variable("IP_TUPLAS");
  CLIENT *clnt;
  enum clnt_stat retval_2;
  int result_2;
  double_vector valor_2;
  clnt = clnt_create(host, CLAVES_PROG, CLAVES_VERSION, "tcp");
  if (clnt == NULL)
  {
    clnt_pcreateerror(host);
    exit(1);
  }
  // copiamos los parámetros los datos que vamos a enviar y reservamos memoria para el vector
  valor_2.double_vector_val = (double *)malloc(N_value2 * sizeof(double));
  memcpy(valor_2.double_vector_val, V_value2, N_value2 * sizeof(double));
  valor_2.double_vector_len = N_value2;

  // enviamos los datos
  retval_2 = set_value_rpc_1(key, value1, valor_2, &result_2, clnt);
  if (retval_2 != RPC_SUCCESS)
  {
    clnt_perror(clnt, "call failed");
  }
  clnt_destroy(clnt);
  // liberamos la memoria del malloc
  free(valor_2.double_vector_val);
  return result_2;
}

// Aquí es necesario guardar los datos extraidos en las variables pasadas por parámetros
int get_value(int key, char *value1, int *N_value2, double *V_value2)
{
  char *host = get_env_variable("IP_TUPLAS");
  CLIENT *clnt;
  enum clnt_stat retval_3;
  respuesta result_3;

  clnt = clnt_create(host, CLAVES_PROG, CLAVES_VERSION, "tcp");
  if (clnt == NULL)
  {
    clnt_pcreateerror(host);
    exit(1);
  }

  result_3.value1 = (char *)malloc(256);
  result_3.V_value2.double_vector_val = (double *)malloc(32 * sizeof(double));
  retval_3 = get_value_rpc_1(key, &result_3, clnt);

  if (retval_3 != RPC_SUCCESS)
  {
    clnt_perror(clnt, "call failed");
  }
  // guardamos los datos recibidos del servidor
  strcpy(value1, result_3.value1);
  *N_value2 = result_3.V_value2.double_vector_len;
  memcpy(V_value2, result_3.V_value2.double_vector_val , *N_value2 * sizeof(double));

  free(result_3.V_value2.double_vector_val);
  free(result_3.value1);
  clnt_destroy(clnt);
  return result_3.todo_ok;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2)
{
  if (N_value2 < 1 || N_value2 > 32) {
    perror("N_value2 out of range\n");
    return -1;
  }
  char *host = get_env_variable("IP_TUPLAS");
  CLIENT *clnt;
  enum clnt_stat retval_4;
  int result_4;
  double_vector valor_2;
  clnt = clnt_create(host, CLAVES_PROG, CLAVES_VERSION, "tcp");
  if (clnt == NULL)
  {
    clnt_pcreateerror(host);
    exit(1);
  }

  valor_2.double_vector_val = (double *)malloc(N_value2 * sizeof(double));
  if (valor_2.double_vector_val == NULL)
  {
    perror("Error: No se pudo asignar memoria para valor_2.double_vector_val\n");
    clnt_destroy(clnt);
    return -1;
  }

  valor_2.double_vector_len = N_value2;
  memcpy(valor_2.double_vector_val , V_value2, N_value2 * sizeof(double));

  retval_4 = modify_value_rpc_1(key, value1, valor_2, &result_4, clnt);
  if (retval_4 != RPC_SUCCESS)
  {
    clnt_perror(clnt, "call failed");
    free(valor_2.double_vector_val); // Liberar memoria asignada
    clnt_destroy(clnt);
    return -1; 
  }

  free(valor_2.double_vector_val); // Liberar memoria asignada
  clnt_destroy(clnt);
  return result_4;
}

int delete_key(int key)
{
  char *host = get_env_variable("IP_TUPLAS");
  CLIENT *clnt;
  enum clnt_stat retval_5;
  int result_5;
  clnt = clnt_create(host, CLAVES_PROG, CLAVES_VERSION, "tcp");
  if (clnt == NULL)
  {
    clnt_pcreateerror(host);
    exit(1);
  };
  retval_5 = delete_key_rpc_1(key, &result_5, clnt);
  if (retval_5 != RPC_SUCCESS)
  {
    clnt_perror(clnt, "call failed");
  }
  clnt_destroy(clnt);
  return result_5;
}

int exist(int key)
{
  char *host = get_env_variable("IP_TUPLAS");
  CLIENT *clnt;
  enum clnt_stat retval_6;
  int result_6;
  clnt = clnt_create(host, CLAVES_PROG, CLAVES_VERSION, "tcp");
  if (clnt == NULL)
  {
    clnt_pcreateerror(host);
    exit(1);
  };
  retval_6 = exist_rpc_1(key, &result_6, clnt);
  if (retval_6 != RPC_SUCCESS)
  {
    clnt_perror(clnt, "call failed");
  }
  clnt_destroy(clnt);
  return result_6;
}