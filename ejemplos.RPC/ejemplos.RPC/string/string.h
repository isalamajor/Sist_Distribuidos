/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _STRING_H_RPCGEN
#define _STRING_H_RPCGEN

#include <rpc/rpc.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


#define STRING 99
#define STRINGVER 1

#if defined(__STDC__) || defined(__cplusplus)
#define vocales 1
extern  enum clnt_stat vocales_1(char *, int *, CLIENT *);
extern  bool_t vocales_1_svc(char *, int *, struct svc_req *);
#define first 2
extern  enum clnt_stat first_1(char *, char *, CLIENT *);
extern  bool_t first_1_svc(char *, char *, struct svc_req *);
#define convertir 3
extern  enum clnt_stat convertir_1(int , char **, CLIENT *);
extern  bool_t convertir_1_svc(int , char **, struct svc_req *);
extern int string_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define vocales 1
extern  enum clnt_stat vocales_1();
extern  bool_t vocales_1_svc();
#define first 2
extern  enum clnt_stat first_1();
extern  bool_t first_1_svc();
#define convertir 3
extern  enum clnt_stat convertir_1();
extern  bool_t convertir_1_svc();
extern int string_1_freeresult ();
#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_STRING_H_RPCGEN */
