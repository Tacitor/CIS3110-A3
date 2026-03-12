#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>

void logStart(char* tID);//function to log that a new thread is started
void logFinish(char* tID);//function to log that a thread has finished its time

void startClock();//function to start program clock
long getCurrentTime();//function to check current time since clock was started
time_t programClock;//the global timer/clock for the program

typedef struct thread //represents a single thread, you can add more members if required
{
	char tid[4];//id of the thread as read from file, set in readFile() for you
	unsigned int startTime;//start time of thread as read from file, set in readFile() for you
	int state;//you can use it as per your desire
	pthread_t handle;//you can use it as per your desire
	int retVal;//you can use it as per your desire
} Thread;

//you can add more functions here if required

void* threadRun(void* t);//the thread function, the code executed by each thread
int readFile(char* fileName, Thread** threads);//function to read the file content and build array of threads

int main(int argc, char *argv[])
{

    //you can add some suitable code anywhere in main() if required

	//Thread* threads = NULL;//This is your list of threads, use it in suitable way; remove the comment when ready to use
	int threadCount = -1;

	//input file must be accepted as command line argument. You can write the suitable code here to check
	//command line arguments and read the content of the file using readFile().

	startClock();

    //write some suitable code here to initiate, progress and terminate the threads following the requirements

	return threadCount;
}

int readFile(char* fileName, Thread** threads)//implement this method as per your desire to read the thread information from file
{
	return 0;
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

	//critical section starts here, it has only the following printf statement
	printf("[%ld] Thread %s is in its critical section\n",getCurrentTime(), ((Thread*)t)->tid);
	//critical section ends here

//your exit section synchronization logic will appear here

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