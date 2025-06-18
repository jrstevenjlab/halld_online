/*************************************************************************
 *
 *  gt_list.c       - Library of routines for readout and buffering of 
 *                    events using a JLAB Trigger Interface V3 (TI) with 
 *                    a Linux VME controller in CODA 3.0.
 * 
 *                    This is for a TI in Slave Mode controlled by a
 *                    Master TI or Trigger Supervisor and hosts one
 *                    or more SubSystem Processors (SSPs) and a
 *                    Global Trigger Processor (GTP)
 *
 */

/* Event Buffer definitions */
#define MAX_EVENT_POOL     10
#define MAX_EVENT_LENGTH   2048*64      /* Size in Bytes */

/* Measured longest fiber length in system */
#define FIBER_LATENCY_OFFSET 0xCA
// #define FIBER_LATENCY_OFFSET 0x4A

#define TI_INIT


/* Define TI Type (TI_MASTER or TI_SLAVE) */
#define TI_SLAVE
/* TS Fiber Link trigger source (from TI Master, TD, or TS), POLL for available data */
#define TI_READOUT TI_READOUT_TS_POLL 
/* TI VME address, or 0 for Auto Initialize (search for TI by slot) */
#define TI_ADDR  0           

#define SSP_INIT_GTP_FIBERSUM_ENABLE 0


#undef SHM_BUF


#ifdef SHM_BUF
#include "shmem_roc.h"
static roc_shmem *shmem_ptr;
SHM_HIST *hrol1;
struct timeval tvA[10], tvB[10]; 
#endif


#include "dmaBankTools.h"   /* Macros for handling CODA banks */
#include "tiprimary_list.c" /* Source required for CODA readout lists using the TI */
#include "sspLib.h"         /* Include SSP library stuff */
#include "gtpLib.h"

#include "conf_utils.h" 
#include "conf_utils.c" 


/* Extern global variables */

extern unsigned int  sspAddrList[MAX_VME_SLOTS+1];
// int nSSP;   /* Number of SSPs found with sspInit(..) */

#ifdef SHM_BUF
#include "fadcLib.h"
#include "monitor.c"
#endif

/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{

  int stat = 0, iFlag = 0, iport = 0, slot; 
  int rc = -1;



  int Nssp_conf = 0;



  gethostname(roc_name,ROCLEN);
  
  /*  Global config file  */
  strcpy(filename,rol->usrConfig);

  //  char filename1[100] = "/home/hdops/CDAQ/config/hd_all/ECAL_m10.conf";
  //  strcpy(filename,filename1);


  printf(BOLDMAGENTA " Global Config File Name %s \n" RESET ,filename);

  rc = read_conf_global(filename,roc_name);  
  if(rc !=  0){
    printf(BOLDRED " FATAL: Fail to read global configuration file %d \n" RESET, rc);
    daLogMsg("ERROR"," Fail to read global configuration file \n");
    ROL_SET_ERROR;
  }

  print_gtp_conf();

  // Initialize TI
  // Overwrite fiber latency according to the config file
  if(ti_bd.fiber_latency > 0){
    tiFiberLatencyOffset = ti_bd.fiber_latency;
    printf(BOLDMAGENTA "Set TI latency offset to: 0x%x \n" RESET, ti_bd.fiber_latency);
  } else {
    tiFiberLatencyOffset = FIBER_LATENCY_OFFSET;
    printf(BOLDMAGENTA "Set TI latency offset to the default value: 0x%x \n" RESET,  FIBER_LATENCY_OFFSET);
  }
  
  tiInit(TI_ADDR,TI_READOUT,0);

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


  /*****************
   *   TI SETUP
   *****************/

  /* Set the sync delay width to 0x40*32 = 2.048us */
  tiSetSyncDelayWidth(0x54, 0x7F, 1);

  /* Set Trigger Buffer Level */
  tiSetBlockBufferLevel(bufferLevel);

  /* Init the SD library so we can get status info */
  stat = sdInit();
  if(stat==0) 
    {
      tiSetBusySource(TI_BUSY_SWB,1);
      sdSetActiveVmeSlots(0);
      sdStatus(0);
    }
  else
    { /* No SD or the TI is not in the Proper slot */
      tiSetBusySource(TI_BUSY_LOOPBACK,1);
    }
  
  tiStatus(0);

  /*****************
   *   SSP SETUP
   *****************/

  iFlag = SSP_INIT_MODE_DISABLED; /* Disabled, initially */
  iFlag |= (1<<15);               /* activate using sspAddrList  */

  // Alex
  iFlag |= (1<<13);


  /*  BCAL SSP */
  //  sspInit((8<<19), (1<<19), 1, iFlag); /* Initialize the SSP in slot 9 */
  /*  PSC  SSP */
  //  sspInit((5<<19), (1<<19), 1, iFlag); /* Initialize the SSP in slot 9 */

  Nssp_conf = 0;

  for(slot = 3; slot < 21; slot++){
    if(ssp[slot].slot == slot){
      sspAddrList[Nssp_conf] = (slot<<19);
      Nssp_conf++;
    }
  }

  printf(" Initialize SSP with the address list   %d  0x%x \n", Nssp_conf, sspAddrList[0]);

  sspInit(sspAddrList[0], 0, Nssp_conf, iFlag); 

  //   sspGStatus(0);


  /*****************
   *   GTP SETUP
   *****************/

  /* Initialize GTP.  NULL (arg 2) specifies library to access GTP through TI (I2C)
     to determine it's network hostname */
  rc = gtpInit(0,NULL);

  if(rc !=  0){
    rc = gtpInit(0,"halldgtp1");
  }

  if(rc !=  0){
    printf(BOLDRED "FATAL: gtpInit failed. Press Reset Button on Run Control %d \n" RESET, rc);
    daLogMsg("ERROR",     "gtpInit failed. Press Reset Button on Run Control  \n");    
    ROL_SET_ERROR;
  }
  
  

  /* Clock source is Internal before prestart */
  gtpSetClockSource(GTP_CLK_CTRL_INT);
  gtpSetSyncSource(GTP_SD_SRC_SEL_SYNC);
  gtpSetTrig1Source(GTP_SD_SRC_SEL_TRIG1);

  gtpEnableVmeSlotMask(sspSlotMask());


  printf("\n");
  printf("  TI FIBER LATENCY MEASUREMENT  DOWNLOAD %d \n", tiGetFiberLatencyMeasurement());
  printf("\n");

  int ti_delta_t  = -10;

  if(strncmp(roc_name, "roctrig2",  8) == 0) {    
    ti_delta_t = tiGetFiberLatencyMeasurement() - trig_ti_offset;
    if(abs(ti_delta_t) > 1) {
      printf( BOLDRED   "Wrong TI FIBER MEASUREMENT: DELTA T  =  %d \n\n" RESET, ti_delta_t);
      daLogMsg("ERROR", "Wrong TI FIBER MEASUREMENT: DELTA T  =  %d   \n",ti_delta_t);
      ROL_SET_ERROR; 
    }
  }
  

 #ifdef SHM_BUF
  monitor_download();
 #endif

  printf("rocDownload: User Download Executed\n");

}


