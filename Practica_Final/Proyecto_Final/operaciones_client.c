#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <dirent.h>
#include "operaciones.h"

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_NAME_LENGTH 256
#define MAX_PORT_LENGTH 5
#define BUFFER_SIZE 20
#define MAX_CONNECTIONS 20 
#define MAX_PATH_LENGTH 50
#define MAX_FILE_SIZE 1024
#define MAX_DESCRIPTION_LENGTH 256
#define MAX_FILENAME_SIZE 256
#define MAX_ARCHIVOS 20
#define FECHA_HORA_LENGTH 20
#define LEN_DATOS (MAX_NAME_LENGTH + MAX_PORT_LENGTH + INET_ADDRSTRLEN + 1 + 1) // Mas uno por el \0 y otro por el " "

char *directory_name = "Usuarios";
char *directory_conexions_name = "Conexiones";
char *directory_files_name = "Archivos";
char *host_rpc; // Servidor rpc


typedef struct {
    char datos[MAX_CONNECTIONS][LEN_DATOS];
    int n_connected;
} ListaUsers;


typedef struct {
    char datos[MAX_ARCHIVOS][MAX_DESCRIPTION_LENGTH * 2];
    int n_archivos;
} ListaContent;


ssize_t readLine(int fd, void *buffer, size_t n)
{
	ssize_t numRead;  /* num of bytes fetched by last read() */
	size_t totRead;	  /* total bytes read so far */
	char *buf;
	char ch;


	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return -1; 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
        	numRead = read(fd, &ch, 1);	/* read a byte */

        	if (numRead == -1) {	
            		if (errno == EINTR)	/* interrupted -> restart read() */
                		continue;
            	else
			return -1;		/* some other error */
        	} else if (numRead == 0) {	/* EOF */
            		if (totRead == 0)	/* no byres read; return 0 */
                		return 0;
			else
                		break;
        	} else {			/* numRead must be 1 if we get here*/
            		if (ch == '\n')
                		break;
            		if (ch == '\0')
                		break;
            		if (totRead < n - 1) {		/* discard > (n-1) bytes */
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
    	return totRead;
}


ssize_t sendLine(int fd, const void *buffer, size_t n) {
    ssize_t numSent;    /* número de bytes enviados por la última send() */
    size_t totSent;     /* total de bytes enviados hasta ahora */
    const char *buf;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = (const char *) buffer;
    totSent = 0;

    while (totSent < n) {
        numSent = send(fd, buf + totSent, 1, 0);  /* enviar un byte */

        if (numSent == -1) {
            if (errno == EINTR)    /* interrumpido -> reiniciar send() */
                continue;
            else
                return -1;          /* algún otro error */
        } else if (numSent == 0) {  /* si send() devuelve 0, indica cierre del socket */
            break;
        } else {
            if (*(buf + totSent) == '\0')  /* si se encuentra el carácter nulo, terminar */
                break;
            totSent += numSent;
        }
    }

    return totSent;
}



// Función para crear el directorio si no existe
int crearDirUsuarios() {
    if (access(directory_name, F_OK) == -1) {
        if (mkdir(directory_name, 0777) != 0) {
            perror("Error al crear el directorio usuarios\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}


// Crear el directorio donde se guardan los users conectados
int crearDirConexiones() {
    if (access(directory_conexions_name, F_OK) == -1) {
        if (mkdir(directory_conexions_name, 0777) != 0) {
            perror("Error al crear el directorio conexiones\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}


int crearDirUser(char * username){
  // Crear el directorio del usuario dentro del directorio base
  char user_dir[256]; 
  snprintf(user_dir, sizeof(user_dir), "%s/%s", directory_name, username);

  // Si el directorio ya existia devolver 1 (username in use)
  if (access(user_dir, F_OK) == 0) {
    return 1;
  }
  // Si el directorio no existe, crearlo
  if (mkdir(user_dir, 0700) != 0) {
    perror("Error al crear el directorio del usuario\n");
    return 2;
  }

  printf("Directorio del usuario \"%s\" creado correctamente.\n", username);
  return 0;
}


int isRegistered(char * username){
  // Crear el directorio del usuario dentro del directorio base
  char user_dir[256]; 
  snprintf(user_dir, sizeof(user_dir), "%s/%s", directory_name, username);
	printf("%s", user_dir);
  // Si el directorio ya existia devolver 1 
  if (access(user_dir, F_OK) == 0) {
    return 1;
  }

  return 0;
}

int isConnected(char *username) {
    // Construir ruta del archivo del usuario en "Conexiones"
  char user_file[256]; 
  snprintf(user_file, sizeof(user_file), "%s/%s", directory_conexions_name, username);

  // Si el archivo ya existia en devolver 1 
  if (access(user_file, F_OK) == 0) {
    return 1;
  }

  return 0;
}



int conectarUser(char *username, const char *port_client, const char *ip_client){
    
    // Ver si el usuario está registrado, si no devolver 1
    if (!isRegistered(username)){
      return 1;
    }
    
    // Construir path del directorio del user
    FILE *file;
    char path[100];
    snprintf(path, sizeof(path), "%s/%s",directory_conexions_name, username);

    // Ver si el usuario ya estaba conectado, si es así se devuelve 2
    if (access(path, F_OK) != -1) {
        return 2;
    }

    // Conectar el usuario, dentro de un fichero con su nombre se guarda el puerto y la ip
    file = fopen(path, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo para escribir en conectarUser\n");
        return 3;
    }
    fprintf(file, "%s %s", port_client, ip_client);
    fclose(file);
    return 0;
}


int disconnectUser(char *username) {
    // Verificar si el usuario está registrado
    if (!isRegistered(username)) {
      return 1; // Usuario no registrado
    }

    // Verificar si el usuario está conectado
    if (!isConnected(username)) {
      return 2; // Usuario no conectado
    }

    // Construir path del archivo del user en Conexiones
    FILE *file;
    char path[100];
    snprintf(path, sizeof(path), "%s/%s",directory_conexions_name, username);

    // Eliminar su archivo, que indica que está conectado
    if (remove(path) != 0) {
        perror("Error desconectando user.\n");
        return 3;
    }   
    return 0; // Éxito
}


// Función para eliminar recursivamente un directorio y su contenido
int eliminarDirectorio(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[1024];
        sprintf(full_path, "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                eliminarDirectorio(full_path);
            }
        } else {
            if (unlink(full_path) != 0) {
                return -1;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) != 0) {
        return -1;
    }
    return 0;
}


int darDeBaja(char * username){
  struct dirent *entry;
  char fullpath[256]; 

  // Construir ruta
  snprintf(fullpath, sizeof(fullpath), "%s/%s", directory_name, username);

  DIR *user_dir = opendir(fullpath);

  if (!user_dir){ // Si no existe el user
    closedir(user_dir);
    return 1;
  }

  // Si el usuario está conectado, borrarlo de carpeta "Conexiones"
  if (isConnected(username) != 0){
    disconnectUser(username);
  }

  // Borrar archivos del directorio del user en "Usuarios" y su Directorio
  if (eliminarDirectorio(fullpath) < 0){ 
    closedir(user_dir);
    return 2;
  }

  closedir(user_dir);
  return 0;
}


char * usuarioDeEstaIP(char * dir_user){
    DIR *dir;
    struct dirent *entry;
    char *filename = NULL;
    char filepath[MAX_NAME_LENGTH + 1];

    // Abrir el directorio
    dir = opendir(directory_conexions_name);
    if (dir == NULL) {
        perror("Error al abrir el directorio\n");
        exit(EXIT_FAILURE);
    }

    // Leer cada entrada en el directorio
    while ((entry = readdir(dir)) != NULL) {
        // Ignorar las entradas especiales "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construir la ruta completa del archivo
        sprintf(filepath, "%s/%s", directory_conexions_name, entry->d_name);

        // Abrir el archivo
        FILE *file = fopen(filepath, "r");
        if (file == NULL) {
            perror("Error al abrir el archivo");
            exit(EXIT_FAILURE);
        }

        // Leer el contenido del archivo y verificar si contiene la dirección IP
        char port[MAX_NAME_LENGTH], ip[MAX_NAME_LENGTH];
        if (fscanf(file, "%s %s", port, ip) == 2 && strcmp(ip, dir_user) == 0) {
            // La dirección IP coincide, guardar el nombre del archivo y salir del bucle
            filename = strdup(entry->d_name);
            break;
        }

        fclose(file);
    }

    closedir(dir);
    return filename;
}


int saveFile(char *username, char *fileName, char *description) {
    
    char userDir[MAX_PATH_LENGTH];
    char filePath[MAX_PATH_LENGTH];
    // Verificar que el usuario esté registrado
    if (!isRegistered(username)) {
        return 1; // Usuario no registrado
    }

    // Verificar que el usuario esté conectado
    if (!isConnected(username)) {
        return 2; // Usuario no conectado
    }

    snprintf(filePath, sizeof(filePath), "%s/%s/%s", directory_name, username, fileName);
    // Comprobar si el archivo ya estaba publicado
    if (access(filePath, F_OK) == 0) {
          return 3;
      }

    // Si no, crear el archivo en el directorio del usuario
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        perror("Error al crear el archivo (saveFile)\n");
        return 4;
    }
    fprintf(file, "%s\n", description);
    fclose(file);

    return 0; // Éxito
}

int deleteFile(char *username, char *fileName) {

    char filePath[MAX_PATH_LENGTH];
	printf("%s %s\n", username, fileName);
    // Verificar que el usuario esté registrado
    if (!isRegistered(username)) {
        return 1; // Usuario no registrado
    }
    // Verificar que el usuario esté conectado
    if (!isConnected(username)) {
        return 2; // Usuario no conectado
    }

    // Construir la ruta completa del archivo
    snprintf(filePath, sizeof(filePath), "%s/%s/%s", directory_name, username, fileName);

    // Si el archivo no existe, devolver 3
    if (access(filePath, F_OK) < 0){
        return 3;
    }  
    
    // Si existe, eliminar el archivo
    if (remove(filePath) != 0) {
        perror("Error al eliminar el archivo\n");
        return 4; 
    }

    return 0; // Éxito
}


ListaUsers listarUsers(){
    ListaUsers lista_usuarios; 
    DIR *dir;
    struct dirent *entry;
    char ruta_archivo[MAX_CONNECTIONS * (strlen(directory_conexions_name) + MAX_NAME_LENGTH + 1)];
    char temp[MAX_PORT_LENGTH + INET_ADDRSTRLEN];
    lista_usuarios.n_connected = 0;

    // Abre el directorio
    if ((dir = opendir(directory_conexions_name)) == NULL) {
        perror("No se pudo abrir el directorio conexiones\n");
        lista_usuarios.n_connected = -1;
        return lista_usuarios;
    }

    // Itera sobre los archivos en el directorio
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {

            // Construye la ruta completa del archivo y guardar el nombre del usuario (nombre del archivo)
            strcpy(lista_usuarios.datos[lista_usuarios.n_connected], entry->d_name);
            sprintf(ruta_archivo, "%s/%s", directory_conexions_name, entry->d_name); 

            // Abrir y leer el archivo
            FILE *file = fopen(ruta_archivo, "r");
            if (file == NULL) {
                perror("Error al abrir el archivo\n");
                lista_usuarios.n_connected = -1; 
                return lista_usuarios;
            }
            char c;
            int index = 0;
            while ((c = fgetc(file)) != EOF) {
              temp[index++] = c;
            }
            temp[index] = '\0';
            strcat(lista_usuarios.datos[lista_usuarios.n_connected], " ");
            strcat(lista_usuarios.datos[lista_usuarios.n_connected], temp); // Concatenar el nombre del archivo con sus datos
            // Pasar al siguiente archivo
            lista_usuarios.n_connected++;

        }
    }

    closedir(dir);
    // Agrega el carácter nulo al final de cada elemento de las listas
    /*for (int i = 0; i < lista_usuarios.n_connected; i++) {
        lista_usuarios.datos[i][LEN_DATOS] = '\0';
        printf("%s\n", lista_usuarios.datos[i]);
    }*/

    return lista_usuarios;
}


ListaContent listarContenido(char *username){
    ListaContent lista_ficheros; 
    DIR *dir;
    struct dirent *entry;
    char ruta_archivo[strlen(directory_name) + MAX_NAME_LENGTH + MAX_FILENAME_SIZE];
    char temp[MAX_FILENAME_SIZE + MAX_DESCRIPTION_LENGTH + 4]; // +4 de las comillas, el espacio y el \0
    lista_ficheros.n_archivos = 0;

    // Construir ruta del directorio del user en "Usuarios"
    char ruta_user[strlen(directory_name) + MAX_NAME_LENGTH + 1]; // +1 para el '/'
    snprintf(ruta_user, sizeof(ruta_user), "%s/%s", directory_name, username);

    // Abre el directorio
    if ((dir = opendir(ruta_user)) == NULL) {
        perror("No se pudo abrir el directorio del user dentro de \"Usuarios\"\n");
        lista_ficheros.n_archivos = -1;
        return lista_ficheros;
    }

    // Itera sobre los archivos en el directorio
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {

            // Construye la ruta completa del archivo y guardar el nombre del usuario (nombre del archivo)
            strcpy(lista_ficheros.datos[lista_ficheros.n_archivos], entry->d_name);
            sprintf(ruta_archivo, "%s/%s", ruta_user, entry->d_name); 

            // Abrir y leer el archivo
            FILE *file = fopen(ruta_archivo, "r");
            if (file == NULL) {
                perror("Error al abrir el archivo\n");
                lista_ficheros.n_archivos = -1; 
                return lista_ficheros;
            }
            char c;
            int len_descripcion = 3;
            int index = 2;
            temp[0] = ' ';
            temp[1] = '\"'; // Empezar descripcion con comillas
            while ((c = fgetc(file)) != EOF) {
              temp[index++] = c;
              len_descripcion ++;
            }
            temp[index-1] = '\"'; // Terminar descripcion con cierre de comillas
            temp[index] = '\0';
            strncat(lista_ficheros.datos[lista_ficheros.n_archivos], temp, len_descripcion); // Concatenar el nombre del archivo con su descripción
            //strcat(lista_ficheros.datos[lista_ficheros.n_archivos], "\0"); // Terminar con \0
            // Pasar al siguiente archivo
            lista_ficheros.n_archivos++;

        }
    }

    closedir(dir);

    return lista_ficheros;
}


int listContentOK(char *usuario_demandante, char *usuario){
  int ret_value;

  // Devolver 1 si el usuario que realiza la operación no existe
  if (!isRegistered(usuario_demandante)){return 1;}

  // Devolver 2 su el usuario que realiza la operación no está conectado
  if (!isConnected(usuario_demandante)){return 2;}

  // Devolver 3 si el usuario del que se quieren obtener los datos no existe
  if (!isRegistered(usuario)){return 3;}

  // Si de momento no hay ningún otro error, devuelvo 0
  return 0;
}


// Función para manejar la conexión con un cliente

void *handle_client(void *arg) {
	char *mensaje = (char *)malloc(BUFFER_SIZE);
	char *username = (char *)malloc(MAX_NAME_LENGTH);
	char *user_demandado = (char *)malloc(MAX_NAME_LENGTH);
	char *port_client = (char *)malloc(MAX_PORT_LENGTH);
	char *fecha_hora = (char *)malloc(FECHA_HORA_LENGTH);
	char filename[MAX_NAME_LENGTH];
	int client_socket = *((int *)arg);
	int ret_n;
	char ret[2];
	struct sockaddr_in addr_cliente;
	socklen_t addrlen = sizeof(addr_cliente);
	char ip_cliente[INET_ADDRSTRLEN];

	// Recibir operación a realizar
	if (readLine(client_socket, mensaje, BUFFER_SIZE) < 0) {
		perror("Error al recibir el mensaje de operación del cliente\n");
		close(client_socket);
		free(arg);
		free(mensaje);
		free(username);
		free(port_client);
		free(fecha_hora);
		pthread_exit(NULL);
	}

	// Recibir fecha y hora
	if (readLine(client_socket, fecha_hora, FECHA_HORA_LENGTH) < 0) {
		perror("Error al recibir la fecha y la hora del cliente\n");
		close(client_socket);
		free(arg);
		free(mensaje);
		free(username);
		free(port_client);
		free(fecha_hora);
		pthread_exit(NULL);
	}

	if (strncmp(mensaje, "REGISTER", 8) == 0){
		// Recibir nombre usuario
		if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
		perror("Error al recibir el username del cliente\n");
		close(client_socket);
		free(arg);
		free(mensaje);
		free(username);
		free(port_client);
		free(fecha_hora);
		pthread_exit(NULL);
		}
		ret_n = crearDirUser(username);

	} else if (strncmp(mensaje, "UNREGISTER", 10) == 0){
		// Recibir nombre usuario
		if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
		perror("Error al recibir el username del cliente\n");
		close(client_socket);
		free(arg);
		free(mensaje);
		free(username);
		free(port_client);
		free(fecha_hora);
		pthread_exit(NULL);
		}
		ret_n = darDeBaja(username);
	
	} else if (strncmp(mensaje, "CONNECT", 7) == 0){
		// Recibir nombre usuario
		if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
			perror("Error al recibir el username del cliente\n");
			close(client_socket);
			free(arg);
			free(mensaje);
			free(username);
			free(port_client);
			free(fecha_hora);
			pthread_exit(NULL);
		}

		// Recibir puerto de escucha del cliente
		if (readLine(client_socket, port_client, MAX_NAME_LENGTH) < 0) {
			perror("Error al recibir el puerto de escucha del cliente\n");
			close(client_socket);
			free(arg);
			free(mensaje);
			free(username);
			free(port_client);
			free(fecha_hora);
			pthread_exit(NULL);
		}

		// Obtener la dirección IP del cliente conectado
		if (getpeername(client_socket, (struct sockaddr *)&addr_cliente, (socklen_t *)&addr_cliente) == -1) {
			perror("Error al obtener la dirección del par conectado\n");
			close(client_socket);
			free(arg);
			free(mensaje);
			free(username);
			free(port_client);
			free(fecha_hora);
			pthread_exit(NULL);
		}
		inet_ntop(AF_INET, &addr_cliente.sin_addr, ip_cliente, INET_ADDRSTRLEN); // Convertir a str

		ret_n = conectarUser(username, port_client, ip_cliente);
	
	} else if (strncmp(mensaje, "DISCONNECT", 10) == 0) {
			// Recibir nombre de usuario
			if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
				perror("Error al recibir el nombre de usuario del cliente\n");
				close(client_socket);
				free(arg);
				free(mensaje);
				free(username);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}

			// Desconectar al usuario
			ret_n = disconnectUser(username);  
		
		} else if (strncmp(mensaje, "PUBLISH", 7) == 0){
			char description[MAX_NAME_LENGTH];
			
			// Recibir nombre de usuario que publica
			if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
			perror("Error al recibir el username del cliente\n");
			close(client_socket);
			free(arg);
			free(mensaje);
			free(username);
			free(port_client);
			free(fecha_hora);
			pthread_exit(NULL);
			}

			// Recibir nombre del archivo
			if (readLine(client_socket, filename, MAX_NAME_LENGTH) < 0) {
				perror("Error al recibir el nombre del archivo del cliente\n");
				close(client_socket);
				free(arg);
				free(mensaje);
				free(username);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}

			// Recibir descripción del contenido
			if (readLine(client_socket, description, MAX_DESCRIPTION_LENGTH) < 0) {
				perror("Error al recibir la descripción del cliente\n");
				close(client_socket);
				free(arg);
				free(mensaje);
				free(username);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}
			
			// Guardar archivo, la propia función comprueba si el usuario y al archivo el válido
			ret_n = saveFile(username, filename, description);
	
	} else if (strncmp(mensaje, "DELETE", 6) == 0) {
			// Recibir nombre de usuario que borra
			if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
				perror("Error al recibir el username del cliente\n");
				close(client_socket);
				free(arg);
				free(mensaje);
				free(username);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}
		
			// Recibir nombre del fichero
			if (readLine(client_socket, filename, 256) < 0) {
				perror("Error al recibir el nombre del fichero del cliente\n");
				close(client_socket);
				free(arg);
				free(username);
				free(mensaje);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}

			// Borrar archivo 
			ret_n = deleteFile(username, filename);
			
	} else if (strncmp(mensaje, "LIST_USERS", 10) == 0) {
			int error = 0;
			char todo_ok = '0';
			char str_n_connected[sizeof(MAX_CONNECTIONS) + 1];
			ListaUsers lista_usuarios;
			
			// Recibir nombre de usuario que solicita la lista
			if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) {
			perror("Error al recibir el username del cliente\n");
			close(client_socket);
			free(arg);
			free(mensaje);
			free(username);
			free(port_client);
			free(fecha_hora);
			pthread_exit(NULL);
			}
			
			// Comprobar que el usuario es válido, sino, enviar código
			if (!isRegistered(username)){
				ret_n = 1; // Se guarda para condicional del rpc
				todo_ok = '1';
				if (send(client_socket, &todo_ok, 1, 0) < 0){error++;}}
			else if (!isConnected(username)){
				ret_n = 2;
				todo_ok = '2';
				if (send(client_socket, &todo_ok, 1, 0) < 0){error++;}}
			
			// Si lo es, obtener la lista de users y enviarla
			else {
				lista_usuarios = listarUsers(); // Obtener lista de usuarios y comprobar que el user es válido

				// Comprobar y enviar código de resultado de la operación
				if (todo_ok == '0' && lista_usuarios.n_connected >= 0){todo_ok = '0'; ret_n = 0;}
				else {todo_ok = '3'; ret_n = 3;} 	// Si ha habido algún problema en listarUsers, se devuelve 3
				if (send(client_socket, &todo_ok, 1, 0) < 0){error++;}

				// Pasar el número de conectados a string y enviarlo
				if ((lista_usuarios.n_connected) < 10){
					sprintf(str_n_connected, "0%d", lista_usuarios.n_connected);
				}
				else{sprintf(str_n_connected, "%d", lista_usuarios.n_connected);}

				if(send(client_socket, &str_n_connected, 2, 0) < 0){error++;} 

				// Enviar datos de cada usuario
				for (int i = 0; i < lista_usuarios.n_connected; i++){ // Si fuera el caso del 3, n_connected es -1 y no se ejecuta el bucle
					int bytes_enviados;
					if ((bytes_enviados = sendLine(client_socket, lista_usuarios.datos[i] , LEN_DATOS)) < 0){error++;};
				}
			}

			// Si ha habido algún error a lo largo de las conexiones, abortar
			if (error != 0) {
				perror("Error al enviar datos de list_users\n");
				close(client_socket);
				free(arg);
				free(username);
				free(mensaje);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}
			
	}  else if (strncmp(mensaje, "LIST_CONTENT", 13) == 0) {
			int error = 0;
			char todo_ok = '0';
			char str_n_ficheros[sizeof(MAX_CONNECTIONS) + 1];
			char filename[256];
			int ret_n = 0;

			// Recibir nombre de usuario que solicita la lista
			if (readLine(client_socket, username, MAX_NAME_LENGTH) < 0) { error++; }
			
			// Recibir nombre del usuario del que devolver su contenido
			if (readLine(client_socket, user_demandado, MAX_NAME_LENGTH) < 0) { error++; }

			// Comprobar si el usuario que hace la operación está registrado y conectado
			if (isRegistered(username) != 1){ ret_n = 1; }
			else if (isConnected(username) != 1){ ret_n = 2; }
			
			// Ver si el usuario del que se quiere la lista está registrado
			else if (isRegistered(user_demandado) != 1){ ret_n = 3; }

			// Enviar código del resultado si ha habido uno de estos casos y abortar
			if (ret_n != 0){
				sprintf(ret, "%d", ret_n);
				if (sendLine(client_socket, &ret, 1) < 0) {
					perror("Error al enviar resultado al cliente\n");
				}
				free(mensaje);
				free(username);
				free(port_client);
				close(client_socket);
				pthread_exit(NULL);
			}

			// Si ambos usuarios son válidos, obtener lista de archivos del usuario
			ListaContent lista_archivos = listarContenido(user_demandado);

			// Enviar código de resultado de la operación
			if (lista_archivos.n_archivos < 0){ todo_ok = '4'; ret_n = 4; } // 4 si ha habido algún error
			if (send(client_socket, &todo_ok, 1, 0) < 0){error++;}

			// Si todo ha ido bien, pasar el número de archivos a string y enviarlo
			if (todo_ok == '0'){
				if ((lista_archivos.n_archivos) < 10){
					sprintf(str_n_ficheros, "0%d", lista_archivos.n_archivos);
				}
				else{sprintf(str_n_ficheros, "%d", lista_archivos.n_archivos);}
				
				if(send(client_socket, &str_n_ficheros, 2, 0) < 0){error++;} 

				// Y enviar datos de cada fichero
				for (int i = 0; i < lista_archivos.n_archivos; i++){
					int bytes_enviados;
					if ((bytes_enviados = send(client_socket, &lista_archivos.datos[i] , MAX_DESCRIPTION_LENGTH * 2 + 4, 0)) < 0){error++;};
				}
			}
			// Si ha habido algún error a lo largo de las conexiones, abortar
			if (error != 0) {
				perror("Error al enviar datos de list_users\n");
				close(client_socket);
				free(arg);
				free(username);
				free(mensaje);
				free(port_client);
				free(fecha_hora);
				pthread_exit(NULL);
			}
			
	} else {
		perror("Comando desconocido\n");
		close(client_socket);
		free(arg);
		free(mensaje);
		free(username);
		free(port_client);
		free(fecha_hora);
		pthread_exit(NULL);
	}

	// Enviar respuesta al cliente (excepto para list_users y list_content, que la envian por su cuenta)
	if ((strncmp(mensaje, "LIST_USERS", 10) != 0) && (strncmp(mensaje, "LIST_CONTENT", 12) != 0)) {
		sprintf(ret, "%d", ret_n);
		if (sendLine(client_socket, &ret, 1) < 0) {
			perror("Error al enviar resultado al cliente\n");
			close(client_socket);
			free(arg);
			free(mensaje);
			free(username);
			free(port_client);
			free(fecha_hora);
			pthread_exit(NULL);
		}
	}

	// Enviar datos al servidor RPC si la operación ha tenido éxito

	if (ret_n == 0){
		CLIENT *clnt;
		enum clnt_stat retval_1;
		void *result_1;
		struct user_operation get_operation_1_arg1;
		clnt = clnt_create (host_rpc, OPERACIONES, OPS_VERSION, "udp");
		if (clnt == NULL) {
		clnt_pcreateerror (host_rpc);
		exit (1);
		}
		// El filename solo se especifica para DELETE y PUBLISH, para el resto string vacía
		if ((strncmp(mensaje, "DELETE", 6) != 0) && (strncmp(mensaje, "PUBLISH", 7) != 0)){
			strcpy(filename, ""); 
		}

		get_operation_1_arg1.date_time = fecha_hora;
		get_operation_1_arg1.filename = filename;
		get_operation_1_arg1.operation = mensaje;
		get_operation_1_arg1.username = username;
		retval_1 = get_operation_1(get_operation_1_arg1, &result_1, clnt);
			if (retval_1 != RPC_SUCCESS) {
				clnt_perror (clnt, "Envio de datos a servidor RPC fallido\n");
			}
		clnt_destroy (clnt);
	}

	// Cerrar la conexión con el cliente
	close(client_socket);
	free(mensaje);
	free(username);
	free(port_client);
	free(fecha_hora);
	pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

	if (argc != 5) {
		printf("Uso: server -p <port> -r <rpc_host>\n");
		return -1;
	}
	char *port = argv[2];
	host_rpc = argv[4];
	crearDirUsuarios(); 
	crearDirConexiones();
	
	int server_socket, client_socket, addr_len;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	pthread_t thread_id;

	// Crear el socket del servidor
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("Error al crear el socket del servidor\n");
		exit(EXIT_FAILURE);
	}

	// Configurar la dirección del servidor
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = server_addr.sin_port = htons(atoi(port)); //htons(*port);

	// Vincular el socket a la dirección del servidor
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Error al vincular el socket a la dirección del servidor\n");
		exit(EXIT_FAILURE);
	}

	// Escuchar conexiones entrantes
	if (listen(server_socket, MAX_CONNECTIONS) < 0) {
		perror("Error al escuchar conexiones entrantes\n");
		exit(EXIT_FAILURE);
	}


	// Aceptar conexiones de clientes entrantes de manera indefinida
	while (1) {

		addr_len = sizeof(struct sockaddr_in);
		printf("Servidor en espera de conexiones...\n");
		// Aceptar la conexión entrante
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
		if (client_socket < 0) {
		perror("Error al aceptar la conexión entrante\n");
		exit(EXIT_FAILURE);
		}
		printf("Conexión establecida desde server\n");

		// Crear un hilo para manejar la petición del cliente
		if (pthread_create(&thread_id, NULL, handle_client, (void *)&client_socket) != 0) {
		perror("Error en la creación del hilo \n");
		close(client_socket);
		exit(EXIT_FAILURE);
		}
		printf("Hilo creado\n");
		// Liberar recursos del hilo
		pthread_detach(thread_id); 
	}

	// Cerrar el socket del servidor
	close(server_socket);

	return 0;
}
