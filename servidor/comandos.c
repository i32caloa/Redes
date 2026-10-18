#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "comandos.h"
#include "login.h"
#include "registro.h"

void inicializar_estado(EstadoServidor *srv){
    for(int i = 0; i < MAX_CLIENTS; i++){
        srv->clientes[i] = 0;
        srv->estadoClientes[i] = 0;
        bzero(srv->nombreClientes[i], 50);
        srv->clientePartida[i] = -1;
        srv->numeroJugador[i] = -1;
    }
    for(int p = 0; p < 4; p++){
        srv->partidas[p].partida_activa = 0;
    }
}

void manejar_desconexion(int i, EstadoServidor *srv){
    printf("Cliente %d desconectado\n", i);
    int sdCliente = srv->clientes[i];

    if(srv->estadoClientes[i] == 5){
        int id_partida = srv->clientePartida[i];
        srv->partidas[id_partida].partida_activa = 0;
        for(int j = 0; j < MAX_CLIENTS; j++){
            if(j != i && srv->estadoClientes[j] == 5 && srv->clientePartida[j] == id_partida){
                srv->estadoClientes[j] = 2; 
                srv->clientePartida[j] = -1;
                srv->numeroJugador[j] = -1;
                char *aviso = "+Ok. Tu oponente ha terminado la partida\n";
                send(srv->clientes[j], aviso, strlen(aviso), 0);
            }
        }
    } else if(srv->estadoClientes[i] == 4){
        srv->estadoClientes[i] = 2;
    }

    close(sdCliente);
    srv->clientes[i] = 0;
    srv->estadoClientes[i] = 0;
    bzero(srv->nombreClientes[i], 50);
    srv->clientePartida[i] = -1;
    srv->numeroJugador[i] = -1;
}

