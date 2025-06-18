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

/* Event Buffer definitions:  4 MB max !!!*/
#define MAX_EVENT_POOL     10          /* max BUFFERLEVEL<MAX_EVENT_POOL */
#define MAX_EVENT_LENGTH   400000      /* Size in Bytes */  // 399620 max. 400 kb x 10 buffers; randy<=MAX_EVENT_POOL

// #define MAX_EVENT_POOL     6          /* max BUFFERLEVEL<MAX_EVENT_POOL */
// #define MAX_EVENT_LENGTH   600000      /* Size in Bytes */  // 399620 max. 400 kb x 10 buffers; randy<=MAX_EVENT_POOL


/* Measured longest fiber length in system */
#define FIBER_LATENCY_OFFSET 0xCA


#define TI_INIT


/* Define TI Type (TI_MASTER or TI_SLAVE) */
#define TI_SLAVE
/* TS Fiber Link trigger source (from TI Master, TD, or TS), POLL for available data */
#define TI_READOUT TI_READOUT_TS_POLL 
/* TI VME address, or 0 for Auto Initialize (search for TI by slot) */
#define TI_ADDR  0           


#define USE_HD 
// #undef SHM_BUF

#ifdef SHM_BUF
#include "shmem_roc.h"
static roc_shmem *shmem_ptr;
SHM_HIST *hrol1;
struct timeval tvA[10], tvB[10]; 
#endif


#include "dmaBankTools.h"      /*  Macros for handling CODA banks  */
#include "tiprimary_list.c"    /*  Source required for CODA readout lists using the TI  */

int  FA_SLOT;

#include "conf_utils.h" 
#include "conf_utils.c" 

#include "conf_output.h" 
#include "conf_output.c" 


#define WITHLL

#include "fa250_init.c"
#include "fa125_init.c"
#include "dsc_init.c"
#include "caen1290_init.c"
#include "f1_init.c"
#include "ctp_init.c"
#include "ssp_init.c"

#ifdef USE_HD
#include "hd_list.c"
#endif
#include "gem_list.c" 
#ifdef SHM_BUF
#include "bor_utils.h" 
#include "bor_utils.c" 
#endif

#include <stdlib.h>  

extern int fadcA32Base;
extern int fa125A32Base;
extern unsigned int dscA32Base;

extern fa125BlockError;


bd_type  board_type = bd_unknown;
det_type detector_type  = -1;


#ifdef SHM_BUF
#include "monitor.c"
#include "dirc_scan.c"
#endif