/****************************************
 *  PRESTART
 ****************************************/
void
rocPrestart()
{

  unsigned int iFlag = 0;
  int stat;
  int issp, iport = 0;


  unsigned int trig_mask = 0;
  unsigned int reg_par   = 0;
  unsigned int mask;

  int ntrig = 0;
  
  int sub_syst = 0;

  int gtp_lane = 0;

  int TrigBitMask[16];


  /* Check the health of the vmeBus Mutex.. re-init if necessary */
  //  vmeCheckMutexHealth(10);

  /* Set GTP to use the global clock */
  gtpSetClockSource(GTP_CLK_CTRL_P0);


  /* Configure the SSP modules */
  //  iFlag  = SSP_INIT_MODE_VXS;


  // CHECK PSC HIT PATTERN
  //  iFlag |= SSP_INIT_FIBER1_ENABLE;
  //  iFlag  |= SSP_INIT_GTP_FIBER1_ENABLE;

  // BCAL 
 //  iFlag  |=  SSP_INIT_GTP_FIBERSUM_ENABLE;


  // Enable SSP and GTP ports

  int nSSP = sspGetNssp();

  for(issp = 0; issp < nSSP; issp++) {
    iFlag = 0;

    int slot = sspSlot(issp);
    if(ssp[slot].slot != slot) continue;
    
    iFlag  = SSP_INIT_MODE_VXS;

    for(iport = 0; iport < 8; iport++){
      if( (ssp[slot].fiber_en &  (1 << iport)) ){
	switch(iport){
	case 0:  
	  iFlag |= SSP_INIT_FIBER0_ENABLE; 
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER0_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 0 Enabled \n" RESET, slot);
	  break;	  
	case 1:  
	  iFlag |= SSP_INIT_FIBER1_ENABLE; 
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER1_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 1 Enabled \n" RESET, slot);
	  break;  
	case 2:  
	  iFlag |= SSP_INIT_FIBER2_ENABLE;
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER2_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 2 Enabled \n" RESET, slot);
	  break;  
	case 3:  
	  iFlag |= SSP_INIT_FIBER3_ENABLE;
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER3_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 3 Enabled \n" RESET, slot);
	  break;  	  
	case 4:  
	  iFlag |= SSP_INIT_FIBER4_ENABLE; 
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER4_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 4 Enabled \n" RESET, slot);
	  break;
	case 5:  
	  iFlag |= SSP_INIT_FIBER5_ENABLE; 
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER5_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 5 Enabled \n" RESET, slot);
	  break;
	case 6:  
	  iFlag |= SSP_INIT_FIBER6_ENABLE; 
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER6_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 6 Enabled \n" RESET, slot);
	  break;
	case 7:  
	  iFlag |= SSP_INIT_FIBER7_ENABLE; 
	  if(ssp[slot].sum_enable == 0)  iFlag  |= SSP_INIT_GTP_FIBER7_ENABLE;
	  printf(BOLDBLUE " SSP_SLOT %d : Fiber 7 Enabled \n" RESET, slot);
	  break;
	default: break;
	}
      }       
    }
    
    if(ssp[slot].sum_enable != 0 ){
      iFlag  |=  SSP_INIT_GTP_FIBERSUM_ENABLE;   
      printf(BOLDBLUE " SSP_SLOT %d : GTP_FIBERSUM_ENABLE \n" RESET, slot);
    }
    
    printf("%s: iFlag = 0x%08x\n",__FUNCTION__,iFlag);
    
    sspSetMode(slot, iFlag, 1);
    taskDelay(2);

    /* Reset crate IDs on all ports */
    for(iport = 0; iport < 8; iport++){
      if( ssp[slot].fiber_en << iport )
	sspResetCrateId(slot,iport);
    }

    sspStatus(slot,3);

  }


  gtpEnableVmeSlotMask(sspSlotMask());


  //  gtpSetTriggerEnableMask(GTP_TRG_CTRL_TOF);
  //  gtpSetTriggerEnableMask(GTP_TRG_CTRL_BCAL_E);

  // BCAL  
  //  gtpSetTriggerEnableMask(GTP_TRG_CTRL_BCAL_C | GTP_TRG_CTRL_BCAL_E);
  //  PSC Crates used in the configuration
  //  gtpSetTriggerEnableMask( GTP_TRG_CTRL_PS );


  trig_mask = 0;
  
  for(ntrig = 0; ntrig < MAX_EQ; ntrig++){
    
    if(gtp_eq[ntrig].enable == 1){

      printf( BOLDRED " SASCHA  %s \n" RESET, gtp_eq[ntrig].type);

      if( strcmp(gtp_eq[ntrig].type, "PS") == 0)           trig_mask  |=  GTP_TRG_CTRL_PS;
      else if( strcmp(gtp_eq[ntrig].type, "ECAL") == 0)    trig_mask  |=  GTP_TRG_CTRL_FCAL;  
      else if( strcmp(gtp_eq[ntrig].type, "FCAL") == 0)    trig_mask  |=  GTP_TRG_CTRL_FCAL; 
      else if( strcmp(gtp_eq[ntrig].type, "BCAL_E") == 0)  trig_mask  |=  GTP_TRG_CTRL_BCAL_E;
      else if( strcmp(gtp_eq[ntrig].type, "CCAL") == 0)    trig_mask  |=  GTP_TRG_CTRL_COMCAL_E;
      else if( strcmp(gtp_eq[ntrig].type, "BCAL_C") == 0)  trig_mask  |=  GTP_TRG_CTRL_BCAL_C;
      else if( strcmp(gtp_eq[ntrig].type, "TOF") == 0)     trig_mask  |=  GTP_TRG_CTRL_TOF;
      else if( strcmp(gtp_eq[ntrig].type, "ST") == 0)      trig_mask  |=  GTP_TRG_CTRL_ST;
      else if( strcmp(gtp_eq[ntrig].type, "TAGM") == 0)    trig_mask  |=  GTP_TRG_CTRL_TAGM;
      else if( strcmp(gtp_eq[ntrig].type, "TAGH") == 0)    trig_mask  |=  GTP_TRG_CTRL_TAGH;
      else {
	printf( BOLDRED " ERROR: Unknown Trigger Mask  %s \n" RESET, gtp_eq[ntrig].type);
	daLogMsg("ERROR", "  Unknown Trigger Mask  %s  \n",    gtp_eq[ntrig].type);
	ROL_SET_ERROR;
      }

      gtpSetTriggerEnableMask( trig_mask );
      
      printf("Trigger MASK, CHECK 0x%x \n",gtpGetTriggerEnableMask());

    }    
  }
  

#if 0
// BCAL coscmic trigger
//  gtpBCalCosmicLUTSetup(GTP_BCAL_COSMIC_TYPE_ANY_N, 2);
  { 
    int type = GTP_BCAL_COSMIC_TYPE_PATTERN;
    // North
    //    int pattern = 0x1008;
    // South
    //    int pattern = 0x810;
    // Combined 
    int pattern = 0x4020;
    //    int pattern = 0x1;
    gtpBCalCosmicLUTSetup(type, pattern);

    printf("gtpBCalCosmicLUTSetup(%d,%08X)\n", type, pattern);
  }
#endif


  //  printf("%s: gtpGetTriggerEnableMask() = 0x%08x\n",__FUNCTION__,gtpGetTriggerEnableMask());

  //  gtpSetSubsystemDelayWidth(GTP_TRIGPER_TOF, 40, 10);
  //  gtpSetSubsystemDelayWidth(GTP_TRIGPER_BCAL, 40, 10);
  // BCAL
  //  gtpSetSubsystemDelayWidth(GTP_TRIGPER_BCAL, 20, 10);
  //  gtpSetSubsystemDelayWidth(GTP_TRIGPER_PS, 20, 10);

  /* Set up trigger bit 0 out - TOF N Hits logic*/
  //  gtpSetTriggerBitEnableMask(0, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_TOF_NHITS_EN);



  // Set global trigger settings. Can probably be combined with the previous loop.
  for(ntrig = 0; ntrig < MAX_EQ; ntrig++){
    if(gtp_eq[ntrig].enable == 1){
      
      // PS
      if( strcmp(gtp_eq[ntrig].type, "PS") == 0){ 
	gtpSetSubsystemDelayWidth(GTP_TRIGPER_PS, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);	
      }  
      // ECAL
      else if( strcmp(gtp_eq[ntrig].type, "ECAL") == 0){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_ECAL, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);

        gtpSetSubsystemGain(GTP_TRIGPER_ECAL, gtp_gain.gain_ecal);
        gtpSetSubsystemGain(GTP_TRIGPER_FCAL, gtp_gain.gain_fcal);
      } 
      // FCAL
      else if( strcmp(gtp_eq[ntrig].type, "FCAL") == 0){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_FCAL, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      } 
      // BCAL_E and BCAL_C
      else if(  (strcmp(gtp_eq[ntrig].type, "BCAL_E") == 0) ||  (strcmp(gtp_eq[ntrig].type, "BCAL_C") == 0)){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_BCAL, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      }
      // CCAL
      else if( strcmp(gtp_eq[ntrig].type, "CCAL") == 0){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_COMCAL, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      }
      // TOF
      else if( strcmp(gtp_eq[ntrig].type, "TOF") == 0){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_TOF, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      }
      // ST
      else if( strcmp(gtp_eq[ntrig].type, "ST") == 0){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_ST, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      }
      // TAGM
      else if( strcmp(gtp_eq[ntrig].type, "TAGM") == 0){
        gtpSetSubsystemDelayWidth(GTP_TRIGPER_TAGM, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      }
      // TAGH
      else if( strcmp(gtp_eq[ntrig].type, "TAGH") == 0){
	gtpSetSubsystemDelayWidth(GTP_TRIGPER_TAGH, gtp_eq[ntrig].delay, gtp_eq[ntrig].int_width);
      } else {
	printf( BOLDRED " ERROR: Unknown Trigger Mask  %s \n" RESET, gtp_eq[ntrig].type);
	daLogMsg("ERROR", "  Unknown Trigger Mask  %s  \n",    gtp_eq[ntrig].type);
	ROL_SET_ERROR;
      }
    }    
  }



  //  Disable all triggers 
  for(ntrig = 0; ntrig < 16; ntrig++){
    gtpSetTriggerBitEnableMask(ntrig, 0);
    TrigBitMask[ntrig] = 0;
    reg_par = 0;
  }

  for(ntrig = 0; ntrig < MAX_TRIG; ntrig++){

    if( (gtp_trig[ntrig].gtp_lane >= 0) &&  (gtp_trig[ntrig].gtp_lane < 16)){  
      if( strcmp(gtp_trig[ntrig].type, "PS") == 0){
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_PS_COIN_EN  );
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |= (GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_PS_COIN_EN);
	gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width );
        gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_PS_MASK , 0xFFFF);
      }   
      //  BFCAL
      else if( strcmp(gtp_trig[ntrig].type, "BFCAL") == 0){
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_BFCAL_EN );
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE  | GTP_TRIGBIT_CTRL_BFCAL_EN | 
						   GTP_TRIGBIT_CTRL_BCAL_EN_EN  |  GTP_TRIGBIT_CTRL_FCAL_EN_EN );

	gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_FCAL_ENERGY_SCALE, gtp_trig[ntrig].fcal_en);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BCAL_ENERGY_SCALE, gtp_trig[ntrig].bcal_en);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_COMCAL_ENERGY_SCALE, 0);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BFCAL_ENERGY_THRESHOLD, gtp_trig[ntrig].en_thr);
	

	if( ((gtp_trig[ntrig].fcal_min >= 0) && (gtp_trig[ntrig].fcal_min <= 65535 )) &&
	    ((gtp_trig[ntrig].fcal_max >= 0) && (gtp_trig[ntrig].fcal_max <= 65535 )) &&
	    (gtp_trig[ntrig].fcal_max > gtp_trig[ntrig].fcal_min )){
	  
	  unsigned int en_range =  ((gtp_trig[ntrig].fcal_min & 0xFFFF) | ((gtp_trig[ntrig].fcal_max & 0xFFFF) << 16));
	  
	  gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_FCAL_ENERGY_RANGE, en_range);
	  
	  printf(BOLDBLUE "\n SET BFCAL ENERGY RANGE:  FCAL_MIN =  %d   BCAL_MAX = %d  \n\n" RESET,
		 gtp_trig[ntrig].fcal_min, gtp_trig[ntrig].fcal_max ); 
	  
	} else {
	  printf(BOLDRED    " WRONG BFCAL ENERGY RANGE: FCAL_MIN   %d   FCAL_MAX   %d  \n" RESET, gtp_trig[ntrig].fcal_min, gtp_trig[ntrig].fcal_max );
	  daLogMsg("ERROR", " WRONG BFCAL ENERGY RANGE: FCAL_MIN   %d   FCAL_MAX   %d  \n", gtp_trig[ntrig].fcal_min, gtp_trig[ntrig].fcal_max );	  
	}
	
	if( ((gtp_trig[ntrig].bcal_min >= 0) && (gtp_trig[ntrig].bcal_min <= 65535 )) &&
	    ((gtp_trig[ntrig].bcal_max >= 0) && (gtp_trig[ntrig].bcal_max <= 65535 )) &&
	    (gtp_trig[ntrig].bcal_max > gtp_trig[ntrig].bcal_min )){
	  
	  unsigned int en_range =  ( (gtp_trig[ntrig].bcal_min & 0xFFFF) |  ((gtp_trig[ntrig].bcal_max & 0xFFFF) << 16) );
	  
	  gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BCAL_ENERGY_RANGE, en_range);
	  
	  printf(BOLDBLUE "\n SET BFCAL ENERGY RANGE:  BCAL_MIN =  %d   BCAL_MAX = %d  \n\n" RESET,
		 gtp_trig[ntrig].bcal_min, gtp_trig[ntrig].bcal_max ); 
	  
	} else {
	  printf(BOLDRED    " WRONG BFCAL ENERGY RANGE  BCAL_MIN  %d  BCAL_MAX   %d   \n" RESET, gtp_trig[ntrig].bcal_min, gtp_trig[ntrig].bcal_max );
	  daLogMsg("ERROR", " WRONG BFCAL ENERGY RANGE  BCAL_MIN  %d  BCAL_MAX    %d  \n", gtp_trig[ntrig].bcal_min, gtp_trig[ntrig].bcal_max );	  
	}
	
        
	printf(BOLDBLUE "SET BFCAL ENERGIES: FCAL_EN =  %d   BCAL_EN = %d   EN_THR = %d \n" RESET,
	       gtp_trig[ntrig].fcal_en, gtp_trig[ntrig].bcal_en, gtp_trig[ntrig].en_thr);
        
      } 
      
      
      //  CFCAL
      
      else if( strcmp(gtp_trig[ntrig].type, "CFCAL") == 0){
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_BFCAL_EN );
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE  | GTP_TRIGBIT_CTRL_BFCAL_EN | 
                                                   GTP_TRIGBIT_CTRL_FCAL_EN_EN  |  GTP_TRIGBIT_CTRL_COMCAL_EN_EN);


        // Use BCAL/CCAL field in the config file to set up CCAL

        gtp_trig[ntrig].ccal_min  = gtp_trig[ntrig].bcal_min ;
        gtp_trig[ntrig].ccal_max  = gtp_trig[ntrig].bcal_max ;
        gtp_trig[ntrig].ccal_en   = gtp_trig[ntrig].bcal_en ;     
        
        
	gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_FCAL_ENERGY_SCALE, gtp_trig[ntrig].fcal_en);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_COMCAL_ENERGY_SCALE, gtp_trig[ntrig].ccal_en);
        gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BCAL_ENERGY_SCALE, 0);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BFCAL_ENERGY_THRESHOLD, gtp_trig[ntrig].en_thr);
	
        
	if( ((gtp_trig[ntrig].fcal_min >= 0) && (gtp_trig[ntrig].fcal_min <= 65535 )) &&
	    ((gtp_trig[ntrig].fcal_max >= 0) && (gtp_trig[ntrig].fcal_max <= 65535 )) &&
	    (gtp_trig[ntrig].fcal_max > gtp_trig[ntrig].fcal_min )){
	  
	  unsigned int en_range =  ((gtp_trig[ntrig].fcal_min & 0xFFFF) | ((gtp_trig[ntrig].fcal_max & 0xFFFF) << 16));
	  
	  gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_FCAL_ENERGY_RANGE, en_range);
	  
	  printf(BOLDBLUE "\n SET CFCAL ENERGY RANGE:  FCAL_MIN =  %d   FCAL_MAX = %d  \n\n" RESET,
		 gtp_trig[ntrig].fcal_min, gtp_trig[ntrig].fcal_max ); 
	  
	} else {
	  printf(BOLDRED    " WRONG CFCAL ENERGY RANGE: FCAL_MIN   %d   FCAL_MAX   %d  \n" RESET, gtp_trig[ntrig].fcal_min, gtp_trig[ntrig].fcal_max );
	  daLogMsg("ERROR", " WRONG CFCAL ENERGY RANGE: FCAL_MIN   %d   FCAL_MAX   %d  \n", gtp_trig[ntrig].fcal_min, gtp_trig[ntrig].fcal_max );	  
	}
	
	if( ((gtp_trig[ntrig].ccal_min >= 0) && (gtp_trig[ntrig].ccal_min <= 65535 )) &&
	    ((gtp_trig[ntrig].ccal_max >= 0) && (gtp_trig[ntrig].ccal_max <= 65535 )) &&
	    (gtp_trig[ntrig].ccal_max > gtp_trig[ntrig].ccal_min )){
	  
	  unsigned int en_range =  ((gtp_trig[ntrig].ccal_min & 0xFFFF) | ((gtp_trig[ntrig].ccal_max & 0xFFFF) << 16));
	  
	  gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_COMCAL_ENERGY_RANGE, en_range);
	  
	  printf(BOLDBLUE "\n SET CFCAL ENERGY RANGE:  CCAL_MIN =  %d   CCAL_MAX = %d  \n\n" RESET,
		 gtp_trig[ntrig].ccal_min, gtp_trig[ntrig].ccal_max ); 
	  
	} else {
	  printf(BOLDRED    " WRONG CFCAL ENERGY RANGE: CCAL_MIN   %d   CCAL_MAX   %d  \n" RESET, gtp_trig[ntrig].ccal_min, gtp_trig[ntrig].ccal_max );
	  daLogMsg("ERROR", " WRONG CFCAL ENERGY RANGE: CCAL_MIN   %d   CCAL_MAX   %d  \n", gtp_trig[ntrig].ccal_min, gtp_trig[ntrig].ccal_max );	  
	}
        
        
        
	printf(BOLDBLUE "SET CFCAL ENERGIES: FCAL_EN =  %d   CCAL_EN  = %d   EN_THR = %d \n" RESET,
	       gtp_trig[ntrig].fcal_en, gtp_trig[ntrig].ccal_en, gtp_trig[ntrig].en_thr); 
      } 




      // BCAL_COSMIC
      else  if( strcmp(gtp_trig[ntrig].type, "BCAL_COS") == 0){
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_BCAL_COSMIC_EN  );
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE |  GTP_TRIGBIT_CTRL_BCAL_COSMIC_EN);
	gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
	//	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_FCAL_ENERGY_SCALE, gtp_trig[ntrig].fcal_en);
	//	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BCAL_ENERGY_SCALE, gtp_trig[ntrig].bcal_en);
	//	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BFCAL_ENERGY_THRESHOLD, gtp_trig[ntrig].en_thr);
      }
      // TOF
      else  if( strcmp(gtp_trig[ntrig].type, "TOF") == 0){
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_TOF_NHITS_EN  );
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE |  GTP_TRIGBIT_CTRL_TOF_NHITS_EN);
	gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
	//	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_MASK, 0xFFFFFFFF);
        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_MASK, 0x80008000);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_MASK, 0xFFFFFFFF);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_HITCOUNT_THRESHOLD, gtp_trig[ntrig].nhit);
	reg_par = gtpGetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_HITCOUNT_THRESHOLD);
	if(reg_par != gtp_trig[ntrig].nhit){
	  printf(BOLDBLUE "TOF:  Number of hits is not set %d   %d \n" RESET, reg_par, gtp_trig[ntrig].nhit );
	  daLogMsg("ERROR", "TOF:  Number of hits is not set %d  \n",reg_par, gtp_trig[ntrig].nhit);	  
	  ROL_SET_ERROR;	  
	} else 
	  printf(BOLDMAGENTA "TOF:  Number of hits is  set to  %d \n" RESET, reg_par);
      }       
      // ST 
      else  if( strcmp(gtp_trig[ntrig].type, "ST") == 0){
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_ST_NHITS_EN );
        //        gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_ST_NHITS_EN |  GTP_TRIGBIT_CTRL_TOF_NHITS_EN);
        //	gtpSetTriggerBitEnableMask(gtp_trig[ntrig].gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_BFCAL_EN | GTP_TRIGBIT_CTRL_ST_NHITS_EN | GTP_TRIGBIT_CTRL_TOF_NHITS_EN);
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_ST_NHITS_EN);
        gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_ST_MASK, 0x10008000);
        gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_ST_MASK, 0xFFFFFFFF);
	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_ST_HITCOUNT_THRESHOLD, gtp_trig[ntrig].nhit);


        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_FCAL_ENERGY_SCALE, gtp_trig[ntrig].fcal_en);
        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BCAL_ENERGY_SCALE, gtp_trig[ntrig].bcal_en);
        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_BFCAL_ENERGY_THRESHOLD, gtp_trig[ntrig].en_thr);

        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_MASK, 0xFFFFFFFF);
        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_HITCOUNT_THRESHOLD, gtp_trig[ntrig].nhit);
        //	gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane,  GTP_TOF_HITCOUNT_THRESHOLD, 6);
      }       
      // TAGM
      else  if( strcmp(gtp_trig[ntrig].type, "TAGM") == 0){
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_TAGM_PATTERN_EN );
        gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
        if((gtp_trig[ntrig].mask >= 0) &&  (gtp_trig[ntrig].mask <= 0xFFFFFFFF)){ 
          printf( BOLDBLUE "Set GTP MASK for TAGM to 0x%X\n" RESET, gtp_trig[ntrig].mask);
          gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_TAGM_MASK, gtp_trig[ntrig].mask);
	} else {
          printf( BOLDBLUE "GTP MASK for TAGM is not set in the config file. Enable all bits\n" RESET);
          gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_TAGM_MASK, 0xFFFFFFFF);	 
	}
      }
      // TAGH
      else  if( strcmp(gtp_trig[ntrig].type, "TAGH") == 0){
        TrigBitMask[gtp_trig[ntrig].gtp_lane] |=  (GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_TAGH_PATTERN_EN );
        gtpSetTrigoutLatencyWidth(gtp_trig[ntrig].gtp_lane, gtp_trig[ntrig].latency, gtp_trig[ntrig].width);
	if((gtp_trig[ntrig].mask >= 0) &&  (gtp_trig[ntrig].mask <= 0xFFFFFFFF)){ 
	  printf( BOLDBLUE "Set GTP MASK for TAGH to 0x%X\n" RESET, gtp_trig[ntrig].mask);
	  gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_TAGH_MASK, gtp_trig[ntrig].mask);	  
	} else {
	   printf( BOLDBLUE "GTP MASK for TAGH is not set in the config file. Enable all bits\n" RESET);
	   gtpSetTrigoutParameter(gtp_trig[ntrig].gtp_lane, GTP_TAGH_MASK, 0xFFFFFFFF);	 
	}

      } else {
	printf( BOLDRED " ERROR: Unknown Trigger Type  %s \n" RESET, gtp_trig[ntrig].type);
	daLogMsg("ERROR", "  Unknown Trigger Type  %s  \n",    gtp_trig[ntrig].type);
	ROL_SET_ERROR;
      }
      
    } else {
      printf( BOLDBLUE " ERROR: Invalid Trigger Lane  %d \n" RESET, gtp_trig[ntrig].gtp_lane);
      //      daLogMsg("ERROR", " Invalid Trigger Lane  %d \n",      gtp_trig[ntrig].gtp_lane);
      //      ROL_SET_ERROR;	
    }
  }
  

  printf(BOLDBLUE " TRIGGER LANES ENABLED \n" RESET );
  for(gtp_lane = 0; gtp_lane < 16; gtp_lane++){
    if(TrigBitMask[gtp_lane] != 0){
      gtpSetTriggerBitEnableMask(gtp_lane, TrigBitMask[gtp_lane]);
      printf("\n");
      printf(BOLDBLUE " LANE:  %d   TrigBitMask =   0x%X   \n" RESET, gtp_lane, TrigBitMask[gtp_lane] );
    }
  }


