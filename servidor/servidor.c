#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include "login.h"
#include "registro.h"
#include "../comun/comun.h"

#define MAX_CLIENTS 10
#define PUERTO 2026

int main(){

    int sd, new_sd;
    struct sockaddr_in sockServidor, from;
    socklen_t from_len;

    int clientes[MAX_CLIENTS];
    int estadoClientes[MAX_CLIENTS];
    char nombreClientes[MAX_CLIENTS][50];

    fd_set readfds;
    int max_sd, activity;
    char buffer[MAX_BUFFER];

    for(int i = 0; i < MAX_CLIENTS; i++){
        clientes[i] = 0;
        estadoClientes[i] = 0;
        bzero(nombreClientes[i], 50);
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        perror("No se pudo abrir el Socket Cliente\n");
        exit(EXIT_FAILURE);
    }

    sockServidor.sin_family = AF_INET;
    sockServidor.sin_port = htons(PUERTO);
    sockServidor.sin_addr.s_addr = inet_addr("127.0.0.1");

    int opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(sd, (struct sockaddr *)&sockServidor, sizeof(sockServidor)) == -1){
        
        perror("Error en bind\n");
        exit(EXIT_FAILURE);

    }

    if(listen(sd, 10) == -1){

        perror("Error en listen\n");
        exit(EXIT_FAILURE);

    }

    printf("Servidor escuchando en el puerto %d\n", PUERTO);

    while(1){
        
        FD_ZERO(&readfds);
        FD_SET(sd, &readfds);
        
        max_sd = sd;

        for(int i = 0; i < MAX_CLIENTS; i++){

            int sdCliente = clientes[i];

            if(sdCliente > 0){

                FD_SET(sdCliente, &readfds);

            }

            if(sdCliente > max_sd){

                max_sd = sdCliente;

            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if((activity < 0)){

            perror("Error en select\n");

        }

        if(FD_ISSET(sd, &readfds)){

            from_len = sizeof(from);

            if((new_sd = accept(sd, (struct sockaddr*)&from, &from_len)) == -1){

                perror("Error accept\n");
                exit(EXIT_FAILURE);

            }

            for(int i = 0; i < MAX_CLIENTS; i++){
                
                if(clientes[i] == 0){

                    clientes[i] = new_sd;
                    estadoClientes[i] = 0;
                    printf("Cliente %d conectado\n", i);

                    char *mensaje = "[OK] Usuario conectado\n";
                    send(new_sd, mensaje, strlen(mensaje), 0);
                    break;

                }
            }
        }

        for(int i = 0; i < MAX_CLIENTS; i++){

            int sdCliente = clientes[i];

            if(sdCliente > 0 && FD_ISSET(sdCliente, &readfds)){

                bzero(buffer, sizeof(buffer));

                int lectura = recv(sdCliente, buffer, sizeof(buffer), 0);

                if(lectura == 0){

                    printf("Cliente %d desconectado\n", i);
                    close(sdCliente);
                    clientes[i] = 0;
                    estadoClientes[i] = 0;
                    bzero(nombreClientes[i], 50);

                } else {

                    buffer[strcspn(buffer, "\r\n")] = 0;
                    printf("Mensaje del cliente %d: %s\n", i, buffer);

                    if(strncmp(buffer, "USUARIO ", 8) == 0){

                        if(estadoClientes[i] == 0){

                            char usuario_temporal[50];
                            strcpy(usuario_temporal, buffer + 8);
                            
                            int ya_conectado = 0;
                            
                            for(int j = 0; j < MAX_CLIENTS; j++){
                                
                                if(j != i && estadoClientes[j] > 0 && strcmp(nombreClientes[j], usuario_temporal) == 0){
                                    
                                    ya_conectado = 1;
                                    break;
                                    
                                }
                            }

                            if(ya_conectado){
                                
                                char *respuesta = "[ERROR] Este usuario ya tiene una sesion activa en otro cliente\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                                
                            } else if(existeUsuario(usuario_temporal)){
                                
                                strcpy(nombreClientes[i], usuario_temporal);
                                estadoClientes[i] = 1;
                                char *respuesta = "[OK] Usuario correcto\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                                
                            } else {
                                
                                char *respuesta = "[ERROR] El usuario no existe en el sistema\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                                
                            }

                        } else {

                            char *respuesta = "[ERROR] Usuario incorrecto o ya conectado\n";
                            send(sdCliente, respuesta, strlen(respuesta), 0);

                        }
                    } else if(strncmp(buffer, "PASSWORD ", 9) == 0){

                        char passwd[50];
                        strcpy(passwd, buffer + 9);

                        if(estadoClientes[i] == 1){

                            if(validarCredenciales(nombreClientes[i], passwd)){
                                estadoClientes[i] = 2;
                                
                                char *respuesta = "[OK] Usuario validado\n"
                                                "--- MENU PRINCIPAL ---\n"
                                                " Opciones disponibles:\n"
                                                " - INICIAR-PARTIDA\n"
                                                " - CHAT\n"
                                                " - ELIMINAR\n"
                                                " - SALIR\n"
                                                "----------------------\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);

                            } else {
                                char *respuesta = "[ERROR] Error en la validacion\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                                estadoClientes[i] = 0;
                                bzero(nombreClientes[i], 50);
                            }

                        } else if(estadoClientes[i] == 3) {
                            
                            if(validarCredenciales(nombreClientes[i], passwd)){
                                
                                if(eliminarUsuario(nombreClientes[i], passwd)) {
                                    char *respuesta = "[OK] Cuenta eliminada. Ya no estas en el sistema.\n";
                                    send(sdCliente, respuesta, strlen(respuesta), 0);
                                    
                                    estadoClientes[i] = 0;
                                    bzero(nombreClientes[i], 50);
                                } else {
                                    char *respuesta = "[ERROR] Fallo en el servidor al eliminar el fichero\n";
                                    send(sdCliente, respuesta, strlen(respuesta), 0);
                                    estadoClientes[i] = 2; 
                                }
                                
                            } else {
                                char *respuesta = "[ERROR] Contraseña incorrecta. Eliminacion cancelada.\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                                estadoClientes[i] = 2; 
                            }
                            
                        } else {
                            char *respuesta = "[ERROR] Primero debes indicar el USUARIO o confirmar operacion\n";
                            send(sdCliente, respuesta, strlen(respuesta), 0);
                        }

                    } else if(strncmp(buffer, "REGISTRO ", 9) == 0){

                        char user[50];
                        char pass[50];
                        
                        if(sscanf(buffer, "REGISTRO -u %s -p %s", user, pass) == 2) {
                            
                            if (existeUsuario(user)) {
                                char *respuesta = "[ERROR] El nombre de usuario ya esta en uso\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                                
                            } else if(registrarUsuario(user, pass)) {
                                char *respuesta = "[OK] Usuario registrado correctamente\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);

                            } else {
                                char *respuesta = "[ERROR] Fallo al registrar en el fichero\n";
                                send(sdCliente, respuesta, strlen(respuesta), 0);
                            }
                        } else {
                            char *respuesta = "[ERROR] Formato de registro incorrecto\n";
                            send(sdCliente, respuesta, strlen(respuesta), 0);
                        }

                    } else if(strncmp(buffer, "ELIMINAR", 8) == 0){

                        if (estadoClientes[i] == 2) {
                            estadoClientes[i] = 3; 
                            char *respuesta = "[OK] Vas a borrar tu cuenta. Para confirmar envia: PASSWORD tu_contraseña\n";
                            send(sdCliente, respuesta, strlen(respuesta), 0);
                        } else {
                            char *respuesta = "[ERROR] Debes iniciar sesion antes de eliminar tu cuenta\n";
                            send(sdCliente, respuesta, strlen(respuesta), 0);
                        }

                    } else if (strncmp(buffer, "HELP", 4) == 0) {

                        char *respuesta = "\n--- COMANDOS DISPONIBLES ---\n"
                                          "USUARIO <nombre>               : Inicia sesion con tu usuario\n"
                                          "PASSWORD <contraseña>          : Introduce tu contrasena\n"
                                          "REGISTRO -u <nombre> -p <pass> : Registra un nuevo usuario\n"
                                          "ELIMINAR                       : Elimina tu cuenta (requiere estar logueado)\n"
                                          "SALIR                          : Desconecta del servidor localmente\n"
                                          "----------------------------\n";
                        send(sdCliente, respuesta, strlen(respuesta), 0);

                    } else {

                        char *respuesta = "[ERROR] Comando no reconocido. Escribe HELP para ver la lista.\n";
                        send(sdCliente, respuesta, strlen(respuesta), 0);

                    }
                }
            }
        }
    }

    return 0;
}