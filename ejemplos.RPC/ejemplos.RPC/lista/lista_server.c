/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "lista.h"

bool_t
sumar_1_svc(t_lista l, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	    printf("Sumando ..... \n");
        *result = 0;
        while(l !=NULL) {
                *result = *result + l->x;
                l = l->next;
        }

        printf("Resultado = %d\n", *result);



	return retval;
}

int
lista_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}