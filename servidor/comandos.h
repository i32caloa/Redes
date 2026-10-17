#ifndef COMANDOS_H
#define COMANDOS_H

#include "juego.h"
#include "../comun/comun.h"

typedef struct{
    int clientes[MAX_CLIENTS];
    int estadoClientes[MAX_CLIENTS];
    char nombreClientes[MAX_CLIENTS][50];
    Partida partidas[4];
    int clientePartida[MAX_CLIENTS];
    int numeroJugador[MAX_CLIENTS];
} EstadoServidor;

void inicializar_estado(EstadoServidor *srv);
void manejar_desconexion(int i, EstadoServidor *srv);
void procesar_mensaje(int i, char *buffer, EstadoServidor *srv);


#endif