

#include "f1tdcLib.h"        /* F1 tdc libs */

#define F1TDC_ADDR (3<<19)

/* F1 TDC Library Variables */

extern int f1tdcA32Base;
extern int f1tdcA32Offset;
extern int nf1tdc;

static int   f1tdc_clock    = 1;
static int   f1tdc_version  = 2;

static float f1tdc_bin_size = 0.056;
static float f1tdc_latency  = 3000.0;
static float f1tdc_window   = 1000.0;

static int f1tdc_calc_hsdiv;      // calculated in f1ConfigWrite
static int f1tdc_calc_refclkdiv;  // calculated in f1ConfigWrite
static int f1tdc_calc_refcnt;     // calculated in f1ConfigWrite

static int use_bin_size_value = -1;

static unsigned int  cfg_data[128];
static unsigned int  t_offset;


int f1_download(){

  int f1_slot;


  f1tdcA32Base   = 0x09000000;

  int iflag  = 0;
  iflag  |=  F1_SRSRC_VXS;   // SyncReset from VXS
  iflag  |=  F1_TRIGSRC_VXS; // Trigger from VXS
  // 8 Oct 2017. Initialize using internal clock
  //  iflag  |=  F1_CLKSRC_VXS;  // Clock from VXS
  iflag  |=  F1_CLKSRC_INT;  //   Internal clock

  f1Init(F1TDC_ADDR,(1<<19),18,iflag);

  f1GSetBlockLevel(blockLevel);
  f1GEnableBusError();


  
  printf(BOLDBLUE " Number of F1TDCs initialized %d \n" RESET, nf1tdc);
  
  if(nf1tdc > 1)
    f1EnableMultiBlock(1);


  /***************************************
   *   SD SETUP
   ***************************************/
  
  sdInit();   /* Initialize the SD library */

  // ALEX
  //  if(strcmp(roc_name, "rocbcal12") == 0 ){
  //    unsigned int mask = 0x27C8;
  //    sdSetActiveVmeSlots(mask); 
  //  }


  sdSetActiveVmeSlots(f1ScanMask()); 
  sdStatus();
  
  int ii;
  for(ii = 0; ii < nf1tdc; ii++)
    f1ResetToken(f1Slot(ii));
  
  
    for(ii = 0; ii < nf1tdc; ii++){
    
    f1_slot = f1Slot(ii);

    printf(BOLDMAGENTA " Global Trigger Delay = %f (ns).  Latency = %f  \n" RESET,
	   (float) (trig_delay * 4.), f1[f1_slot].latency + trig_delay*4);

    f1tdc_bin_size      =   f1[f1_slot].bin_size;
    f1tdc_latency       =   f1[f1_slot].latency + trig_delay*4;
    f1tdc_window        =   f1[f1_slot].window;
    f1tdc_version       =   f1[f1_slot].version;
    use_bin_size_value  =   f1[f1_slot].global;

    printf(BOLDMAGENTA "F1 Version =  %d\n" RESET, f1[f1_slot].version);

    printf(BOLDMAGENTA "Initialize F1 TDC with the resolution of  %f (ns).  Global =  %d\n" RESET,f1tdc_bin_size, use_bin_size_value );

    if( (use_bin_size_value != 0) && (use_bin_size_value != 1) ){
      printf(BOLDRED " FATAL: Select Resolution for F1 TDC %d \n" RESET, f1_slot);
      daLogMsg("ERROR"," Select Resolution for F1 TDC \n");
      ROL_SET_ERROR;      
    }

	// Initialize these so we don't use stale values if they are not set
	// in f1ConfigWrite().  (They should ALWAYS be set!)
	f1tdc_calc_hsdiv     = 0xFFFF;
	f1tdc_calc_refclkdiv = 0xFFFF;
	f1tdc_calc_refcnt    = 0xFFFF;


    f1tdcModifyConfigData();

    f1ConfigWrite(f1_slot,(int *)&cfg_data,0xff);
    //    f1SetWindow(f1Slot(ii),f1tdc_window,f1tdc_latency,0xff);
	
	// Copy calculated config values back into f1 global data structure
	f1[f1_slot].hsdiv     = f1tdc_calc_hsdiv;
	f1[f1_slot].refclkdiv = f1tdc_calc_refclkdiv;
	f1[f1_slot].refcnt    = f1tdc_calc_refcnt;
	f1[f1_slot].group     = 1;
	
  }

    //  f1GSetWindow( 500,900,0xff );

  f1GStatus(0);

  //  f1ConfigShow(f1Slot(0),0);

  //  

  return(0);
}

int f1_prestart(){

  int ii, f1_slot;
  
  for(ii = 0; ii < nf1tdc; ii++){
    f1SetClkSource(f1Slot(ii), 2); /* 0: Internal, 1: FP, 2: VXS */    
    printf( BOLDBLUE " Switch to the external clock board in Slot = %d  \n" RESET, f1Slot(ii) );
  }


  f1GClear();

  for(ii = 0; ii < nf1tdc; ii++){
    f1ResetToken(f1Slot(ii));
  }

  f1GStatus(0);
  tiStatus(1);

  return(0);
}

