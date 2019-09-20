/*
* Archivo:	fuente2.c
* Autor:	MISRRAIM SUÁREZ PÉREZ
* Mail:		misrraimsp@gmail.com
* Revisión:	09/01/2017
*
* Código del proceso P2 y del lanzamiento del proceso P3
*/



/******************************************* DIRECTIVAS
*************************************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#define MAX 256 // máximo tamaño del mensaje


main() {

/******************************************* DECLARACIONES
*************************************************************************************************************/
	int pid;				// identificador de proceso
	int shmid;				// identificador de la memoria compartida
	int semid;				// identificador del semáforo
	char mensaje[MAX];			// buffer para el mensaje
	int fifo_fd;				// file descriptor del fichero FIFO
	key_t llave_shm, llave_sem;		// llaves de los IPC's
	char *vc1;				// variable asociada a la memoria compartida
	struct sembuf sops[1];			// estructura de operaciones en semáforo


/******************************************* PROCESO P2
*************************************************************************************************************/
	
	//código de sobrecarga para probar las estadísticas
	//DESCOMENTAR PARA TESTAR
	/*
	int i;
	char buf[100];
	for (i = 0; i < 10000000; i++) {
		sprintf(buf, "%d", getpid());
	}
	*/
	
	//abre fichero FIFO fichero1
	fifo_fd = open("fichero1", O_RDONLY);
	if (fifo_fd == -1) {
		perror("P2 (Ej2): open_fifo");
		exit(-1);
	}
	
	//lee mensaje en fichero
	if (read(fifo_fd, mensaje, MAX) == -1) {
		perror("P2: read_fifo");
		exit(-1);
	}
	printf("El proceso P2 (PID=%d. Ej2) recibe un mensaje de sí mismo por un fichero FIFO\n",
		getpid());
	fflush(stdout);
	
	//cerrar fichero FIFO
	if (close(fifo_fd) == -1) {
		perror("P2: close_fifo");
		exit(-1);
	}
	
	//crea llave para memoria compartida
	llave_shm = ftok("fichero1", 'M');
	if (llave_shm == (key_t) -1) {
		perror("P2: ftok_shm");
		exit(-1);
	}
	
	//crea region de memoria compartida
	shmid = shmget(llave_shm, MAX, 0600 | IPC_CREAT);
	if (shmid == -1) {
		perror("P2: shmget");
		exit(-1);
	}
	
	//adjunta el nuevo segmento al espacio del proceso P2
	vc1 = shmat(shmid, NULL, 0);
	if (vc1 == (void *) -1) {
		perror("P2: shmat");
		exit(-1);
	}
	
	//crea llave para semaforo
	llave_sem = ftok("fichero1", 'S');
	if (llave_sem == (key_t) -1) {
		perror("P2: ftok_sem");
		exit(-1);
	}
	
	//crea semáforo
	semid = semget(llave_sem, 1, 0600 | IPC_CREAT);
	if (semid == -1) {
		perror("P2: semget");
		exit(-1);
	}
	
	//inicializa el semaforo a 0
	if (semctl(semid, 0, SETVAL, 0) == -1) {
		perror("P2: semctl");
		exit(-1);
	}
	
	// Crear proceso hijo P3
	pid = fork();
	if (pid == -1) {
		perror("P2: fork P3");
		exit(-1);
	}
	
/******************************************* PROCESO HIJO (P3)
*************************************************************************************************************/
	if (pid == 0) { 
		//ejecutar Ej3
		if (execl("Ej3", "Ej3", (char *) NULL) == -1) {
			perror("P3: exec_Ej3");
			exit(-1);
		}
	}
	
/******************************************* PROCESO PADRE (P2)
*************************************************************************************************************/
	else { 
		//esperar un segundo
		sleep(1);
		
		//copia el mensaje en la variable compartida
		if(strcpy(vc1, mensaje) == NULL) {
			perror("P2: strcpy");
			exit(-1);
		}
		printf("El proceso P2 (PID=%d. Ej2) transmite un mensaje al proceso P3 por una región de memoria compartida\n",
			getpid());
		fflush(stdout);
		
		//desbloquea el semaforo
		sops[0].sem_num = 0;
		sops[0].sem_op = 1;
		sops[0].sem_flg = 0;
		if (semop(semid, sops, 1) == -1) {
			perror("P2: semop(+1)");
			exit(-1);
		}
		
		//espera que P3 esté zombie para recolectar sus estadísticas
		wait();
		
		//suspende ejecucion
		pause();
	}
}