/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{

  // Default settings
  int rc = -1;

  int stat;

  memset(fa250, 0, sizeof(fa250));
  memset(fa125, 0, sizeof(fa125));
  memset(&caen1290, 0, sizeof(caen1290));
  memset(&dsc, 0, sizeof(dsc));
  memset(&ti_bd, 0, sizeof(ti_bd));
  memset(&f1, 0, sizeof(f1));
  memset(&ctp, 0, sizeof(ctp));

  fadcA32Base=0x09000000;


  gethostname(roc_name,ROCLEN);

  board_type = get_board_type(roc_name);

  detector_type = get_det_type(roc_name);


  /* ------------------------------- */ 
  /*        Global config file       */
  /* ------------------------------- */
  
  strcpy(filename,rol->usrConfig);
  

  //  char filename1[100] = "/home/hdops/CDAQ/config/hd_all/ECAL_m10.conf";
  //  strcpy(filename,filename1);
  //  printf(BOLDMAGENTA " Global Config File Name %s \n" RESET ,filename1);

  
  rc = read_conf_global(filename,roc_name);  
  if(rc !=  0){
    printf(BOLDRED "FATAL: Fail to read global configuration file %d \n" RESET, rc);
    daLogMsg("ERROR",     "Fail to read global configuration file \n");    
    ROL_SET_ERROR;
  }
  
  print_fadc250_conf(3);
  

  /* ------------------------------- */ 
  /*    Crate specific config file   */
  /* ------------------------------- */ 
  


  if(use_trd != 1){
    rc = read_conf_common(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read file with crate specific parameters %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read file with crate specific parameters \n");    
      ROL_SET_ERROR;
    }       
  } else if( use_trd == 1){
    
    printf("\n");
    printf(BOLDBLUE " Read out ST and TRD  " RESET);
    printf("\n");

    fa125A32Base = fadcA32Base + 0x800000*9 + 0x800000;

    /* Read crate specific config file for ST */
    use_trd = 0;    
    printf("Read conf_common for ST fadc250 \n");
    rc = read_conf_common(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read file with crate specific parameters %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read file with crate specific parameters \n");    
      ROL_SET_ERROR;
    }   

    /* Read crate specific config file for TRD */
    use_trd = 1;
    printf("Read conf_common for TRD fadc125 \n");
    rc = read_conf_common(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read file with crate specific parameters %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read file with crate specific parameters \n");  
      ROL_SET_ERROR;
    }   
  }

 /* Read crate specific config file for Muon chamber prototype */
  if( use_muon == 1){

    printf("\n");
    printf(BOLDBLUE " Set A32 address for MUON prototype  " RESET);
    printf("\n");

    
    fadcA32Base = fa125A32Base + 0x800000*9 + 0x800000;

    use_muon = 0;
    /*  Initiazlize fa250 module in slot 15 */
    rc = read_conf_common(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read file with crate specific parameters %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read file with crate specific parameters \n");    
      ROL_SET_ERROR;
    }  
    use_muon = 1;
    print_fadc125_conf(3);
    print_fadc250_conf(15); 
        
  }

  print_ssp_conf(); 

  /* --------------------- */ 
  /*    User config file   */
  /* ----------------------*/ 

  if(use_playback > 0){
    printf( BOLDMAGENTA "Trigger Playback Mode activated \n" RESET); 
    use_playback = 1;
    rc = read_conf_pb(roc_name);

    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read play back config file %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read play back config file \n");
      ROL_SET_ERROR;
    }    

  } else {
    use_playback = 0;
  }


  if(use_trd != 1){
    rc = read_conf_user(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read user config file %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read user config file \n");
      ROL_SET_ERROR;
    }  
  } else if( use_trd == 1){

    use_trd = 0;
    printf("Read conf_user for ST fadc250 \n");
    rc = read_conf_user(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read user config file %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read user config file \n");
      ROL_SET_ERROR;
    }  

    use_trd = 1;
    printf("Read conf_user for TRD fadc125 \n");
    rc = read_conf_user(roc_name);
    if(rc !=  0){
      printf(BOLDRED "FATAL: Fail to read user config file %d \n" RESET, rc);
      daLogMsg("ERROR",     "Fail to read user config file \n");
      ROL_SET_ERROR;
    }      

    print_fadc250_conf(3);
    print_fadc125_conf(13);    
  }

  if(use_playback)
    print_fadc250_pb_all();


  printf( BOLDMAGENTA "TRIGGER GLOBAL DELAY  =  %d \n"  RESET, trig_delay);   
  printf( BOLDMAGENTA "DAC CALIBRATION RUN   =  %d  \n" RESET, dac_calib); 


  // TRD setup
  //  if( (use_trd == 1) && (detector_type == st) && (board_type == fadc250)){
  //    board_type = fadc125;
  //    fa125A32Base = fadcA32Base + 0x800000*9 + 0x800000;
  //    print_fadc125_conf(13);
  //  }
  


  // Initialize TI
  // Overwrite fiber latency according to the config file
  if(ti_bd.fiber_latency > 0){
    tiFiberLatencyOffset = ti_bd.fiber_latency;
    printf(BOLDMAGENTA "Set TI latency offset to: 0x%x \n" RESET, ti_bd.fiber_latency);
  } else {
    tiFiberLatencyOffset = FIBER_LATENCY_OFFSET;
    printf(BOLDMAGENTA "Set TI latency offset to the default value: 0x%x \n" RESET,  FIBER_LATENCY_OFFSET);
  }
  
  rc = tiInit(TI_ADDR,TI_READOUT,0);

  
  if(rc < 0){
    daLogMsg("ERROR",  "Fail to Initialize TI  %d \n", rc );
    ROL_SET_ERROR;
  }


  if(rc > 0){
    daLogMsg("ERROR",  "Number of wrong TI fiber measurements %d \n", rc );
  }

  /* Check latency of optical fibers */

  ti_check_latency(roc_name);
 
  rc = tiCheckFiberSyncDelay();
  if(rc != 0){
    printf(BOLDRED "  Wrong content of TI fiberSyncDelay register  %d \n" RESET, rc);
    daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register  %d  \n", rc);
    ROL_SET_ERROR;
  }



  /* Set crate ID */
  tiSetCrateID(ROCID);
  printf("TI CrateID register set to %d\n",ROCID);

  /* Set timestamp format 48 bits */
  tiSetEventFormat(3);
  

  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  
  vmeDmaConfig(2,5,1); 
  

  /* Define BLock Level variable to a default */
  blockLevel = 1;

  if(ts_bd.blocklevel > 0){
    printf(BOLDBLUE " BLOCKLEVEL set to  %d \n" RESET, ts_bd.blocklevel);  
    blockLevel = ts_bd.blocklevel;
  } else {
    printf(BOLDBLUE " BLOCKLEVEL not set in the config file. Use delault BLOCKLEVEL = 1  \n" RESET );
  }

  if(ts_bd.bufferlevel > 0){
    printf(BOLDBLUE " BUFFERLEVEL set to  %d \n" RESET, ts_bd.bufferlevel);  
    bufferLevel = ts_bd.bufferlevel;
  } else {
    printf(BOLDBLUE " BUFFERLEVEL not set in the config file. Use delault BUFFERLEVEL = %d  \n" RESET, bufferLevel );
  }


  fa250_set_block_level(blockLevel);
  fa125_set_block_level(blockLevel);


  /*****************
   *   TI SETUP
   *****************/

  if(ti_bd.ti_master == 1){
    printf(BOLDBLUE " Stand alone run with the Master TI. Enable optical port 5 \n" RESET);
    tiSetSlavePort(5);
    tiSetTriggerSource(TI_TRIGGER_HFBR5);
  } else {
    printf(BOLDBLUE " Run with the TS. Optical port 1 is used  \n" RESET);
  }


  /* Set the sync delay width to 0x7F*32 = 4.064us */
  tiSetSyncDelayWidth(0x54, 0x7F, 1);
  
  /* Set Trigger Buffer Level */
  //  tiSetBlockBufferLevel(bufferLevel);

  // Alex
  tiSetBlockBufferLevel(0);


  // Alex
  //  tiSetBusySource(TI_BUSY_LOOPBACK,1);  //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<  fsv <<<<<<<<<<<<
  //  usleep(1);   // fsv 

  /* Set a Maximum Block count before trigger autmatically disables  (0 disables block limit)*/
  tiSetBlockLimit(0);

  if((use_trd  == 1) && (board_type == fadc125)) board_type = fadc250;  
  if((use_muon == 1) && (board_type == fadc125)) board_type = fadc250;

  if(board_type == fadc250){
    printf(BOLDBLUE " FA250  Download \n" RESET);
    rc = fa250_download();

    // Alex
    if( (board_type == fadc250) || (board_type == fadc125))
      tiSetBusySource(TI_BUSY_SWB, 1);
    //    tiSetBusySource(0, 1);


    if(rc != 0){
      printf(BOLDRED "  Error executing fa250_download()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing fa250_download()  \n");
      ROL_SET_ERROR;
    }

    if(ctp.used == 1){
      printf(BOLDBLUE " CTP  Download \n" RESET);
      rc = ctp_download();
    }
  }

  if((use_trd  == 1) && (board_type == fadc250)) board_type = fadc125;
  if((use_muon == 1) && (board_type == fadc250)) board_type = fadc125;

  if(board_type == fadc125){
    printf(BOLDBLUE "FA125 Download \n" RESET);
    rc = fa125_download();

    if(rc != 0){
      printf(BOLDRED "  Error executing fa125_download()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing fa125_download()  \n");
      ROL_SET_ERROR;
    }
  }


  if(board_type == tdc1290){

    /* Set the busy source to non-default value (no Switch Slot B busy) */
    printf(BOLDBLUE " SET FP BUSY FOR TDC1290  \n" RESET);
    tiSetBusySource(TI_BUSY_FP_FADC,1); 

    printf(BOLDBLUE " CAEN Download \n" RESET);
    rc = caen1290_download();

    if(rc != 0){
      printf(BOLDRED "  Error executing caen1290_download()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing caen1290_download()  \n");
      ROL_SET_ERROR;
    }
  }  

  
  if(board_type == f1tdc){
    printf(BOLDBLUE " F1 TDC  Download \n" RESET);
    rc = f1_download();
    
    if(rc != 0){
      printf(BOLDRED "  Error executing f1_download()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing f1_download()  \n");
      ROL_SET_ERROR;
    }
  }
  
  if((use_trd == 1) && (board_type == fadc125)) board_type = fadc250;
  
  // Initialize DSC in SLOT 14 for the Muon Chamber Test
  if(use_muon == 1){

    //    printf("\n TEST A32 ADDRESS FOR DSC2 0x%X \n", dsc2GetAdr32(14));

    unsigned int dscA32Base_my =  0xA000000;

    rc = dsc_download();
    
    dsc2SetAdr32(14, dscA32Base_my, 1);
    

    dsc2Status(14, 1);

    if(rc != 0){
      printf(BOLDRED "  Muon Chamber Test: Error executing dsc_download()  %d \n" RESET, rc);
      daLogMsg("ERROR","Muon Chamber Test: Error executing dsc_download()  \n");
      ROL_SET_ERROR;
    }
  }

  if(board_type == ssp_mod){
    ssp_download();
  }
  

