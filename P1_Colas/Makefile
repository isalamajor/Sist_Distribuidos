all: servidor cliente

servidor: servidor.c
	gcc -o servidor servidor.c -lrt -g

cliente: cliente.c libclaves.so
	gcc -o cliente cliente.c -L. -lclaves -g -Wl,-rpath,.

libclaves.so: claves.c
	gcc -shared -o libclaves.so claves.c -lrt -fPIC

clean:
	rm -f servidor cliente libclaves.so