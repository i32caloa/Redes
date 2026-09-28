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

#define MAX_CLIENTS 50
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

    if(listen(sd, 10) == -1){

        perror("Error en listen\n");
        exit(EXIT_FAILURE);

    }
    

    return 0;
}