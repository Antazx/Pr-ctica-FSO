/* Guillermo Anta Alonso 12424235A */
/* Oscar Fernandez Angulo 12425734F*/ 

#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>
#include<string.h>

//Declaración de variables globales.
char **buffer1;
char buffer2[5][11];

//i es el índice del buffer uno.
//j es el índice del buffer dos.
//restCons es el número de consumidores en ejecución.
int i=0,j=0,restCons=0;

//Semáforos que indican si hay dato o espacio en los buffers.
sem_t espacioB1;
sem_t datoB1;
sem_t espacioB2;
sem_t datoB2;

//Semáforos para garantizar la exclusión mutua en las variables globales.
sem_t mutexI;
sem_t mutexJ;
sem_t mutexRest;

//Función que comprueba si las palabras son palíndromas o no.
int palindromo (char *palabra) {
 	int esPalindromo = 1; 
	int i,j;
	j=strlen(palabra)-1;
	for (i=0; i<strlen(palabra)/2 && esPalindromo; i++, j--) {
		if (*(palabra+i)!=*(palabra+j)) {
			esPalindromo = 0; 
		}
	}
	return esPalindromo;
}

//Inicialización del hilo productor.
void *productor(void *arg){
	
	//Declaración de variables locales.
	FILE *archivo;
	char **argumentos = (char **) arg;
	char *nombreFich = argumentos[2];	
	char palabra[10];
	int tmp,contador,tam;
	
	//Inicialización de variables locales.
	tam = atoi(argumentos[1]);
	contador = 0;

	//Abrimos el archivo de entrada y comprobamos que no se han producido errores.
	archivo = fopen(nombreFich,"r");
	if (archivo==NULL) {
		fprintf(stderr,"*** ERROR al abrir el archivo de entrada\n");
		exit (1);
	}
	//Mientras no encontremos el final del archivo.
	while (tmp!=EOF){
			
		//Esperamos a que el buffer uno tenga espacio.
		sem_wait(&espacioB1);

		//Guardamos el contenido del archivo en la posición del buffer uno correspondiente.
		tmp = fscanf (archivo,"%s",buffer1[contador]);

		//Si encontramos el final del archivo:
		if(tmp==EOF){
			
			//Escribimos "FIN" en la siguiente posición del buffer uno.
			sprintf(buffer1[contador],"FIN");
			
			//Avisamos de que hay dato en buffer uno.
			sem_post(&datoB1);	
			
			//Cerramos el archivo y finalizamos el hilo.
			fclose(archivo);
			pthread_exit(0);
		}
		//Incrementamos el hilo de manera circular en función del tamaño del buffer uno.
		contador=(contador+1)%tam;
		
		//Avisamos de que hay dato en buffer uno.
		sem_post(&datoB1);
	}
}


//Inicialización del hilo consumidor encargado de comprobar palindromos.
void *consumidor1 (void *arg){
	
	//Declaración de variables locales.
	char ** argumentos = (char**) arg;
	int tam;
	char palabra[20];
	
	//Inicialización de variables locales.
	tam = atoi(argumentos[1]);
	
	//Bucle infinito del hilo:
	while(1){

		//Esperamos a que la variable global i quede libre.	
		sem_wait(&mutexI);
		
		//Si encontramos "FIN" en la posición actual de buffer uno:
		if(strcmp(buffer1[i],"FIN")==0){
			
			//Avisamos de que la variable global i queda libre.
			sem_post(&mutexI);

			//Esperamos a poder usar la variable global restCons.
			sem_wait(&mutexRest);

			//Si solo queda un consumidor uno:
			if(restCons==1){
	
				//Esperamos a que la variable global j quede libre.
				sem_wait(&mutexJ);
			
				//Esperamos a que haya espacio en buffer dos.
				sem_wait(&espacioB2);

				//Escribimos "FIN" en la posición actual del buffer dos.
				sprintf(buffer2[j],"FIN");
		
				//Avisamos de que hay dato en el buffer dos.
				sem_post(&datoB2);
		
				//Avisamos de que la variable global j queda libre.
				sem_post(&mutexJ);			
			}
			//Decrementamos la variable global restCons.
			restCons--;
	
			//Avisamos de que la variable global restCons queda libre.
			sem_post(&mutexRest);

			//Avisamos de que hay espacio en buffer dos.
			sem_post(&espacioB2);

			//Finalizamos el hilo.
			pthread_exit(0);
		}
		//Si no encontramos "FIN" en la posición actual:
		//Esperamos a que haya dato en buffer uno.
		sem_wait(&datoB1);

		//Si la palabra en la posición actual del buffer uno es palíndroma o no:
		if(palindromo(buffer1[i])){

			//Escribimos en la variable local "palabra" la palabra del buffer uno seguida de si.
			sprintf(palabra,"%s si", buffer1[i]);	
		}else{

			//Escribimos en la variable local "palabra" la palabra del buffer uno seguida de no.
			sprintf(palabra,"%s no", buffer1[i]);
		}
		//Avisamos de que hay espacio en el buffer uno.
		sem_post(&espacioB1);

		//Incrementamos la i de manera circular en función del tamaño del buffer uno.
		i=(i+1)%tam;

		//Avisamos de que la variable global i queda libre.
		sem_post(&mutexI);

		//Esperamos a que la variable global j quede libre.
		sem_wait(&mutexJ);

		//Esperamos a que haya espacio en el buffer dos.
		sem_wait(&espacioB2);

		//Escribimos en la posición actual de buffer dos la variable local "palabra".
		sprintf(buffer2[j],"%s",palabra);
		
		//Avisamos de que hay dato en el buffer dos.
		sem_post(&datoB2);

		//Incrementamos la variable global j de manera circular en función del tamaño del buffer dos.
		j=(j+1)%5;

		//Avisamos de que la variable global j queda libre.
		sem_post(&mutexJ);
	}
}