#ifdef SHM_BUF
  conf_output_init(board_type);
  printf(BOLDRED " INSIDE SHM BUFFER  \n" RESET);
#endif
 
  printf("\n rocDownload: User Download Executed \n\n");

  //#########################################################################################
  

#ifdef SHM_BUF
  monitor_download();
#endif
  
  printf("\n");
  printf("  TI FIBER LATENCY MEASUREMENT  DOWNLOAD %d \n", tiGetFiberLatencyMeasurement());
  printf("\n"); 

#ifdef USE_HD
  //  if (blockLevel!=1 || blockLevel!=1) { use_gem=0; }
  printf("call hd_download();  ... \n");
  hd_download();
#endif
 
}

/****************************************
 *  PRESTART
 ****************************************/
void
rocPrestart()
{

  int rc = -1;

  int ped_status = -1;


  /* Unlock the VME Mutex */
  vmeBusUnlock();
  
  tiStatus(0); 
  
  if((use_trd  == 1) && (board_type == fadc125)) board_type = fadc250;
  if((use_muon == 1) && (board_type == fadc125)) board_type = fadc250;

  if(board_type == fadc250){
    rc = fa250_prestart();
    if(rc != 0){
      printf(BOLDRED "  Error executing fa250_prestart()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing fa250_prestart()  \n");
      ROL_SET_ERROR;
    }    
    
    if(ctp.used == 1){
      rc = ctp_prestart();
      if(rc != 0){
	printf(BOLDRED "  Error executing ctp_prestart()  %d \n" RESET, rc);
	daLogMsg("ERROR","Error executing ctp_prestart()  \n");
	ROL_SET_ERROR;
      }
    }    
  }  
  
  if((use_trd  == 1)  && (board_type == fadc250)) board_type = fadc125;
  if((use_muon == 1)  && (board_type == fadc250)) board_type = fadc125;

  if(board_type == fadc125){
    rc = fa125_prestart();
    if(rc != 0){
      printf(BOLDRED "  Error executing fa125_prestart()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing fa125_prestart()  \n");
      ROL_SET_ERROR;
    }
  }
  
  if(board_type == f1tdc){
    rc = f1_prestart();
    if(rc != 0){
      printf(BOLDRED "  Error executing f1_prestart()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing f1_prestart()  \n");
      ROL_SET_ERROR;
    }
  }

  if(board_type != tdc1290)
    sdStatus(1);
  

  tiSetPromptTriggerWidth(5);
  printf(BOLDMAGENTA "    SET PROMPT TRIGGER WIDTH  to %d   \n" RESET, tiGetPromptTriggerWidth());  



  if((use_trd  == 1) && (board_type == fadc125)) board_type = fadc250;

  if(board_type == ssp_mod){
    ssp_prestart();
  }


#ifdef SHM_BUF
  monitor_prestart();  

  /* Read module configuration into Beginning Of Run */
  /* record of shared memory buffer.                 */
  /*                 Jan. 22, 2016 DL                */
  ReadModuleConfigs(&shmem_ptr->BOR);
#endif

  tiStatus(1); 


#ifdef USE_HD
  printf("call hd_prestart();  ... \n");
  hd_prestart();
#endif

  printf("\n rocPrestart: User Prestart Executed \n\n");
  /* SYNC is issued after this routine is executed */
  
}

/****************************************
 *  GO
 ****************************************/
