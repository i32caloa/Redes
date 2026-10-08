#include "registro.h"
#include <stdio.h>
#include <string.h>

int registrarUsuario(char *usuario, char *password){

    FILE *f = fopen("usuarios.txt", "a");

    if(f == NULL){

        perror("No se pudo abrir el fichero\n");
        return 0;

    }

    fprintf(f, "%s %s\n", usuario, password);
    fclose(f);

    return 1;
}

int eliminarUsuario(char *usuario, char *password){

    FILE *f = fopen("usuarios.txt", "r");
    FILE *fTemp = fopen("temp.txt", "w");

    if(f == NULL || fTemp == NULL) return 0;

    char linea[150];
    char usertxt[50];
    char passwordtxt[50];

    int borrado = 0;

    while(fgets(linea, sizeof(linea), f) != NULL){

        bzero(usertxt, 50);
        bzero(passwordtxt, 50);

        if(sscanf(linea, "%s %s", usertxt, passwordtxt)){

            if(strcmp(usuario, usertxt) != 0){

                fprintf(fTemp, "%s %s\n", usertxt, passwordtxt);

            } else {

                borrado = 1;

            }

        }

    }

    fclose(f);
    fclose(fTemp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");
    return borrado;

}
