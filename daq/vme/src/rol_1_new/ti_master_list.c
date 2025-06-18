/*************************************************************************
 *
 *  vme_list.c - Library of routines for readout and buffering of 
 *                events using a JLAB Trigger Interface V3 (TI) with 
 *                a Linux VME controller.
 *
 */

/* Event Buffer definitions */
#define MAX_EVENT_POOL     5
#define MAX_EVENT_LENGTH   400000      /* Size in Bytes */
/* Define Interrupt source and address */
#define TI_MASTER


#define TI_READOUT TI_READOUT_EXT_POLL  /* Poll for available data, external triggers */
#define TI_ADDR    0 

#ifdef SHM_BUF
#include "shmem_roc.h"
static roc_shmem *shmem_ptr;
SHM_HIST *hrol1;
struct timeval tvA[10], tvB[10]; 
#endif


#define TI_DATA_READOUT


/* Decision on whether or not to readout the TI for each block 
   - Comment out to disable readout 
*/


// #define FIBER_LATENCY_OFFSET 0xC7
#define FIBER_LATENCY_OFFSET 0x4A

#include "dmaBankTools.h"
#include "tiprimary_list.c" /* source required for CODA */

#include "conf_utils.h" 
#include "conf_utils.c" 

int  FA_SLOT;

#include "fa125_init.c"
#include "fa250_init.c"

extern int fadcA32Base;
extern int fa125A32Base;

#define BLOCKLEVEL  1
#define BUFFERLEVEL 1


bd_type  board_type = bd_unknown;
det_type detector_type  = -1;

#ifdef SHM_BUF
// #include "fadcLib.h"
// extern   int nfadc;
// extern   int fadcID[FA_MAX_BOARDS];
#include "monitor.c"
#endif


void rocTrigger(int arg);

