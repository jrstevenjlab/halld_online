/*************************************************************************
 *
 *  hd_list.c - Library of routines for readout and buffering of
 *              events using a JLAB Trigger Interface V3 (TI) with a
 *              Linux VME controller in CODA 3.0.
 *
 *     This is an example readout list for the JLAB helicity decoder
 *
 *
 */

/* Event Buffer definitions */
/*
#define MAX_EVENT_POOL     10
#define MAX_EVENT_LENGTH   1024*64      // Size in Bytes 

 // Define TI Type (TI_MASTER or TI_SLAVE) 
#define TI_MASTER
 // EXTernal trigger source (e.g. front panel ECL input), POLL for available data 
#define TI_READOUT TI_READOUT_EXT_POLL
 // TI VME address, or 0 for Auto Initialize (search for TI by slot) 
#define TI_ADDR  0

 // Measured longest fiber length in system 
#define FIBER_LATENCY_OFFSET 0x10

#include "dmaBankTools.h"
#include "tiprimary_list.c" // Source required for CODA readout lists using the TI 
*/

#include "hdLib.h"
static int hd_found = 0;
//#define HELICITY_DECODER_ADDR 0xed0000
#define HELICITY_DECODER_ADDR   0x00980000
#define HELICITY_DECODER_BANK   0xDEC
#define HDMAXWORDS 1024
static uint8_t fiber_input;
static uint8_t cu_input;
static uint8_t cu_output;   

/* Define initial blocklevel and buffering level */
//#define BLOCKLEVEL 1
//#define BUFFERLEVEL 1

//#define INTRANDOMPULSER

/****************************************
 *  DOWNLOAD
 ****************************************/
void
hd_download()
{
  int stat;

  if(strncmp(roc_name, "roctagm1", 8)) return ;
  /* Setup Address and data modes for DMA transfers
   *
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  //vmeDmaConfig(2,5,1);


  /* Initialize the library and module with its internal clock*/
  //int ret = hdInit(HELICITY_DECODER_ADDR, HD_INIT_INTERNAL, 0, 0);

  //int ret = hdInit(HELICITY_DECODER_ADDR, HD_INIT_VXS, 0, 0);

  //-- hallb ---
  int ret = hdInit(HELICITY_DECODER_ADDR, HD_INIT_VXS, HD_INIT_EXTERNAL_FIBER, 0);
  if(ret==0) hd_found = 1; 
  else       hd_found = 0;
  
  
  //if(hd_found)
  //{
    //hdSetA32(0x10000000);
     hdSetA32(0x08800000);
       // TI:   0x08000000 
    //fadcA32Base=0x09000000;
    hdStatus(1);
    //}
 
  printf("hd_download(): Executed on %s  ret=%d found=%d \n",roc_name,ret,hd_found);



  printf("rocDownload: User Download Executed \n");

}

/****************************************
 *  PRESTART
 ****************************************/
void
hd_prestart()
{
  unsigned short iflag;
  int stat;
  int islot;

  uint8_t hd_clock, hd_clock_ret;

  if(strncmp(roc_name, "roctagm1", 8)) return ;
  printf("hd_prestart(): Executed on %s\n",roc_name);

  //====================  TEST ==============================================
#if 0
  /* Switch helicity decoder to VXS master clock, trigger, syncreset */
  hdSetSignalSources(HD_INIT_VXS, HD_INIT_VXS, HD_INIT_VXS);

  /* Setting data input (0x100 = 2048 ns) and
     trigger latency (0x40 = 512 ns) processing delays */
  hdSetProcDelay(0x100, 0x40);

  /* Enable the module decoder, well before triggers are enabled */
  hdEnableDecoder();

  /* Using internal helicity generation for testing */
  hdSetHelicitySource(1, 0, 1);
  hdHelicityGeneratorConfig(2,     /* Pattern = 2 (OCTET) */
			    0,     /* Window Delay = 0 (0 windows) */
			    0x40,  /* SettleTime = 0x40 (512 ns) */
			    0x80,  /* StableTime = 0x80 (1024 ns) */
			    0xABCDEF01); /* Seed */
  hdEnableHelicityGenerator();
#else
  //======================================================================
 
  if(hd_found) {  // --- hallb --

    //hdSetSignalSources(HD_INIT_VXS, HD_INIT_VXS, HD_INIT_VXS);

  /* Setting data input (0x100 = 2048 ns) and
       trigger latency (1000*8ns = 8000 ns) processing delays */
    //hdSetProcDelay(0x100, 1000); // hallb trigger latency
    //hdSetProcDelay(0x100, 400); // trig latency 400=3.2us
    hdSetProcDelay(0x100, 0x40); // trig latency  512 ns


    hdSetBlocklevel(blockLevel);  // moved from GO


    
    /* Enable the module decoder, well before triggers are enabled */
    hdEnableDecoder();

#if 0
    /*set i/o signals inversion if needed; 3 parameters have following meaning: fiber_input, cu_input, cu_output
      (0 means no inversion, 1 means inversion)*/

    //ret = hdSetHelicityInversion(0, 0, 0); /*reproduces old (v7) firmware (before Jan 31, 2024)*/

    /* signals on fiber inputs are reversed because of polarity-flipping
       optical fanout in counting room, so we do (1,0,1); if optical fanout
       replaced with non-flipping one, will change it to (0,0,1)*/
    int ret = hdSetHelicityInversion(1, 0, 1);
    //int ret = hdSetHelicityInversion(0, 0, 1);
    //int ret = hdSetHelicityInversion(0, 0, 0);
    //int ret = hdSetHelicityInversion(1, 1, 1);

    ret = hdGetHelicityInversion(&fiber_input, &cu_input, &cu_output);
    printf("\nHelicity Decoder inversion settings: fiber_input=%d, cu_input=%d, cu_output=%d (0-no flip, 1-flip)\n\n",
           fiber_input,cu_input,cu_output);

    /* set tsettle filtering:
     *     0   Disabled
     *     1   4 clock cycles
     *     2   8 clock cycles
     *     3   16 clock cycles
     *     4   24 clock cycles
     *     5   32 clock cycles
     *     6   64 clock cycles
     *     7   128 clock cycles */
    hd_clock = 3;
    ret = hdSetTSettleFilter(hd_clock); 
    ret = hdGetTSettleFilter(&hd_clock_ret);
#endif
    
    printf("\nHD T-settle filtering: set %d, read back %d\n\n",hd_clock,hd_clock_ret);    
    hdStatus(0);
  }
#endif
    printf("rocPrestart: User Prestart Executed\n");
}

