//From https://www.geeksforgeeks.org/posix-shared-memory-api/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <semaphore.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <sys/wait.h> 
#include "syslog.h"

typedef struct {
	uint8_t data0;
	uint8_t data1;
	char* firstname;
	char* lastname;
} number;

char *prod1_semaphore = "producer1_sem_main";
char *prod2_semaphore = "producer2_sem_main";
char *cons_semaphore = "consumer_sem_main";

int producer1() 
{   
	syslog(LOG_INFO,"Message from PRODUCER 1");
	sem_t* producer1_sem;
	number prod1 = {1,2,"Embedded","Systems"};

	number *prod1_ptr = &prod1;

	/* shared memory file descriptor */
	int file_share; 

	/* pointer to shared memory obect */
	number *ptr = NULL; 

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);

	/* memory map the shared memory object */
	ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0); 

	producer1_sem = sem_open(prod1_semaphore,0,0666,0);
	printf("Before wait producer 1\n");
	//sem_wait(producer1_sem);
	printf("After wait producer 1\n");
	memcpy((void *)(&ptr[0]),(void*)prod1_ptr,sizeof(number));
	//sem_post(producer1_sem);
	sem_close(producer1_sem);
	close(file_share);

	return 0; 
} 

int producer2()
{
	syslog(LOG_INFO,"Message from PRODUCER 2");
	sem_t* producer2_sem;
	/* strings written to shared memory */

	number prod2 = {3,4,"Boulder","Colorado"};

	number *prod2_ptr = &prod2;

	/* shared memory file descriptor */
	int file_share;

	/* pointer to shared memory obect */
	number *ptr = NULL;

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);

	/* memory map the shared memory object */
	ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0);

	producer2_sem = sem_open(prod2_semaphore,0,0666,0);
	printf("Before wait producer 2\n");
	//sem_wait(producer2_sem);
	printf("After wait producer 2\n");
	memcpy((void *)(&ptr[1]),(void*)prod2_ptr,sizeof(number));
	//sem_post(producer2_sem);	
	sem_close(producer2_sem);
	close(file_share);

	return 0;
}

int consumer()
{
	syslog(LOG_INFO,"Message from CONSUMER");
	sem_t* consumer_sem;
	
	number cons;
	number *cons_ptr = &cons;

	/* shared memory file descriptor */
	int file_share;

	/* pointer to shared memory obect */
	number *ptr = NULL;

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);
	/* memory map the shared memory object */
	ptr = (number *)mmap(0, sizeof(number), PROT_READ, MAP_SHARED, file_share, 0);
	
	consumer_sem = sem_open(cons_semaphore,0,0666,0);
	sem_post(consumer_sem);
	printf("Before wait consumer\n");
	//sem_wait(consumer_sem);
	printf("After wait consumer\n");
	sem_wait(consumer_sem);	
	memcpy((void*)cons_ptr,(void*)(&ptr[0]),sizeof(number));
	memcpy((void*)cons_ptr,(void*)(&ptr[1]),sizeof(number));
	sem_post(consumer_sem);
	/* read from the shared memory object */ 
	printf("%d\n", ptr[0].data0);   
	printf("%d\n", ptr[0].data1);
	printf("%d\n", ptr[1].data0);
	printf("%d\n", ptr[1].data1);
	printf("%s\n", ptr[0].firstname);
	printf("%s\n", ptr[0].lastname);
	printf("%s\n", ptr[1].firstname);
	printf("%s\n", ptr[1].lastname);
	//sem_post(consumer_sem);
	sem_close(consumer_sem);
	/* remove the shared memory object */
	shm_unlink("Trial_Share");
	munmap(ptr,sizeof(number));
	printf("Exiting consumer\n");
	return 0;
}

int main(void)
{
	sem_t *main_sem;
	pid_t process_id = 0;
	pid_t sid = 0;
	pid_t cid = 0;
	int status = 0;

	openlog("SharedMemory",LOG_PID|LOG_CONS,LOG_USER);
	syslog(LOG_INFO,"Message from MAIN");

	//int ret;
	main_sem = sem_open(prod1_semaphore, O_CREAT, 0600, 0);
	sem_close(main_sem);	
	main_sem = sem_open(prod2_semaphore, O_CREAT, 0600, 0);
	sem_close(main_sem);	
	main_sem = sem_open(cons_semaphore, O_CREAT, 0600, 0);
	sem_close(main_sem);
	int file_share = shm_open("Trial_Share",O_CREAT | O_RDWR, 0666);
	if(file_share < 0)
	{ 
		printf("SHM OPEN Error\n"); 
	}

	ftruncate(file_share, 4096);
	
	if (close(file_share) < 0) 
	{ 
		printf("FILE CLOSE ERROR\n"); 
	}

	/*printf("Creating child process\n");
	// Create child process
	process_id = fork();
	printf("Child process is  = %d\n",process_id);
	
	//cid = waitpid(process_id,&status,0);
	cid = wait(&status);	
	printf("After wait for PID %d\n",cid);

	
	ret = kill(process_id,0);
	printf("Kill returned %d\n", ret);

	// Indication of fork() failure
	if (process_id < 0)
	{
		// Return failure in exit status
		exit(1);
	}
	// PARENT PROCESS. Need to kill it.
	if (process_id > 0)
	{
		exit(0);
	}
	//unmask the file mode
	umask(0);
	printf("IN CHILD\n");
	

	//set new session
	sid = setsid();
	if(sid < 0)
	{
		printf("SID is less than 0\n");
		// Return failure
		exit(1);
	}
	// Change the current working directory to root.
	chdir("/");*/

        producer1();

printf("Creating child process\n");
	// Create child process
	process_id = fork();
	printf("Child process is  = %d\n",process_id);
	
	//cid = waitpid(process_id,&status,0);
	cid = wait(&status);	
	printf("After wait for PID %d\n",cid);

	
	//ret = kill(process_id,0);
	//printf("Kill returned %d\n", ret);

	// Indication of fork() failure
	if (process_id < 0)
	{
		// Return failure in exit status
		exit(1);
	}
	// PARENT PROCESS. Need to kill it.
	if (process_id > 0)
	{
		exit(0);
	}
	//unmask the file mode
	umask(0);
	printf("IN CHILD\n");
	

	//set new session
	sid = setsid();
	if(sid < 0)
	{
		printf("SID is less than 0\n");
		// Return failure
		exit(1);
	}
	// Change the current working directory to root.
	chdir("/");

    	producer2();
	consumer();
	
	sem_unlink(prod1_semaphore);
	sem_unlink(prod2_semaphore);
	sem_unlink(cons_semaphore);
	printf("Semaphore unlinked!\n");

	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

        return 0;
}
