# Proyecto de Redes: Sistema Cliente-Servidor TCP

Este proyecto implementa una arquitectura cliente-servidor en C utilizando sockets TCP y la llamada al sistema `select` para la multiplexacion de entrada/salida. El sistema gestiona conexiones concurrentes, autenticacion de usuarios y sentara las bases para un sistema de partidas multijugador.

## Autores
* Antonio Cañete
* Ricardo Izquierdo Muñoz

## Estructura del Proyecto
El codigo fuente esta dividido en directorios modulares para separar la logica:
* `/servidor`: Contiene el codigo principal del servidor, asi como los modulos de login, registro y gestion de ficheros (`usuarios.txt`).
* `/cliente`: Contiene el codigo del cliente TCP.
* `/comun`: Contiene las cabeceras compartidas (puertos, tamaños de buffer) para asegurar la sincronizacion entre cliente y servidor.

## Compilacion
El proyecto cuenta con un archivo Makefile en la raiz para facilitar la compilacion.

Para compilar tanto el servidor como el cliente, ejecuta en la raiz del proyecto:
```bash
make
```

Para limpiar los archivos ejecutables compilados previamente:
```bash
make clean
```

## Ejecucion
Es necesario arrancar primero el servidor y posteriormente los clientes deseados.

1. Iniciar el servidor:
```bash
cd servidor
./servidor.exe
```

2. Iniciar el cliente:
```bash
cd cliente
./cliente.exe
```

## Comandos Disponibles
Una vez el cliente esta conectado, el sistema responde a comandos en texto plano:

* `USUARIO <nombre>`: Inicia el proceso de autenticacion con un nombre de usuario.
* `PASSWORD <contrasena>`: Introduce la contrasena para validar el login o confirmar el borrado de cuenta.
* `REGISTRO -u <nombre> -p <contrasena>`: Crea un nuevo usuario en la base de datos del servidor.
* `ELIMINAR`: Inicia el proceso para borrar la cuenta activa (requiere estar validado).
* `HELP`: Muestra la lista de comandos y su funcionamiento.
* `SALIR`: Cierra la conexion y termina el proceso del cliente.