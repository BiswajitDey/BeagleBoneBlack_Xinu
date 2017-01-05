/*  main.c  - main */

#include <xinu.h>

#define MAX_MESSAGE_DATA_SIZE 10000
#define MAX_PIDS 3

/* Process Ids */
pid32 sender_id1;
pid32 sender_id2;
pid32 sender_id3;
pid32 receiver_id1;
pid32 receiver_id2;
pid32 receiver_id3;

/*------------------------------------------------------------------------------------
 * sender - Sender process which send/s message/s to  receiver/s
 * # Case 1: Sending a single message : sender_id1 -> receiver_id1
 * # Case 2: Sending n messages to a single process : sender_id2 -> receiver_id2
 * # Case 3: Send a single message to n processes : sender_id3 -> receiver_id1 & receiver_id3
 *------------------------------------------------------------------------------------
 */
process sender(void)
{
	umsg32 msg;
	umsg32 msgs[MAXMSG];
	const uint32 msg_count = 2;
	pid32 pids[MAX_PIDS];
	const uint32 pid_count = 2;
	uint32 pr_id = 0;
	uint32 success_count = 0;

	pr_id = getpid(); // currpid can also be used
	while(1) {

		if(pr_id == sender_id1) {
			sleep(1);
			/* Case 1: Sending a single message */
			msg = clktime_ms % MAX_MESSAGE_DATA_SIZE;
			if( sendMsg(receiver_id1, msg) != SYSERR) {
				printf("\nSender_id1(sendMsg) PID:%d Message sent:%d", getpid(), msg);
			}
			kill(pr_id);
		} else if(pr_id == sender_id2) {			
			sleep(2);
			/* Case 2 : Sending n(example = 2) message to a single process */
			msgs[0] = clktime_ms % MAX_MESSAGE_DATA_SIZE;
			msgs[1]	= (clktime_ms % MAX_MESSAGE_DATA_SIZE) + 1;
			success_count = sendMsgs(receiver_id2, msgs, 2); 
			if( success_count != SYSERR ) {
				printf("\nSender_id2(sendMsgs) PID:%d  %d messages sent:%d %d success_count=%d", getpid(), msg_count, msgs[0], msgs[1], success_count);
			}
			kill(pr_id);
		} else if(pr_id == sender_id3) {
			sleep(3);
			msg = clktime_ms % MAX_MESSAGE_DATA_SIZE;
		
			/* Case 3 : Send a single message to n processes */
			pids[0] = receiver_id1;
			pids[1] = receiver_id3;
			success_count = sendnMsg(pid_count, pids, msg); 
			if( success_count != SYSERR ) {
				printf("\nSender_id3(sendnMsg) PID:%d Message sent:%d to PIDS:%d, %d success_count=%d", getpid(), msg, pids[0], pids[1], success_count);
			}
			kill(pr_id);
		}
	}

	return OK;
}


/*------------------------------------------------------------------------------------
 * receiver - Receiver process which receive/s message/s from sender/s  
 * # Case 1: Receive a single message : sender_id1 -> receiver_id1
 * # Case 2: Receive multiple messages : sender_id2 -> receiver_id2
 * # Case 3: Receive single message sent to n receivers : sender_id3 -> receiver_id1 & receiver_id3
 *------------------------------------------------------------------------------------
 */
process receiver(void)
{
	umsg32 msg;
	umsg32 msgs[MAXMSG];
	const uint32 msg_count = 2;
	uint32 pr_id = 0;
	pr_id = getpid();
	
	while(1) {

		if(pr_id == receiver_id1) {
			/*Case 1 - Receive a single message sent by sender_id1 to receiver_id1 & Case 3: sender_id3 to receiver_id1 & receiver_id3*/ 
			msg = receiveMsg();
			sleep(2); // To allow sender to print message first
			printf("\nReceiver_id1(receiveMsg) PID:%d Message received:%d", getpid(), msg);
		} else if(pr_id == receiver_id2) {
			/* Case 2 - Receive multiple messages sent by sender_id2 to receiver_id2 */
			if(receiveMsgs(msgs, msg_count) != SYSERR) {
				sleep(2);
				printf("\nReceiver_id2(receiveMsgs) PID:%d %d messages received. Msg :%d,%d", getpid(), msg_count,msgs[0],msgs[1]);
				
			}
		} else if(pr_id == receiver_id3) {
			/* Case 3: Receive a message sent by sender_id3 to receiver_id1 & receiever_id3*/
			msg = receiveMsg();
			sleep(4); // To allow sender to print message first
			printf("\nReceiver_id3(receiveMsg) PID:%d Message received:%d", getpid(), msg);
		}
	}

	return OK;
}


/*------------------------------------------------------------------------------------
 * main - Solution for sending/receiving multiple messages between processes
 *------------------------------------------------------------------------------------
 */
process	main(void)
{
	recvclr();
	
	sender_id1 = create(sender, 4096, 50, "sender1", 0);
	sender_id2 = create(sender, 4096, 50, "sender2", 0);
	sender_id3 = create(sender, 4096, 50, "sender3", 0);
	
	receiver_id1 = create(receiver, 4096, 50, "receiver1", 0);
	receiver_id2 = create(receiver, 4096, 50, "receiver2", 0);
	receiver_id3 = create(receiver, 4096, 50, "receiver3", 0);

	resched_cntl(DEFER_START);
	resume(sender_id1);
	resume(sender_id2);
	resume(sender_id3);
	
	resume(receiver_id1);
	resume(receiver_id2);
	resume(receiver_id3);
	
	resched_cntl(DEFER_STOP);
	
	return OK;
}