void rocGo(){

  int rc    =  -1;
  int rval  =  -1;

  int ped_status   =  -1;
  int conf_status  =  -1;

  int ii, iboard;

  /* Get the broadcasted Block Level from TS or TI Master */
  if(blockLevel != tiGetCurrentBlockLevel() ){
    daLogMsg("ERROR", "Block Level does not match  %d  %d \n",  blockLevel,  tiGetCurrentBlockLevel() );  
    printf(BOLDRED "ERROR: Block Level does not match  %d  %d \n " RESET, blockLevel, tiGetCurrentBlockLevel() );
    ROL_SET_ERROR;
  } else {
    printf(BOLDBLUE "rocGo: Block Level set to %d\n" RESET, blockLevel);
  }

  if((use_trd  == 1) && (board_type == fadc125)) board_type = fadc250;
  if((use_muon == 1) && (board_type == fadc125)) board_type = fadc250;

  if(board_type == fadc250){    
    rc = fa250_go();
    if(rc != 0){
      printf(BOLDRED "  Error executing fa250_go()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing fa250_go()  \n");
      ROL_SET_ERROR;
    }       
    
    if(ctp.used == 1){
      rc = ctp_go();
      if(rc != 0){
	printf(BOLDRED "  Error executing ctp_go()  %d \n" RESET, rc);
	daLogMsg("ERROR","Error executing ctp_go()  \n");
	ROL_SET_ERROR;
      }
    } 

  }

  if((use_trd  == 1) && (board_type == fadc250)) board_type = fadc125;
  if((use_muon == 1) && (board_type == fadc250)) board_type = fadc125;

  printf("go:: use_trd=%d \n",use_trd);
  if(board_type == fadc125){
    rc = fa125_go();
    if(rc != 0){
      printf(BOLDRED "  Error executing fa125_go()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing fa125_go()  \n");
      ROL_SET_ERROR;
    }

  }
  if(board_type == f1tdc){
    rc = f1_go();
    if(rc != 0){
      printf(BOLDRED "  Error executing f1_go()  %d \n" RESET, rc);
      daLogMsg("ERROR","Error executing f1_go()  \n");
      ROL_SET_ERROR;
    }
  }


  tiStatus(1);



  printf("\n");
  printf(" BEFORE CHECK CONFIG  %d \n",tiGetFiberLatencyMeasurement());
  printf("\n");

  
  //  conf_status = fa250_check_conf();
  
  //  if(conf_status > 0 && CALIBRATION_FLAG==0) {        
  //    printf(BOLDRED   "Wrong initialization of FADC 250   %d \n" RESET, conf_status);
  //    daLogMsg("ERROR","Wrong initialization of FADC 250   %d \n", conf_status);
  
  //    ROL_SET_ERROR;
  //  }



  // CHECK DAC RESET
#if 0 
  int fail_mask = 0;

  printf(BOLDBLUE " BEFORE RESETING FADC DAC \n" RESET);

  ped_status = check_baseline(&fail_mask);

  printf(BOLDBLUE " BAD BOARD STATUS 0x%x \n" RESET, fail_mask);
  

  faDACReset(15);
  faDACReset(16);

  printf(BOLDRED " AFTER RESETING FADC DAC \n" RESET);

  fail_mask = 0;

  ped_status = check_baseline(&fail_mask);

  printf(BOLDRED " BAD BOARD STATUS 0x%x \n" RESET, fail_mask);


  printf(BOLDMAGENTA " REINITIALIZE DAC \n" RESET);

  for(ii = 0; ii < MAX_FADC250_CH; ii++) {
    faSetDAC(15, fa250[15].dac[ii], (1<<ii));
    faSetDAC(16, fa250[16].dac[ii], (1<<ii));
  }

  fail_mask = 0;

  ped_status = check_baseline(&fail_mask);

  printf(BOLDMAGENTA " BAD BOARD STATUS 0x%x \n" RESET, fail_mask);
#endif  





  // ALEX JANUARY

#if 0


  if( board_type == fadc250 && CALIBRATION_FLAG == 0 ){

    if( (strcmp(roc_name,  "rocps2")   == 0)     ||  (strcmp(roc_name, "rocps1")   == 0)    || 
        (strncmp(roc_name, "rocfcal", 7) == 0)   ||  (strncmp(roc_name, "rocbcal", 7) == 0) || 
	(strcmp(roc_name, "roctagh1") == 0 ) ){
      
      int fail_mask = 0;
      

      ped_status = check_baseline(&fail_mask);
      

      printf( BOLDBLUE "\n  check_baseline  mask = 0x%x \n"  RESET , fail_mask); 


      //  Wrong DAC found, try to reset
      if( (ped_status != 0) | (fail_mask != 0)){
        
        for(iboard = 0; iboard < NFADC_250; iboard++){
          
          FA_SLOT = fadcID[iboard];
          
          if(fail_mask == (1 << (FA_SLOT - 1))){
            
            printf(BOLDBLUE "\n RESET DAC VALUES FOR FADC in SLOT  %d \n" RESET, FA_SLOT); 
            

            faSetNormalMode(FA_SLOT, 1);


            printf(BOLDBLUE "\n SET DAC VALUES FOR FADC in SLOT  %d \n" RESET, FA_SLOT); 
            
            for(ii = 0; ii < MAX_FADC250_CH; ii++) {
              faSetDAC(FA_SLOT, fa250[FA_SLOT].dac[ii], (1<<ii));
            }   
            

          }          
        }
        
        printf(BOLDBLUE "\n CHECK BASELINE AGAIN  \n" RESET); 
        
        // Check DAC again
        fail_mask = 0;
        
        ped_status = check_baseline(&fail_mask);
        
        
        if(ped_status != 0){        

          printf(BOLDRED   "Wrong pedestals   %d \n"   RESET, ped_status);          
          daLogMsg("CONSOLE","Wrong pedestals   %d \n",  ped_status);
          
          if(strncmp(roc_name, "rocbcal", 7) == 0){
            printf(BOLDRED   "FATAL:  Reboot the BCAL crate   %d \n"   RESET, ped_status);
            daLogMsg("ERROR","FATAL:  Reboot the BCAL crate   %d \n",  ped_status);
            ROL_SET_ERROR;
          }
        }
        
      } 
      
  

      printf(BOLDBLUE "  \n  Check FADC250 Config  \n  "  RESET);
      
      conf_status = fa250_check_conf();
      
      if(conf_status > 0) {        
        printf(BOLDRED   "Wrong initialization of FADC 250   %d \n" RESET, conf_status);
        daLogMsg("ERROR","Wrong initialization of FADC 250   %d \n", conf_status);
        ROL_SET_ERROR; 
      } 
        
      
    }
    
  }
  

#endif


  // Check config of FADC 125    

  if( board_type == fadc125 && CALIBRATION_FLAG == 0 ){

    printf(BOLDBLUE "  \n  Check FADC125 Config  \n  "  RESET);
 
    if( strncmp(roc_name, "roccdc", 6) == 0){
      
      conf_status = fa125_check_conf();
      
      if(conf_status > 0) {        
        printf(BOLDRED   "Wrong initialization of FADC 125   %d \n" RESET, conf_status);
        daLogMsg("ERROR","Wrong initialization of FADC 125   %d \n", conf_status);
        ROL_SET_ERROR; 
      }  
      
    }
  }

#ifdef USE_HD
  printf("call hd_go();  ... \n");
  hd_go();
#endif

  printf("\n");
  printf(" TI FIBER LATENCY MEASUREMENT  GO %d \n",tiGetFiberLatencyMeasurement());
  printf("\n");


}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{

  int rc = -1;

  if(board_type == fadc250){
    rc = fa250_end();
    ctpStatus(1);
  }

  if(board_type == f1tdc)
    rc = f1_end();

  tiStatus(1);

#ifdef USE_HD
  printf("call hd_end();  ... \n");
  hd_end();
#endif

  printf("rocEnd: Ended after %d blocks\n",tiGetIntCount());
  
}

