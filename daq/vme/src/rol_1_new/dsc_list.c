/*************************************************************************
 *
 *  ti_slave_list.c - Library of routines for readout and buffering of 
 *                    events using a JLAB Trigger Interface V3 (TI) with 
 *                    a Linux VME controller in CODA 3.0.
 * 
 *                    This is for a TI in Slave Mode controlled by a
 *                    Master TI or Trigger Supervisor
 *
 */

/* Event Buffer definitions */
#define MAX_EVENT_POOL     10
// #define MAX_EVENT_LENGTH   4096*64      /* Size in Bytes */
#define MAX_EVENT_LENGTH   4096*1000      /* Size in Bytes */

/* Define TI Type (TI_MASTER or TI_SLAVE) */
#define TI_SLAVE
/* TS Fiber Link trigger source (from TI Master, TD, or TS), POLL for available data */
#define TI_READOUT TI_READOUT_TS_POLL 
/* TI VME address, or 0 for Auto Initialize (search for TI by slot) */
#define TI_ADDR  0           

/* Measured longest fiber length in system */
// #define FIBER_LATENCY_OFFSET 0x4A
#define FIBER_LATENCY_OFFSET 0xCA


#include "dmaBankTools.h"      /*  Macros for handling CODA banks  */
#include "asyncprimary_list.c"    /*  Source required for CODA readout lists using the TI  */


#include "conf_utils.h" 
#include "conf_utils.c" 

#include "dsc_init.c"


bd_type  board_type = bd_unknown;


/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{

  int rc = -1;

  int stat;
  
  memset(&dsc, 0, sizeof(dsc));

  gethostname(roc_name,ROCLEN);
  
  /*  Global config file  */
  strcpy(filename,rol->usrConfig);
  printf(BOLDMAGENTA " Global Config File Name %s \n" RESET ,filename);
  
  rc = read_conf_global(filename,roc_name);
  
  if(rc !=  0){
    printf(BOLDRED "FATAL: Fail to read global configuration file %d \n" RESET, rc);
    daLogMsg("ERROR",     "Fail to read global configuration file \n");  
    ROL_SET_ERROR;
  }
    
  rc  =  read_conf_common(roc_name);
  
  if(rc !=  0){
    printf(BOLDRED "FATAL: Fail to read file with crate specific parameters %d \n" RESET, rc);
    daLogMsg("ERROR",     "Fail to read file with crate specific parameters \n");  
    ROL_SET_ERROR;
  }
  
  rc = read_conf_user(roc_name);

  if(rc !=  0){
    printf(BOLDRED "FATAL: Fail to read user config file %d \n" RESET, rc);
    daLogMsg("ERROR",     "Fail to read user config file \n");
    ROL_SET_ERROR;
  }
  
  
  board_type = get_board_type(roc_name);


  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  
  vmeDmaConfig(2,5,1); 
  
  
  if(board_type == dsc2){
    rc = dsc_download();
    
    if(rc != 0){
      printf(BOLDRED "  Error executing dsc_download()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing dsc_download()  \n");
      ROL_SET_ERROR;
    }    
 
  
#if 1

    printf(BOLDBLUE "  \n   Check DSC2  Config  \n  "  RESET);
    
    rc = dsc2_check_conf();
    
    if(rc > 0) {        
      printf(BOLDRED   "Wrong initialization of DSC2   %d \n" RESET, rc);
      daLogMsg("ERROR","Wrong initialization of DSC2   %d \n", rc);
      ROL_SET_ERROR; 
    }  

#endif 
    
    }  





  printf(BOLDBLUE "rocDownload: User Download Executed \n" RESET);
  
}

/****************************************
 *  PRESTART
 ****************************************/
void
rocPrestart()
{

  int rc = -1;
  
  /* Unlock the VME Mutex */
  vmeBusUnlock();

  if(board_type == dsc2){
    rc = dsc_prestart();
    
    if(rc != 0){
      printf(BOLDRED "  Error executing dsc_prestart()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing dsc_prestart()  \n");
      ROL_SET_ERROR;
    }  
       
  }

  printf(BOLDBLUE "rocPrestart: User Prestart Executed\n" RESET);
  
}

/****************************************
 *  GO
 ****************************************/
void
rocGo()
{


}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{

}

/****************************************
 *  TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{

}

void
rocCleanup()
{
  
}