#if 0
  int gtp_lane;
  
  for(gtp_lane = 0; gtp_lane < 16; gtp_lane++){
    //    gtpSetTriggerBitEnableMask(gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_BFCAL_EN );
    // BCAL
    //    gtpSetTriggerBitEnableMask(gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_BCAL_COSMIC_EN );
    // PSC
    gtpSetTriggerBitEnableMask(gtp_lane, GTP_TRIGBIT_CTRL_ENABLE | GTP_TRIGBIT_CTRL_PS_COIN_EN  );
    
    
    gtpSetTrigoutLatencyWidth(gtp_lane, 420, 20);
    

    
    /* Equation for TOF_hitpattern
       TOFMasked=TOFHitPattern&TOFHitPattern_Mask
       TOFhori = TOFMasked(15:0)
       TOFvert = TOFMasked(31:16)
       (bit_cnt(TOFMasked) > TOFNHits_Thr) &&
       vec_or(TOFhori) &&
       vec_or(TOFvert) 
    */
    
    /* Set trigbit 0 equation parameters */
    //  gtpSetTrigoutParameter(0, GTP_TOF_MASK, 0x00011111); /* As prescribed by fadc playback data  */
    //  gtpSetTrigoutParameter(0, GTP_TOF_HITCOUNT_THRESHOLD, 0); /* Atleast 1 hit */
    
#if 0
    /* BCAL */
    gtpSetTrigoutParameter(gtp_lane, GTP_BCAL_ENERGY_SCALE, 1); /* As prescribed by fadc playback data  */
    gtpSetTrigoutParameter(gtp_lane, GTP_FCAL_ENERGY_SCALE, 0); /* Atleast 1 hit */
    gtpSetTrigoutParameter(gtp_lane, GTP_BFCAL_ENERGY_THRESHOLD, 100);
#endif

    
    /* PSC*/
    gtpSetTrigoutParameter(0, GTP_PS_MASK , 0xFFFF); /* As prescribed by fadc playback data  */


  } 
