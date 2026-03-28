#include "utils.h"

//t_log* logger; //no es buena practica el logger pero habria que averiguar - aunque los profes lo colocaron en utils.c del server
void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes); //BYTES ES EL SIZE EXACTO QUE uqiero reservar
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	//magic+desplazamiento le pedimos que escriba luego del contenidod e magic, cuanto es luego? la cant de DESPLAZAMIENTO
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints; //addrinfo es una estructura que almacena direcciones de red
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints)); //limpia la memoria
	hints.ai_family = AF_INET; //indicamo que usaremos IPv4
	hints.ai_socktype = SOCK_STREAM; //indicamos que usarmeos protocolo TCP
	hints.ai_flags = AI_PASSIVE; //decimos que llene automaticamente con una direccion IP en caso de que no posea

	getaddrinfo(ip, puerto, &hints, &server_info); //aca pedimos que quiero IP y PUERTO para enviarle datos

	// Ahora vamos a crear el socket.
	//int socket_cliente = 0;
	int socket_cliente=socket(server_info->ai_family,server_info->ai_socktype,server_info->ai_protocol); //nuevo-guiaSocket
	//la funcion socket(...) le pide al ssoo que cree una conexion retornando un fd. los parametros es para que sea compatible con el servidor que quiero conectar
	// Ahora que tenemos el socket, vamos a conectarlo
	
	//intenta conectar al servidor en la direccion "server_info->ai_addr" AHI esta la IP y el PUERTO , es el handshake para decir cliente-servidor tuvieron conexion
	/*connect(fd,IP-PUERTO,tamañoServerInfo) - 
	- 0 es OK y se puede hacer SEND 
	- -1 NOT OK, la conexion no es posible y el socket no sirve
	*/
	if(connect(socket_cliente,server_info->ai_addr,server_info->ai_addrlen)==-1){
		freeaddrinfo(server_info);
		close(socket_cliente);
		return -1;
	}else{
		freeaddrinfo(server_info);
		return socket_cliente;
	}
	
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	//preparacion del payload
	paquete->buffer->size = strlen(mensaje) + 1; //+1 para el "\0"
	paquete->buffer->stream = malloc(paquete->buffer->size); //se pide memoria para stream
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size); //el contenido del mensaje va al stream
	//memcpy(punteroDondeIraLosDatos, PunteroDondeSeSacaLosDatos, cantBytesACopiar)

	int bytes = paquete->buffer->size + 2*sizeof(int); 
	/*paquete->buffer->size es el tamaño del mensaje SUMADO la cant necesaria para tener BYTES el tamaño del paquete*/

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio); //sepone contenido del valor al flujo stream

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);
	//log_info(logger,"Mensaje enviado con exito");
	

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
