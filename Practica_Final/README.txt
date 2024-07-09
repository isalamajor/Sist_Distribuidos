Para compilar tenemos nuestro Makefile. En este Makefile solo hemos incluido el servidor y el servidor RPC. Estos son los pasos que seguimos para ejecutar el proyecto.
 
Hacer Make: generará todos los archivos del servidor RPC y su cliente, que es nuestro servidor principal (el de la primera parte, ahora llamado operaciones_client.c).
Poner Servidor RPC en marcha, servirá desde el localhost:
./operaciones_server
Poner Servidor Principal en marcha, servirá desde el host y puerto que le especificamos, en nuestro caso el 8085 y el “192.168.1.62”, que es uno que tenía disponible nuestro dispositivo: 
./operaciones_client -p 8085 -r 192.168.1.62 
Poner el Servidor Web en marcha, en el puerto 8000: 
python3 serverweb.py &
python3 -mzeep http://localhost:8000/?wsdl 

Lanzar nuestros clientes, uno en cada terminal (usamos una terminal o cliente por usuario):
python3 client.py -s 192.168.1.62 -p 8085
