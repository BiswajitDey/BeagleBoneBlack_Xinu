/* pubsub.c - publish, subscribe, unsubscribe */
#include <xinu.h>

/* topic table */
struct pubsubent pubsub[MAX_TOPIC];
/* publishing queue */
struct pubqueue *publishq;
sid32 mutex;
sid32 print_mutex;
/* max entries in publishing queue - dynamic */
uint32 max_pub_queue;

/*-------------------------------------------------------------------------
 * subscribe - subscribe a function to a particular group and topic
 *--------------------------------------------------------------------------
 */
syscall subscribe(topic16 topic, void (*handler)(topic16, uint32))
{
	uint32 topic_id;
	uint32 group_id;
	uint32 i = 0;
	
	topic_id = topic & 0x00FF;
	group_id = (topic >> 8) & 0x00FF;
	
	wait(mutex);
	
	//return error if the process has already subscribed for the topic in some other group
	for(i = 0; i < MAX_SUBSCRIBER; i++) {
		if(pubsub[topic_id].psfp_array[i].subscription_state == 1 && pubsub[topic_id].psfp_array[i].pid == getpid()) {
			return SYSERR;	
		}
	}	
	
	i = 0;
	if( pubsub[topic_id].count < MAX_SUBSCRIBER ) {

		while(i++ < MAX_SUBSCRIBER) {
			if(pubsub[topic_id].psfp_array[i].subscription_state == 0) {
				wait(print_mutex);
				printf("In subscribe. group_id=%d topic_id=%d\n", group_id, topic_id);
				signal(print_mutex);
				pubsub[topic_id].psfp_array[i].pid = getpid();
				pubsub[topic_id].psfp_array[i].handler = handler;
				pubsub[topic_id].psfp_array[i].subscription_state = 1;
				pubsub[topic_id].psfp_array[i].group_id = group_id;
				pubsub[topic_id].count++;
				break;
			}
			
		}
		
	}
	signal(mutex);
	return OK;
}

/*-------------------------------------------------------------------------
 * unsubscribe - unsubscribe from a particular group and topic
 *--------------------------------------------------------------------------
 */
syscall unsubscribe(topic16 topic)
{
	uint32 topic_id;
	uint32 group_id;
	uint32 i = 0;

	topic_id = topic & 0x00FF;
	group_id = (topic >> 8) & 0x00FF;

	wait(mutex);
	while(i++ < MAX_SUBSCRIBER) {
		if( pubsub[topic_id].psfp_array[i].pid == getpid() && pubsub[topic_id].psfp_array[i].group_id == group_id ) {
			wait(print_mutex);
			printf("In unsubscribe. group_id=%d topic_id=%d\n", group_id, topic_id);
			signal(print_mutex);
			pubsub[topic_id].psfp_array[i].subscription_state = 0;
			pubsub[topic_id].count--;
			break;
		}
	}
	signal(mutex);		
	return OK;	
}


/*-------------------------------------------------------------------------
 * publish - publish data to a particular group and topic
 *--------------------------------------------------------------------------
 */
syscall publish(topic16 topic, uint32 data)
{
	wait(mutex);
	if(publishq->count < max_pub_queue) {
		wait(print_mutex);
		printf("In publish. topic=%d data=%d\n", topic, data);
		signal(print_mutex);
		publishq->pubq[publishq->tail].topic = topic;
		publishq->pubq[publishq->tail].data = data;
		publishq->count++;
		publishq->tail = (publishq->tail + 1) % max_pub_queue;
	} else {
		wait(print_mutex);
		printf("In publish.queue reallocation. topic=%d data=%d\n", topic, data);
		signal(print_mutex);
		
		//Current queue size is unable to handle the publishing queue, reallocating
		struct pubqueue *new_publishq, *tempq;
		int i = 0;
		int old_pub_queue_size;
		old_pub_queue_size = max_pub_queue;
		max_pub_queue = max_pub_queue * 2; // exponential reallocation

		new_publishq = (struct pubqueue *) getmem(sizeof(struct pubqueue));
		
		new_publishq->pubq = (struct publishqueue *) getmem(max_pub_queue * sizeof(struct publishqueue));

		for(i = 0; i < old_pub_queue_size; i++ ) {
			new_publishq->pubq[i].topic = publishq->pubq[i].topic;
			new_publishq->pubq[i].data = publishq->pubq[i].data;			
		}
		new_publishq->count = publishq->count;
		new_publishq->tail = publishq->count;
		new_publishq->head = publishq->head;

		//Freeing mem for old queue
		tempq = publishq;
		publishq = new_publishq;
		freemem((char *)tempq->pubq, old_pub_queue_size * sizeof(struct publishqueue)); 
		freemem((char *)tempq, sizeof(struct pubqueue));
		
		publishq->pubq[publishq->tail].topic = topic;
		publishq->pubq[publishq->tail].data = data;
		publishq->count++;
		publishq->tail = (publishq->tail + 1) % max_pub_queue;
		
	}
	signal(mutex);
	return OK;
	
}


