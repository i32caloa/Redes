#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comandos.h"
#include "../comun/comun.h"

#define MAX_CLIENTS 10
#define PUERTO 2026

int main(){

    int sd, new_sd;
    struct sockaddr_in sockServidor, from;
    socklen_t from_len;
    fd_set readfds;
    int max_sd, activity;
    char buffer[MAX_BUFFER];

    // Inicializamos la estructura de estado del servidor
    EstadoServidor srv;
    inicializar_estado(&srv);

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
            int sdCliente = srv.clientes[i];
            if(sdCliente > 0) FD_SET(sdCliente, &readfds);
            if(sdCliente > max_sd) max_sd = sdCliente;
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
                if(srv.clientes[i] == 0){
                    srv.clientes[i] = new_sd;
                    srv.estadoClientes[i] = 0;
                    printf("Cliente %d conectado\n", i);
                    char *mensaje = "+Ok. Usuario conectado\n";
                    send(new_sd, mensaje, strlen(mensaje), 0);
                    break;
                }
            }
        }

        for(int i = 0; i < MAX_CLIENTS; i++){
            int sdCliente = srv.clientes[i];

            if(sdCliente > 0 && FD_ISSET(sdCliente, &readfds)){
                bzero(buffer, sizeof(buffer));
                int lectura = recv(sdCliente, buffer, sizeof(buffer), 0);

                if(lectura == 0){
                    manejar_desconexion(i, &srv);
                } else {
                    buffer[strcspn(buffer, "\r\n")] = 0;
                    printf("Mensaje del cliente %d: %s\n", i, buffer);
                    
                    // Toda la logica fue delegada a esta simple funcion
                    procesar_mensaje(i, buffer, &srv);
                }
            }
        }
    }
    return 0;
}