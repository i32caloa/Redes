#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "juego.h"

void iniciar_partida(Partida *p){
    for(int i = 0; i<5; i++){
        for(int j = 0; j<5; j++){
            p->tablero[i][j].dueño = NEUTRAL;
            p->tablero[i][j].unidades = 0;
        }
    }
    srand(time(NULL));

    //Asigno aleatoriamente la posicion del J1
    int f1 = rand() % 5;
    int c1 = rand() % 5;

    int f2, c2;

    //Ahora asigno la posicion del J2 y me aseguro de que la posicion es distinta que la del J1

    do{
        f2 = rand() % 5;
        c2 = rand() % 5;
    }while(f1 == f2 && c1 == c2);

    p->tablero[f1][c1].dueño = JUGADOR1;
    p->tablero[f1][c1].unidades = 30;

    p->tablero[f2][c2].dueño = JUGADOR2;
    p->tablero[f2][c2].unidades = 30;

    p->turno_actual = 1; //Empieza el J1
    p->num_turno = 1;
    p->partida_activa = 1;
}

int cord_valida(int f, int c){
    return (f >= 0 && f <= 4 && c >= 0 && c <= 4);
}

bool es_adyacente(int f1, int c1, int f2, int c2){
    if((f1 == f2 + 1 || f1 == f2 - 1) && c1 == c2){ //Comprueba si la fila es adyacente
        return true;
    }
    else if((c1 == c2 + 1 || c1 == c2 - 1) && f1 == f2){ //Comprueba si la columna es adyacente
        return true;
    }
    else{
        return false;
    } 
}

int letra_a_indice(char letra){
    if(letra >= 'A' && letra <= 'E'){ return letra - 'A';}
    if(letra >= 'a' && letra <= 'e'){ return letra - 'a';}
    return -1; //error
}

int convertir_coordenadas(const char * coord_str, int *f, int *c){
    if(strlen(coord_str) < 2){ return 0;}
    *f = letra_a_indice(coord_str[0]);
    char num_col = coord_str[1];

    if(*f == -1 || num_col < '1' || num_col > '5'){ return 0;}
    *c = num_col - '1';
    return 1;
}


void obtener_estado(Partida *p, int jugador, char* salida){

    char temp[1024] = "\n       1      2      3      4      5\n"; //En temp se dibujara el tablero para pasarlo al final de la funcion
    char linea[128];

    for(int i = 0; i<5; i++){
        strcat(temp, "    +------+------+------+------+------+\n");
        sprintf(linea, "  %c ", 'A' + i); //Imprime las letras de las filas
        strcat(temp, linea);
        for(int j = 0; j<5; j++){
            char celda[32];
            if(p->tablero[i][j].dueño == NEUTRAL){
                sprintf(celda, "|  -   ");
            }
            else if(p->tablero[i][j].dueño == jugador){
                sprintf(celda, "| J%d(%d)", p->tablero[i][j].dueño, p->tablero[i][j].unidades);
            }
            else{
                sprintf(celda, "| J%d   ", p->tablero[i][j].dueño);
            }
            strcat(temp, celda);
        }
        strcat(temp, "|\n");
    }
    strcat(temp, "    +------+------+------+------+------+\n");
    strcpy(salida, temp);
}

int conquistar(Partida* p, int jugador, const char* origen, const char* destino, int unidades, char* respuesta){
    int f_origen, f_destino, c_origen, c_destino;
    if(!convertir_coordenadas(origen, &f_origen, &c_origen) || !convertir_coordenadas(destino, &f_destino, &c_destino)){
        strcpy(respuesta, "-Err. Error en las coordenadas\n");
        return 0;
    }
    if(p->turno_actual != jugador){
        strcpy(respuesta, "-Err. No es tu turno\n");
        return 0;
    }
    if(p->tablero[f_origen][c_origen].dueño != jugador){
        strcpy(respuesta, "-Err. El territorio de origen no te pertenece\n");
        return 0;
    }
    if(p->tablero[f_destino][c_destino].dueño != NEUTRAL){
        strcpy(respuesta, "-Err. El territorio ya está ocupado\n");
        return 0;
    }
    if(!es_adyacente(f_origen, c_origen, f_destino, c_destino)){
        strcpy(respuesta, "-Err. El territorio seleccionado no es adyacente\n");
        return 0;
    }
    if(p->tablero[f_origen][c_origen].unidades <= unidades){
        strcpy(respuesta, "-Err. No dispones de suficientes unidades\n");
        return 0;
    }

    p->tablero[f_origen][c_origen].unidades -= unidades;
    p->tablero[f_destino][c_destino].dueño = jugador;
    p->tablero[f_destino][c_destino].unidades = unidades;

    sprintf(respuesta, "+Ok. Territorio %s conquistado\n", destino);
    return 1;

}