int f1_go(){
  
  f1GEnableData(0xff);
  f1GEnable();

  f1GClearStatus(0xff);


  return(0);
}

int f1_end(){

  f1GDisable();
  f1GStatus(0);
  
  return(0);
}


int f1_cleanup(){
  int islot = 0;

  printf("%s: Reset all F1TDCs\n",__FUNCTION__);  
  return(0);
}



void f1tdcModifyConfigData()
{
  int  ii, jj;
  unsigned int exponent;
  unsigned int refclkdiv;
  unsigned int hsdiv;
  unsigned int refcnt;
  unsigned int triglat;
  unsigned int trigwin;
  unsigned int cfg_offset;
  float clk_period;
  float factor;
  float bin_size_value;
  float bin_size;
  float full_range;
  float tframe;

  int f1ConfigData[4][16] = {         /* default chip settings */
    { 0x0180, 0x8000, 0x407F, 0x407F, 
      0x407F, 0x407F, 0x003F, 0x9CC0, 
      0x22E2, 0x68A6, 0x1FEB, 0x0000, 
      0x0000, 0x0000, 0x0000, 0x000C}, 
    { 0x0180, 0x8000, 0x407F, 0x407F, 
      0x407F, 0x407F, 0x003F, 0x9C00, 
      0x22EF, 0x68CE, 0x1FF1, 0x0000, 
      0x0000, 0x0000, 0x0000, 0x000C}, 
    { 0x0180, 0x0000, 0x4040, 0x4040, 
      0x4040, 0x4040, 0x003F, 0xBA00, 
      0x63A4, 0xCDEC, 0x1FEB, 0x0000, 
      0x0000, 0x0000, 0x0000, 0x000C}, 
    { 0x0180, 0x0000, 0x4040, 0x4040, 
      0x4040, 0x4040, 0x003F, 0xB880,
      0x61D1, 0xCE1E, 0x1FF1, 0x0000,
      0x0000, 0x0000, 0x0000, 0x000C}
  };


  int choice=0;
  switch( f1tdc_version )	/* create default cfg_data for 8 chips */
  {
  case 2:
    choice = 0 + f1tdc_clock;
    for(ii = 0; ii < 8; ii++)
    {
      for(jj = 0; jj < 16; jj++)
	cfg_data[16*ii + jj] = f1ConfigData[choice][jj];
    }
    break;
  case 3:
    choice = 2 + f1tdc_clock;
    for(ii = 0; ii < 8; ii++)
    {
      for(jj = 0; jj < 16; jj++)
	cfg_data[16*ii + jj] = f1ConfigData[choice][jj];
    }
    break;
  }


  /* modify cfg_data for registers 7,8,9,10 */

  if(f1tdc_clock==0) clk_period = 31.25; /* Internal Clock Source */
  else               clk_period = 32.;   /* External Clock Source */

  if(f1tdc_version==2) factor = 0.5; /*   High Resolution mode */
  else                 factor = 1.;  /* Normal Resolution mode */

  bin_size_value = f1tdc_bin_size;

  exponent =  ( (*(cfg_data + 10)) >> 8 ) & 0x7;
  refclkdiv = 1;
  for(ii = 0; ii < exponent; ii++)
    refclkdiv = 2 * refclkdiv;


  if( use_bin_size_value == 0 )                                                 /* use provided 'binsize_value' to drive config values */
  {
        hsdiv = (clk_period/152.) * factor * ((float)refclkdiv)/bin_size_value  +  0.5;   /* force round up */
        bin_size = factor * (clk_period/152.) * ( (float)refclkdiv )/( (float)hsdiv );    /* actual bin size */
        full_range = 65536. * bin_size;
        refcnt = full_range/clk_period - 3.0;
        tframe = (float)(clk_period * (refcnt + 2 ));
        triglat = f1tdc_latency * factor/bin_size + 0.5;        /* force round up */
        trigwin = f1tdc_window * factor/bin_size + 0.5;         /* force round up */

        *(cfg_data + 7)  = (0x8000) | ( (refcnt & 0x1FF) << 6 );
        *(cfg_data + 8)  = trigwin & 0xFFFF;
        *(cfg_data + 9)  = triglat & 0xFFFF;
        *(cfg_data + 10) = (0x1F00) | (hsdiv & 0xFF);

        printf("\nf1tdc:  clk=%d   V=%d   b_size=%.4f   L=%.1f   W=%.1f\n",
                f1tdc_clock,f1tdc_version,f1tdc_bin_size,f1tdc_latency,f1tdc_window);
        printf("full_range/clk_period - 3.0 = %.4f\n", (full_range/clk_period - 3.0));
        printf("clk_period = %.2f   refcnt = %d\n", clk_period, refcnt);
        printf("refclkdiv = %d   hsdiv = %d   bin_size (ns) = %.4f   full_range (ns) = %.1f   tframe (ns) = %.1f\n",
                refclkdiv,hsdiv,bin_size,full_range,tframe);
  }
  else                            /* constrain to values that match global HIRES bin size */
  {                               /* first compute values for HIRES */
    printf("Use bin size = 1\n");
        hsdiv = (clk_period/152.) * (0.5) * ((float)refclkdiv)/bin_size_value  +  0.5;   /* for HIRES - force factor = 0.5 */
        bin_size = (0.5) * (clk_period/152.) * ( (float)refclkdiv )/( (float)hsdiv );                   /* actual HIRES bin size */
        full_range = 65536. * bin_size;                                                         /* actual HIRES range */
        refcnt = full_range/clk_period - 3.0;                                                   /* actual HIRES refcnt */

	 if(f1tdc_version == 3)
	   refcnt = (2 * refcnt) + 2;              /* compute equivalent refcnt for NORMAL resolution */ 
	                                           /* use HIRES values of hsdiv, refclkdiv */
        bin_size = factor * (clk_period/152.) * ( (float)refclkdiv )/( (float)hsdiv );                  /* normal res bin size */
        full_range = 65536. * bin_size;                                                                                                 
                /* normal res full range */
        tframe = (float)(clk_period * (refcnt + 2 ));                                           /* normal res full range */

        triglat = f1tdc_latency * factor/bin_size + 0.5;        /* force round up */
        trigwin = f1tdc_window * factor/bin_size + 0.5;         /* force round up */

        *(cfg_data + 7)  = (0x8000) | ( (refcnt & 0x1FF) << 6 );
        *(cfg_data + 8)  = trigwin & 0xFFFF;
        *(cfg_data + 9)  = triglat & 0xFFFF;
        *(cfg_data + 10) = (0x1F00) | (hsdiv & 0xFF);

        printf("\nf1tdc:  clk=%d   V=%d   b_size=%.4f   L=%.1f   W=%.1f\n",
                f1tdc_clock,f1tdc_version,f1tdc_bin_size,f1tdc_latency,f1tdc_window);
        printf("full_range/clk_period - 3.0 = %.4f\n", (full_range/clk_period - 3.0));
        printf("clk_period = %.2f   refcnt = %d\n", clk_period, refcnt);
        printf("refclkdiv = %d   hsdiv = %d   bin_size (ns) = %.4f   full_range (ns) = %.1f   tframe (ns) = %.1f\n",
                refclkdiv,hsdiv,bin_size,full_range,tframe);
  }

  // Record values actually used
  f1tdc_calc_hsdiv     = hsdiv;
  f1tdc_calc_refclkdiv = refclkdiv;
  f1tdc_calc_refcnt    = refcnt;


#if 0
  hsdiv = (clk_period/152.) * factor * ((float)refclkdiv)/bin_size_value  +  0.5;   /* force round up */
  bin_size = factor * (clk_period/152.) * ( (float)refclkdiv )/( (float)hsdiv );    /* actual bin size */
  full_range = 65536. * bin_size;
  refcnt = full_range/clk_period - 3.0;
  tframe = (float)(clk_period * (refcnt + 2 ));
  triglat = f1tdc_latency * factor/bin_size + 0.5;	/* force round up */
  trigwin = f1tdc_window * factor/bin_size + 0.5;	/* force round up */

  *(cfg_data + 7)  = (0x8000) | ( (refcnt & 0x1FF) << 6 );
  *(cfg_data + 8)  = trigwin & 0xFFFF;
  *(cfg_data + 9)  = triglat & 0xFFFF;
  *(cfg_data + 10) = (0x1F00) | (hsdiv & 0xFF);

  printf("\nf1tdc:  clk=%d   V=%d   b_size=%.4f   L=%.1f   W=%.1f\n",
	 f1tdc_clock,f1tdc_version,f1tdc_bin_size,f1tdc_latency,f1tdc_window);
  printf("full_range/clk_period - 3.0 = %.4f\n", (full_range/clk_period - 3.0));
  printf("clk_period = %.2f   refcnt = %d\n", clk_period, refcnt);
  printf("refclkdiv = %d   hsdiv = %d   bin_size (ns) = %.4f   full_range (ns) = %.1f   tframe (ns) = %.1f\n",
	 refclkdiv,hsdiv,bin_size,full_range,tframe);
  /*
  unsigned long long int timestamp;
  unsigned long long int word=4;
  timestamp = (((unsigned long long)word&0xffffff)<<32);
  printf(" timestamp = %lld   timestamp = 0x%llx)\n",timestamp,timestamp);
  */

#endif

  for(ii = 1; ii < 8; ii++)   /* copy reg 7,8,9,10 of chip 0 to chip 1-7 */
  {
    cfg_offset = 16 * ii;	
    *(cfg_data + cfg_offset + 7)  = *(cfg_data + 7);
    *(cfg_data + cfg_offset + 8)  = *(cfg_data + 8);
    *(cfg_data + cfg_offset + 9)  = *(cfg_data + 9);
    *(cfg_data + cfg_offset + 10) = *(cfg_data + 10);
  }
  t_offset = tframe/bin_size;
  printf("t_offset = %d\n", t_offset);
}