//Inicialización del hilo consumidor dos
void *consumidor2 (){
	
	//Declaración de variables locales.
        int x;
	FILE *archSalida;
	
	//Inicialización de variables locales.
	//La variable x es el indice para recorrer el buffer dos.
	x=0;

	//Abrimos el archivo de salida y comprobamos que no se han producido errores.
	archSalida = fopen("archivo_Salida.txt","w");
	if (archSalida==NULL) {
		fprintf(stderr,"*** ERROR al abrir el archivo de salida\n");
		exit (1);
	}
	//Bucle infinito del hilo:
	while(1){
		
		//Esperamos a que haya dato en buffer dos.
                sem_wait(&datoB2);

		//Si encontramos "FIN" en la posición actual del buffer dos:
                if(strcmp(buffer2[x],"FIN")==0){
			
			//Cerramos el archivo de salida y finalizamos el hilo.
			fclose(archSalida);
                        pthread_exit(0);
                }
		//Si no:
		//Escribimos en el archivo de salida "La palabra "POSICION ACTUAL BUFFER DOS" es un palíndromo"
		fprintf(archSalida,"La palabra %s es un palindromo.\n",buffer2[x]);
		
		//Avisamos de que hay espacio en el buffer dos.
                sem_post(&espacioB2);

		//Incrementamos la variable local x de manera circular en función del tamaño del buffer dos.
                x=(x+1)%5;
        }
}

//Programa principal.
int main(int argc, char *argv[]){

	//Declaración de variables globales.
	int *b1, tam, numCons, x, arg1;
	char arg2;

	//Comprobamos que la cantidad de argumentos recibida es la correcta.
	if (argc != 4){
		fprintf(stderr,"*** ERROR numero incorrecto de argumentos, se necesita: ./[string(nombre del programa)][int(tamaño de buffer uno)] [string(Nombre fichero de entrada)] [int(Numero de consumidores)]\n");
		exit(1);
	}
	
	//Comprobamos que el tipo de argumentos introducidos son correctos.
	if ( sscanf(argv[1], "%i", &arg1) != 1){
  		fprintf(stderr,"*** ERROR en la lectura del primer argumento. Valor leido: %s.\n    Se necesita un [int(Tamaño de buffer uno].\n", argv[1]);
  		exit(1);
  	}
	if ( sscanf(argv[2], "%s", &arg2) != 1){
  		fprintf(stderr,"*** ERROR en la lectura del segundo argumento. Valor leido: %s.\n    Se necesita un [string(Nomber fichero de entrada].\n", argv[2]);
  		exit(1);
  	}
	if ( sscanf(argv[3], "%i", &arg1) != 1){
  		fprintf(stderr,"*** ERROR en la lectura del tercer argumento. Valor leido: %s.\n    Se necesita un [int(Numero de consumidores)].\n", argv[3]);
  		exit(1);
  	}

	//Inicialización de variables globales.
	x=0;
	tam = atoi( argv[1]);
	numCons = atoi(argv[3]);
	restCons = numCons;

	//Inicialización de los semáforos:
	//El semáforo datoB1 a 0 indicando que el buffer uno está vacío al comenzar el programa.
	sem_init(&datoB1,0,0);
	
	//El semáforo espacioB1 con tamaño del buffer uno, indicando los huecos disponibles al comenzar el programa.
	sem_init(&espacioB1,0,tam);

	//El semáforo datoB2 a 0 indicando que el buffer dos está vacio al comenzar el programa.
	sem_init(&datoB2,0,0);

	//El semáforo espacioB2 a 5 (tamaño del buffer dos), indicando los huecos disponibles al comenzar el programa.
	sem_init(&espacioB2,0,5);

	//El semáforo mutexI a 1, indicando que la variable global i solo puede ser usada por un hilo a la vez.
	sem_init(&mutexI,0,1);

	//El semáforo mutexJ a 1, indicando que la variable global j solo puede ser usada por un hilo a la vez.
	sem_init(&mutexJ,0,1);

	//El semáforo mutexRest a 1, indicando que la variable global restCons solo puede ser usada por un hilo a la vez.
	sem_init(&mutexRest,0,1);
	
	//Reservamos memoria de manera dinámica para el buffer uno.
	if((buffer1=(char**)malloc(tam*sizeof(char*)))==NULL){
		fprintf(stderr,"*** ERROR al reservar memoria\n");
		return 1;
	}

	for(x=0;x<tam;x++){
		if((buffer1[x]=(char*)malloc(11*sizeof(char)))==NULL){
			fprintf(stderr,"*** ERROR al reservar memoria\n");
			return 1;
		}
	}
	//Declaración de los procesos que vamos a utilizar.
	pthread_t tid[2+numCons];
	
	//Creamos el hilo productor.
	pthread_create(&tid[0], NULL, productor, (void*) argv);

	//Creamos el hilo consumidor dos.
	pthread_create(&tid[1], NULL, consumidor2, (void*) NULL);
	
	//Creamos los hilos consumidor uno deseados.
	for(x=2;x<numCons+2;x++)
		pthread_create(&tid[x], NULL, consumidor1, (void*) argv);

	//Esperamos a que finalicen todos los hilos creados.
	for(x=0;x<numCons+2;x++)
		pthread_join(tid[x], NULL);
	
	//Finalizamos todos los semáforos que hemos utilizado.
	sem_destroy(&datoB1);
	sem_destroy(&espacioB1);
	sem_destroy(&datoB2);
	sem_destroy(&espacioB2);
	sem_destroy(&mutexI);
	sem_destroy(&mutexJ);
	sem_destroy(&mutexRest);
	
	//Terminamos el programa principal.
	return 0;
}
