#include"utils.h"

t_log* logger;

int iniciar_servidor(void)
{
	int err;
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");-quitado

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PUERTO, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	int fd_escucha=socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol); //parametro por lo de la linea 15,16,17
	// Asociamos el socket a un puerto
	err=setsockopt(fd_escucha,SOL_SOCKET,SO_REUSEPORT,&(int){1},sizeof(int)); //nuevo-guiaSocket
	err=bind(fd_escucha,servinfo->ai_addr,servinfo->ai_addrlen); //nuevo-guiaSocket

	// Escuchamos las conexiones entrantes
	err=listen(fd_escucha,SOMAXCONN); //nuevo-guiaSocket

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	//return socket_servidor;
	return fd_escucha; 
	
}

int esperar_cliente(int socket_servidor)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!"); - quitadp

	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente=accept(socket_servidor,NULL,NULL); //guia de sockets - sin esto el socket_cliente no tiene ningun valor entonces no conectara con naide
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