#endif



  //   Make sure that optical links are up
  for(issp = 0; issp < nSSP; issp++) {
    
    int slot = sspSlot(issp);
    
    if(ssp[slot].slot != slot) continue;
    
    int iwait = 0;
    
    while(sspCheckLinksUp(slot) && iwait < 1000){
      taskDelay(1);
      iwait++;
    }
    

    if(iwait == 1000){
      printf(BOLDRED   " FATAL: Optical links are not up for the SSP in slot = %d. Links = 0x%x   \n" RESET, slot, sspCheckLinksUp(slot) );
      daLogMsg("ERROR"," FATAL: Optical links are not up for the SSP in slot = %d. Links = 0x%x   \n" RESET, slot, sspCheckLinksUp(slot) );
      //      ROL_SET_ERROR;
    }

  }
  

  sspGStatus(0);
  
  vmeClearException(1);

  gtpStatus(0);
  
  printf("rocPrestart: User Prestart Executed\n");


}

/****************************************
 *  GO
 ****************************************/
void
rocGo()
{
  int issp, iFlag=0;
  int connected_ports=0;

  /* Get the broadcasted Block Level from TS or TI Master */
  if(blockLevel != tiGetCurrentBlockLevel() ){
    daLogMsg("ERROR", "Block Level does not match  %d  %d \n",  blockLevel,  ts_bd.blocklevel);  
    printf(BOLDRED "ERROR: Block Level does not match  %d  %d \n " RESET, blockLevel, ts_bd.blocklevel);
    ROL_SET_ERROR;
  } else {
    printf(BOLDBLUE "rocGo: Block Level set to %d\n" RESET, blockLevel);
  }

  sspGStatus(0);
  gtpStatus(0);


  // Check Latency Errors

  printf(BOLDMAGENTA "Check GTP Latency 0x%x \n\n" RESET, gtpCheckTrigOutLatency());

  if(gtpCheckTrigOutLatency() != 0){
    daLogMsg("ERROR",     "GTP Latency Check Failed  0x%x \n ", gtpCheckTrigOutLatency() );  
    printf(BOLDRED "FATAL: GTP Latency Check Failed  0x%x \n " RESET, gtpCheckTrigOutLatency() ); 
    ROL_SET_ERROR;
  }



}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{
  int islot;

  tiStatus(0);
  sspGStatus(0);
  gtpStatus(0);

  printf("rocEnd: Ended after %d blocks\n",tiGetIntCount());
  
}