int atacar(Partida *p, int jugador, const char* origen, const char* destino, int unidades, char* respuesta){
    int f_origen, f_destino, c_origen, c_destino;
    if(!convertir_coordenadas(origen, &f_origen, &c_origen) || !convertir_coordenadas(destino, &f_destino, &c_destino)){
        strcpy(respuesta, "-Err. Error en las coordenadas\n");
        return 0;
    }
    if(p->turno_actual != jugador){
        strcpy(respuesta, "-Err. No es tu turno\n");
        return 0;
    }
    if(p->tablero[f_origen][c_origen].dueño != jugador){
        strcpy(respuesta, "-Err. El territorio de origen no te pertenece\n");
        return 0;
    }
    if(!es_adyacente(f_origen, c_origen, f_destino, c_destino)){
        strcpy(respuesta, "-Err. El territorio seleccionado no es adyacente\n");
        return 0;
    }
    if(p->tablero[f_origen][c_origen].unidades <= unidades){
        strcpy(respuesta, "-Err. No dispones de suficientes unidades\n");
        return 0;
    }
    if(p->tablero[f_destino][c_destino].dueño == jugador || p->tablero[f_destino][c_destino].dueño == NEUTRAL){
        strcpy(respuesta, "-Err. El territorio seleccionado no es enemigo\n");
        return 0;
    }

    int unidades_defensor = p->tablero[f_destino][c_destino].unidades;
    p->tablero[f_origen][c_origen].unidades -= unidades;
    if(unidades >= unidades_defensor){
        p->tablero[f_destino][c_destino].dueño = jugador;
        p->tablero[f_destino][c_destino].unidades = unidades;
        sprintf(respuesta, "+Ok. Territorio %s conquistado\n", destino);
    }
    else{
        sprintf(respuesta, "+Ok. Has perdido el ataque\n");
    }
    return 1;

}

int reforzar(Partida* p, int jugador, const char* origen, const char* destino, int unidades, char* respuesta){
    int f_origen, f_destino, c_origen, c_destino;
    if(!convertir_coordenadas(origen, &f_origen, &c_origen) || !convertir_coordenadas(destino, &f_destino, &c_destino)){
        strcpy(respuesta, "-Err. Error en las coordenadas\n");
        return 0;
    }
    if(p->turno_actual != jugador){
        strcpy(respuesta, "-Err. No es tu turno\n");
        return 0;
    }
    if(p->tablero[f_origen][c_origen].dueño != jugador){
        strcpy(respuesta, "-Err. El territorio de origen no te pertenece\n");
        return 0;
    }
    if(!es_adyacente(f_origen, c_origen, f_destino, c_destino)){
        strcpy(respuesta, "-Err. El territorio seleccionado no es adyacente\n");
        return 0;
    }
    if(p->tablero[f_origen][c_origen].unidades <= unidades){
        strcpy(respuesta, "-Err. No dispones de suficientes unidades\n");
        return 0;
    }
    if(p->tablero[f_destino][c_destino].dueño != jugador){
        strcpy(respuesta, "-Err. El territorio seleccionado no te pertenece\n");
        return 0;
    }

    p->tablero[f_origen][c_origen].unidades -= unidades;
    p->tablero[f_destino][c_destino].unidades += unidades;

    sprintf(respuesta, "+Ok. Territorio reforzado\n");
    return 1;
}

int pasar_turno(Partida *p, int jugador, char* respuesta){
    if(p->turno_actual != jugador){
        strcpy(respuesta, "-Err. No es tu turno\n");
        return 0;
    }

    p->turno_actual = (p->turno_actual == 1) ? 2 : 1; //Si es turno de J1, cambia a J2, y viceversa
    if (p->turno_actual == 1) {
        p->num_turno++;
    }

    if(p->num_turno > MAX_TURNOS){
        strcpy(respuesta, "+Ok. Turno finalizado. Fin de la partida por limite de turnos\n");
    }
    else{
        strcpy(respuesta, "+Ok. Turno finalizado\n");
    }
    return 1;
}

int main(){
    Partida partida;
    char buffer_estado[1024];

    // Inicializamos la partida (colocará J1 y J2 de forma aleatoria con 30 unidades)
    iniciar_partida(&partida);

    printf("=== PRUEBA DE VISUALIZACION DEL TABLERO ===\n\n");

    // Mostrar el tablero desde la perspectiva del JUGADOR 1
    printf("--- Vista desde el JUGADOR 1 ---\n");
    obtener_estado(&partida, 1, buffer_estado);
    printf("%s\n", buffer_estado);

    // Mostrar el tablero desde la perspectiva del JUGADOR 2
    printf("--- Vista desde el JUGADOR 2 ---\n");
    obtener_estado(&partida, 2, buffer_estado);
    printf("%s\n", buffer_estado);

    return 0;
}