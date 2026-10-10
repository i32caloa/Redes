.PHONY: all servidor cliente clean

CC = gcc
CFLAGS = -Wall

all: servidor cliente

servidor:
	$(CC) $(CFLAGS) servidor/servidor.c servidor/login.c servidor/registro.c -o servidor/servidor.exe

cliente:
	$(CC) $(CFLAGS) cliente/cliente.c -o cliente/cliente.exe

clean:
	rm -f servidor/servidor.exe cliente/cliente.exe