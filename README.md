# Práctica 2 Redes: Dominio Territorial

Este proyecto implementa una arquitectura cliente-servidor en C utilizando sockets TCP y la llamada al sistema `select` para la multiplexación de entrada/salida. El sistema gestiona conexiones concurrentes, autenticación de usuarios y ejecuta un sistema de partidas multijugador por turnos (Dominio Territorial).

## Autores
- Antonio Cañete López
- Ricardo Izquierdo Muñoz

## Estructura del Proyecto
El código fuente está dividido en directorios modulares para separar la lógica de red de la lógica de negocio:
- `/servidor`: Contiene el código principal de red (`servidor.c`), el enrutador de peticiones (`comandos.c`), el motor del tablero (`juego.c`), los módulos de autenticación (`login.c`, `registro.c`) y la base de datos de texto (`usuarios.txt`).
- `/cliente`: Contiene el código del cliente TCP.
- `/comun`: Contiene las cabeceras compartidas (puertos, tamaños de buffer) para asegurar la sincronización entre cliente y servidor.

## Compilación
El proyecto cuenta con un archivo Makefile en la raíz para facilitar la compilación unificada.

Para compilar tanto el servidor como el cliente, ejecuta en la raíz del proyecto:
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
* `REGISTRO <nombre> <contrasena>`: Crea un nuevo usuario en la base de datos del servidor.
* `HELP`: Muestra la lista de comandos y su funcionamiento.
* `INICIAR-PARTIDA`: Comienza el emparejamiento a la espera de otro cliente.
* `SALIR`: Cierra la conexion y termina el proceso del cliente.
