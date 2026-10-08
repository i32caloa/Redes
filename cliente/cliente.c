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
#include <sys/select.h>
#include "../comun/comun.h"

void mostrar_menu_principal(char *usuario){

    system("clear");
    printf("========================================\n");
    printf("          USUARIO: %s\n", usuario);
    printf("========================================\n");
    printf("1. Iniciar partida\n");
    printf("2. Chat\n");
    printf("3. Eliminar cuenta\n");
    printf("4. Salir\n");
    printf("========================================\n");
    printf("Elige una opcion: ");
    
    fflush(stdout);

}

int main(){

    int sd;
    struct sockaddr_in socketCliente;
    char buffer[MAX_BUFFER];
    char peticion[MAX_BUFFER + 50];
    socklen_t len_socket;
    int fin = 0;

    int estado = 0;
    char usuario_actual[50];

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

    while(!fin){

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sd, &readfds);

        if(select(sd + 1, &readfds, NULL, NULL, NULL) == -1){

            perror("Error en el select\n");
            break;

        }

        if(FD_ISSET(sd, &readfds)){
            
            memset(buffer, 0, sizeof(buffer));
            int bytes = recv(sd, buffer, sizeof(buffer) -1, 0);

            if(bytes <= 0){

                printf("\nDesconectado del servidor\n");
                break;

            }

            // Limpiamos los saltos de linea que envia el servidor para comparar bien
            buffer[strcspn(buffer, "\r\n")] = 0;

            // Transiciones de estado basadas en lo que responde el servidor
            if(estado == 0 && strncmp(buffer, "[OK] Usuario conectado", 22) == 0){
                
                system("clear");
                printf("Usuario: ");
                fflush(stdout);
                estado = 1;
                
            } else if(estado == 1){
                
                if(strncmp(buffer, "[OK]", 4) == 0){
                    
                    printf("Contrasena: ");
                    fflush(stdout);
                    estado = 2;
                    
                } else {
                    
                    // Si el usuario no existe, volvemos a pedirlo
                    printf("%s\nUsuario: ", buffer);
                    fflush(stdout);
                    
                }
                
            } else if(estado == 2){
                
                if(strncmp(buffer, "[OK]", 4) == 0){
                    
                    // Contraseña correcta, Limpiamos pantalla y mostramos menú
                    estado = 3;
                    mostrar_menu_principal(usuario_actual);
                    
                } else {
                    
                    // Fallo de contraseña
                    printf("%s\nContrasena: ", buffer);
                    fflush(stdout);
                    
                }
                
            } else if(estado == 3){
                
                // Si estamos en el menú y recibimos la petición de confirmación de borrado
                if(strncmp(buffer, "[OK] Vas a borrar", 17) == 0){
                    
                    printf("\n%s\nContrasena: ", buffer);
                    fflush(stdout);
                    estado = 4;
                    
                } else {
                    
                    // Para cualquier otra respuesta
                    printf("\n-> %s\n", buffer);
                    printf("Elige una opcion: ");
                    fflush(stdout);
                    
                }

            } else if(estado == 4){

                // Resultado final tras meter la contraseña para eliminar la cuenta
                if(strncmp(buffer, "[OK] Cuenta eliminada", 21) == 0){

                    printf("\n%s\nSaliendo del sistema...\n", buffer);
                    fin = 1;

                } else {

                    printf("\n%s\n", buffer);
                    estado = 3; // Devolvemos al menú
                    mostrar_menu_principal(usuario_actual);

                }
            }
        }

        if(FD_ISSET(0, &readfds)){

            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;

            memset(peticion, 0, sizeof(peticion));

            // Dependiendo del estado en el que estemos, el cliente construye el comando real
            if(estado == 1){
                
                strcpy(usuario_actual, buffer);
                snprintf(peticion, sizeof(peticion), "USUARIO %s", buffer);                send(sd, peticion, strlen(peticion), 0);
                
            } else if(estado == 2 || estado == 4){
                
                snprintf(peticion, sizeof(peticion), "PASSWORD %s", buffer);
                send(sd, peticion, strlen(peticion), 0);
                
            } else if(estado == 3){
                
                int opcion = atoi(buffer);
                
                switch(opcion){

                    case 1:
                        send(sd, "INICIAR-PARTIDA", 15, 0);
                        break;
                    case 2:
                        send(sd, "CHAT", 4, 0);
                        break;
                    case 3:
                        send(sd, "ELIMINAR", 8, 0);
                        break;
                    case 4:
                        send(sd, "SALIR", 5, 0);
                        fin = 1;
                        break;
                    default:
                        printf("Opcion no valida.\nElige una opcion: ");
                        fflush(stdout);
                        break;
                        
                }
            }
        }
    }

    close(sd);
    exit(EXIT_SUCCESS);
}