/****************************************
 *  TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{
  int ii, islot;
  int stat, dCnt, len=0, idata;
  unsigned int val;
  unsigned int *start;
  unsigned int intCount=0;
  int syncEvent = tiGetSyncEventFlag();
  int syncEventRec = tiGetSyncEventReceived();
   
  intCount = tiGetIntCount();

  if(syncEvent)
    printf("%d: Sync Event\n",intCount);

  if(syncEventRec)
    printf("%d: Sync Event Received\n",intCount);

  /* Set TI output 1 high for diagnostics */
  tiSetOutputPort(1,0,0,0);

  /* Readout the trigger block from the TI 
     Trigger Block MUST be reaodut first */
  dCnt = tiReadTriggerBlock(dma_dabufp);
  if(dCnt<=0) 
    {
      printf("%10d: No data or error.  dCnt = %d\n",intCount,dCnt);
    }
  else
    { /* TI Data is already in a bank structure.  Bump the pointer */
      dma_dabufp += dCnt;
    }


  /* EXAMPLE: How to open a bank (type=5) and add data words by hand */
  BANKOPEN(5,BT_UI4,blockLevel);
  *dma_dabufp++ = tiGetIntCount();
  *dma_dabufp++ = 0xdead;
  *dma_dabufp++ = 0xcebaf111;
  *dma_dabufp++ = 0xcebaf222;
  BANKCLOSE;

  /* Set TI output 0 low */
  tiSetOutputPort(0,0,0,0);

}

void
rocCleanup()
{
  int islot=0;

  printf("%s: Reset all Modules\n",__FUNCTION__);
  
}
