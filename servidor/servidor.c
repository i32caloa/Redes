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

#define MAX_CLIENTS 10
#define PUERTO 2026

int main(){

    int sd, new_sd;
    struct sockaddr_in sockCliente, from;
    socklen_t from_len;

    int clientes[MAX_CLIENTS];

    fd_set readfds;
    int max_sd, activity;
    char buffer[250];

    //Iniciamos el array a 0 para tener hueco libre para que entren los usuarios

    for(int i = 0; i < MAX_CLIENTS; i++){
        clientes[i] = 0;
    }

    //abrir socket

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        perror("No se pudo abrir el Socket Cliente\n");
        exit(EXIT_FAILURE);
    }

    //campos de la estructura, IP siempre 127.0.0.1 ya que es nuestra maquina con linux

    sockCliente.sin_family = AF_INET;
    sockCliente.sin_port = htons(PUERTO);
    sockCliente.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(sd, (struct sockaddr *)&sockCliente, sizeof(sockCliente)) == -1){
        
        perror("Error en bind\n");
        exit(EXIT_FAILURE);

    }

    //hablita socket

    if(listen(sd, 10) == -1){

        perror("Error en listen\n");
        exit(EXIT_FAILURE);

    }

    printf("Servidor escuchando en el puerto %d", PUERTO);

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

                    clientes[i] == new_sd;
                    printf("Cliente %d conectado\n", i);

                    char *mensaje = "Usuario conectado\n";
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
                    clientes[i] == 0;

                } else {

                    printf("Mensaje del cliente %d: %s\n", i, buffer);

                }
            }
        }
    }

    return 0;
}