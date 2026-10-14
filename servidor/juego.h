#ifndef JUEGO_H
#define JUEGO_H

#define MAX_TURNOS 10

typedef enum{
    NEUTRAL = 0,
    JUGADOR1 = 1,
    JUGADOR2 = 2
} Dueño;

typedef struct{
    Dueño dueño;
    int unidades;
}Territorio;

typedef struct{
    Territorio tablero[5][5];
    int turno_actual; //1 = Jugador 1 ; 2 = Jugador 2
    int num_turno; //Contador de los turnos
    int partida_activa;
}Partida;

#endif