/****************************************
 *  GO
 ****************************************/
void
hd_go()
{
  int islot;

  if(strncmp(roc_name, "roctagm1", 8)) return ;
  printf("hd_go(): Executed on %s\n",roc_name);

  /* Get the current Block Level */
  //blockLevel = tiGetCurrentBlockLevel();
  //printf("rocGo: Block Level set to %d\n",blockLevel);

  /* Enable/Set Block Level on modules, if needed, here */
  //hdSetBlocklevel(blockLevel);  moved to PRESTART

  hdEnable();
  hdStatus(0);

//==============  TEST =========================================
#if 0
  /* Example: How to start internal pulser trigger */
  #ifdef INTRANDOMPULSER
    /* Enable Random at rate 500kHz/(2^7) = ~3.9kHz */
    tiSetRandomTrigger(1,0x7);
  #elif defined (INTFIXEDPULSER)
    /* Enable fixed rate with period (ns) 120 +30*700*(1024^0) = 21.1 us (~47.4 kHz) - Generated 1000 times */
    tiSoftTrig(1,1000,700,0);
  #endif
#endif
//==============================================================
}

/****************************************
 *  END
 ****************************************/
void
hd_end()
{
  int islot;

  if(strncmp(roc_name, "roctagm1", 8)) return ;
  printf("hd_end(): Executed on %s\n",roc_name);

  hdDisable();

  hdStatus(0);

  printf("hd_end: Ended after %d blocks\n",tiGetIntCount());

}

/****************************************
 *  TRIGGER
 ****************************************/
void
hd_trigger()
{
  int dCnt;
  int timeout=0;

  if(strncmp(roc_name, "roctagm1", 8)) return ;
  //printf("hd_trigger(): Executed on %s\n",roc_name);


  BANKOPEN(HELICITY_DECODER_BANK, BT_UI4, blockLevel);
  while((hdBReady(0)!=1) && (timeout<100))
    {
      timeout++;
    }

  if(timeout>=100)
    {
      printf("%s: ERROR: TIMEOUT waiting for Helicity Decoder Block Ready\n", __func__);
    }
  else
    {
      dCnt = hdReadBlock(dma_dabufp, 1024, 1);
      if(dCnt<=0)
	{
	  printf("%s: ERROR or NO data from hdReadBlock(...) = %d\n",
		 __func__, dCnt);
	}
      else
	{
	  dma_dabufp += dCnt;
          //printf("hd_trigger(): Executed on %s data=%d\n",roc_name,dCnt);
	}
    }

  BANKCLOSE;
  //printf("hd_trigger(): Executed on %s data=%d\n",roc_name,dCnt);

  /* Set TI output 0 low */
  //tiSetOutputPort(0,0,0,0);

}

void
hd_cleanup()
{
  int islot=0;

  printf("%s: Reset all Modules\n",__FUNCTION__);

}

/*
  Local Variables:
  compile-command: "make hd_list.so"
  End:
 */
