/*  main.c  - main */

#include <xinu.h>

/* MAX_BUF_SIZE - Maximum size of the data buffer shared by producer-consumer */
#define MAX_BUF_SIZE 10
/* MAX_RAND_NO - Maximum value for random number generation */
#define MAX_RAND_NO 10000

/* Process Ids */
pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

/* Consumed count & Max number of consumed elements*i per run */
int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;

/* Circular data buffer and semaphore declarations */
struct data_buffer {
	int32 number;
	int32 square;
};

struct data_buffer data_buf[MAX_BUF_SIZE];
int32 head = 0;
int32 tail = 0;
sid32 mutex;                    // Mutex to handle critical section
int32 elements_count = 0;


/*------------------------------------------------------------------------------------
 * mutex_acquire - Wrapper function to call wait(acquire) on a semaphore
 *------------------------------------------------------------------------------------
 */
void mutex_acquire(sid32 mutex)
{
	/* */
	wait(mutex);
}

/*------------------------------------------------------------------------------------
 * mutex_release - Wrapper function to call signal(release) on a semaphore
 *------------------------------------------------------------------------------------
 */
void mutex_release(sid32 mutex)
{
	/* */
	signal(mutex);
}

/*------------------------------------------------------------------------------------
 * producer - Producer process which genrates a random number for consumer
 *------------------------------------------------------------------------------------
 */
process producer(void)
{
	int32 temp = 0;

	while(1) {
		mutex_acquire(mutex);
		if(tail == MAX_BUF_SIZE) {
			tail = tail % MAX_BUF_SIZE;
		}
		if(elements_count >= MAX_BUF_SIZE ) {
			printf("\nMutex released by producer");
			mutex_release(mutex);
			continue;
		}

		srand(clktime_ms);
		temp = rand() % MAX_RAND_NO;

		/* Introduce random sleep upto 10ms to simulate practical situation */
		sleepms(temp % 10);
		
		++elements_count;
		printf("\nProducer produced item : %d, tail=%d, elem_count=%d\n", temp, tail, elements_count);
		data_buf[tail++].number = temp;
		
		mutex_release(mutex);
	}
	return OK;
}

/*------------------------------------------------------------------------------------
 * consumer - Consumer process which consumes the random number generated by producer
 *            and calculates square of the number  
 *------------------------------------------------------------------------------------
 */
process consumer(void)
{
	int32 temp = 0;
	
	while(1) {
		mutex_acquire(mutex);

		if(head == MAX_BUF_SIZE) {
			head = head % MAX_BUF_SIZE;
		}
		
		if(elements_count <= 0) {
			printf("\nMutex released by consumer");
			mutex_release(mutex);
			continue;
		}

	
		srand(clktime_ms);
		temp = rand() % MAX_RAND_NO;
		
		/* Introduce random sleep upto 10ms to simulate practical situation */
		sleepms(temp % 10);
				
		++consumed_count;
		--elements_count;
		data_buf[head].square = data_buf[head].number * data_buf[head].number;
		printf("\nConsumer consumed item : %d, head=%d cons_count=%d elem_count=%d", data_buf[head].number, head, consumed_count, elements_count);
		++head;
		mutex_release(mutex);
	}
	return OK;
}


/*------------------------------------------------------------------------------------
 * time_and_send - Timer utility function to measure times for 100-500 element samples
 *------------------------------------------------------------------------------------
 */
process time_and_end(void)
{
	int32 times[5];
	int32 i;

	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();

		consumed_count = 0;
		while (consumed_count < (CONSUMED_MAX * (i+1)))
		{
			yield();
		}

		times[i] = clktime_ms - times[i];
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i) {
		kprintf("\nTIME ELAPSED (%d): %d", (i+1) * CONSUMED_MAX, times[i]);
	}
}

/*------------------------------------------------------------------------------------
 * main - Solution for producer consumer problem using mutex
 *------------------------------------------------------------------------------------
 */
process	main(void)
{
	recvclr();

	mutex = semcreate(1);
	
	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	resume(timer_id); 
	resched_cntl(DEFER_STOP);

	return OK;
}
