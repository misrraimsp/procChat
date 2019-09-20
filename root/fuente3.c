/*
* archivo:	fuente3.c
* Autor:	MISRRAIM SUÁREZ PÉREZ
* mail:		misrraimsp@gmail.com
* revisión:	09/01/2017
*
* Código del proceso P3
*/



/******************************************* DIRECTIVAS
*************************************************************************************************************/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define MAX 256 // máximo tamaño del mensaje


main() {

/******************************************* DECLARACIONES
*************************************************************************************************************/
	int shmid;					// identificador de la memoria compartida
	int semid;					// identificador del semáforo
	int msqid;					// identificador de la cola de mensajes
	int p3id;					// identificador del proceso
	key_t llave_shm, llave_sem, llave_msq;		// llaves de los IPC's
	char *vc1;					// variable asociada a la memoria compartida
	char mensaje[MAX];				// buffer para el mensaje
	struct sembuf sops[1];				// estructura de operaciones en semáforo
	struct {
		int tipo;
		char mtext[MAX];
	} msg;						// cuerpo de los mensajes


/******************************************* PROCESO P3
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
	
	//crea llave para memoria compartida
	llave_shm = ftok("fichero1", 'M');
	if (llave_shm == (key_t) -1) {
		perror("P3: ftok_shm");
		exit(-1);
	}
	
	//cargar region de memoria compartida
	shmid = shmget(llave_shm, MAX, 0600);
	if (shmid == -1) {
		perror("P3: shmget");
		exit(-1);
	}
	
	//adjunta el nuevo segmento al espacio del proceso P3
	vc1 = shmat(shmid, NULL, 0);
	if (vc1 == (void *) -1) {
		perror("P3: shmat");
		exit(-1);
	}
	
	//crea llave para semaforo
	llave_sem = ftok("fichero1", 'S');
	if (llave_sem == (key_t) -1) {
		perror("P3: ftok_sem");
		exit(-1);
	}
	
	//crea semáforo
	semid = semget(llave_sem, 0, 0600);
	if (semid == -1) {
		perror("P3: semget");
		exit(-1);
	}
	
	//solicita decrementar el semaforo. Se bloquea mientras tanto
	sops[0].sem_num = 0;
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;
	if (semop(semid, sops, 1) == -1) {
		perror("P3: semop (-1)");
		exit(-1);
	}
	
	//lee región de memoria compartida
	if(strcpy(mensaje, vc1) == NULL) {
		perror("P3: strcpy");
		exit(-1);
	}
	printf("El proceso P3 (PID=%d. Ej3) recibe un mensaje del proceso P2 por una región de memoria compartida\n",
		getpid());
	fflush(stdout);
	
	//muestra mensaje por pantalla
	if (printf("Muestra del mensaje: \"%s\"\n", mensaje) < 0) {
		perror("P3: printf");
		exit(-1);
	}
	
	//crear la llave de la cola de mensajes
	llave_msq = ftok("Ej1", 'Q');
	if (llave_msq == (key_t) -1) {
		perror("P3: ftok_msq");
		exit(-1);
	}
	
	//cargar cola de mensajes
	msqid = msgget(llave_msq, 0600);
	if (msqid == -1) {
		perror("P3: msgget");
		exit(-1);
	}
	
	//guardar pid de P3
	if (sprintf(msg.mtext, "%d", getpid()) < 0) {
		perror("P3: sprintf");
		exit(-1);
	}
	
	//envia mensaje a P1 con su PID
	msg.tipo = 1;
	printf("El proceso P3 (PID=%d. Ej3) transmite un mensaje al proceso P1 por una cola de mensajes\n",
		getpid());
	fflush(stdout);
	if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
		perror("P3: msgsnd");
		exit(-1);
	}
	
	//suspende ejecución
	pause();
}
