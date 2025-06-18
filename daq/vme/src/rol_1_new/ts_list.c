/*************************************************************************
 *
 *  ts_list.c - Library of routines for readout and buffering of 
 *                events using a JLAB Pipeline Trigger Supervisor (TS) with 
 *                a Linux VME controller in CODA 3.0
 *
 */

/* Event Buffer definitions */
//#define MAX_EVENT_POOL     20
#define MAX_EVENT_POOL     10
#define MAX_EVENT_LENGTH   2048*64      /* Size in Bytes */

/* Define Interrupt source and address */
#define TS_READOUT TS_READOUT_EXT_POLL  /* Poll for available data, external triggers */
#define TS_ADDR    (21<<19)          /* GEO slot 3 */


#include "dmaBankTools.h"
#include "tsprimary_list.c" /* source required for CODA */
#include "tdLib.h"      

#include "conf_utils.h" 
#include "conf_utils.c" 

/* Copied from the top of tsLib.c  2014-11-14 DL */
extern pthread_mutex_t   tsMutex;
#define TSLOCK     if(pthread_mutex_lock(&tsMutex)<0) perror("pthread_mutex_lock");
#define TSUNLOCK   if(pthread_mutex_unlock(&tsMutex)<0) perror("pthread_mutex_unlock");
extern volatile struct TS_A24RegStruct  *TSp;


//extern unsigned int  tdAddrList[MAX_VME_SLOTS+1];
extern unsigned int  tdAddrList[22];
extern int tdID[21];
extern int nTD;

#define BLOCKLEVEL  1

extern unsigned int tsTriggerSource;


/* function prototype */
void rocTrigger(int arg);