/*-------------------------------------------------------------------------
 * broker - handle publishing queue to invoke callback function with  
 *          published data to a topic
 *--------------------------------------------------------------------------
 */
process broker()
{
	uint32 topic = 0;
	uint32 topic_id = 0;
	uint32 group_id = 0;
	uint32 data = 0;
	uint32 i = 0;

	wait(print_mutex);
	printf("Broker process started\n");
	signal(print_mutex);
	
	while(1) {
		wait(mutex);
		if(publishq->count > 0) {
			topic_id = publishq->pubq[publishq->head].topic & 0x00FF;
			group_id = (publishq->pubq[publishq->head].topic >> 8) & 0x00FF;
			data = publishq->pubq[publishq->head].data;
			publishq->count--;
			topic = publishq->pubq[publishq->head].topic;
			publishq->head = (publishq->head + 1) % max_pub_queue;
			wait(print_mutex);
			printf("Inside broker. group_id=%d, topic_id=%d\n", group_id, topic_id);
			signal(print_mutex);		
			// special case - wildcard
			if(group_id == 0) {			     
				for(i = 0; i < MAX_SUBSCRIBER; i++) {
					if(pubsub[topic_id].psfp_array[i].subscription_state == 1) {
						pubsub[topic_id].psfp_array[i].handler(topic, data);
					}
				}
				
			} else {
				for(i = 0; i < MAX_SUBSCRIBER; i++) {
					if(pubsub[topic_id].psfp_array[i].subscription_state == 1 && pubsub[topic_id].psfp_array[i].group_id == group_id ) {
						pubsub[topic_id].psfp_array[i].handler(topic, data);
					}
				}

			}
		} 
		signal(mutex);
		if(publishq->count == 0) {
			resched();
		}
	}     
}

/*----------------------------------------------------------------------------------------------
 * pubsub_init - initialize global datastructures and variables releated to publisher subscriber
 *               event mechanism
 *----------------------------------------------------------------------------------------------
 */
syscall pubsub_init()
{
	uint32 i = 0, j = 0;

	
	printf("In pubsub_init()\n");

	
	for(i = 0; i < MAX_TOPIC; i++) {
		pubsub[i].count = 0;
		for(j = 0; j < MAX_SUBSCRIBER; j++) {
			pubsub[i].psfp_array[j].subscription_state = 0;
		}
	}
	
	//create mutex
	mutex = semcreate(1);
	print_mutex = semcreate(1);

	//initial size of publishing queue
	max_pub_queue = 10; 

	//allocate initial memory for dynamic publishing queue
	publishq = (struct pubqueue *) getmem(sizeof(struct pubqueue));
	publishq->pubq = (struct publishqueue *) getmem(max_pub_queue * sizeof(struct publishqueue));

	publishq->head = 0;
	publishq->count = 0;
	publishq->tail = 0;

	return OK;
}
	

/*-------------------------------------------------------------------------------------------------
 * unsubscribe_pub_sub - when the process gets over unsubscribe from all topics of process
 *-------------------------------------------------------------------------------------------------
 */
syscall unsubscribe_pub_sub(pid32 pid) 
{
	int i = 0, j = 0;

	for(i = 0; i < MAX_TOPIC; i++) {
		for(j = 0; j < MAX_SUBSCRIBER; j++) {
			if(pubsub[i].psfp_array[j].subscription_state == 1 && pubsub[i].psfp_array[j].pid == pid) {
				unsubscribe( ( (pubsub[i].psfp_array[j].group_id << 8) + i ) );
			}
		}
	}

}

