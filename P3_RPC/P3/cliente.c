#include <stdio.h>
#include "claves.h"

#define MAXSIZEVECTOR 32

int main()
{
  char valor1[256];
  int N_valor2;
  double V_valor2[MAXSIZEVECTOR];

  // INIT TEST
  if (init() < 0)
  {
    printf("Error en el init\n");
    return -1;
  };

  printf("Init OK.\n");

  // SET_VALUE TEST
  double initial_V_valor2[] = {1.0, 2.0, 3.0};
  if (set_value(1000, "Hola", 3, initial_V_valor2) < 0)
  {
    printf("Error en el set_value\n");
    return -1;
  };

  printf("Set_value OK.\n");

  // GET_VALUE TEST
  if (get_value(1000, valor1, &N_valor2, V_valor2) < 0)
  {
    printf("Error en el get_value\n");
    return -1;
  };
  printf("Valor1 debe ser Hola: %s\n", valor1);
  printf("N_valor2 debe ser 3: %d\n", N_valor2);
  printf("V_valor2 debe ser 1.0, 2.0, 3.0\n");
  for (int i = 0; i < N_valor2; i++)
  {
    printf("  V_valor2[%d]: %f\n", i, V_valor2[i]);
  }

  printf("Get_value OK.\n");

  // MODIFY_VALUE TEST (+COMPROBAR EXIST)
  double modify_V_valor2[] = {4.0, 5.0, 6.0};
  if (modify_value(1000, "Adios", 3, modify_V_valor2) < 0)
  {
    printf("Error en el modify_value\n");
    return -1;
  };

  if (get_value(1000, valor1, &N_valor2, V_valor2) < 0)
  {
    printf("Error en el get_value\n");
    return -1;
  };
  printf("Valor1 debe ser Adios: %s\n", valor1);
  printf("N_valor2 debe ser 3: %d\n", N_valor2);
  printf("V_valor2 debe ser 4.0, 5.0, 6.0\n");
  for (int i = 0; i < N_valor2; i++)
  {
    printf("  V_valor2[%d]: %f\n", i, V_valor2[i]);
  }

  printf("Resultado de Exist debe ser 1: %d\n", exist(1000));

  printf("Modify Value y Exist OK. \n");

  // DELETE_KEY TEST (+COMPROBAR EXIST)
  if (delete_key(1000) < 0)
  {
    printf("Error en el delete_key\n");
    return -1;
  };
  printf("Resultado de Exist debe ser 0: %d\n", exist(1000));
  printf("Delete Key OK.\n");

  // DELETE_KEY TEST 2 - BORRAR CLAVE QUE NO EXISTE
  if (delete_key(2000) < 0)
  {
    printf("Error en el delete_key 2 ( OK. ERROR ESPERADO)\n");
  };

  // SET_VALUE TEST + INIT TEST 2 - INICIALIZAR EL SISTEMA, DEBERÍAN BORRARSE LOS FICHEROS
  double initial_V_valor2_3[] = {1.0, 2.0, 3.0};
  if (set_value(1000, "Hola", 3, initial_V_valor2_3) < 0)
  {
    printf("Error en el set_value\n");
    return -1;
  };
  printf("Set_value OK.\n");

  if (init() < 0)
  {
    printf("Error en el init\n");
    return -1;
  };

  printf("Resultado de Exist debe ser 0: %d\n", exist(1000));
  printf("Init 2 TEST OK. \n");

  // TEST 3 - SET VALUE CON N_value2 FUERA DE RANGO
  printf("Procede test de set_value con valores fuera de rango (<0): \n");
  double initial_V_valor2_4[] = {};
  if (set_value(1000, "Hola", 0, initial_V_valor2_4) < 0)
  {
    printf("Error en el set_value ( OK. ERROR ESPERADO)\n");
  };

  printf("Fin de los tests, todo OK\n");

  return 0;
}