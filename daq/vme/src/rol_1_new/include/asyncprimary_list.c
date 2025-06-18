/*****************************************************************
 * 
 * asyncprimary_list.c - "Primary" Readout list routines for asyncprimary
 *
 * Usage:
 *
 *    #include "asyncprimary_list.c"
 *
 *  then define the following routines:
 * 
 *    void rocDownload(); 
 *    void rocPrestart(); 
 *    void rocGo(); 
 *    void rocEnd(); 
 *
 */


#define ROL_NAME__ "ASYNCPRIMARY"
#ifndef MAX_EVENT_LENGTH
/* Check if an older definition is used */
#ifdef MAX_SIZE_EVENTS
#define MAX_EVENT_LENGTH MAX_SIZE_EVENTS
#else
#define MAX_EVENT_LENGTH 10240
#endif /* MAX_SIZE_EVENTS */
#endif /* MAX_EVENT_LENGTH */
#ifndef MAX_EVENT_POOL
/* Check if an older definition is used */
#ifdef MAX_NUM_EVENTS
#define MAX_EVENT_POOL MAX_NUM_EVENTS
#else
#define MAX_EVENT_POOL   400
#endif /* MAX_NUM_EVENTS */
#endif /* MAX_EVENT_POOL */
/* POLLING_MODE */
#define POLLING___
#define POLLING_MODE
/* INIT_NAME should be defined by roc_### (maybe at compilation time - check Makefile-rol) */
#ifndef INIT_NAME
#warn "INIT_NAME undefined. Setting to asyncprimary_list__init"
#define INIT_NAME asyncprimary_list__init
#endif
#include <stdio.h>
#include <rol.h>
#include "jvme.h"
#include <ASYNCPRIMARY_source.h>
extern int bigendian_out;

/*! Buffer node pointer */
extern DMANODE *the_event;
/*! Data pointer */
extern unsigned int *dma_dabufp;

#define ISR_INTLOCK INTLOCK
#define ISR_INTUNLOCK INTUNLOCK

pthread_mutex_t ack_mutex=PTHREAD_MUTEX_INITIALIZER;
#define ACKLOCK {				\
    if(pthread_mutex_lock(&ack_mutex)<0)	\
      perror("pthread_mutex_lock");		\
  }
#define ACKUNLOCK {				\
    if(pthread_mutex_unlock(&ack_mutex)<0)	\
      perror("pthread_mutex_unlock");		\
  }
pthread_cond_t ack_cv = PTHREAD_COND_INITIALIZER;
#define ACKWAIT {					\
    if(pthread_cond_wait(&ack_cv, &ack_mutex)<0)	\
      perror("pthread_cond_wait");			\
  }
#define ACKSIGNAL {				\
    if(pthread_cond_signal(&ack_cv)<0)		\
      perror("pthread_cond_signal");		\
  }
int ack_runend=0;

/* ROC Function prototypes defined by the user */
void rocDownload();
void rocPrestart();
void rocGo();
void rocEnd();
void rocCleanup();
int  getOutQueueCount();
int  getInQueueCount();

/* Asynchronous (to asyncprimary rol) trigger routine, connects to rocTrigger */
void asyncTrigger();

/* Input and Output Partitions for VME Readout */
DMA_MEM_ID vmeIN, vmeOUT;


static void __download()
{
  int status;

  daLogMsg("INFO","Readout list compiled %s", DAYTIME);
#ifdef POLLING___
  rol->poll = 1;
#endif
  *(rol->async_roc) = 1; /* Async ROC */

  bigendian_out=1;

  pthread_mutex_init(&ack_mutex, NULL);
  pthread_cond_init(&ack_cv,NULL);
 
  /* Open the default VME windows */
  vmeOpenDefaultWindows();

  /* Initialize memory partition library */
  dmaPartInit();

  /* Setup Buffer memory to store events */
  dmaPFreeAll();
  vmeIN  = dmaPCreate("vmeIN",MAX_EVENT_LENGTH,MAX_EVENT_POOL,0);
  vmeOUT = dmaPCreate("vmeOUT",0,0,0);

  /* Reinitialize the Buffer memory */
  dmaPReInitAll();
  dmaPStatsAll();

  /* Execute User defined download */
  rocDownload();

  daLogMsg("INFO","Download Executed");

} /*end download */     

static void __prestart()
{
  ACKLOCK;
  ack_runend=0;
  ACKUNLOCK;
  CTRIGINIT;
  *(rol->nevents) = 0;
  unsigned long jj, adc_id;
  daLogMsg("INFO","Entering Prestart");

  ASYNCPRIMARY_INIT;
  CTRIGRSS(ASYNCPRIMARY,1,usrtrig,usrtrig_done);
  CRTTYPE(1,ASYNCPRIMARY,1);

  /* Check the health of the vmeBus Mutex.. re-init if necessary */
  vmeCheckMutexHealth(10);

  /* Execute User defined prestart */
  rocPrestart();

  daLogMsg("INFO","Prestart Executed");

  if (__the_event__) WRITE_EVENT_;
  *(rol->nevents) = 0;
  rol->recNb = 0;
} /*end prestart */     

