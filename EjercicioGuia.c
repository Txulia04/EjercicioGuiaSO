#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h> 

int contador;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *AtenderCliente (void *socket)
{
	int sock_conn;
	int *s;
	s = (int *) socket;
	sock_conn = *s;
	
	char peticion[512];
	char respuesta[512];
	int ret;
	
	
	int terminar =0;
	while (terminar ==0)
	{
		// Ahora recibimos la petici?n
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		
		printf ("Peticion: %s\n",peticion);
		
		// vamos a ver que quieren
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		// Ya tenemos el c?digo de la petici?n
		char nombre[20];
		if (codigo !=0 && codigo != 7)
		{
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			// Ya tenemos el nombre
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
		}
		
		if (codigo ==0) //petici?n de desconexi?n
			terminar=1;
		else if (codigo == 7)
			sprintf (respuesta, "%d", contador);
		else if (codigo ==1){
			//piden la longitd del nombre
			int x = strlen(nombre);
			sprintf (respuesta,"%d",x);
		}
		else if (codigo ==2){
			// quieren saber si el nombre es bonito
			if((nombre[0]=='M') || (nombre[0]=='S'))
				strcpy (respuesta,"SI");
			else
				strcpy (respuesta,"NO");
		}	
		else if (codigo == 3){
			p = strtok( NULL, "/");
			float altura =  atof (p);
			if (altura > 1.70)
				sprintf (respuesta, "%s: eres alto",nombre);
			else
				sprintf (respuesta, "%s: eresbajo",nombre);
		}
		else if (codigo == 4){
			int i = 0;
			int n = strlen(nombre) - 1;
			while (i < strlen (nombre)){
				if (nombre[i] == nombre[n])
					strcpy(respuesta, "Tu nombre es palindromo");
				else
					strcpy(respuesta, "Tu nombre no es palindromo");
				i ++;
				n --;
			}
		}
		else if (codigo == 5){
			for (int i = 0; nombre[i] != '\0'; ++i){
				nombre[i] = toupper(nombre[i]);
			}
			strcpy(respuesta, nombre);
		}
		
		else if (codigo == 6){
			char temp[80];
			float temperatura = atof(nombre);
			float farenheit = temperatura * 9/5 + 32;
			sprintf(temp, "%f", farenheit);
			strcpy(respuesta, temp);
		}
		
		if (codigo !=0)
		{
			printf ("Respuesta: %s\n", respuesta);
			// Enviamos respuesta
			write (sock_conn,respuesta, strlen(respuesta));
		}
		if (codigo >= 1 && codigo <=6)
			pthread_mutex_lock( &mutex );
			contador = contador + 1;
			pthread_mutex_unlock ( &mutex );
	}
	// Se acabo el servicio para este cliente
	close(sock_conn);
}

int main(int argc, char *argv[])
{
	
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;
	

	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(9070);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen");
	
	contador = 0;
	int i = 0;
	int sockets[100];
	pthread_t thread;
	
	// Bucle infinito
	for (;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		
		sockets[i] = sock_conn;
		
		pthread_create (&thread, NULL, AtenderCliente, &sockets[i]);
		i = i + 1;
	}
}
