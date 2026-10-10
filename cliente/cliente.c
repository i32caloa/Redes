#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "../comun/comun.h"

int main(){

    int sd;
    struct sockaddr_in socketCliente;
    char buffer[MAX_BUFFER];
    socklen_t len_socket;
    int fin = 0;

    fd_set readfds;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){

        perror("Error al abrir el socket cliente\n");
        exit(EXIT_FAILURE);

    }

    socketCliente.sin_family = AF_INET;
    socketCliente.sin_port = htons(PUERTO);
    socketCliente.sin_addr.s_addr = inet_addr("127.0.0.1");

    len_socket = sizeof(socketCliente);

    if(connect(sd, (struct sockaddr *)&socketCliente, len_socket) == -1){
        
        perror("Error de conexión\n");
        exit(EXIT_FAILURE);

    }

    printf("Conectado con el servidor. Escribe tus comandos:\n");

    while(!fin){

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sd, &readfds);

        if(select(sd + 1, &readfds, NULL, NULL, NULL) == -1){

            perror("Error en el select\n");
            break;

        }

        // 1. Recibir mensajes del servidor
        if(FD_ISSET(sd, &readfds)){
            
            memset(buffer, 0, sizeof(buffer));
            int bytes = recv(sd, buffer, sizeof(buffer) -1, 0);

            if(bytes <= 0){

                printf("Desconectado del servidor\n");
                break;

            }

            printf("%s\n", buffer);
        }

        // 2. Escribir comandos crudos por teclado
        if(FD_ISSET(0, &readfds)){

            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);

            // Quitamos el salto de línea del Enter
            buffer[strcspn(buffer, "\n")] = 0;

            send(sd, buffer, strlen(buffer), 0);

            if(strcmp(buffer, "SALIR") == 0){
                fin = 1;
            }
        }
    }

    close(sd);
    exit(EXIT_SUCCESS);
}