/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{
  int rc = -1, iFlag = 0;
  
  int stat;
  int slot, lane, ts_input, rule;

  int nTD_conf = 0;  

  memset(&ts_bd, 0, sizeof(ts_bd));
  
  gethostname(roc_name,ROCLEN);

  /*  Global config file  */
  strcpy(filename,rol->usrConfig);


  //  char filename1[100] = "/home/hdops/CDAQ/config/hd_all/ECAL_m10.conf";
  //  strcpy(filename,filename1);
  
  printf(BOLDMAGENTA " Global Config File Name %s \n" RESET ,filename);


  rc = read_conf_global(filename,roc_name);

  if(rc !=  0){
    printf(BOLDRED " FATAL: Fail to read global configuration file %d \n" RESET, rc);
    daLogMsg("ERROR", "Fail to read global configuration file \n");    
    ROL_SET_ERROR;
  }
  

  print_ts_conf();

  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  vmeDmaConfig(2,5,1); 

  /* Define BLock Level */
  blockLevel = BLOCKLEVEL;

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

  /*****************
   *   TS SETUP
   *****************/

  
  //  Internal Pulser
  if(ts_bd.trig_type == 1){
    tsSetTriggerSource(TS_TRIGSRC_PULSER);
    tsSetFPInput(0);  // Disable FP inputs
    tsSetGTPInput(0);                  // Disable GTP
    printf(BOLDBLUE "TS Trigger Source:  Pulser \n" RESET);
  }

  // External Triggers
  else if (ts_bd.trig_type == 2){
    tsSetTriggerSourceMask(TS_TRIGGER_VME | TS_TRIGGER_LOOPBACK |
			   TS_TRIGGER_ENABLE );
    tsSetFPInput(ts_bd.ts_fp_inputs);
    tsSetGTPInput(0);                  // Disable GTP
    printf(BOLDBLUE "TS Trigger Source:  External Triggers \n" RESET);
  } 

  //  GTP only 
  else if (ts_bd.trig_type == 4){
    tsSetTriggerSourceMask(TS_TRIGGER_VME | TS_TRIGGER_LOOPBACK |
    			   TS_TRIGGER_ENABLE );
       
    tsSetFPInput(0);
    tsSetGTPInput(0xFFFF);

    printf(BOLDBLUE "TS Trigger Source:  GTP \n" RESET);
    for(lane = 0; lane < 16; lane++){
      tsSetTriggerPrescale(1, lane, 0);
      if(ts_bd.gtp_pres[lane] > 0){
	tsSetTriggerPrescale(1, lane, ts_bd.gtp_pres[lane]);
	printf(BOLDBLUE "Set TS prescaling for the GTP trigger lane %d  to %d\n" RESET, lane, ts_bd.gtp_pres[lane]);
      }
    }
  }

  //  GTP + TS internal pulser (Playback mode) 
  else if (ts_bd.trig_type == 5){

    tsSetTriggerSourceMask(TS_TRIGGER_VME | TS_TRIGGER_LOOPBACK |
			   TS_TRIGGER_ENABLE | TS_TRIGGER_PULSER);
   
    tsSetFPInput(0);
    tsSetGTPInput(0xFFFF);

    printf(BOLDBLUE "TS Trigger Source:  GTP + Internal Pulser (Playback mode)  \n" RESET);
    for(lane = 0; lane < 16; lane++){
      tsSetTriggerPrescale(1, lane, 0);
      if(ts_bd.gtp_pres[lane] > 0){
	tsSetTriggerPrescale(1, lane, ts_bd.gtp_pres[lane]);
	printf(BOLDBLUE "Set TS prescaling for the GTP trigger lane %d  to %d\n" RESET, lane, ts_bd.gtp_pres[lane]);
      }
    }
  }

  //  GTP  +  External  
  else if (ts_bd.trig_type == 6){
    tsSetTriggerSourceMask(TS_TRIGGER_VME | TS_TRIGGER_LOOPBACK |
			   TS_TRIGGER_ENABLE );
    tsSetFPInput(ts_bd.ts_fp_inputs);  // Enable Front Pannel Inputs

    tsSetGTPInput(0xFFFF);  

    printf(BOLDBLUE "TS Trigger Source:  GTP + External triggers\n" RESET);
    for(lane = 0; lane < 16; lane++){
      tsSetTriggerPrescale(1, lane, 0);
      if(ts_bd.gtp_pres[lane] > 0){
	tsSetTriggerPrescale(1, lane, ts_bd.gtp_pres[lane]);
	printf(BOLDBLUE "Set TS prescaling for the GTP trigger lane %d  to %d\n" RESET, lane, ts_bd.gtp_pres[lane]);
      }
    }    
  } 

  // GTP + External triggers + TS internal pulser 
  else if (ts_bd.trig_type == 7){
    tsSetTriggerSourceMask(TS_TRIGGER_VME | TS_TRIGGER_PULSER | TS_TRIGGER_LOOPBACK |
                           TS_TRIGGER_ENABLE );
    tsSetFPInput(ts_bd.ts_fp_inputs);  // Enable Front Pannel Inputs
    tsSetGTPInput(0xFFFF);  

    printf(BOLDBLUE "TS Trigger Source:  GTP + External triggers + TS pulser \n" RESET);
    for(lane = 0; lane < 16; lane++){
      tsSetTriggerPrescale(1, lane, 0);
      if(ts_bd.gtp_pres[lane] > 0){
        tsSetTriggerPrescale(1, lane, ts_bd.gtp_pres[lane]);
        printf(BOLDBLUE "Set TS prescaling for the GTP trigger lane %d  to %d\n" RESET, lane, ts_bd.gtp_pres[lane]);
      }
    }
  } else {
    printf(BOLDRED "ERROR:  Unknown Trigger Source  %d \n", ts_bd.trig_type);
    daLogMsg("ERROR", "Unknown Trigger Source  %d \n", ts_bd.trig_type);    
    ROL_SET_ERROR;
  }

  
  // Set TS FP delay  
  for(ts_input = 0; ts_input < 32; ts_input++){
    tsSetFPDelay(ts_input,ts_bd.ts_fp_delay[ts_input]);
  }


  // SASHA 2022
  //  tsSetTriggerPrescale(2,5,4);


  printf("\n");
  tsPrintFPDelay();


  /* Set Crate ID to the defined ROC ID) */
  tsSetCrateID(ROCID);
  printf(" TS CrateID register set to %d\n ",ROCID);
  
  // Does not exist in new libs
  //  tsSetGenInput(0);
  //  tsSetGTPInput(0x2);


  /* Load the default trigger table */
  tsLoadTriggerTable();


  //
  //   Trigger Holdoff rules: 
  //
  //
  //   Rule 1:  16 ns  (range 0),  480 ns  (range 1)
  //   Rule 2:  16 ns  (range 0),  960 ns  (range 1)
  //   Rule 3:  32 ns  (range 0), ~2 usec  (range 1)
  //   Rule 4:  64 ns  (range 0), ~4 usec  (range 1)
  //
  //       Default values
  //       --------------
  //
  //  tsSetTriggerHoldoff(1, 62, 1);
  //  tsSetTriggerHoldoff(2, 2,  0);
  //  tsSetTriggerHoldoff(3, 3,  0);
  //  tsSetTriggerHoldoff(4, 32, 0);
  
  //  tsSetTriggerHoldoff(1,62,1);  //--- ~ 30 kHz
  //  tsSetTriggerHoldoff(1,20,0);  //--- ~2.7 MHz,   / FDC fa125 , does not work ?


  for(rule = 0; rule < 4; rule++){   
    if( (ts_bd.ts_trig_hold[rule][0] >= 0 && ts_bd.ts_trig_hold[rule][0] < 128) && 
        (ts_bd.ts_trig_hold[rule][1] >= 0 && ts_bd.ts_trig_hold[rule][1] < 2)  ){    
      tsSetTriggerHoldoff(rule + 1, ts_bd.ts_trig_hold[rule][0], ts_bd.ts_trig_hold[rule][1]);
      int hold_value = ts_bd.ts_trig_hold[rule][0]*ts_rule_scale[rule][ts_bd.ts_trig_hold[rule][1]];
      printf(BOLDBLUE " Set TriggerHoldoff: Rule  =  %d,   Value  =  %3d,  Range =  %3d     (%6d  ns)  \n" RESET,  
             rule + 1, ts_bd.ts_trig_hold[rule][0], ts_bd.ts_trig_hold[rule][1],hold_value);
    } else {
      printf(BOLDRED    " Wrong settings for TriggerHoldoff:  Rule  %d   %d   %d \n"  RESET, rule + 1, ts_bd.ts_trig_hold[rule][0], ts_bd.ts_trig_hold[rule][1]);
      daLogMsg("ERROR", " Wrong settings for TriggerHoldoff:  Rule  %d   %d   %d \n", rule + 1, ts_bd.ts_trig_hold[rule][0], ts_bd.ts_trig_hold[rule][1]);
      ROL_SET_ERROR;
    }
    
  }

  printf(BOLDMAGENTA "TS Hold off rules set to: \n" RESET);
  printf(BOLDMAGENTA "                          Rule 1  = 0x%x  \n" RESET, tsGetTriggerHoldoff(1));
  printf(BOLDMAGENTA "                          Rule 2  = 0x%x  \n" RESET, tsGetTriggerHoldoff(2));
  printf(BOLDMAGENTA "                          Rule 3  = 0x%x  \n" RESET, tsGetTriggerHoldoff(3));
  printf(BOLDMAGENTA "                          Rule 4  = 0x%x  \n" RESET, tsGetTriggerHoldoff(4));

  /* Set the sync delay width to 0x40*32 = 2.048us */

  // DEFAULT
  tsSetSyncDelayWidth(0x30, 0x7F, 1);
  //  tsSetSyncDelayWidth(0x54, 0x7F, 1);
  

  // Set Sync Events
  if( ts_bd.ts_sync_int > 0 && ts_bd.ts_sync_int <  0x00FFFFFF ){
    printf(BOLDMAGENTA " SYNC Event period is set to:  %d\n" RESET, ts_bd.ts_sync_int);
    tsSetSyncEventInterval(ts_bd.ts_sync_int);
  } else if ( ts_bd.ts_sync_int == 0){
    printf(BOLDMAGENTA " Sync Events are disabled\n" RESET);
    tsSetSyncEventInterval(0);
  } else {
    printf(BOLDRED    " SYNC Event period is out of range:  %d\n" RESET, ts_bd.ts_sync_int);
    printf(BOLDRED    " Sync Events are disabled\n" RESET);
    daLogMsg("ERROR", " SYNC Event period is out of range:  %d.  Sync Events are disabled\n", ts_bd.ts_sync_int);
    tsSetSyncEventInterval(0);
  }   



  /* Set timestamp format 48 bits */
  tsSetEventFormat(3);
  tsSetGTPInputReadout(1);  
  tsSetFPInputReadout(1);   

  // ALEX TEST
  //  tsSetDataReadout(1);


  if (ts_bd.ts_coin_wind > 0 && ts_bd.ts_coin_wind < 256){
    tsSetTrigCoinWindow(ts_bd.ts_coin_wind);
    if(tsGetTrigCoinWindow() != ts_bd.ts_coin_wind){
      printf(BOLDRED " Fail to set tsGetTrigCoinWindow  = %d  %d\n" RESET, ts_bd.ts_coin_wind, tsGetTrigCoinWindow());
    }
    printf(BOLDMAGENTA "Set TrigCoinWindow to %d \n" RESET, tsGetTrigCoinWindow());
  } else {
    printf(BOLDRED " Wrong value of TrigCoinWindow  = %d. Set to the default value = 5 \n" RESET, ts_bd.ts_coin_wind);
    daLogMsg("ERROR", " Wrong value of TrigCoinWindow  = %d. Set to the default value = 5 \n", ts_bd.ts_coin_wind);
    tsSetTrigCoinWindow(5);
  }  


  /* 
   * Set the Block Buffer Level
   *  0:  Pipeline mode
   *  1:  One Block per readout - "ROC LOCK" mode
   *  2-255:  "Buffered" mode.
   */
  tsSetBlockBufferLevel(bufferLevel);

  /* Set a Maximum Block count before trigger autmatically disables  (0 disables block limit)*/
  tsSetBlockLimit(0);

  /* Override the busy source set in tsInit (only if TS crate running alone) */


  nTD_conf = 0;

  // SASHA CHECK ECAL
  //  ts_bd.ts_td_slots = ( 1 << 14); ts_bd.ts_td_slots = ts_bd.ts_td_slots | ( 1 << 15);

  for(slot = 3; slot < 21; slot++){

    if( (ts_bd.ts_td_slots & ( 1 << slot)) ){

      printf(BOLDRED "CHECK TD = %d \n" RESET, slot);


      tdAddrList[nTD_conf] = (slot<<19);
      printf(BOLDMAGENTA " ENABLE TD SLOT = %d \n" RESET, slot);
      nTD_conf++;
    }
  }

  


  //  iFlag = 0;
  //  iFlag |= (1<<17);         


  // Initialization by address doesn't work
  //  printf( BOLDMAGENTA " NUMBER OF TDs FOUND =  %d \n" RESET, nTD_conf);  
  //  tdInit(tdAddrList[0], 0, nTD_conf, iFlag);
  //  printf( BOLDMAGENTA " NUMBER OF TDs FOUND by CONFIG =  %d   %d  %d \n" RESET, nTD, tdSlot(0), tdSlot(1) );  


  tdInit(0,0,0,0);

  tdGSetBlockBufferLevel(bufferLevel);





  /* Init SD Board. and add TD Slaves */
  sdInit();

  if(ts_bd.ts_td_slots != 0){
    //    bit  0: Vme Slot 0
    sdSetActiveVmeSlots( ts_bd.ts_td_slots );
    printf(BOLDMAGENTA " Set active SD slots  %d \n" RESET, ts_bd.ts_td_slots);
  } else {
    printf(BOLDRED "TD slots are not specified  %d \n" RESET, ts_bd.ts_td_slots);
    daLogMsg("ERROR", "TD slots are not specified  %d \n ", ts_bd.ts_td_slots);    
    ROL_SET_ERROR;
  }

  sdStatus(0);



  tsTriggerReadyReset();

  /*   tsSetPrescale(0); */
  
  printf("rocDownload: User Download Executed\n");

}

