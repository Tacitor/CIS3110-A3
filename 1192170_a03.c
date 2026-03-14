#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>

#define FILE_DELIMETERS ";\r\n"

void logStart(char* tID);//function to log that a new thread is started
void logFinish(char* tID);//function to log that a thread has finished its time

void startClock();//function to start program clock
long getCurrentTime();//function to check current time since clock was started
time_t programClock;//the global timer/clock for the program

sem_t sem_even;
sem_t sem_odd;

enum State {
	NEW=0,
	STARTED,
	TERMINATED
} State;

typedef struct thread //represents a single thread, you can add more members if required
{
	char tid[4];//id of the thread as read from file, set in readFile() for you
	unsigned int startTime;//start time of thread as read from file, set in readFile() for you
	int state;//you can use it as per your desire
	pthread_t handle;//you can use it as per your desire
	int retVal;//you can use it as per your desire

	sem_t* sem_pend; // The sem that this thread waits for
	sem_t* sem_post; // The sem that this thread will post when done
	int id_y_part;
} Thread;

typedef struct NextThread {
	int odd_index;
	int even_index; //TODO: Remove this?

	int start_index_count;
	int* start_indices;
	int live_thread_count;
} NextThread;

//you can add more functions here if required
void preProcessThreads(Thread* threads, int t_count);
NextThread getNextThread(Thread* threads, int t_count, time_t currentTime);

void* threadRun(void* t);//the thread function, the code executed by each thread
int readFile(char* fileName, Thread** threads);//function to read the file content and build array of threads

int main(int argc, char *argv[])
{
    //you can add some suitable code anywhere in main() if required

	Thread* threads = NULL;//This is your list of threads, use it in suitable way; remove the comment when ready to use
	int threadCount = -1;
	int threads_ptr_len = -1;
	bool started = false;

	//input file must be accepted as command line argument. You can write the suitable code here to check
	//command line arguments and read the content of the file using readFile().

	// Guarding for CLI args. Causes early return and program termination.
	if (argc < 2) {
        printf("ERROR: You must provide an input file as a CLI argument. Program Terminating.\n");
        return 0;
    } else if (argc > 2) {
        printf("ERROR: You must provide ONLY an input file as a CLI argument. No other flags or arguments are allowed. Program Terminating.\n");
        return 0;
    }

	if (sem_init(&sem_even, 0, 0) != 0) {
		printf("sem_init on sem_even failed");
		return 0;
	}

	if (sem_init(&sem_odd, 0, 0) != 0) {
		printf("sem_init on sem_odd failed");
		return 0;
	}

	threads_ptr_len = threadCount = readFile(argv[1], &threads);
	preProcessThreads(threads, threads_ptr_len);

	startClock();
	time_t currentTime = -1;
	NextThread next;

    //write some suitable code here to initiate, progress and terminate the threads following the requirements

	while (threadCount > 0) {
		if(getCurrentTime() == currentTime + 1)//this condition simulates the clock ticks and calls scheduler whenever local clock time progresses by 1
		{
			currentTime++;
			
			next = getNextThread(threads, threads_ptr_len, currentTime);
			//printf("Next even: %d, odd: %d, start_count: %d\n", next.even_index, next.odd_index, next.start_index_count);

			for (int i = 0; i < next.start_index_count; i++) {
				Thread *t = &(threads[next.start_indices[i]]);
				pthread_create(&(t->handle), NULL, threadRun, t);

				if (!started) {
					// The first thread, in terms of creation time, enters first in its critical section.
					// TODO: What if two threads have the same creation time?
					sem_post(threads[next.start_indices[0]].sem_pend);
					started = true;
				}
			}

			free(next.start_indices);

			threadCount = next.live_thread_count;
			//printf("threadCount: %d\n", threadCount);
		}
	}

	// Don't terminate until all the threads are done
	for (int i = 0; i < threads_ptr_len; i++) {
		pthread_join(threads[i].handle, NULL);
	}

	free(threads);

	return threadCount;
}

void preProcessThreads(Thread* threads, int t_count) {

	for (int i = 0; i < t_count; i++) {
		threads[i].id_y_part = threads[i].tid[2] - 48; //Convert from ascii single digit to decimal value

		if (threads[i].id_y_part % 2 == 0) {
			threads[i].sem_pend = &sem_even;
			threads[i].sem_post = &sem_odd;
			//printf("Thread id: %s, id_y_part %d, is EVEN\n", threads[i].tid, threads[i].id_y_part);
		} else {
			threads[i].sem_pend = &sem_odd;
			threads[i].sem_post = &sem_even;
			//printf("Thread id: %s, id_y_part %d, is ODD\n", threads[i].tid, threads[i].id_y_part);
		}
	}
}

