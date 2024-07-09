/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _CLAVES_RPC_H_RPCGEN
#define _CLAVES_RPC_H_RPCGEN

#include <rpc/rpc.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	u_int double_vector_len;
	double *double_vector_val;
} double_vector;

struct respuesta {
	char *value1;
	double_vector V_value2;
	int todo_ok;
};
typedef struct respuesta respuesta;

struct set_value_rpc_1_argument {
	int key;
	char *value1;
	double_vector V_value2;
};
typedef struct set_value_rpc_1_argument set_value_rpc_1_argument;

struct modify_value_rpc_1_argument {
	int key;
	char *value1;
	double_vector V_value2;
};
typedef struct modify_value_rpc_1_argument modify_value_rpc_1_argument;

#define CLAVES_PROG 2
#define CLAVES_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define init_rpc 0
extern  enum clnt_stat init_rpc_1(int *, CLIENT *);
extern  bool_t init_rpc_1_svc(int *, struct svc_req *);
#define set_value_rpc 1
extern  enum clnt_stat set_value_rpc_1(int , char *, double_vector , int *, CLIENT *);
extern  bool_t set_value_rpc_1_svc(int , char *, double_vector , int *, struct svc_req *);
#define get_value_rpc 2
extern  enum clnt_stat get_value_rpc_1(int , respuesta *, CLIENT *);
extern  bool_t get_value_rpc_1_svc(int , respuesta *, struct svc_req *);
#define modify_value_rpc 3
extern  enum clnt_stat modify_value_rpc_1(int , char *, double_vector , int *, CLIENT *);
extern  bool_t modify_value_rpc_1_svc(int , char *, double_vector , int *, struct svc_req *);
#define delete_key_rpc 4
extern  enum clnt_stat delete_key_rpc_1(int , int *, CLIENT *);
extern  bool_t delete_key_rpc_1_svc(int , int *, struct svc_req *);
#define exist_rpc 5
extern  enum clnt_stat exist_rpc_1(int , int *, CLIENT *);
extern  bool_t exist_rpc_1_svc(int , int *, struct svc_req *);
extern int claves_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define init_rpc 0
extern  enum clnt_stat init_rpc_1();
extern  bool_t init_rpc_1_svc();
#define set_value_rpc 1
extern  enum clnt_stat set_value_rpc_1();
extern  bool_t set_value_rpc_1_svc();
#define get_value_rpc 2
extern  enum clnt_stat get_value_rpc_1();
extern  bool_t get_value_rpc_1_svc();
#define modify_value_rpc 3
extern  enum clnt_stat modify_value_rpc_1();
extern  bool_t modify_value_rpc_1_svc();
#define delete_key_rpc 4
extern  enum clnt_stat delete_key_rpc_1();
extern  bool_t delete_key_rpc_1_svc();
#define exist_rpc 5
extern  enum clnt_stat exist_rpc_1();
extern  bool_t exist_rpc_1_svc();
extern int claves_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_double_vector (XDR *, double_vector*);
extern  bool_t xdr_respuesta (XDR *, respuesta*);
extern  bool_t xdr_set_value_rpc_1_argument (XDR *, set_value_rpc_1_argument*);
extern  bool_t xdr_modify_value_rpc_1_argument (XDR *, modify_value_rpc_1_argument*);

#else /* K&R C */
extern bool_t xdr_double_vector ();
extern bool_t xdr_respuesta ();
extern bool_t xdr_set_value_rpc_1_argument ();
extern bool_t xdr_modify_value_rpc_1_argument ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_CLAVES_RPC_H_RPCGEN */