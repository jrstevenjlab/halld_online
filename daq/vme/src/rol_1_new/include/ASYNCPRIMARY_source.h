/******************************************************************************
*
* header file for use Linux VME defined rols with CODA
*
*
*******************************************************************************/
#ifndef __ASYNCPRIMARY_ROL__
#define __ASYNCPRIMARY_ROL__

static int ASYNCPRIMARY_handlers,ASYNCPRIMARYflag;
static int ASYNCPRIMARY_isAsync;
static unsigned int *ASYNCPRIMARYPollAddr = NULL;
static unsigned int ASYNCPRIMARYPollMask;
static unsigned int ASYNCPRIMARYPollValue;
static unsigned long ASYNCPRIMARY_prescale = 1;
static unsigned long ASYNCPRIMARY_count = 0;


/* Put any global user defined variables needed here for ASYNCPRIMARY readout */
extern DMA_MEM_ID vmeOUT, vmeIN;

/*----------------------------------------------------------------------------
  asyncprimary_trigLib.c -- Dummy trigger routines for GENERAL USER based ROLs

 File : asyncprimary_trigLib.h

 Routines:
	   void asyncprimarytenable();        enable trigger
	   void asyncprimarytdisable();       disable trigger
	   char asyncprimaryttype();          return trigger type 
	   int  asyncprimaryttest();          test for trigger  (POLL Routine)
------------------------------------------------------------------------------*/

static void 
asyncprimarytenable(int val)
{
  ASYNCPRIMARYflag = 1;
}

static void 
asyncprimarytdisable(int val)
{
  ASYNCPRIMARYflag = 0;
}

static unsigned long 
asyncprimaryttype()
{
  return(1);
}

static int 
asyncprimaryttest()
{
  if(dmaPEmpty(vmeOUT)) 
    return (0);
  else
    return (1);
}



/* Define CODA readout list specific Macro routines/definitions */

#define ASYNCPRIMARY_TEST  asyncprimaryttest

#define ASYNCPRIMARY_INIT { ASYNCPRIMARY_handlers =0;ASYNCPRIMARY_isAsync = 0;ASYNCPRIMARYflag = 0;}

#define ASYNCPRIMARY_ASYNC(code,id)  {printf("*** No Async mode is available for ASYNCPRIMARY ***\n"); \
                              printf("linking sync ASYNCPRIMARY trigger to id %d \n",id); \
			       ASYNCPRIMARY_handlers = (id);ASYNCPRIMARY_isAsync = 0;}

#define ASYNCPRIMARY_SYNC(code,id)   {printf("linking sync ASYNCPRIMARY trigger to id %d \n",id); \
			       ASYNCPRIMARY_handlers = (id);ASYNCPRIMARY_isAsync = 1;}

#define ASYNCPRIMARY_SETA(code) ASYNCPRIMARYflag = code;

#define ASYNCPRIMARY_SETS(code) ASYNCPRIMARYflag = code;

#define ASYNCPRIMARY_ENA(code,val) asyncprimarytenable(val);

#define ASYNCPRIMARY_DIS(code,val) asyncprimarytdisable(val);

#define ASYNCPRIMARY_CLRS(code) ASYNCPRIMARYflag = 0;

#define ASYNCPRIMARY_GETID(code) ASYNCPRIMARY_handlers

#define ASYNCPRIMARY_TTYPE asyncprimaryttype

#define ASYNCPRIMARY_START(val)	 {;}

#define ASYNCPRIMARY_STOP(val)	 {asyncprimarytdisable(val);}

#define ASYNCPRIMARY_ENCODE(code) (code)


#endif