NextThread getNextThread(Thread* threads, int t_count, time_t currentTime) {
	NextThread next;
	next.odd_index = -1;
	next.even_index = -1; //TODO: Remove this?
	next.start_index_count = 0;
	next.start_indices = NULL;
	next.live_thread_count = 0;

	for (int i = 0; i < t_count; i++) {
		if (threads[i].state != TERMINATED) {
			next.live_thread_count++;
		}

		if (threads[i].state == NEW && currentTime == (time_t)threads[i].startTime) {
			// Enlarge array by 1
			next.start_indices = realloc(next.start_indices, sizeof(int) * (next.start_index_count + 1));

			// TODO: If two threads have the same start time does it matter which one gets the logStart() called first?
			next.start_indices[next.start_index_count] = i;
			next.start_index_count++;
		}

		// TODO: Add starvation detection
	}

	return next;
}

int readFile(char* fileName, Thread** threads)//implement this method as per your desire to read the thread information from file
{
	int returnSuccess;
	long fileCharNum;
	char *threadString;

	FILE *inputFile = fopen(fileName, "r");

	if (inputFile == NULL) {
        printf("ERROR: Could not open file %s. Program Terminating.\n", fileName);
        return 0;
    }

	returnSuccess = fseek(inputFile, 0, SEEK_END);

    // Guarding for moving file pointer to end of file so its size can be found. Causes early return and program termination.
    if (returnSuccess !=0 ) {
        printf("ERROR: Could not seek to end of %s. Program Terminating.\n", fileName);
        return 0;
    }

    fileCharNum = ftell(inputFile);

	if (-1 == fileCharNum) {
        printf("ERROR: Could not tell the length of file %s. Program Terminating.\n", fileName);
        return 0;
    }

    // Guarding for moving file pointer back to the start of file so can now be read into mem. Causes early return and program termination.
    returnSuccess = fseek(inputFile, 0, SEEK_SET);
    
    if (returnSuccess !=0 ) {
        printf("ERROR: Could not seek back to start of %s. Program Terminating.\n", fileName);
        return 0;
    }

    threadString = calloc((fileCharNum + 1), sizeof(char));

	if (NULL == threadString) {
        printf("ERROR: Could not allocate enough memory to fit %ld characters from %s. Program Terminating.\n", fileCharNum, fileName);
        return 0;
    }

    size_t elmtRead = fread(threadString, sizeof(char), fileCharNum, inputFile);
    returnSuccess = fclose(inputFile);

	if (returnSuccess !=0 ) {
        printf("ERROR: Could not close the file %s. Program Terminating.\n", fileName);
        return 0;
    }

    if (((size_t)fileCharNum) != elmtRead) {
        printf("ERROR: Bad read from %s. Size read does not match size of file. Program Terminating.\n", fileName);
        return 0;
    }

	// Split the string on using the '\n' or '\r' or ';' deliminator
	char *tok = strtok(threadString, FILE_DELIMETERS);
	int parseCount = 0;
	int tempThreadCount = 0;

	while (NULL != tok) {
		if (0 == parseCount) {
			// Enlarge array by 1
			*threads = realloc(*threads, sizeof(Thread) * (tempThreadCount + 1));

			// Take the first (and only) three chars of the thread number plus the null terminator
			strncpy((*threads)[tempThreadCount].tid, tok, 4);
			(*threads)[tempThreadCount].state = NEW;
			(*threads)[tempThreadCount].retVal = 0; //TODO: Figure out what this is good for
			(*threads)[tempThreadCount].handle = 0;

			(*threads)[tempThreadCount].sem_pend = NULL;
			(*threads)[tempThreadCount].id_y_part = -1;
			parseCount++;
		} else if (1 == parseCount) {
			// Try converting string to int
			int num = atoi(tok);

			(*threads)[tempThreadCount].startTime = num;
			parseCount = 0;
			tempThreadCount++;
		} else {
			printf("ERROR: You must provide input file in correct format.\n");
		}

		tok = strtok(NULL, FILE_DELIMETERS);
	}

	free(threadString);

	return tempThreadCount;
}

void logStart(char* tID)//do not change this method; you can use this method as per your desire
{
	printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char* tID)//do not change this method; you can use this method as per your desire
{
	printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

void* threadRun(void* t)//implement this function in a suitable way
{
	logStart(((Thread*)t)->tid);
	
//your entry section synchronization logic will appear here
	sem_wait(((Thread*)t)->sem_pend);

	//critical section starts here, it has only the following printf statement
	printf("[%ld] Thread %s is in its critical section\n",getCurrentTime(), ((Thread*)t)->tid);
	//critical section ends here

//your exit section synchronization logic will appear here
	sem_post(((Thread*)t)->sem_post);

	((Thread*)t)->state = TERMINATED;
	logFinish(((Thread*)t)->tid);
	return NULL;
}

void startClock()//do not change this method
{
	programClock = time(NULL);
}

long getCurrentTime()//invoke this method whenever you want to check how much time units passed
//since you invoked startClock()
{
	time_t now;
	now = time(NULL);
	return now-programClock;
}
