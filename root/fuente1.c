/*
* Archivo:	fuente1.c
* Autor:	MISRRAIM SUÁREZ PÉREZ
* Mail:		misrraimsp@gmail.com
* Revisión:	09/01/2017
*
* Código del proceso P1 y del inicio del proceso P2
*/



/******************************************* DIRECTIVAS
*************************************************************************************************************/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/times.h>
#define MAX 256 // máximo tamaño del mensaje


main() {

/******************************************* DECLARACIONES
*************************************************************************************************************/
	int tuberia[2];				// file descriptors de la tuberia
	int pid;				// identificador de proceso
	int msqid;				// identificador de la cola de mensajes
	int fd_fifo;				// identificador del fichero fifo
	char mensaje[MAX];			// array donde almacenar el mensaje
	key_t llave_msq;			// llave de la cola de mensajes
	struct {
		int tipo;
		char mtext[MAX];
	} msg;					// cuerpo de los mensajes
	struct tms tiempos_i, tiempos_f;	// estructura de la info de tiempos
	clock_t total_i, total_f;		// tiempo transcurrido desde EPOCH
	float total, p1_u_p, p1_s_p, p123_u_p,
	p123_s_p, p1_u, p1_s, p123_u, p123_s;	// tiempos para estadísticas
	float CLK_TCK;				// clock tics per second


/******************************************* PROCESO P1
*************************************************************************************************************/

	//obtiene tiempo inicial para estadísticas
	total_i = times(&tiempos_i);
	if (total_i == -1) {
		perror("P1: times_i");
		exit(-1);
	}
	
	//código de sobrecarga para probar las estadísticas
	//DESCOMENTAR PARA TESTAR
	/*
	int i;
	char buf[100];
	for (i = 0; i < 10000000; i++) {
		sprintf(buf, "%d", getpid());
	}
	*/
	
	// Crear la tubería
	if (pipe(tuberia) == -1) {
		perror("P1: pipe");
		exit(-1);
	}
	
	// Crear proceso hijo P2
	pid = fork();
	if (pid == -1) {
		perror("P1: fork P2");
		exit(-1);
	}
	
/******************************************* PROCESO HIJO (P2)
*************************************************************************************************************/
	if (pid == 0) {
		//leer mensaje de la tubería
		if (read(tuberia[0], mensaje, MAX) == -1) {
			perror("P2: read_pipe");
			exit(-1);
		}
		printf("El proceso P2 (PID=%d. Ej1) recibe un mensaje del proceso P1 por una tubería sin nombre\n",
			getpid());
		fflush(stdout);
		
		//cerrar tuberia
		if (close(tuberia[0]) == -1) {
			perror("P2: close tuberia[0]");
			exit(-1);
		}
		if (close(tuberia[1]) == -1) {
			perror("P2: close tuberia[1]");
			exit(-1);
		}
	
		//crear fichero FIFO
		if (mkfifo("fichero1", 0600) == -1) {
			perror("P2: mkfifo");
			exit(-1);
		}
		
		//abrir fichero FIFO	
		fd_fifo = open("fichero1", O_RDWR);
		if (fd_fifo == -1) {
			perror("P2 (Ej1): open_fifo");
			exit(-1);
		}
			
		//escribir mensaje en fichero FIFO
		if (write(fd_fifo, mensaje, strlen(mensaje) + 1) == -1) {
			perror("P2: write_fifo");
			exit(-1);
		}
		printf("El proceso P2 (PID=%d. Ej1) se transmite un mensaje a sí mismo por un fichero FIFO\n",
			getpid());
		fflush(stdout);
		
		//ejecutar Ej2
		if (execl("Ej2", "Ej2", (char *) NULL) == -1) {
			perror("P2: exec_Ej2");
			exit(-1);
		}
	}
/******************************************* PROCESO PADRE (P1)
*************************************************************************************************************/
	else {
		//leer mensaje desde el usuario
		if (printf("Introducir mensaje (max 256): ") < 0) {
			perror("P1: printf");
			exit(-1);
		}
		if (scanf("%s", mensaje) != 1) {
			perror("P1: scanf");
			exit(-1);
		}
		
		//colocar mensaje en la tubería
		if (write(tuberia[1], mensaje, strlen(mensaje) + 1) == -1) {
			perror("P1: write_pipe");
			exit(-1);
		}
		printf("El proceso P1 (PID=%d. Ej1) transmite un mensaje al proceso P2 por una tubería sin nombre\n",
			getpid());
		fflush(stdout);
			
		//cerrar tuberia
		if (close(tuberia[0]) == -1) {
			perror("P1: close tuberia[0]");
			exit(-1);
		}
		if (close(tuberia[1]) == -1) {
			perror("P1: close tuberia[1]");
			exit(-1);
		}
		
		//crear la llave de la cola de mensajes
		llave_msq = ftok("Ej1", 'Q');
		if (llave_msq == (key_t) -1) {
			perror("P1: ftok_msq");
			exit(-1);
		}
		
		//crear la cola de mensajes
		msqid = msgget(llave_msq, 0600 | IPC_CREAT);
		if (msqid == -1) {
			perror("P1: msgget");
			exit(-1);
		}
			
		//esperar hasta recibir el mensaje
		if (msgrcv(msqid, &msg, MAX, 1, 0) == -1) {
			perror("P1: msgrcv");
			exit(-1);
		}
		printf("El proceso P1 (PID=%d. Ej1) recibe un mensaje del proceso P3 por una cola de mensajes\n",
			getpid());
		fflush(stdout);
		
		//envia señal sigkill a P3
		if (kill(atoi(msg.mtext), SIGKILL) == -1) {
			perror("P1: kill_P3");
			exit(-1);
		}
		
		//doy tiempo a P2 para que sea planificado y entre en pausa
		sleep(1);
			
		//envia señal sigkill a P2
		if (kill(pid, SIGKILL) == -1) {
			perror("P1: kill_P2");
			exit(-1);
		}
		
		//espera que P2 esté zombie para recolectar sus estadísticas
		wait();
			
		//borra fichero1
		if (remove("fichero1") == -1) {
			perror("P1: remove");
			exit(-1);
		}
		
		//obtiene tiempo final para estadísticas
		total_f = times(&tiempos_f);
		if (total_f == -1) {
			perror("P1: times_f");
			exit(-1);
		}
		
		//calcular estadisticas
		CLK_TCK = (float)sysconf(_SC_CLK_TCK);
		total = (total_f - total_i) / CLK_TCK;
		
		p1_u = (tiempos_f.tms_utime - tiempos_i.tms_utime) / CLK_TCK;
		p1_u_p = 100 * p1_u / total;
		p1_s = (tiempos_f.tms_stime - tiempos_i.tms_stime) / CLK_TCK;
		p1_s_p = 100 * p1_s / total;
		
		p123_u = p1_u + (tiempos_f.tms_cutime - tiempos_i.tms_cutime) / CLK_TCK;
		p123_u_p = 100 * p123_u / total;
		p123_s = p1_s + (tiempos_f.tms_cstime - tiempos_i.tms_cstime) / CLK_TCK;
		p123_s_p = 100 * p123_s / total;
		
		//mostrar estadísticas
		printf("\n\n\n\n***** ESTADÍSTICAS *****\n\n");
		printf("Tiempo total transcurrido: %g segundos\n", total);
		
		printf("Tiempo de P1 en modo Usuario: %g ms (%g%%)\n", p1_u * 1000, p1_u_p);
		printf("Tiempo de P1 en modo Kernel: %g ms (%g%%)\n", p1_s * 1000, p1_s_p);
		printf("Uso CPU de P1: %g%%\n", p1_u_p + p1_s_p);
		
		printf("Tiempo de {P1 + P2 + P3} en modo Usuario: %g ms (%g%%)\n", p123_u * 1000, p123_u_p);
		printf("Tiempo de {P1 + P2 + P3} en modo Kernel: %g ms (%g%%)\n", p123_s * 1000, p123_s_p);
		printf("Uso CPU de {P1 + P2 + P3}: %g%%\n", p123_u_p + p123_s_p);
	}
}