/****************************************
 *  PRESTART
 ****************************************/
void
rocPrestart()
{
  unsigned short iflag;
  int stat;
  int islot;

  int ii;

  printf(BOLDMAGENTA " ------  MAX_EVENT_LENGTH  ---------- %d \n" RESET, MAX_EVENT_LENGTH );


  if(use_playback)
    printf(BOLDMAGENTA " \n  Trigger Playback is activated \n\n" RESET);


  /* Set number of events per block */
  tsSetBlockLevel(blockLevel);
  printf("rocPrestart: Block Level to be broadcasted: %d\n",blockLevel);
  /* On TD's too */
  tdGSetBlockLevel(blockLevel);


  /* Reset Active ROC Masks on all TD modules */
  for (ii = 0; ii < nTD; ii++) {
      tdTriggerReadyReset(tdSlot(ii));
    }


  tdGStatus(0);
  tsStatus(0);

  printf(BOLDBLUE "\n rocPrestart: User Prestart Executed \n" RESET);

}

/****************************************
 *  GO
 ****************************************/
rocGo()
{

  int ii, islot, tmask;

  printf(BOLDBLUE "\n\n  ----  GO ---  \n\n " RESET );

  sdStatus(1);
  
  /* Enable TD module Ports that have indicated they are active */
  for (ii = 0; ii < nTD; ii++) {

    tdResetSlaveConfig(tdID[ii]);
    tmask = tdGetTrigSrcEnabledFiberMask(tdID[ii]);

    printf("TD (Slot %d) Source Enable Mask = 0x%x\n",tdID[ii],tmask);


    // Slot 7 - BCAL north
    // Slot 8 - BCAL south
    //    if(tdID[ii] == 7 || tdID[ii] == 14)
    //      if(tmask!=0) tdAddSlaveMask(tdID[ii],tmask);

    //    if(tdID[ii] == 7 || tdID[ii] == 8 || tdID[ii] == 14)
    //      if(tmask!=0) tdAddSlaveMask(tdID[ii],tmask);

    // CDC, BCAL, GTP
    //    if(tdID[ii] == 4 || tdID[ii] == 7 || tdID[ii] == 8 || tdID[ii] == 14)
    //      if(tmask!=0) tdAddSlaveMask(tdID[ii],tmask);

    // PS, PSC, ST
    //    if(tdID[ii] == 3 ||  tdID[ii] == 14)
    //      if(tmask!=0) tdAddSlaveMask(tdID[ii],tmask);


    


#if 0
    // ALEX SNAKE SCAN
    //    ts_bd.ts_td_slots = 0xC008;
    ts_bd.ts_td_slots = 0xC008;
    
    printf("SNAKE SCAN: TD MODULES = 0x%x \n",ts_bd.ts_td_slots);
    
    
    if( ts_bd.ts_td_slots & (1 << tdID[ii])){
      printf(" SASCHA TD SLOT = %d   Mask = 0x%x \n", tdID[ii], tmask );
      
      // ALEX SNAKE SCAN
      if(tdID[ii] != 3)
        if(tmask != 0) tdAddSlaveMask(tdID[ii],tmask);
      
      //      CCAL SNAKE SCAN
      if(tdID[ii] == 3){
        // CCAL + TAGGER
        //            tmask = 0x5;
        // CCAL + TAGGER + PS
        tmask = 0x3F;
        tdAddSlaveMask(tdID[ii],tmask);
        printf(BOLDRED " FORCE MASK 0x%X   for slot = %d \n" RESET, tmask, tdID[ii]);
      }
      
    }  else {
      //      printf(BOLDRED "ERROR: tdGetTrigSrcEnabledFiberMask: No connected TIs found for TD in slot %d \n" RESET, tdID[ii]);
      //      daLogMsg("ERROR", "tdGetTrigSrcEnabledFiberMask: No connected TIs found  for TD in slot %d", tdID[ii]);    
      //      ROL_SET_ERROR;
    }
    
    //    else {
    //      printf(BOLDRED "ERROR: Wrong TD in slot %d %x\n" RESET, tdID[ii]);
    //      daLogMsg("ERROR", "Wrong TD in slot %d  \n ", tdID[ii]);    
    //      ROL_SET_ERROR;	
    //    }
#endif
    
#if 1
    if( ts_bd.ts_td_slots & (1 << tdID[ii])){
      printf(" SASCHA TD SLOT = %d \n", tdID[ii]);
      if(tmask != 0) tdAddSlaveMask(tdID[ii],tmask);
      else {
        printf(BOLDRED "ERROR: tdGetTrigSrcEnabledFiberMask: No connected TIs found for TD in slot %d \n" RESET, tdID[ii]);
        daLogMsg("ERROR", "tdGetTrigSrcEnabledFiberMask: No connected TIs found  for TD in slot %d", tdID[ii]);    
        ROL_SET_ERROR;  
      }
    }
    else {
      //      printf(BOLDRED "ERROR: Wrong TD in slot %d %x\n" RESET, tdID[ii]);
      //      daLogMsg("ERROR", "Wrong TD in slot %d  \n ", tdID[ii]);    
      //      ROL_SET_ERROR;    
    }
#endif
   

  }
  
  
  //  Set periodic or random pulser

  if( (ts_bd.trig_type == 1) || (ts_bd.trig_type == 7 )){
    
    if(ts_bd.ts_soft_trig[3] == 2){
      printf(BOLDBLUE "\n Enable random pulser\n" RESET);
      tsSetRandomTrigger(1,ts_bd.ts_soft_trig[2]);
    } else {
      printf(BOLDBLUE "\n Enable periodic pulser\n" RESET);
      if(ts_bd.ts_soft_trig[1] == 0){
        tsSoftTrig(ts_bd.ts_soft_trig[0], 0xFFFF, ts_bd.ts_soft_trig[2], ts_bd.ts_soft_trig[3]);
       } else
        tsSoftTrig(ts_bd.ts_soft_trig[0], ts_bd.ts_soft_trig[1], ts_bd.ts_soft_trig[2], ts_bd.ts_soft_trig[3]); 
    }
  }
  

  /*  Enable Random at rate 500kHz/(2^7) = ~3.9kHz */
  //  tsSetRandomTrigger(1,0x7);
  //  tsSetRandomTrigger(1,0x6);  //  8 kHz
  //  tsSetRandomTrigger(1,0x5);  //  16 kHz
  //  tsSetRandomTrigger(1,0x4);  //  32 kHz
  //  tsSetRandomTrigger(1,0x3);  //  64 kHz
  

  if(use_playback){
    printf(BOLDMAGENTA " Enable Internal Pulser for the Play Back Trigger \n" RESET);
    
    if(ts_bd.ts_soft_trig[3] == 2){
      printf(BOLDBLUE "\n Enable random pulser\n" RESET);
      tsSetRandomTrigger(2, ts_bd.ts_soft_trig[2]);
    } else {
      tsSoftTrig(ts_bd.ts_soft_trig[0], ts_bd.ts_soft_trig[1], ts_bd.ts_soft_trig[2], ts_bd.ts_soft_trig[3]);
    }
  }

  tdGStatus(0);
  tsStatus(2);

  
#if 0
  printf(BOLDMAGENTA " RESET AND LATCH ALL SCALERS  \n" RESET);  
  TSLOCK;
  vmeWrite32(&TSp->reset, (TS_RESET_RESET_L1A_NUMBER | TS_RESET_LATCH_TIMERS) );
  
  uint32_t livetime     = vmeRead32(&TSp->livetime);
  uint32_t busytime     = vmeRead32(&TSp->busytime);
  uint32_t trigger      = vmeRead32(&TSp->trigger);
  TSUNLOCK;
#endif
  
}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{

  int islot;

  /* Stop Internal Pulse */
  if( (ts_bd.trig_type == 1) || (ts_bd.trig_type == 7 )){

    if(ts_bd.ts_soft_trig[3] == 2){
      /* Disable random trigger */
      tsDisableRandomTrigger();
    } else {
      /* Disable Fixed Rate trigger */
      tsSoftTrig(1,0,700,0);
    }
  }

  tsStatus(0);

  printf("rocEnd: Ended after %d blocks\n",tsGetIntCount());
  
}

/****************************************
 *  TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{
  int ii, islot;
  int stat, dCnt, len = 0, idata;
  int timeout;

  int syncFlag  =  0;

  syncFlag = tsGetSyncEventFlag();
    
  /* Readout the trigger block from the TS
     Trigger Block MUST be reaodut first */

  dCnt = tsReadTriggerBlock(dma_dabufp);

  if(dCnt<=0)
    {
      logMsg("No data or error.  dCnt = %d\n",dCnt);
    }
  else
    { /* TS Data is already in a bank structure.  Bump the pointer */

      dma_dabufp += dCnt;
      if(tsGetIntCount() % 10000 == 0){
      //        if(tsGetIntCount() % 2 == 0){
        printf("TS read block %d \n",dCnt);
        printf("Read out  TS  event %d \n", tsGetIntCount());
      }
    }

  if(syncFlag){
    
    printf("%s: Sync Flag Received at readout event %d\n", __FUNCTION__,tsGetIntCount());
    
    ts_scalers();    
  }
  
}

void
rocCleanup()
{
  int islot=0;

}
