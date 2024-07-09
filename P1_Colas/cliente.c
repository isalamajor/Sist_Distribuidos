#include <stdio.h>
#include "claves.h"
#include "mensaje.h"
#define MAXSIZEVECTOR 32

int main() {
  char valor1[256];
  int N_valor2;
  double V_valor2[MAXSIZEVECTOR];

  // INIT TEST
  printf("## Test INIT ##\n\n");

  if (init() < 0) {
    printf("Error en el init\n");
    return -1;
  };

  printf("Init OK. \n\n ## Test SET_VALUE ## \n\n ");

  // SET_VALUE TEST
  double initial_V_valor2[] = {1.0, 2.0, 3.0};
  if (set_value(1000, "Hola", 3, initial_V_valor2) < 0){
    printf("Error en el set_value\n");
    return -1;
  };

  printf("Set_value OK. \n\n ## GET_VALUE TEST ## \n\n");

  // GET_VALUE TEST
  if (get_value(1000, valor1, &N_valor2, V_valor2) < 0){
    printf("Error en el get_value\n");
    return -1;
  };
  printf("Valor1: %s\n", valor1);
  printf("N_valor2: %d\n", N_valor2);
  for (int i = 0; i < N_valor2; i++) {
    printf("V_valor2[%d]: %f\n", i, V_valor2[i]);
  }

  printf("Get_value OK. \n\n ## MODIFY_VALUE TEST + EXIST ## \n\n");

  // MODIFY_VALUE TEST (+COMPROBAR EXIST)
  double modify_V_valor2[] = {4.0, 5.0, 6.0};
  if (modify_value(1000, "Adios", 3, modify_V_valor2) < 0){
    printf("Error en el modify_value\n");
    return -1;
  };
  if (get_value(1000, valor1, &N_valor2, V_valor2) < 0){
  printf("Error en el get_value 2\n");
  return -1;
  };
  printf("Valor1: %s\n", valor1);
  printf("N_valor2: %d\n", N_valor2);
  for (int i = 0; i < N_valor2; i++) {
    printf("V_valor2[%d]: %f\n", i, V_valor2[i]);
  }
  printf("Result of Exist should be 1: %d\n", exist(1000));

  printf("Modify Value OK. \n\n ## DELETE_KEY TEST + EXIST ## \n\n");
  
  // DELETE_KEY TEST (+COMPROBAR EXIST)
  if (delete_key(1000) < 0){
    printf("Error en el delete_key\n");
    return -1;
  };
  printf("Result of Exist should be 0: %d\n", exist(1000));

  printf("Delete_key OK. \n\n ## DELETE_KEY TEST 2 - Clave inexistente ## \n\n");

  // DELETE_KEY TEST 2 - BORRAR CLAVE QUE NO EXISTE
  if (delete_key(2000) < 0){
      printf("Error en el delete_key 2 ( OK. ERROR ESPERADO)\n");
  };
  
  printf("\n ## SET_VALUE TEST 2 ## \n\n");

  // SET_VALUE TEST + INIT TEST 2 - INICIALIZAR EL SISTEMA, DEBERÍAN BORRARSE LOS FICHEROS
  double initial_V_valor2_3[] = {1.4, 8.0, 13.4, 7.2};
  if (set_value(3000, "Colores", 4, initial_V_valor2_3) < 0){
    printf("Error en el set_value\n");
    return -1;
  };

  printf("Set_value OK.\n\n # INIT TEST 2 + EXIST ## \n\n");

  if (init() < 0) {
    printf("Error en el init\n");
    return -1;
  };
  
  //COMPROBAMOS QUE LOS FICHEROS SE HAN BORRADO CON EL INIT
  printf("Result of Exist should be 0: %d\n", exist(1000));

  printf("Init 2 TEST OK. \n\n ## SET_VALUE TEST 3 - Valor N_value2 fuera de rango\n\n ");

  //TEST 3 - SET VALUE CON N_value2 FUERA DE RANGO
  double initial_V_valor2_4[] = {};
  if (set_value(1000, "Hola", 0, initial_V_valor2_4) < 0){
    printf("Error en el set_value ( OK. ERROR ESPERADO)\n");
  };

  printf("\nFin de pruebas!\n"); 
  
  return 0;
}