void procesar_mensaje(int i, char *buffer, EstadoServidor *srv){
    int sdCliente = srv->clientes[i];

    if(strncmp(buffer, "USUARIO ", 8) == 0){
        if(srv->estadoClientes[i] == 0){
            char usuario_temporal[50];
            strcpy(usuario_temporal, buffer + 8);
            
            int ya_conectado = 0;
            for(int j = 0; j < MAX_CLIENTS; j++){
                if(j != i && srv->estadoClientes[j] > 0 && strcmp(srv->nombreClientes[j], usuario_temporal) == 0){
                    ya_conectado = 1;
                    break;
                }
            }

            if(ya_conectado){
                char *respuesta = "-Err. Este usuario ya tiene una sesion activa en otro cliente\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            } else if(existeUsuario(usuario_temporal)){
                strcpy(srv->nombreClientes[i], usuario_temporal);
                srv->estadoClientes[i] = 1;
                char *respuesta = "+Ok. Usuario correcto\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            } else {
                char *respuesta = "-Err. El usuario no existe en el sistema\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            }
        } else {
            char *respuesta = "-Err. Usuario incorrecto o ya conectado\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "PASSWORD ", 9) == 0){
        char passwd[50];
        strcpy(passwd, buffer + 9);

        if(srv->estadoClientes[i] == 1){
            if(validarCredenciales(srv->nombreClientes[i], passwd)){
                srv->estadoClientes[i] = 2;
                char *respuesta = "+Ok. Usuario validado\n"
                                "\n--- MENU PRINCIPAL ---\n"
                                " Opciones disponibles:\n"
                                " - INICIAR-PARTIDA\n"
                                " - SALIR\n"
                                "----------------------\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            } else {
                char *respuesta = "-Err. Error en la validacion\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
                srv->estadoClientes[i] = 0;
                bzero(srv->nombreClientes[i], 50);
            }
        } else {
            char *respuesta = "-Err. Primero debes indicar el USUARIO o confirmar operacion\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "REGISTRO ", 9) == 0){
        char user[50];
        char pass[50];
        char extra[50]; 
        
        int leidos = sscanf(buffer, "REGISTRO %s %s %s", user, pass, extra);
        
        if(leidos == 2 && strcmp(user, "-u") != 0 && strcmp(user, "-p") != 0){
            if(existeUsuario(user)){
                char *respuesta = "-Err. El nombre de usuario ya esta en uso\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            } else if(registrarUsuario(user, pass)){
                char *respuesta = "+Ok. Usuario registrado correctamente\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            } else {
                char *respuesta = "-Err. Fallo al registrar en el fichero\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            }
        } else {
            char *respuesta = "-Err. Formato incorrecto. Uso: REGISTRO usuario password\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "INICIAR-PARTIDA", 15) == 0){
        if(srv->estadoClientes[i] == 2){
            int emparejado = 0;
            for(int j = 0; j < MAX_CLIENTS; j++){
                if(j != i && srv->estadoClientes[j] == 4){
                    
                    int hueco = -1;
                    for(int p = 0; p < 4; p++){
                        if(srv->partidas[p].partida_activa == 0){
                            hueco = p;
                            break;
                        }
                    }
                    
                    if(hueco != -1){
                        iniciar_partida(&srv->partidas[hueco]);
                        
                        srv->estadoClientes[j] = 5;
                        srv->clientePartida[j] = hueco;
                        srv->numeroJugador[j] = 1;
                        
                        srv->estadoClientes[i] = 5;
                        srv->clientePartida[i] = hueco;
                        srv->numeroJugador[i] = 2;
                        
                        char *msg_inicio = "+Ok. Empieza la partida. Es el turno de J1.\n";
                        send(sdCliente, msg_inicio, strlen(msg_inicio), 0);
                        send(srv->clientes[j], msg_inicio, strlen(msg_inicio), 0);
                        
                        emparejado = 1;
                        break;
                    } else {
                        char *respuesta = "-Err. No hay slots de partida libres (Max 4)\n";
                        send(sdCliente, respuesta, strlen(respuesta), 0);
                        emparejado = 1; 
                        break;
                    }
                }
            }
            
            if(!emparejado){
                srv->estadoClientes[i] = 4;
                char *msg_espera = "+Ok. Esperando otro jugador\n";
                send(sdCliente, msg_espera, strlen(msg_espera), 0);
            }
            
        } else {
            char *respuesta = "-Err. Debes estar logueado y no estar ya jugando\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "ESTADO", 6) == 0){
        if(srv->estadoClientes[i] == 5){
            int id_partida = srv->clientePartida[i];
            int jugador = srv->numeroJugador[i];
            char estado_tablero[1024];
            bzero(estado_tablero, 1024);
            
            obtener_estado(&srv->partidas[id_partida], jugador, estado_tablero);
            send(sdCliente, estado_tablero, strlen(estado_tablero), 0);
        } else {
            char *respuesta = "-Err. No estas en ninguna partida activa\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "CONQUISTAR ", 11) == 0){
        if(srv->estadoClientes[i] == 5){
            char origen[10], destino[10];
            int unidades;
            if(sscanf(buffer, "CONQUISTAR %s %s %d", origen, destino, &unidades) == 3){
                int id_partida = srv->clientePartida[i];
                int jugador = srv->numeroJugador[i];
                char respuesta_juego[250];
                bzero(respuesta_juego, 250);
                
                conquistar(&srv->partidas[id_partida], jugador, origen, destino, unidades, respuesta_juego);
                send(sdCliente, respuesta_juego, strlen(respuesta_juego), 0);
            } else {
                char *respuesta = "-Err. Formato incorrecto. Uso: CONQUISTAR origen destino unidades\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            }
        } else {
            char *respuesta = "-Err. No estas en ninguna partida activa\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "ATACAR ", 7) == 0){
        if(srv->estadoClientes[i] == 5){
            char origen[10], destino[10];
            int unidades;
            if(sscanf(buffer, "ATACAR %s %s %d", origen, destino, &unidades) == 3){
                int id_partida = srv->clientePartida[i];
                int jugador = srv->numeroJugador[i];
                char respuesta_juego[250];
                bzero(respuesta_juego, 250);
                
                atacar(&srv->partidas[id_partida], jugador, origen, destino, unidades, respuesta_juego);
                send(sdCliente, respuesta_juego, strlen(respuesta_juego), 0);
                
                if(strncmp(respuesta_juego, "+Ok", 3) == 0){
                    for(int j = 0; j < MAX_CLIENTS; j++){
                        if(j != i && srv->estadoClientes[j] == 5 && srv->clientePartida[j] == id_partida){
                            char aviso[300];
                            sprintf(aviso, "+Ok. El oponente ha atacado. Resultado: %s", respuesta_juego);
                            send(srv->clientes[j], aviso, strlen(aviso), 0);
                        }
                    }
                }

                // Condición de victoria: si la partida termina, expulsa al menú
                if(srv->partidas[id_partida].partida_activa == 0){
                    srv->estadoClientes[i] = 2;
                    srv->clientePartida[i] = -1;
                    srv->numeroJugador[i] = -1;
                    for(int j = 0; j < MAX_CLIENTS; j++){
                        if(j != i && srv->estadoClientes[j] == 5 && srv->clientePartida[j] == id_partida){
                            srv->estadoClientes[j] = 2;
                            srv->clientePartida[j] = -1;
                            srv->numeroJugador[j] = -1;
                        }
                    }
                }
            } else {
                char *respuesta = "-Err. Formato incorrecto. Uso: ATACAR origen destino unidades\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            }
        } else {
            char *respuesta = "-Err. No estas en ninguna partida activa\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "REFORZAR ", 9) == 0){
        if(srv->estadoClientes[i] == 5){
            char origen[10], destino[10];
            int unidades;
            if(sscanf(buffer, "REFORZAR %s %s %d", origen, destino, &unidades) == 3){
                int id_partida = srv->clientePartida[i];
                int jugador = srv->numeroJugador[i];
                char respuesta_juego[250];
                bzero(respuesta_juego, 250);
                
                reforzar(&srv->partidas[id_partida], jugador, origen, destino, unidades, respuesta_juego);
                send(sdCliente, respuesta_juego, strlen(respuesta_juego), 0);
            } else {
                char *respuesta = "-Err. Formato incorrecto. Uso: REFORZAR origen destino unidades\n";
                send(sdCliente, respuesta, strlen(respuesta), 0);
            }
        } else {
            char *respuesta = "-Err. No estas en ninguna partida activa\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "PASAR", 5) == 0){
        if(srv->estadoClientes[i] == 5){
            int id_partida = srv->clientePartida[i];
            int jugador = srv->numeroJugador[i];
            char respuesta_juego[250];
            bzero(respuesta_juego, 250);
            
            pasar_turno(&srv->partidas[id_partida], jugador, respuesta_juego);
            send(sdCliente, respuesta_juego, strlen(respuesta_juego), 0);
            
            if(strncmp(respuesta_juego, "+Ok", 3) == 0){
                for(int j = 0; j < MAX_CLIENTS; j++){
                    if(j != i && srv->estadoClientes[j] == 5 && srv->clientePartida[j] == id_partida){
                        // Si la partida terminó, avisa el resultado. Si sigue activa, da el turno.
                        if (srv->partidas[id_partida].partida_activa == 0) {
                            char aviso[300];
                            sprintf(aviso, "+Ok. El oponente ha pasado. Resultado: %s", respuesta_juego);
                            send(srv->clientes[j], aviso, strlen(aviso), 0);
                        } else {
                            char *aviso = "+Ok. El oponente ha pasado. Es tu turno.\n";
                            send(srv->clientes[j], aviso, strlen(aviso), 0);
                        }
                    }
                }
            }

            // Condición de victoria: si la partida termina por límite de turnos, expulsa al menú
            if(srv->partidas[id_partida].partida_activa == 0){
                srv->estadoClientes[i] = 2;
                srv->clientePartida[i] = -1;
                srv->numeroJugador[i] = -1;
                for(int j = 0; j < MAX_CLIENTS; j++){
                    if(j != i && srv->estadoClientes[j] == 5 && srv->clientePartida[j] == id_partida){
                        srv->estadoClientes[j] = 2;
                        srv->clientePartida[j] = -1;
                        srv->numeroJugador[j] = -1;
                    }
                }
            }
        } else {
            char *respuesta = "-Err. No estas en ninguna partida activa\n";
            send(sdCliente, respuesta, strlen(respuesta), 0);
        }

    } else if(strncmp(buffer, "SALIR", 5) == 0){
        manejar_desconexion(i, srv);

    } else if(strncmp(buffer, "HELP", 4) == 0){
        char *respuesta = "\n--- COMANDOS DISPONIBLES ---\n"
                          "USUARIO <nombre>               : Inicia sesion con tu usuario\n"
                          "PASSWORD <contrasena>          : Introduce tu contrasena\n"
                          "REGISTRO usuario password      : Registra un nuevo usuario\n"
                          "INICIAR-PARTIDA                : Entra en la cola de juego\n"
                          "ESTADO                         : Ver tablero (solo en partida)\n"
                          "CONQUISTAR origen destino unid : Conquista territorio (solo en partida)\n"
                          "ATACAR origen destino unid     : Ataca territorio (solo en partida)\n"
                          "REFORZAR origen destino unid   : Refuerza territorio (solo en partida)\n"
                          "PASAR                          : Pasa turno (solo en partida)\n"
                          "SALIR                          : Cierra sesion y desconecta\n"
                          "----------------------------\n";
        send(sdCliente, respuesta, strlen(respuesta), 0);

    } else {
        char *respuesta = "-Err. Comando no reconocido o incorrecto en tu estado actual. Escribe HELP.\n";
        send(sdCliente, respuesta, strlen(respuesta), 0);
    }
}