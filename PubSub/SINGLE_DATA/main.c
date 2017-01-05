/*  main.c  - main */

#include <xinu.h>

/* Process Ids */
pid32 pub_id1;
pid32 sub_id1;
pid32 sub_id2;

extern sid32 print_mutex;

/*-------------------------------------------------------------------------------
 * test_callback - callback function for subscriber 1 process
 *-------------------------------------------------------------------------------- 
 */
void test_callback(topic16 topic, uint32 data)	
{
	wait(print_mutex);
	printf("Inside test_callback1. Topic:0x%x Data :%d\n", topic, data);
	signal(print_mutex);
}

/*-------------------------------------------------------------------------------
 * test_callback2 - callback function for subscriber 2 process
 *-------------------------------------------------------------------------------- 
 */
void test_callback2(topic16 topic, uint32 data)	
{
	wait(print_mutex);
	printf("Inside test_callback2. Topic:0x%x Data :%d\n", topic, data);
	signal(print_mutex);
}

/*------------------------------------------------------------------------------------
 * subscriber_1 - process to simulate a subcriber 
 *------------------------------------------------------------------------------------
 */
process subscriber_1(void)
{
	if( subscribe(0x0101, &test_callback) == SYSERR) {
		printf("Subscriber_1: Subscribing - topic 0x0101 failed.\n");
	} else {
		wait(print_mutex);
		printf("Subscriber_1: Subscribing - topic 0x0101 success.\n");
		signal(print_mutex);
	}
	sleep(5);
}

/*------------------------------------------------------------------------------------
 * subscriber_2 - process to simulate a subcriber 
 *------------------------------------------------------------------------------------
 */
process subscriber_2(void)
{
	if( subscribe(0x0501, &test_callback2) == SYSERR) {
		printf("Subscriber_2: Subscribing - topic 0x0501 failed.\n");
	} else {
		wait(print_mutex);
		printf("Subscriber_2: Subscribing - topic 0x0501 successful.\n");
		signal(print_mutex);
	}
	sleep(5);
}


/*------------------------------------------------------------------------------------
 * publisher - process to simulate a publisher 
 *------------------------------------------------------------------------------------
 */
process publisher(void)
{
	//group 1 topic 1
	publish(0x0101, 1111);

	//group 1 topic 1
	publish(0x0101, 2222);

	// group 5 topic 1
        publish(0x0501, 3333);

	// wildcard group
	publish(0x0001, 9999);
	
}

/*------------------------------------------------------------------------------------
 * main - Solution for publisher-subscriber event handling in Xinu
 *------------------------------------------------------------------------------------
 */
process	main(void)
{
	recvclr();

	// 2 subscribers , 1 broker, 1 publisher
	sub_id1 = create(subscriber_1, 4096, 50, "subscriber1", 0);
	sub_id2 = create(subscriber_2, 4096, 50, "subscriber2", 0);
	pub_id1 = create(publisher, 4096, 50, "publisher", 0);
	
	
	resched_cntl(DEFER_START);
	resume(sub_id1);
	resume(sub_id2);
	resume(pub_id1);	
	resched_cntl(DEFER_STOP);
	
	return OK;
}
