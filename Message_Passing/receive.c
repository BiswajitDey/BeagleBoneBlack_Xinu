/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	msg = prptr->prmsg;		/* Retrieve message		*/
	prptr->prhasmsg = FALSE;	/* Reset message flag		*/
	restore(mask);
	return msg;
}

/*------------------------------------------------------------------------
 *  receiveMsg  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receiveMsg(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	
	if (prptr->prmsgcount == 0) {
		printf("\nSYSERR : Message queue empty");
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	prptr->prmsgreceiverindex = prptr->prmsgreceiverindex % MAXMSG;
	
	msg = prptr->prmsgqueue[(prptr->prmsgreceiverindex)++];		/* Retrieve message		*/
	prptr->prmsgcount--;
	
	restore(mask);
	return msg;
}

/*------------------------------------------------------------------------
 *  receiveMsgs  -  Wait for multiple messages and return the message to the caller
 *------------------------------------------------------------------------
 */
syscall	receiveMsgs(umsg32 *msgs,
		    uint32 msg_count
		  )
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	uint32 i = 0;
	
	mask = disable();
	prptr = &proctab[currpid];

	if(msg_count > MAXMSG) {
		restore(mask);
		return SYSERR;
	}
	while (prptr->prmsgcount < msg_count) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until messages arrive	*/
	}

	for(i = 0; i < msg_count; i++) {
		prptr->prmsgreceiverindex = prptr->prmsgreceiverindex % MAXMSG;
		msgs[i] = prptr->prmsgqueue[(prptr->prmsgreceiverindex)++];		/* Retrieve message		*/
		prptr->prmsgcount--;		
	}
	
	restore(mask);
	return OK ;
}
