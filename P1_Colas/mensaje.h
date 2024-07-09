#define MAXSIZECHAR	256
#define MAXSIZEVECTOR	32


struct peticion {
  int op; /* Tipo de operación */
  int clave; /* Clave del fichero */
  char value1[MAXSIZECHAR];
  int N_value2;
  double V_value2[MAXSIZEVECTOR];
  char q_name[MAXSIZECHAR]; 	/* nombre de la cola cliente donde debe enviar la respuesta el servidor */
};

struct respuesta{
  char value1[MAXSIZECHAR];
  int N_value2;
  double V_value2[MAXSIZEVECTOR];
  int todo_ok; /* Este valor se devuelve para confirmar que la operación ha sido realizada con éxito 
  (0) o si ha habido algún problema (-1)*/
};