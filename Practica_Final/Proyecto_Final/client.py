from enum import Enum
import argparse
import socket
import threading
import zeep


class client:

  # ******************** TYPES *********************
  # *
  # * @brief Return codes for the protocol methods
  class RC(Enum):
    OK = 0
    ERROR = 1
    USER_ERROR = 2

  # ****************** ATTRIBUTES ******************
  _server = None
  _port = -1
  _list_users = {}
  _user = "None" 

  # ******************** METHODS *******************

  @staticmethod
  def register(user):

    # Comprobar que el user < 256 chars
    if len(user) > 256:
      return client.RC.USER_ERROR
    # Conectarse al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "REGISTER"
    message = b"REGISTER\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar "user"
    user_msg = (user + '\0').encode()
    client_socket.sendall(user_msg)


    # Recibir 0, 1 o 2
    res = client_socket.recv(1)  # Recv 1 byte
    result = res.decode()

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
      print("REGISTER OK\n")
      # Guardar el nombre de usuario de este cliente
      client._user = user
    elif result == "1":
      print("USERNAME IN USE\n")
    elif result == "2":
      print("REGISTER FAIL\n")
    else:
      return client.RC.USER_ERROR

    return client.RC.OK

  @staticmethod
  def unregister(user):

    # Comprobar que el user < 256 chars
    if len(user) > 256:
      return client.RC.USER_ERROR

    # Conectarse al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "UNREGISTER"
    message = b"UNREGISTER\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar "user"
    user_msg = (user + '\0').encode()
    client_socket.sendall(user_msg)

    # Recibir 0, 1 o 2
    res = client_socket.recv(1) 
    result = res.decode()

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
      print("UNREGISTER OK\n")
      # Borra el nombre de usuario de este cliente
      client._user = "None"
    elif result == "1":
      print("USER DOES NOT EXIST\n")
    elif result == "2":
      print("UNREGISTER FAIL\n")
    else:
      return client.RC.USER_ERROR

    return client.RC.OK

  @staticmethod
  def connect(user):

    # Comprobar que el user < 256 chars
    if len(user) > 256:
      return client.RC.USER_ERROR

    # Obtener un puerto válido libre para escuchar las solicitudes de otros usuarios
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('localhost', 0))
    server_port = server_socket.getsockname()[1]

    # Crear un hilo servidor
    def mini_servidor():
        while True: #TODO
            # Escuchar conexiones entrantes (maximo 5 en cola)
            server_socket.listen(5)
            conn, addr = server_socket.accept()
            print(f"Conexión entrante de {addr}")

            # Recibir la cadena "GET_FILE" del cliente
            data = conn.recv(8).decode()
            if data != "GET_FILE":
                print("El cliente no ha enviado la cadena GET_FILE.")
                conn.close()
                continue

            # Recibir el nombre del archivo del cliente
            file_name = conn.recv(256).decode()
            print(f"El cliente solicita el archivo: {file_name}")

            try:
                # Abrir el archivo en modo lectura binaria
                with open(file_name, 'rb') as file:
                    # Leer el contenido del archivo
                    file_content = file.read()
                    #file_content = file.read().encode()

                    # Enviar un código de confirmación al cliente
                    conn.sendall(b"0")

                    # Enviar el tamaño del contenido del archivo al cliente
                    file_size = len(file_content).to_bytes(8, byteorder='big')
                    conn.sendall(file_size)

                    # Enviar el contenido del archivo al cliente
                    conn.sendall(file_content)

                    print("Archivo enviado con éxito al cliente.")

            except FileNotFoundError:
                # Si el archivo no se encuentra, enviar un código de error al cliente
                conn.sendall(b"1")
                print("El archivo solicitado no se encontró.")
            finally:
                # Cerrar la conexión
                conn.close()

    # Iniciar el hilo para manejar las solicitudes de descarga
    request_thread = threading.Thread(target=mini_servidor)
    request_thread.daemon = True
    request_thread.start()
    
    # Conectarse con el servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "CONNECT"
    message = b"CONNECT\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)
    
    # Enviar "user"
    user_msg = (user + '\0').encode()
    client_socket.sendall(user_msg)

    # Enviar puerto en el que escucha el cliente
    server_port_msg = (str(server_port) + '\0').encode()
    client_socket.sendall(server_port_msg)

    # Recibir 0, 1 o 2
    res = client_socket.recv(1) 
    result = res.decode()

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
      print("CONNECT OK\n")
      # Guarda el nombre de usuario de este cliente
      client._user = user
    elif result == "1":
      print("CONNECT FAIL, USER DOES NOT EXIST\n")
    elif result == "2":
      print("USER ALREADY CONNECTED\n")
    elif result == "3":
      print("CONNECT FAIL\n")
    else:
      return client.RC.USER_ERROR

    return client.RC.OK

  @staticmethod
  def disconnect(user):
    # Comprobar que el user < 256 chars
    if len(user) > 256:
      return client.RC.USER_ERROR

    # Conectarse al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "DISCONNECT"
    message = b"DISCONNECT\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar "user"
    user_msg = (user + '\0').encode()
    client_socket.sendall(user_msg)

    # Recibir resultado de la operación
    res = client_socket.recv(1) 
    result = res.decode()

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
      print("DISCONNECT OK\n")
    elif result == "1":
      print("DISCONNECT FAIL / USER DOES NOT EXIST\n")
    elif result == "2":
      print("DISCONNECT FAIL / USER NOT CONNECTED\n")
    elif result == "3":
      print("CONNECT FAIL\n")
    else:
      return client.RC.USER_ERROR

    return client.RC.OK

  @staticmethod
  def publish(fileName, description):
    # Verificar que el nombre del archivo no contenga espacios en blanco
    if ' ' in fileName:
        print("Error: El nombre del archivo no puede contener espacios en blanco.")
        return client.RC.USER_ERROR
    
    # Conectarse con el servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))
    
    # Enviar mensaje "PUBLISH"
    message = b"PUBLISH\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar nombre de usuario
    user_msg = (client._user + '\0').encode()
    client_socket.sendall(user_msg)

    # Enviar nombre del archivo
    fileName_msg = (fileName + '\0').encode()
    client_socket.sendall(fileName_msg)

    # Enviar descripción del contenido
    description_msg = (description + '\0').encode()
    client_socket.sendall(description_msg)

    # Recibir resultado de la operación
    res = client_socket.recv(1)
    result = res.decode()

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
      print("PUBLISH OK\n")
    elif result == "1":
      print("PUBLISH FAIL, USER DOES NOT EXIST\n")
    elif result == "2":
      print("PUBLISH FAIL, USER NOT CONNECTED\n")
    elif result == "3":
      print("PUBLISH FAIL, CONTENT ALREADY PUBLISHED\n")
    elif result == "4":
      print("PUBLISH FAIL\n")
    else:
      return client.RC.USER_ERROR

    return client.RC.OK

  @staticmethod
  def delete(file_name):
    # Comprobar que el user < 256 chars
    if len(file_name) > 256:
        return client.RC.USER_ERROR

    # Conectarse al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "DELETE"
    message = b"DELETE\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar nombre de usuario
    user_msg = (client._user + '\0').encode()
    client_socket.sendall(user_msg)

    # Enviar nombre del fichero
    file_name_msg = (file_name + '\0').encode()
    client_socket.sendall(file_name_msg)

    # Recibir resultado de la operación
    res = client_socket.recv(1)
    result = res.decode()

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
        print("DELETE OK\n")
    elif result == "1":
        print("DELETE FAIL , USER DOES NOT EXIST\n")
    elif result == "2":
        print("DELETE FAIL , USER NOT CONNECTED\n")
    elif result == "3":
        print("DELETE FAIL , CONTENT NOT PUBLISHED\n")
    else:
        print("DELETE FAIL\n")

    return client.RC.OK

  @staticmethod
  def listusers():
    
    # Conectarse al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "LIST_USERS"
    message = b"LIST_USERS\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar nombre de usuario
    user_msg = (client._user + '\0').encode()
    client_socket.sendall(user_msg)

    # Recibir la respuesta del servidor (todo_ok)
    res = client_socket.recv(1)
    result = res.decode()

    # Imprimir resultado de la conexión
    if result == '0':
        print("LIST_USERS OK")
        # Recibir el número de usuarios
        num_users = int((client_socket.recv(2)).decode())
        client._list_users = {}
        # Recibir información de cada usuario
        for _ in range(num_users):
            
            # Recibir datos hasta el \0
            mensaje = ""
            while True:
                data = client_socket.recv(1).decode("utf-8")
                if data == '\0':  # Verificar si hemos recibido el carácter nulo
                    break
                mensaje += data

            datos_user = mensaje
            
            print("\t", datos_user)
            # Guardar datos en el diccionario
            datos_user_list = datos_user.split() #TODO
            client._list_users[datos_user_list[0]] = [datos_user_list[1], datos_user_list[2]]
    elif result == '1':
        print("LIST_USERS FAIL, USER DOES NOT EXIST")
    elif result == '2':
        print("LIST_USERS FAIL, USER NOT CONNECTED")
    else:
        print("LIST_USERS FAIL")
      
    # Cerrar socket
    client_socket.close()

    return client.RC.OK

  
  @staticmethod
  def listcontent(user):

    # Conectarse al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((client._server, client._port))

    # Enviar "LIST_CONTENT"
    message = b"LIST_CONTENT\0"
    client_socket.sendall(message)

    # Obtener fecha y hora
    wsdl = "http://localhost:8000/?wsdl"
    cliente_fh = zeep.Client(wsdl=wsdl)
    fecha_y_hora = cliente_fh.service.fecha_hora()
    cliente_fh.transport.session.close() # Desconectarse del serverweb
    msg_fh = (fecha_y_hora + '\0').encode()

    # Enviar fecha y hora
    client_socket.sendall(msg_fh)

    # Enviar nombre de usuario del cliente
    user_msg = (client._user + '\0').encode()
    client_socket.sendall(user_msg)

    # Enviar "user" (usuario del que quieres el contenido)
    user_msg = (user + '\0').encode()
    client_socket.sendall(user_msg)

    # Recibir la respuesta del servidor (todo_ok)
    res = client_socket.recv(1)
    result = res.decode()

    # Imprimir resultado de la conexión
    if result == "0":
        print("LIST_CONTENT OK")
        # Recibir el número de archivos del usuario
        num_users = int((client_socket.recv(2)).decode())
        
        # Recibir información de cada archivo
        for _ in range(num_users):
            
            # Recibir datos hasta el \0
            mensaje = ""
            data = None
            while True:
                data = client_socket.recv(516).decode("utf-8")
                if not data or '\0' in data:  # Verificar si hemos recibido el carácter nulo
                    mensaje += data.split('\0')[0]  # Agregar la parte antes del carácter nulo
                    break
                mensaje += data

            datos_user = mensaje
            print("\t", datos_user)

    elif result == "1":
        print("LIST_CONTENT FAIL , USER DOES NOT EXIST")
    elif result == "2":
        print("LIST_CONTENT FAIL , USER NOT CONNECTED")
    elif result == "3":
        print("LIST_CONTENT FAIL , REMOTE USER DOES NOT EXIST ")
    else:
        print("LIST_CONTENT FAIL")
      
    # Cerrar socket
    client_socket.close()

    return client.RC.OK

  @staticmethod
  def getfile(user, remote_FileName, local_FileName):
    print(client._list_users)
    # Obtener puerto e ip del user al que me quiero conectar
    if user in client._list_users:
      datos_user = client._list_users[user]
      puerto = int(datos_user[0])
      direccion = datos_user[1]
    else: 
        print(f"DELETE FAIL , USER {user} DOES NOT EXIST OR IS NOT CONNECTED\n")
    print(f"dir {direccion}, port{puerto}")
    # Conectarse al usuario 
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((direccion, puerto))

    # Enviar "GET_FILE" al user #TODO
    message = b"GET_FILE\0"
    client_socket.sendall(message)

    # Enviar nombre del fichero
    file_name_msg = (remote_FileName + '\0').encode()
    client_socket.sendall(file_name_msg)

    # Recibir código de confirmación (0 o 1) del servidor
    confirmation_code = client_socket.recv(1).decode()

    if confirmation_code == "0":

        # Recibir tamaño del contenido del archivo
        file_size = int.from_bytes(client_socket.recv(8), byteorder='big')

        # Recibir contenido del fichero
        received_data = bytearray()
        while len(received_data) < file_size:
            chunk = client_socket.recv(1024)
            if not chunk:
                break
            received_data += chunk

        # Guardar el contenido del archivo en el archivo local
        with open(local_FileName, 'wb') as file:
            file.write(received_data)

        print(f"Archivo '{remote_FileName}' recibido y guardado como '{local_FileName}' con éxito.")

    # Cerrar socket
    client_socket.close()

    # Imprimir resultado de la conexión
    if result == "0":
        print("DELETE OK\n")
    elif result == "1":
        print("DELETE FAIL , USER DOES NOT EXIST\n")
    elif result == "2":
        print("DELETE FAIL , USER NOT CONNECTED\n")
    elif result == "3":
        print("DELETE FAIL , CONTENT NOT PUBLISHED\n")
    else:
        print("DELETE FAIL\n")

    return client.RC.OK


  # *
  # **
  # * @brief Command interpreter for the client. It calls the protocol functions.
  @staticmethod
  def shell():

    while (True):
      try:
        command = input("c> ")
        line = command.split(" ")
        if (len(line) > 0):

          line[0] = line[0].upper()

          if (line[0] == "REGISTER"):
            if (len(line) == 2):
              client.register(line[1])
            else:
              print("Syntax error. Usage: REGISTER <userName>")

          elif (line[0] == "UNREGISTER"):
            if (len(line) == 2):
              client.unregister(line[1])
            else:
              print("Syntax error. Usage: UNREGISTER <userName>")

          elif (line[0] == "CONNECT"):
            if (len(line) == 2):
              client.connect(line[1])
            else:
              print("Syntax error. Usage: CONNECT <userName>")

          elif (line[0] == "PUBLISH"):
            if (len(line) >= 3):
              #  Remove first two words
              description = ' '.join(line[2:])
              client.publish(line[1], description)
            else:
              print("Syntax error. Usage: PUBLISH <fileName> <description>")

          elif (line[0] == "DELETE"):
            if (len(line) == 2):
              client.delete(line[1])
            else:
              print("Syntax error. Usage: DELETE <fileName>")

          elif (line[0] == "LIST_USERS"):
            if (len(line) == 1):
              client.listusers()
            else:
              print("Syntax error. Use: LIST_USERS")

          elif (line[0] == "LIST_CONTENT"):
            if (len(line) == 2):
              client.listcontent(line[1])
            else:
              print("Syntax error. Usage: LIST_CONTENT <userName>")

          elif (line[0] == "DISCONNECT"):
            if (len(line) == 2):
              client.disconnect(line[1])
            else:
              print("Syntax error. Usage: DISCONNECT <userName>")

          elif (line[0] == "GET_FILE"):
            if (len(line) == 4):
              client.getfile(line[1], line[2], line[3])
            else:
              print(
                  "Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>"
              )

          elif (line[0] == "QUIT"):
            if (len(line) == 1):
              break
            else:
              print("Syntax error. Use: QUIT")
          else:
            print("Error: command " + line[0] + " not valid.")
      except Exception as e:
        print("Exception: " + str(e))

  # *
  # * @brief Prints program usage
  @staticmethod
  def usage():
    print("Usage: python3 client.py -s <server> -p <port>")

  # *
  # * @brief Parses program execution arguments
  @staticmethod
  def parseArguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', type=str, required=True, help='Server IP')
    parser.add_argument('-p', type=int, required=True, help='Server Port')
    args = parser.parse_args()

    if (args.s is None):
      parser.error("Usage: python3 client.py -s <server> -p <port>")
      return False

    if ((args.p < 1024) or (args.p > 65535)):
      parser.error("Error: Port must be in the range 1024 <= port <= 65535")
      return False

    client._server = args.s
    client._port = args.p

    return True

  # ******************** MAIN *********************
  @staticmethod
  def main(argv):
    if (not client.parseArguments(argv)):
      client.usage()
      return

    #  Write code here
    client.shell()
    print("+++ FINISHED +++")


if __name__ == "__main__":
  client.main([])