/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{
  
  int rc = -1;
  
  int stat;

  int opt_port, port;
  
  memset(fa250,  0, sizeof(fa250));
  memset(fa125, 0, sizeof(fa125));
  memset(&ti_bd, 0, sizeof(ti_bd));
  memset(&ctp, 0, sizeof(ctp));

  fadcA32Base=0x09000000;
  
  gethostname(roc_name,ROCLEN);
  
  board_type = get_board_type(roc_name);

  detector_type = get_det_type(roc_name);

  /*  Global config file  */
  strcpy(filename,rol->usrConfig);
  printf(BOLDMAGENTA " Global Config File Name %s \n" RESET ,filename);
  
  rc = read_conf_global(filename,roc_name);
  
  if(rc !=  0){
    printf(BOLDRED " FATAL: Fail to read global configuration file %d \n" RESET, rc);
    daLogMsg("ERROR",      "Fail to read global configuration file \n");
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
  

  printf(BOLDMAGENTA " Use trd =  %d  Detector type = %d  Board type = %d \n" RESET, use_trd, detector_type, board_type);


  // TRD setup
  if( (use_trd == 1) && (detector_type == st) && (board_type == fadc250)){
    board_type = fadc125;
    fa125A32Base = fadcA32Base + 0x800000*9 + 0x800000;
    print_fadc125_conf(13);
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
  
  
  blockLevel = BLOCKLEVEL;


  print_ti_conf();

  /*****************
   *   TI SETUP
   *****************/
  /* 
   * Set Trigger source 
   *    For the TI-Master, valid sources:
   *      TI_TRIGGER_FPTRG     2  Front Panel "TRG" Input
   *      TI_TRIGGER_TSINPUTS  3  Front Panel "TS" Inputs
   *      TI_TRIGGER_TSREV2    4  Ribbon cable from Legacy TS module
   *      TI_TRIGGER_PULSER    5  TI Internal Pulser (Fixed rate and/or random)
   */
  
  
  int overall_offset=0x80;
  

  // Disable Soft and Random triggers
  tiDisableRandomTrigger();
  tiSoftTrig(1,0,700,0);
  tiSoftTrig(2,0,700,0);


     /*  Pulser  */
  if(ti_bd.ti_master_trig == 1){
    tiSetTriggerSource(TI_TRIGGER_PULSER);
    /* External Front Pannel */
  } else if(ti_bd.ti_master_trig == 2){
    tiSetTriggerSource(TI_TRIGGER_TSINPUTS);
    /* Play Back */
  } else if(ti_bd.ti_master_trig == 3){
    //    tiSetTriggerSource(TI_TRIGGER_PULSER | TI_TRIGGER_TSINPUTS);
    printf(BOLDRED " ENABLE TRIGEGR SOURCES \n" RESET); 
    tiSetTriggerSource(TI_TRIGGER_TSINPUTS);
    use_playback = 1;
  } else {
    printf(BOLDRED " Unknown trigger type %d \n" RESET, ti_bd.ti_master_trig);
    return(-2);
    // Return will be replaced by a function provided by Dave A.
  }

  /* Enable/Disable specific inputs */
  if( ((ti_bd.ti_master_trig == 2) || (ti_bd.ti_master_trig == 3))
      && ti_bd.ti_fp_inputs != 0){
    
    tiEnableTSInput(  ti_bd.ti_fp_inputs );
    
    printf(BOLDBLUE "Enable TI_FP_INPUTS     =  " RESET);    
    for(port = 0; port < 16; port++){
      if( ti_bd.ti_fp_inputs & (1 << port)) printf("  %d", port+1); 
    }
    printf("\n");
    printf("    0x%x \n",ti_bd.ti_fp_inputs);
    
  } 
  

  /* Load the trigger table that associates 
     pins 21/22 | 23/24 | 25/26 : trigger1
     pins 29/30 | 31/32 | 33/34 : trigger2
  */

  tiLoadTriggerTable(2);
  
  tiSetTriggerHoldoff(1,10,0);
  tiSetTriggerHoldoff(2,10,0);

  if(board_type == fadc125){  //---  test fa125 ----
    tiSetTriggerHoldoff(1,5,1);
  }

  /* Set the sync delay width to 0x40*32 = 2.048us */
  tiSetSyncDelayWidth(0x54, 0x40, 1);
  
  /* Set the busy source to non-default value (no Switch Slot B busy) */
  tiSetBusySource(TI_BUSY_LOOPBACK,1); 
  
  /* Set number of events per block */
  blockLevel = BLOCKLEVEL;
  tiSetBlockLevel(blockLevel);
  
  /* Set Trigger Buffer Level */
  tiSetBlockBufferLevel(BUFFERLEVEL);
  
  /* Set timestamp format 48 bits */
  tiSetEventFormat(3);
  
  
  for(opt_port = 2; opt_port < 7; opt_port++){
    if(ti_bd.ti_fiber_en & (1 << (opt_port - 1))){      
      tiAddSlave(opt_port);
      printf(BOLDBLUE "TI Add Slave  for optical port = %d  \n" RESET, opt_port);
    }
  }
  
  tiStatus(0);
  
  if(board_type == fadc125){
    printf(BOLDBLUE " FA125 Download \n" RESET);
    rc = fa125_download();
  }

  if(board_type == fadc250){
    rc = fa250_download();
    
    if(ctp.used == 1){
      printf(BOLDBLUE " CTP  Download \n" RESET);
      ctpInit(1);
      
      //    ctpSetFinalSumThreshold(3500,0);    
      //    printf(" CTP final sum threshold %d \n",ctpGetFinalSumThreshold());
      
      /* BCAL cosmic settings */
      //    ctpSetBCALWindowWidth(20);  
      //    ctpSetBCALThreshold(6000);
      
      //    Single crate cosmic test   
      //    unsigned int ctp_vmemask = (1<<9) | (1<<10);
      unsigned int ctp_vmemask = fadcSlotMask;
      
      if(strcmp(roc_name, "rocps2") == 0){
       	ctp_vmemask = (1<<13) | (1<<14) |  (1<<15) | (1<<16);
	printf(BOLDMAGENTA " ENABLE CTP SLOTS 0x%x \n" RESET, ctp_vmemask);
	//    ctp_vmemask = (1<<6);
      }
      
      
      ctpSetVmeSlotEnableMask(ctp_vmemask);
  
      if(strcmp(roc_name, "rocst") == 0){
	ctpSetSpectrometerMode(0, 10);
	printf(BOLDMAGENTA "SET CTP SPECTROMETER MODE TO 0 = \n" RESET);
      }
      
      
      if(strcmp(roc_name, "rocps2") == 0 ){
	
	if(ctp.HitWindowWidth > 0)
	  ctpSetSpectrometerMode(2, ctp.HitWindowWidth );
	else ctpSetSpectrometerMode(2, 10 );
	printf(BOLDMAGENTA "SET CTP SPECTROMETER MODE TO  2    Window Width  =  %d \n" RESET, 
	       ctpGetSpectrometerWidth() );
	
	ctpSetSpectrometerFPOutput(1);
	
	printf(BOLDMAGENTA "ENABLE FRONT PANNEL OUTPUT FOR TPOL  %d \n" RESET,ctpGetSpectrometerFPOutput());
      }
  
      
      printf("----------------------------- \n");
      ctpStatus();
      printf("----------------------------- \n");
      
      int iwait = 0;
      int allchanup = 0;
      while(allchanup  != (0x7) ) {
	iwait++;
	allchanup = ctpGetAllChanUp();
	if(iwait>1000)
	  {
	    printf("iwait timeout   allchup - 0x%x\n",allchanup);
	    break;
	  }
      }      
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
  
  unsigned short iflag;
  int stat;
  int islot;

  int rc = -1;
  
  /* Set number of events per block */
  tiSetBlockLevel(blockLevel);
  printf(BOLDBLUE "rocPrestart: Block Level set to %d\n" RESET,blockLevel);

  if(board_type == fadc125)
    rc = fa125_prestart();


  if(board_type == fadc250){
    rc = fa250_prestart();

    if(ctp.used == 1){
      ctpAlignAtSyncReset(1);
      
      ctpStatus(0xFFFF);
      ctpResetScalers();
      printf("CTP sync counter = %d\n",ctpGetSyncScaler());
    }

  }

#ifdef SHM_BUF
  monitor_prestart();  
#endif


  tiStatus(1);

   printf("rocPrestart: User Prestart Executed\n");
  /* SYNC is issued after this routine is executed */

   
}
 

/****************************************
 *  GO
 ****************************************/
void
rocGo()
{
   int iwait = 0;
   int allchanup = 0;

   int rc = -1;

   tiStatus(1);

   
   if(board_type == fadc125)
     rc = fa125_go();


   if(board_type == fadc250){

     fa250_go();

     if(ctp.used == 1){
       ctpAlignAtSyncReset(0);
       ctpGetAlignmentStatus(1,10);
     }
   }     
   

   //#define RANDOMPULSER 1

   printf(" NUMBER OF SOFT TRIGEGRS = 40 \n");

   /* Enable Soft Trigger */
  if(ti_bd.ti_master_trig == 1 || ti_bd.ti_master_trig == 3 ){

#ifdef RANDOMPULSER
    /* Enable Random at rate 500kHz/(2^7) = ~3.9kHz */
    //tiSetRandomTrigger(1,0x7);
    tiSetRandomTrigger(1,0x4);  //  32 kHz
    //tiSetRandomTrigger(1,0x3);  //  64 kHz
    //tiSetRandomTrigger(1,0x2);  //  128 kHz
#else   
    tiSoftTrig(ti_bd.ti_soft_trig[0], ti_bd.ti_soft_trig[1], ti_bd.ti_soft_trig[2],	      
    	       ti_bd.ti_soft_trig[3]);

    //    tiSoftTrig(2, 10000, 0x7FFF, 1);

    printf(BOLDBLUE " SOFT TRIGGER ENABLED  %d  %d  %d  %d  \n" RESET, 
	   ti_bd.ti_soft_trig[0], ti_bd.ti_soft_trig[1], ti_bd.ti_soft_trig[2],	      
	   ti_bd.ti_soft_trig[3]);
#endif 
  }



}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{

  int rc = -1;

  /* Disable Fixed Rate trigger */
  if(ti_bd.ti_master_trig == 1 ){
    tiSoftTrig(1,0,700,0);
    tiDisableRandomTrigger();
  }    
  

  if(board_type == fadc250)
    rc = fa250_end();

  tiStatus(0);

  printf("rocEnd: Ended after %d blocks\n",tiGetIntCount());
  

}

/****************************************
 *  TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{

  int ii, islot;
  int stat = 0, dCnt, len = 0, idata;
  
  int  itime,  gbready;

  int roflag = 1;

  unsigned int val;
  unsigned int *start;
  int timeout = 100000;

  int iread;

#ifdef SHM_BUF
  gettimeofday(&tvA[2], NULL);
  monitor_trigger();
  gettimeofday(&tvB[4], NULL);
  int idiffm=tvB[4].tv_usec - tvA[2].tv_usec +  1000000 * (tvB[4].tv_sec - tvA[2].tv_sec);
  //---- Fill Hist monitor  ------------
  hf1(hrol1,6,idiffm); 
  unsigned int *dma1 =  dma_dabufp;
#endif

  //  tiStatus(1);

  //  printf(" Inside rocTrigger \n");

  /* Set TI output 1 high for diagnostics */
  tiSetOutputPort(1,0,0,0);

  //  vmeDmaConfig(2,3,0); 
  /* Readout the trigger block from the TI 
     Trigger Block MUST be reaodut first */
  dCnt = tiReadTriggerBlock(dma_dabufp);

  if(dCnt<=0)
    {
      printf("No TI Trigger data or error.  dCnt = %d\n",dCnt);
    }
  else
    { /* TI Data is already in a bank structure.  Bump the pointer */
      dma_dabufp += dCnt;
      printf("Read TI block %d\n",dCnt);
    }


  //  printf(" Before reading out FADC \n");
  //  printf("Sascha  %d \n",blockLevel);


  BANKOPEN(6,BT_UI4,blockLevel);

    /* Readout FADC */
  /* Configure Block Type... temp fix for 2eSST trouble with token passing */
  if(NFADC_250 != 0)
    {
      FA_SLOT = fadcID[0];
      for(itime=0;itime<100;itime++) 
	{
	  gbready = faGBready();
	  stat = (gbready == fadcSlotMask);
	  if (stat>0) 
	    {
	      break;
	    }
	}
      if(stat>0) 
	{
	  if(NFADC_250 > 1) roflag=2; /* Use token passing scheme to readout all modules */

	  //          printf("Read out FADC250 \n");

	  dCnt = faReadBlock(FA_SLOT,dma_dabufp,MAXFADCWORDS,roflag);

	  printf(" FADC words %d\n",dCnt);

	  if(dCnt<=0)
	    {
	      printf("FADC%d: No data or error.  dCnt = %d\n",FA_SLOT,dCnt);
	    }
	  else
	    {
	      dma_dabufp += dCnt;
	    }
	} 
      else 
	{
	  printf ("FADC%d: no events   stat=%d  intcount = %d   gbready = 0x%08x  fadcSlotMask = 0x%08x\n",
		  FA_SLOT,stat,tiGetIntCount(),gbready,fadcSlotMask);
	}

      /* Reset the Token */
      if(roflag==2)
	{
	  for(islot=0; islot < NFADC_250; islot++)
	    {
	      FA_SLOT = fadcID[islot];
	      faResetToken(FA_SLOT);
	    }
	}
    }

  BANKCLOSE;


  if(board_type == fadc125){
    BANKOPEN(16,BT_UI4,blockLevel);

    printf("NFADC_125 = %d \n",NFADC_125); 

    int rflag = 1;
    if(NFADC_125 > 1) rflag=2;
    
    for(iread = 0; iread < timeout; iread++) 
      {
	gbready = fa125GBready();
	if(gbready == fa125ScanMask())
	  break;
      }
    if(iread==timeout) 
      {
	printf("fa125  timeout  (fa125GBready: 0x%08x != 0x%08x)\n", gbready,fa125ScanMask());
	fa125ResetToken(fa125Slot(0));
        *dma_dabufp++  = gbready;
        *dma_dabufp++  = fa125ScanMask();
        dCnt=0;
      }
    else 
      {
	dCnt = fa125ReadBlock(0,(volatile UINT32 *)dma_dabufp, 99000, 0x2); //--  399620 Byte max ?
	if(fa125ReadBlockStatus(1) != 0) 
	  printf(" .. token mask = 0x%08x\n",fa125GetTokenMask());		
      }

    if(fa125ReadBlockStatus(0) || (iread==timeout) )
      {
        fa125GStatus(0);
        //printf("vmeBerrStatus = %d\n", vmeBerrStatus=0);
        printf("token mask = 0x%08x\n",fa125GetTokenMask());
        daLogMsg("ERROR"," fa125 Reset %s (0x%x 0x%x)\n", roc_name,fa125GBready(),fa125ScanMask());
        sdPrintBusyoutCounters();
        sdPrintTrigoutCounters();
        int iadc;
	for(iadc=0; iadc<nfadc125; iadc++)
          {
            fa125Reset(fa125Slot(iadc), 0);
          }        
        //printf("**** AFTER RESET(0) ****\n");
        //fa125GStatus(0);
        //printf("token mask = 0x%08x\n",fa125GetTokenMask());
      }

    if(dCnt<=0) 
      {
	fa125ResetToken(fa125Slot(0));
	printf("FADC%d: No data or error \n",FA_SLOT);
      } else {
      dma_dabufp += dCnt;
      printf(" READ FADC  %d \n",dCnt);
    }
    
    fa125ResetToken(fa125Slot(0));
        
    BANKCLOSE;
  }    

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

  /* Set TI output 0 low */
  tiSetOutputPort(0,0,0,0);

}


void rocCleanup()
{

  int rc = -1;
  int islot = 0;
  /*
   * Perform a RESET on all FADC250s.
   *   - Turn off all A32 (block transfer) addresses
   */
  /*   printf("%s: Reset all FADCs\n",__FUNCTION__); */
  
  
  if(board_type == fadc250)
    rc = fa250_cleanup();
  
  if(board_type == fadc125){
    for(islot = 0; islot < NFADC_125; islot++){
      FA_SLOT = fadcID[islot];
      faReset(FA_SLOT,1); /* Reset, and DO NOT restore A32 settings (1) */
    }   
  }
  

}
