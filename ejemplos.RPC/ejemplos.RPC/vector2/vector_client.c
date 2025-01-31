/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "vector.h"

#define MAX 100000000


void
vector_1(char *host)
{
	CLIENT *clnt;
	enum clnt_stat retval_1;
	int result_1;
	t_vector  sumar_1_v;
	int i;

	clnt = clnt_create (host, VECTOR, VECTORVER, "tcp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}



	sumar_1_v.t_vector_len= MAX;
	sumar_1_v.t_vector_val= (int *) malloc(MAX * sizeof(int));
	for (i =0; i < MAX; i ++)
		sumar_1_v.t_vector_val[i] = 2;	
	

	retval_1 = sumar_1(sumar_1_v, &result_1, clnt);
	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	printf("La suma es %d\n", result_1);

	free (sumar_1_v.t_vector_val);
	clnt_destroy (clnt);
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	vector_1 (host);
exit (0);
}
