/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "operaciones.h"

bool_t
xdr_user_operation (XDR *xdrs, user_operation *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->username, 256))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->operation, 13))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->filename, 256))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->date_time, 20))
		 return FALSE;
	return TRUE;
}