static void __end()
{
  int ii, ievt, rem_count;
  int len, type, lock_key;
  DMANODE *outEvent;
  int oldnumber;
  int iwait=0;
  int blocksLeft=0;
  unsigned int blockstatus=0;
  int bready=0;

  /* Execute User defined end */
  rocEnd();

  CDODISABLE(ASYNCPRIMARY,1,0);
 
  dmaPStatsAll();
      
  daLogMsg("INFO","End Executed");

  if (__the_event__) WRITE_EVENT_;
} /* end end block */

static void __pause()
{
  CDODISABLE(ASYNCPRIMARY,1,0);
  daLogMsg("INFO","Pause Executed");
  
  if (__the_event__) WRITE_EVENT_;
} /*end pause */

static void __go()
{
  daLogMsg("INFO","Entering Go");
  ACKLOCK;
  ack_runend=0;
  ACKUNLOCK;

  CDOENABLE(ASYNCPRIMARY,1,1);
  rocGo();
  
  if (__the_event__) WRITE_EVENT_;
}

void usrtrig(unsigned long EVTYPE,unsigned long EVSOURCE)
{
  long EVENT_LENGTH;
  int ii, len, data, type=3, lock_key;
  int syncFlag=0, lateFail=0;
  unsigned int event_number=0;
  unsigned int currentWord=0;
  DMANODE *outEvent;

  outEvent = dmaPGetItem(vmeOUT);
  if(outEvent != NULL) 
    {
      len = outEvent->length;
      type = outEvent->type;
      event_number = outEvent->nevent;
      CEOPEN(ROCID, BT_BANK, blockLevel);

      if(rol->dabufp != NULL) 
	{
#undef PRINTOUT
#ifdef PRINTOUT
	  printf("Received %d triggers... \n",
		 event_number);
#endif
	  for(ii=0;ii<len;ii++) 
	    {
	      currentWord = outEvent->data[ii];
	      *rol->dabufp++ = currentWord;
#ifdef PRINTOUT
	      if((ii%5)==0) printf("\n\t");
	      printf("  0x%08x ",(unsigned int)LSWAP(outEvent->data[ii]));
#endif
	    }
#ifdef PRINTOUT
	  printf("\n\n");
#endif
	}
      else 
	{
	  printf("asyncprimary_list: ERROR rol->dabufp is NULL -- Event lost\n");
	}

      CECLOSE;

      ACKLOCK;

      dmaPFreeItem(outEvent);

      ACKUNLOCK;
    }
  else
    {
      logMsg("Error: no Event in vmeOUT queue\n",0,0,0,0,0,0);
    }
  
 
} /*end trigger */

void asyncTrigger()
{
  int intCount=0;
  int length,size;
  unsigned int tirval;
  int clocksource=-1;

  intCount = 0;

  /* grap a buffer from the queue */
  GETEVENT(vmeIN,intCount);
  if(the_event->length!=0) 
    {
      printf("Interrupt Count = %d\t",intCount);
      printf("the_event->length = %d\n",the_event->length);
    }

  the_event->type = 1;
  
  /* Execute user defined Trigger Routine */
  rocTrigger(0);

  /* Put this event's buffer into the OUT queue. */
  ACKLOCK;
  PUTEVENT(vmeOUT);
  /* Check if the event length is larger than expected */
  length = (((int)(dma_dabufp) - (int)(&the_event->length))) - 4;
  size = the_event->part->size - sizeof(DMANODE);

  if(length>size) 
    {
      printf("rocLib: ERROR: Event length > Buffer size (%d > %d).  Event %d\n",
	     length,size,the_event->nevent);
    }
  if(dmaPEmpty(vmeIN))
    {

      /*       printf("WARN: vmeIN out of event buffers (intCount = %d).\n",intCount); */

      if(ack_runend==0)
	{
	  /* Wait for the signal indicating that a buffer has been freed */
	  ACKWAIT;
	}

    }

  ACKUNLOCK;

}

void usrtrig_done()
{
} /*end done */

void __done()
{
  poolEmpty = 0; /* global Done, Buffers have been freed */
} /*end done */

static void __reset()
{
  printf("** Reset called **\n");
} /* end reset */

int
getOutQueueCount()
{
  if(vmeOUT) 
    return(vmeOUT->list.c);
  else
    return(0);
}

int
getInQueueCount()
{
  if(vmeIN) 
    return(vmeIN->list.c);
  else
    return(0);
}

/* This routine is automatically executed just before the shared libary
   is unloaded.

   Clean up memory that was allocated 
*/
__attribute__((destructor)) void end (void)
{
  static int ended=0;

  if(ended==0)
    {
      printf("ROC Cleanup\n");
      
      rocCleanup();
      
      dmaPFreeAll();
      vmeCloseDefaultWindows();

      ended=1;
    }

}
