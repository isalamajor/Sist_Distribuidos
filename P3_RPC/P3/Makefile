# This is a template Makefile generated by rpcgen

# Parameters

CLIENT = cliente
SERVER = servidor
CLAVES = libclaves.so

SOURCES_CLNT.c = 
SOURCES_CLNT.h = 
SOURCES_SVC.c = 
SOURCES_SVC.h = 
SOURCES.x = claves_rpc.x

TARGETS_SVC.c = claves_rpc_svc.c servidor.c claves_rpc_xdr.c 
TARGETS_CLNT.c = claves_rpc_clnt.c  claves_rpc_xdr.c 
TARGETS = claves_rpc.h claves_rpc_xdr.c claves_rpc_clnt.c claves_rpc_svc.c claves_rpc_client.c claves_rpc_server.c

OBJECTS_CLNT = $(SOURCES_CLNT.c:%.c=%.o) $(TARGETS_CLNT.c:%.c=%.o)
OBJECTS_SVC = $(SOURCES_SVC.c:%.c=%.o) $(TARGETS_SVC.c:%.c=%.o)
# Compiler flags 

CPPFLAGS += -D_REENTRANT
CFLAGS += -g -I/usr/include/tirpc
LDLIBS += -lnsl -lpthread -ldl -ltirpc
RPCGENFLAGS = 

# Targets 

all : cliente libclaves.so $(SERVER)

$(TARGETS) : $(SOURCES.x)
	rpcgen $(RPCGENFLAGS) $(SOURCES.x)

$(OBJECTS_CLNT) : $(SOURCES_CLNT.c) $(SOURCES_CLNT.h) $(TARGETS_CLNT.c) 

$(OBJECTS_SVC) : $(SOURCES_SVC.c) $(SOURCES_SVC.h) $(TARGETS_SVC.c) 

$(SERVER) : $(OBJECTS_SVC)
	$(LINK.c) -o $(SERVER) $(OBJECTS_SVC) $(LDLIBS)

cliente: cliente.c libclaves.so
	gcc -o cliente cliente.c -L. -lclaves -g $(LDLIBS)  -pthread -Wl,-rpath,.

libclaves.so: claves.c  $(OBJECTS_CLNT)
	gcc $(CFLAGS) -shared -o libclaves.so claves.c  $(OBJECTS_CLNT)  $(LDLIBS) -lrt -fPIC

clean:
	$(RM) core $(TARGETS) $(OBJECTS_CLNT) $(OBJECTS_SVC) cliente libclaves.so $(SERVER)