/****************************************
 *  TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{
  int stat = 0,  dCnt = 0;

  int timeout_fa125    =  100000;
  int timeout_fa250    =  1000;
  int timeout_f1       =  100;
  int timeout_tdc1290  =  100;

  int iboard, itime, gbready = 0;

  int roflag = 1;

  unsigned int datascan = 0;

  int syncFlag = 0;
  
  int ii;

  int jj;


  syncFlag = tiGetSyncEventFlag();


#ifdef SHM_BUF
  if (RUN_FLAG<0) { 
    printf("=1=   RUN_FLAG=%d CALIBRATION_FLAG=%d nfadc=%d nfadc125=%d \n",*RUN_FLAG,CALIBRATION_FLAG,nfadc,nfadc125);
    daLogMsg("ERROR"," %s ERROR: Data corruption var:RUN_FLAG=%d\n", roc_name, *RUN_FLAG);
  }
  gettimeofday(&tvA[2], NULL);
  monitor_trigger();
  gettimeofday(&tvB[4], NULL);
  int idiffm=tvB[4].tv_usec - tvA[2].tv_usec +  1000000 * (tvB[4].tv_sec - tvA[2].tv_sec);
  //---- Fill Hist monitor  ------------
  hf1(hrol1,6,idiffm); 
  unsigned int *dma1 =  dma_dabufp;


  //  printf("     )))))))))))))))))    \n");
#endif

  /* Set TI output 1 high for diagnostics */
  //  tiSetOutputPort(1,0,0,0);
  
  
  /* Readout the trigger block from the TI 
     Trigger Block MUST be reaodut first */

  dCnt = tiReadTriggerBlock(dma_dabufp);
  
  if(dCnt <= 0) {
    printf("TI no data or error.  dCnt = %d\n",dCnt);
    daLogMsg("ERROR", "TI no data or error.  dCnt = %d\n",dCnt);

    getchar();
  }
  else
    { /* TI Data is already in a bank structure.  Bump the pointer */

      // ALEX
      //      for(ii = 0; ii < dCnt; ii++){
      //        printf("TI = 0x%x ",dma_dabufp[ii]); fflush(0);
      //      }
      //      printf("\n");

      dma_dabufp += dCnt;     

      if(tiGetIntCount() % 1000000 == 0)	printf("TI read block %d \n",dCnt);
    }

  
  //  board_type = -1;
  
  
#ifdef SHM_BUF
  /* Write DAQ Configuration bank */
  conf_output_write();
  //tiLatchTimers();
  //unsigned int tibusy=tiGetBusyTime();
  //unsigned int tilive=tiGetLiveTime();
  int tilivetime=tiLive(0); //-- 0:differential; 1:integrated
  shmem_ptr->Time_mark=tilivetime/10;
  //if(tiGetIntCount() % 1000 == 0) printf("TI busy= %d live=%d  = %ld \n",tibusy,tilive,shmem_ptr->Time_mark);
