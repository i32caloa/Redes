#include "login.h"
#include <stdio.h>
#include <string.h>

int existeUsuario(char *usuario){

    FILE *f = fopen("usuarios.txt", "r");

    if(f == NULL){

        perror("Error al abrir el fichero txt\n");
        return 0;

    }

    char linea[150];
    char usertxt[50];
    char passwordtxt[50];

    while(fgets(linea, sizeof(linea), f) != NULL){

        bzero(usertxt, 50);
        bzero(passwordtxt, 50);

        if(sscanf(linea, "%s %s", usertxt, passwordtxt) == 2) {
            if(strcmp(usuario, usertxt) == 0){
                fclose(f);
                return 1;
            }
        }
    }

    fclose(f);
    return 0;
}

int validarCredenciales(char *usuario, char *password){

    FILE *f = fopen("usuarios.txt", "r");

    if(f == NULL){

        perror("Error al abrir el fichero de texto\n");
        return 0;

    }

    char linea[150];
    char usertxt[50];
    char passwordtxt[50];

    while(fgets(linea, sizeof(linea), f) != NULL){
       
        bzero(usertxt, 50);
        bzero(passwordtxt, 50);

        if(sscanf(linea, "%s %s", usertxt, passwordtxt) == 2) {
            if(strcmp(usuario, usertxt) == 0 && strcmp(password, passwordtxt) == 0){
                fclose(f);
                return 1;
            }
        }
    }

    fclose(f);
    return 0;

}
