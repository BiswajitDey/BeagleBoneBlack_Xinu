/* send.c - send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	send(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	prptr->prmsg = msg;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

/*------------------------------------------------------------------------
 *  sendMsg  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	sendMsg(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];

	if (prptr->prmsgcount == MAXMSG) {
		restore(mask);
		return SYSERR;
	}

	prptr->prmsgsenderindex = prptr->prmsgsenderindex % MAXMSG;
		
	prptr->prmsgqueue[(prptr->prmsgsenderindex)++] = msg;		/* Deliver message		*/
	prptr->prmsgcount++;
	
	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

/*------------------------------------------------------------------------
 *  sendMsgs  -  Pass multiple messages to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
uint32	sendMsgs(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	*msgs,		/* Contents of message		*/
	  uint32        msg_count
	 )
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr; /* Ptr to process's table entry	*/
	uint32 i = 0;
	uint32 success_count = 0;

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];

	if (prptr->prmsgcount == MAXMSG) {
		printf("\nSYSERR : Max message queue reached");
		restore(mask);
		return SYSERR;
	}


	for(i = 0; i < msg_count; i++) {
		if(prptr->prmsgcount == MAXMSG) {
			success_count = i + 1;
			break;
		}
		prptr->prmsgsenderindex = prptr->prmsgsenderindex % MAXMSG;
		prptr->prmsgqueue[(prptr->prmsgsenderindex)++] = msgs[i]; 
		prptr->prmsgcount++;
	}
	
	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask); /* Restore interrupts */
	if(success_count != 0) {
		return success_count;
	}
	return msg_count;
}

/*------------------------------------------------------------------------
 *  sendnMsg  -  Pass a message to multiple processes and start recipient if waiting
 *------------------------------------------------------------------------
 */
uint32	sendnMsg(
		 uint32 pid_count,       /* Count of processes to send the msg*/
		 pid32	*pids,		/* IDs of recipient process	*/
		 umsg32	msg		/* Contents of message		*/
	 )
{
	uint32 i = 0;
	uint32 success_count = 0;

	for(i = 0; i < pid_count; i++) {
		if ( isbadpid(pids[i]) ) {
			return SYSERR;
		}
		if(sendMsg(pids[i], msg) != SYSERR) {
			success_count++;
		}
	}
	return success_count;	
}