#endif

  
  if((use_trd  == 1) && (board_type == fadc125)) board_type = fadc250;
  if((use_muon == 1) && (board_type == fadc125)) board_type = fadc250;

  //  MAXFADCWORDS = 15 * (1 + 2 + 100*16 + 1) + 2*32;
  MAXFADCWORDS = 95000;
  
  // ALEX
  //  MAXFADCWORDS = 120000;


  /* ---------------------------------  FADC 250 ------------------------------ */

  if(board_type == fadc250){
    
    if(NFADC_250 != 0){
      
      // Alex
      BANKOPEN(6,BT_UI4,blockLevel);
      
      
      /* Timeout set to 1000; */
      FA_SLOT = fadcID[0];

      for(itime = 0; itime < timeout_fa250; itime++){        
        gbready = faGBready();
        stat = (gbready == fadcSlotMask);
        
        if (stat > 0){
          break;
        }
      }
      
      if(itime == timeout_fa250){

        getchar();

        printf(BOLDRED   "Timeout reading out FA250  (faGBready: 0x%08x != 0x%08x) \n" RESET, gbready, fadcSlotMask);
        daLogMsg("ERROR","Timeout reading out FA250  (faGBready: 0x%08x != 0x%08x) \n",       gbready, fadcSlotMask);
        
        for(iboard = 0; iboard < NFADC_250; iboard++){
          faResetToken(fadcID[iboard]);
        }
        
        *dma_dabufp++  = LSWAP(0xD8000001);
        *dma_dabufp++  = LSWAP(fadcSlotMask);
        *dma_dabufp++  = LSWAP(gbready);

        dCnt=0;        
      }
      
      if(stat > 0){
        
        if(NFADC_250 >1) roflag = 2; /* Use token passing scheme to readout all modules */
        
        dCnt = faReadBlock(FA_SLOT, dma_dabufp, MAXFADCWORDS, roflag);
        
          
        //        if(tiGetIntCount() % 100000 == 0){
        if(tiGetIntCount() % 1000 == 0){
          printf(" FADC250 words %d\n",dCnt);
          //          printf(" FADC250 words %d  %d\n",dCnt,tiGetIntCount() );
        }

        if(dCnt <= 0){
          printf(BOLDRED    "No data or error reading out FA250.  dCnt  =  %d\n" RESET, dCnt);
          daLogMsg("ERROR", "No data or error reading out FA250.  dCnt  =  %d\n",       dCnt);
          
          *dma_dabufp++  = LSWAP(0xD8000002);       
          *dma_dabufp++  = LSWAP(fadcSlotMask);
          *dma_dabufp++  = LSWAP(faGTokenStatus());
          
        } else {
          // Alex 1
          dma_dabufp += dCnt;          

        }
        


        if(faGetBlockError(1) != FA_BLOCKERROR_NO_ERROR){
          
          printf(BOLDRED    "%s: DMA error during read out of FA250. TokenStatus = 0x%08x,   gbready = 0x%08x \n" RESET,__FUNCTION__, faGTokenStatus(), gbready);
          daLogMsg("ERROR", "%s: DMA error during read out of FA250. TokenStatus = 0x%08x,   gbready = 0x%08x \n",      __FUNCTION__, faGTokenStatus(), gbready);
          
          uint32_t berr_status = 0;
          
          for(iboard = 0; iboard < NFADC_250; iboard++){
            
            FA_SLOT = fadcID[iboard];

            int bit = (faReadCSR(FA_SLOT) & FA_CSR_BERR_STATUS) ? 1 : 0;
            berr_status |= (bit << FA_SLOT);
            
            printf("\t %2d: CSR = 0x%08x - %d\n", FA_SLOT, faReadCSR(FA_SLOT), (faReadCSR(FA_SLOT) & FA_CSR_BERR_STATUS) ? 1 : 0);
            daLogMsg("ERROR"," %s ERROR: Data corruption for fadc250 in slot %d \n",__FUNCTION__, FA_SLOT );
          }
          
          *dma_dabufp++  = LSWAP(0xD8000004);       
          *dma_dabufp++  = LSWAP(fadcSlotMask);
          *dma_dabufp++  = LSWAP(faGTokenStatus());
          *dma_dabufp++  = LSWAP(berr_status);
          
        }
      }  
      
      /* Reset the Token */
      if(roflag == 2) {
        for(iboard = 0; iboard < NFADC_250; iboard++){
          FA_SLOT = fadcID[iboard];
          faResetToken(FA_SLOT);
        }
      }
      
      BANKCLOSE;
      
    }    //   NFADC_250  > 0   
  }      //   Board type FA250
  
  

  if((use_trd  == 1) && (board_type == fadc250)) board_type = fadc125;
  if((use_muon == 1) && (board_type == fadc250)) board_type = fadc125;


  /* ---------------------------------  FADC 125  ------------------------------ */
  
  if(board_type == fadc125){
    
    if(NFADC_125 != 0){
      
      BANKOPEN(16,BT_UI4,blockLevel);

      //printf("NFADC_125 = %d \n",NFADC_125); 
      
      roflag = 1;
      if(NFADC_125 > 1) roflag = 2;
      
      for(itime = 0; itime  < timeout_fa125; itime++){
        gbready = fa125GBready();
        stat = (gbready == fa125ScanMask());
        if(stat > 0)
          break;
      }
      
      if(itime == timeout_fa125){

        printf(BOLDRED   "Timeout reading out FA125  (fa125GBready: 0x%08x != 0x%08x)\n" RESET, gbready, fa125ScanMask());
        daLogMsg("ERROR","Timeout reading out FA125  (fa125GBready: 0x%08x != 0x%08x) \n",      gbready, fa125ScanMask());
        
        fa125GStatus(0);

        // Alex
        if(roflag == 2)
          fa125ResetToken(fa125Slot(0));
        
        for(iboard = 0; iboard < nfadc125; iboard++){
          fa125Reset(fa125Slot(iboard), 0);
        }        
        
        *dma_dabufp++  = LSWAP(0xD8000001);
        *dma_dabufp++  = LSWAP(fa125ScanMask());
        *dma_dabufp++  = LSWAP(gbready);
        
        dCnt=0;
      }
      
      if(stat > 0){
        
        // Alex
        //        dCnt = fa125ReadBlock(0,(volatile UINT32 *)dma_dabufp, 99000, 0x2); //-- max: 399620 Bytes (old:95000)
        //        dCnt = fa125ReadBlock(0,(volatile UINT32 *)dma_dabufp, 99000, 0x1); //-- max: 399620 Bytes (old:95000)
        if(roflag == 2)
          dCnt = fa125ReadBlock(0,(volatile UINT32 *)dma_dabufp, 99000, 0x2); //-- max: 399620 Bytes (old:95000)
        else 
          dCnt = fa125ReadBlock(0,(volatile UINT32 *)dma_dabufp, 99000, 0x1); //-- max: 399620 Bytes (old:95000)


        if(tiGetIntCount() % 10000 == 0 || tiGetIntCount() < 100)
          printf(" READ FADC125  %d Ev=%d\n",dCnt,tiGetIntCount());      
 
        if(fa125ReadBlockStatus(1) != 0) 
          printf(" .. token mask = 0x%08x\n",fa125GetTokenMask());		
        
        if(dCnt <= 0){
          
          fa125ResetToken(fa125Slot(0));
          
          printf(BOLDRED    "No data or error reading out FA125.  dCnt  =  %d\n" RESET, dCnt);
          daLogMsg("ERROR", "No data or error reading out FA125.  dCnt  =  %d\n", dCnt);
          
          *dma_dabufp++  = LSWAP(0xD8000002);       
          *dma_dabufp++  = LSWAP(fa125ScanMask());
          *dma_dabufp++  = LSWAP(fa125GetTokenMask());
          
        } else {   
          // Alex 2
          dma_dabufp += dCnt;          
        }      
        


        if(fa125ReadBlockStatus(0)){

          fa125GStatus(0);
        
          printf(BOLDRED    "%s: Read block error during read out of FA125. TokenStatus = 0x%08x,  Mask  = 0x%08x,   gbready = 0x%08x \n" RESET,__FUNCTION__, 
                 fa125GetTokenMask(), fa125ScanMask(), gbready);
          daLogMsg("ERROR", "%s: Read block error during read out of FA125. TokenStatus = 0x%08x,  Mask  = 0x%08x,   gbready = 0x%08x \n",      __FUNCTION__, 
                   fa125GetTokenMask(), fa125ScanMask(), gbready);
          
          // sdPrintBusyoutCounters();
          // sdPrintTrigoutCounters();
          
          
          for(iboard = 0; iboard < nfadc125; iboard++) {
            fa125Reset(fa125Slot(iboard), 0);
          }        

          *dma_dabufp++  = LSWAP(0xD8000004);       
          *dma_dabufp++  = LSWAP(fa125ScanMask());
          *dma_dabufp++  = LSWAP(fa125GetTokenMask());

          // printf("**** AFTER RESET(0) ****\n");
          // fa125GStatus(0);
          // printf("token mask = 0x%08x\n",fa125GetTokenMask());
        }
      }
      
      // Alex
      if(roflag == 2)
        fa125ResetToken(fa125Slot(0));
    
      BANKCLOSE;
      
    }
  }    
  

  /* ---------------------------------  F1 TDC ------------------------------ */

  if(board_type == f1tdc){

    BANKOPEN(26,BT_UI4,blockLevel);
    
    unsigned int f1_scanmask = f1ScanMask();
    /* f1 timeout 100 */
    for(itime = 0; itime < timeout_f1; itime++) {
      gbready = f1GBready();
      stat = (gbready == f1_scanmask);
      if (stat > 0) {
	break;
      }
    }

    if(itime == timeout_f1){
      printf(BOLDRED   "Timeout reading F1TDC  (f1GBready: 0x%08x != 0x%08x) \n" RESET, gbready, f1_scanmask);
      daLogMsg("ERROR","Timeout reading F1TDC  (f1GBready: 0x%08x != 0x%08x) \n",       gbready, f1_scanmask);
      
      for(iboard = 0; iboard < nf1tdc; iboard++) {
        f1ResetToken(f1Slot(iboard));
      }

      *dma_dabufp++  = LSWAP(0xD8000001);
      *dma_dabufp++  = LSWAP(f1_scanmask);
      *dma_dabufp++  = LSWAP(gbready);
      
      dCnt=0;      
    }


    if(stat > 0) {
      
      dCnt = f1ReadBlock(f1Slot(0), dma_dabufp, 95000, 2);

      if(tiGetIntCount() % 1000 == 0)
        printf(" READ F1  %d \n",dCnt);
      
      if(dCnt <= 0){
        
        printf(BOLDRED    "No data or error reading out F1TDC.  dCnt  =  %d\n" RESET, dCnt);
        daLogMsg("ERROR", "No data or error reading out F1TDC.  dCnt  =  %d\n",       dCnt);
        
        *dma_dabufp++  = LSWAP(0xD8000002);       
        *dma_dabufp++  = LSWAP(f1_scanmask);
        *dma_dabufp++  = LSWAP(f1GTokenStatus());
        
      } else {
        // Alex 3
        dma_dabufp += dCnt;
      }
      

      if(f1GetBlockError(1) != F1_BLOCKERROR_NO_ERROR){
        
        printf(BOLDRED    "%s: DMA error during read out of F1TDC. TokenStatus = 0x%08x,   gbready = 0x%08x \n" RESET,__FUNCTION__, f1GTokenStatus(), gbready);
        daLogMsg("ERROR", "%s: DMA error during read out of F1TDC. TokenStatus = 0x%08x,   gbready = 0x%08x \n",      __FUNCTION__, f1GTokenStatus(), gbready);
        

        uint32_t berr_status = 0;

        for(iboard = 0; iboard < nf1tdc; iboard++){

          int bit =  (f1ReadCSR(f1Slot(iboard)) & F1_CSR_BERR_STATUS) ? 1 : 0;
          berr_status |= (bit << f1Slot(iboard));

          printf(BOLDRED "\t %2d: CSR = 0x%08x - %d\n" RESET, f1Slot(iboard), f1ReadCSR(f1Slot(iboard)), (f1ReadCSR(f1Slot(iboard)) & F1_CSR_BERR_STATUS) ? 1 : 0);
          daLogMsg("ERROR"," %s ERROR: Data corruption for F1TDC in slot %d \n",__FUNCTION__, f1Slot(iboard) );
        }
       
        *dma_dabufp++  = LSWAP(0xD8000004);       
        *dma_dabufp++  = LSWAP(f1_scanmask);
        *dma_dabufp++  = LSWAP(f1GTokenStatus());
        *dma_dabufp++  = LSWAP(berr_status);
        
      }
    }  
    
    for(iboard = 0; iboard < nf1tdc; iboard++)
      f1ResetToken(f1Slot(iboard));
    
    BANKCLOSE;
  }    
  
  

  /* ---------------------------------  CAEN TDC 1290  ------------------------------ */

  if(board_type == tdc1290){

    vmeDmaConfig(2,3,0); // MBLT only for CBLT

    BANKOPEN(20,BT_UI4,blockLevel);
    
    /* Timeout set to 100; */
    for(itime = 0; itime < timeout_tdc1290; itime++) {
      datascan = tdc1190Dready(0);      
      if (datascan > 0) {
	break;
      }
    }
    
    if(itime == timeout_tdc1290) {
      printf(BOLDRED    "Timeout reading out CAEN TDC1290 in event %d \n" RESET, tiGetIntCount());
      daLogMsg("ERROR", "Timeout reading out CAEN TDC1290 in event %d \n",       tiGetIntCount());

      *dma_dabufp++ = LSWAP(0xda000bad);
      dCnt=0;        
    }



    if(datascan > 0) {
      /* Get the TDC data from all modules... rflag=2 for Linked List DMA 
         "64" is ignored in Linux */
      // Multi-block readout

      dCnt = tdc1190CBLTReadBlock(0, dma_dabufp, 9500, 2);

      if(tiGetIntCount() % 100000 == 0)
        printf(" CAEN TDC1290  words %d\n",dCnt);

      if(dCnt <= 0) {
        printf(BOLDRED    "No data or error reading out CAEN TDC1290.  dCnt  =  %d\n" RESET, dCnt);
        daLogMsg("ERROR", "No data or error reading out CAEN TDC1290.  dCnt  =  %d\n",       dCnt);
	
        *dma_dabufp++ = LSWAP(0xda000bad);
      }  else {
        // Alex 4
        dma_dabufp += dCnt;
      }
    } 

    *dma_dabufp++ = LSWAP(0xd00dd00d); /* Event EOB */
    
    BANKCLOSE;
    
    // Restore DMA for the next TI eadout
    vmeDmaConfig(2,5,1); 
    
  }



  /* ---------------------------------  SSP ----------------------------------- */

  if(board_type == ssp_mod){

    BANKOPEN(40,BT_UI4,blockLevel);
    
    int nssp = sspGetNssp();
    int slot, type, len;
    
    for(ii = 0; ii < nssp; ii++){
      
      slot = sspSlot(ii);
      
      for(itime=0; itime<100000; itime++) 
        {
          //          vmeBusLock();
          gbready = sspBReady(slot);
          //          vmeBusUnlock();
          if(gbready){
            break;
          }
          //          else
          //            printf("SSP NOT READY (slot=%d)\n",slot); fflush(stdout);
        }
      
      if(!gbready)
        {
          printf("SSP NOT READY (slot=%d)\n",slot); fflush(stdout);        
          printf(" ------  TEST ----------- \n");
          sspStatus(3,2);
          sspStatus(4,2);
          sspStatus(5,2);

          daLogMsg("ERROR","SSP module in Slot %d is not ready, restart the run    \n", slot);

          tiReset(1);

        }
      
      //          vmeBusLock();
      dCnt  = sspReadBlock(slot,dma_dabufp,0x10000,1);
      //          vmeBusUnlock();

      //printf("Ev=%d Slot =  %d.   SSP words: %d dma_dabufp=%d \n",tiGetIntCount(),slot,dCnt,dma_dabufp-dma1);

      if(tiGetIntCount() % 5000 == 0){

        printf("Event=%d DIRC Slot =  %d.   SSP words: %d \n",tiGetIntCount(),slot,dCnt);
        
        
        // ALEX
        //        for(jj = 0; jj < 20; jj++){
        //          printf("  0X%08X", dma_dabufp[jj]);
        //        }
        //        printf("\n");

      }      

      
      if(dCnt > 0){
        // Alex 5
        dma_dabufp += dCnt;         
      }


    }
    
    BANKCLOSE;
    
  }
  
