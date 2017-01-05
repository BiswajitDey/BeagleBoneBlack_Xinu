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
void test_callback(topic16 topic, void *data, uint32 size)	
{
	char *test_data = data;
	int i = 0;
	wait(print_mutex);
	printf("Inside test callback1. Topic:0x%x Data: ", topic);
	for(i = 0; i < size; i++) {
		printf("[%d] ", (int) test_data[i]);
	}
	printf("\n");
	signal(print_mutex);

}

/*-------------------------------------------------------------------------------
 * test_callback2 - callback function for subscriber process
 *-------------------------------------------------------------------------------- 
 */
void test_callback2(topic16 topic, void *data, uint32 size)	
{
	char *test_data = data;
	int i = 0;
	wait(print_mutex);
	printf("Inside test callback2. Topic:0x%x Data: ", topic);
	for(i = 0; i < size; i++) {
		printf("[%d] ", (int) test_data[i]);
	}
	printf("\n");
	signal(print_mutex);
}

/*------------------------------------------------------------------------------------
 * subscriber_1 - process to call subscribe to subscribe to a particular 
 *------------------------------------------------------------------------------------
 */
process subscriber_1(void)
{
	topic16 topic;

	topic = 0x0101;
	subscribe(topic, &test_callback);
	sleep(10);
}

/*------------------------------------------------------------------------------------
 * subscriber_2 - process to call subscribe to subscribe to a particular 
 *------------------------------------------------------------------------------------
 */
process subscriber_2(void)
{
	topic16 topic;

	topic = 0x0501;
	subscribe(topic, &test_callback2);
	sleep(10);

}


/*------------------------------------------------------------------------------------
 * publisher - process which calls publish to publish an array of data to a topic
 *------------------------------------------------------------------------------------
 */
process publisher(void)
{
	char data[5] = {1,2,3,4,5};
	
	//group 1 topic 1
	publish(0x0101, (void *) data, 5);

	// group 5 topic 1
	data[0] = 2;
        publish(0x0501, (void *) data, 5);

	// wildcard group
	data[0] = 9;
	publish(0x0001, (void *) data, 5);
		
}

/*------------------------------------------------------------------------------------
 * main - solution for event handling - publisher subscriber in Xinu
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
