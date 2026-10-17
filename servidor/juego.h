#ifndef JUEGO_H
#define JUEGO_H

#define MAX_TURNOS 10

typedef enum{
    NEUTRAL = 0,
    JUGADOR1 = 1,
    JUGADOR2 = 2
}User;

typedef struct{
    User user;
    int unidades;
}Territorio;

typedef struct{
    Territorio tablero[5][5];
    int turno_actual; //1 = Jugador 1 ; 2 = Jugador 2
    int num_turno; //Contador de los turnos
    int partida_activa;
}Partida;

void iniciar_partida(Partida *p);
void obtener_estado(Partida *p, int jugador, char *salida);
int conquistar(Partida *p, int jugador, const char *origen, const char *destino, int unidades, char *respuesta);
int atacar(Partida *p, int jugador, const char *origen, const char *destino, int unidades, char *respuesta);
int reforzar(Partida *p, int jugador, const char *origen, const char *destino, int unidades, char *respuesta);
int pasar_turno(Partida *p, int jugador, char *respeusta);

#endif