#ifdef USE_HD
  hd_trigger();
#endif


  if(syncFlag){    
    if(board_type == fadc250){
      //  Alex
      fa250_pedestals();
      fa250_scalers();
      
      //      printf(BOLDBLUE   "Check FADC250 Config   %d  \n  "  RESET,  fa250_check_conf() );

    }
  }
  
  
#if 0
  if( strcmp(roc_name,  "rocbcal1")   == 0){
    taskDelay(2);    
  }

   if( strcmp(roc_name,  "rocbcal7")   == 0){
    taskDelay(2);    
  } 

   if( strcmp(roc_name,  "rocfcal1")   == 0){
     taskDelay(2);
     printf("I am here \n");
   }  

   if( strcmp(roc_name,  "rocfcal12")   == 0){
     taskDelay(2);    
   }  

   if( strcmp(roc_name,  "roccdc1")   == 0){
     taskDelay(2);    
   }  

   if( strcmp(roc_name,  "rocfdc1")   == 0){
     taskDelay(2);    
   }  

   if( strcmp(roc_name,  "rocfdc11")   == 0){
     taskDelay(2);    
   }  
#endif

   

  /* Set TI output 0 low */
  //  tiSetOutputPort(0,0,0,0);


#ifdef SHM_BUF
  gettimeofday(&tvB[2], NULL);
  int idiff=tvB[2].tv_usec - tvA[2].tv_usec +  1000000 * (tvB[2].tv_sec - tvA[2].tv_sec);
  //---- Fill Hist send wait for shmem  ------------
  hf1(hrol1,2,idiff); 
  hf1(hrol1,3,idiff/1000.); 
  // move to tiprim:  shmem_ptr->VME_Time=idiff;
  unsigned int eventSize =  dma_dabufp - dma1;
  shmem_ptr->VME_Size=eventSize;
#endif

}

void
rocCleanup()
{

  int islot = 0;
  int rc = -1;

  printf("%s: Reset all Modules\n",__FUNCTION__);


  if(board_type == fadc250)
    rc = fa250_cleanup();

  if(board_type == f1tdc)
    rc = f1_cleanup();

  if(board_type == tdc1290)
    vmeDmaFreeLLBuffer();

}
