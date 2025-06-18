// #include "fadcLib.h"        /* library of FADC250 routines */


unsigned int  MAXFADCWORDS  =  0;

/* FADC Library Variables */


extern   int nfadc;                 /* Number of FADC250s verified with the library */
//extern   int fadcID[FA_MAX_BOARDS]; /* Array of slot numbers, discovered by the library */
extern   unsigned int fadcAddrList[FA_MAX_BOARDS];   /* Array of a24 addresses for FADCs */

unsigned int fadcSlotMask   = 0;    /* bit=slot (starting from 0) */

//extern int blockLevel;

int fa250_download(){

  int slot, ii;

  int Naddr = 0;

  unsigned int set_nsb = 0;


  memset(fadcAddrList, 0, sizeof(fadcAddrList));

  for(slot = 3; slot < 21; slot++){
    if(fa250[slot].group == 1){
      fadcAddrList[Naddr] = (slot<<19);
      Naddr++;
    }
  }

  printf("\n");
  printf(" NUMBER OF FADC250  boards found in config file =  %d \n",Naddr);
  printf("\n");


  NFADC_250 = 0;

  /* Initially set block level, will be reseted when blockLevel is updated */
  //  MAXFADCWORDS = 15 * blockLevel * (1 + 2 + 100*16 + 1) + 2*32;

  MAXFADCWORDS = 15 * blockLevel * (1 + 2 + 100*16 + 1) + 2*32;



  /* Setup the iFlag.. flags for FADC initialization */
  int iFlag;
  iFlag = 0;
  /* Sync Source */
  iFlag |= (1<<0);    /* VXS */
  /* Trigger Source */
  iFlag |= (1<<2);    /* VXS */
  /* Clock Source */
  iFlag |= (0<<5);    /* self*/

  iFlag |= (1<<17);     /* activate using fadcAddrList in faInit */


  vmeSetQuietFlag(1);   /* skip the errors associated with BUS Errors */
  
  faInit(fadcAddrList[0], 0, Naddr, iFlag);
  
  if(nfadc !=  Naddr){
    printf(BOLDRED "FATAL: faInit failed. Nfadc config =  %d, Nfadc found =  %d \n" RESET, Naddr, nfadc );
    daLogMsg("ERROR", "faInit: Fail to initialize fadc250.  Nfadc config =  %d, Nfadc found =  %d  \n",  Naddr, nfadc );
    ROL_SET_ERROR;
  }

  vmeSetQuietFlag(0);   /* Turn the error statements back on */

  NFADC_250 = nfadc;        /* Redefine our NFADC with what was found from the driver */


  printf("\n");
  printf(BOLDBLUE " Number of boards found by faInit =  %d \n" RESET, NFADC_250);
  printf("\n");


  if(NFADC_250 > 1) 
    faEnableMultiBlock(1);

  for(slot = 0; slot < NFADC_250; slot++){

    FA_SLOT = fadcID[slot];    
    fadcSlotMask |= (1<<FA_SLOT); /* Add it to the mask */
    
  }


  for(slot = 0; slot < NFADC_250; slot++){
    
    FA_SLOT = fadcID[slot];
    
    printf(" \nfadc250Download:  FA_SLOT=%d  group=%d  mode=%d  nsb=%d \n",FA_SLOT,
	   fa250[FA_SLOT].group, fa250[FA_SLOT].mode, fa250[FA_SLOT].nsb);
    
    printf(BOLDMAGENTA " TRIGGER GLOBAL DELAY = %d \n" RESET, trig_delay);


    if( (fa250[FA_SLOT].nsa < 2) ||  (fa250[FA_SLOT].nsa > 511 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for NSA  %d \n"  RESET, fa250[FA_SLOT].nsa );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for NSA  %d \n", fa250[FA_SLOT].nsa );
      ROL_SET_ERROR;
    }


    // Allow for negative NSB, bit 3 has to be set to 1

    if(fa250[FA_SLOT].nsb < 0) {
      if(abs(fa250[FA_SLOT].nsb <= 3)){
        set_nsb = (1 << 3) | (abs(fa250[FA_SLOT].nsb) & 0x3);
      } else {
        printf(BOLDRED    "FADC250 initialization:  Wrong value for NSB  %d \n"  RESET, fa250[FA_SLOT].nsb );
        daLogMsg("ERROR", "FADC250 initialization:  Wrong value for NSB  %d \n", fa250[FA_SLOT].nsb );
        ROL_SET_ERROR;
      }
    }

    if(fa250[FA_SLOT].nsb >= 0) {
      if( fa250[FA_SLOT].nsa < 512 )
        set_nsb = fa250[FA_SLOT].nsb;
      else {
        printf(BOLDRED    "FADC250 initialization:  Wrong value for NSB  %d \n"  RESET, fa250[FA_SLOT].nsb );
        daLogMsg("ERROR", "FADC250 initialization:  Wrong value for NSB  %d \n", fa250[FA_SLOT].nsb );
        ROL_SET_ERROR;
      }
    }


    if( (fa250[FA_SLOT].npeak < 0) ||  (fa250[FA_SLOT].npeak > 4 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for NPEAK  %d \n"  RESET, fa250[FA_SLOT].npeak );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for NPEAK  %d \n", fa250[FA_SLOT].npeak );
      ROL_SET_ERROR;
    }

    if( (fa250[FA_SLOT].nped < 4) ||  (fa250[FA_SLOT].nped > 15 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for NPED  %d \n"  RESET, fa250[FA_SLOT].nped );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for NPED  %d \n", fa250[FA_SLOT].nped );
      ROL_SET_ERROR;
    }

    if( (fa250[FA_SLOT].max_ped < 0) ||  (fa250[FA_SLOT].max_ped > 1023 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for MAX_PED  %d \n"  RESET, fa250[FA_SLOT].max_ped );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for MAX_PED  %d \n", fa250[FA_SLOT].max_ped );
      ROL_SET_ERROR;
    }

    if( (fa250[FA_SLOT].nsat < 1) ||  (fa250[FA_SLOT].nsat > 4 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for NSAT  %d \n"  RESET, fa250[FA_SLOT].nsat );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for NSAT  %d \n", fa250[FA_SLOT].nsat );
      ROL_SET_ERROR;
    }

    if( (fa250[FA_SLOT].trig_nsat < 1) ||  (fa250[FA_SLOT].trig_nsat > 4 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for TRIG_NSAT  %d \n"  RESET, fa250[FA_SLOT].trig_nsat );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for TRIG_NSAT  %d \n", fa250[FA_SLOT].trig_nsat );
      ROL_SET_ERROR;
    }

    if( (fa250[FA_SLOT].mon_nped < 4) ||  (fa250[FA_SLOT].mon_nped > 15 )){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for MON_NPED  %d \n"  RESET, fa250[FA_SLOT].mon_nped );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for MON_NPED  %d \n", fa250[FA_SLOT].mon_nped );
      ROL_SET_ERROR;
    }

    if( fa250[FA_SLOT].mon_max_ped > 1023 ){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for MON_MAX_PED  %d \n"  RESET, fa250[FA_SLOT].mon_max_ped );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for MON_MAX_PED  %d \n", fa250[FA_SLOT].mon_max_ped );
      ROL_SET_ERROR;
    } 

    if( (fa250[FA_SLOT].data_format > 2) || (fa250[FA_SLOT].data_format < 0)  ){
      printf(BOLDRED    "FADC250 initialization:  Wrong data format FORMAT  %d \n"  RESET, fa250[FA_SLOT].data_format );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong data format FORMAT  %d \n", fa250[FA_SLOT].data_format );
      ROL_SET_ERROR;
    } 


    if( fa250[FA_SLOT].stop > 0xFF ){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for STOP  %d \n"  RESET, fa250[FA_SLOT].stop );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for STOP  %d \n", fa250[FA_SLOT].stop );
      ROL_SET_ERROR;
    } 

    if( fa250[FA_SLOT].busy > 0xFF ){
      printf(BOLDRED    "FADC250 initialization:  Wrong value for BUSY  %d \n"  RESET, fa250[FA_SLOT].busy );
      daLogMsg("ERROR", "FADC250 initialization:  Wrong value for BUSY  %d \n", fa250[FA_SLOT].busy );
      ROL_SET_ERROR;
    } 


    faSetProcMode(FA_SLOT,
		  fa250[FA_SLOT].mode,
		  fa250[FA_SLOT].winOffset + trig_delay,
		  fa250[FA_SLOT].winWidth,
		  set_nsb,
		  fa250[FA_SLOT].nsa,
		  fa250[FA_SLOT].npeak,
		  fa250[FA_SLOT].nped,
		  fa250[FA_SLOT].max_ped,
		  fa250[FA_SLOT].nsat);

    
    for(ii = 0; ii < MAX_FADC250_CH; ii++) {
      faSetDAC(             FA_SLOT, fa250[FA_SLOT].dac[ii], (1<<ii));
      faSetThreshold(       FA_SLOT, fa250[FA_SLOT].read_thr[ii],(1<<ii));
      /*  Disable channels for the trigger  */
      //      if( ((1 << ii ) &  fa250[FA_SLOT].trigMask )  == 0)
      //      	faSetChannelPedestal( FA_SLOT, ii, fa250[FA_SLOT].trig_bl[ii]);
      //      else {
      //        faSetChannelPedestal( FA_SLOT, ii, 0xFFFF);
      //        printf(BOLDMAGENTA "FADC250 in slot %d:  Disable channel %d  for the trigger  \n"  RESET, FA_SLOT, ii);   
      //      }      
      // TEST
      /*  Disable channels for the trigger  */
      if( ((1 << ii ) &  fa250[FA_SLOT].trigMask )  == 0){

        //        if(strncmp(roc_name, "rocfcal", 7) == 0)
        //          faSetChannelPedestal( FA_SLOT, ii, 125);
        //        else if(strncmp(roc_name, "rocecal", 7) == 0)
        //          faSetChannelPedestal( FA_SLOT, ii, 130);
        //        else

          faSetChannelPedestal( FA_SLOT, ii, fa250[FA_SLOT].trig_bl[ii]);
      }
      else {
        faSetChannelPedestal( FA_SLOT, ii, 0xFFFF);
        printf(BOLDMAGENTA "FADC250 in slot %d:  Disable channel %d  for the trigger  \n"  RESET, FA_SLOT, ii);   
      }      

    }


#if 0        
      // Alex    debug energy flow
    for(ii = 0; ii < MAX_FADC250_CH; ii++) {
      //      if(FA_SLOT == 4 )
      //      if(FA_SLOT == 16 && ii == 0)
        if(FA_SLOT == 16)
        faSetChannelPedestal( FA_SLOT, ii, fa250[FA_SLOT].trig_bl[ii]);
      else	faSetChannelPedestal( FA_SLOT, ii, 0xFFFF);
    }      
#endif

    // CHECK
    //    if( strcmp(roc_name, "rocps2")  == 0){
    //      if(FA_SLOT == 13) faSetChannelPedestal( FA_SLOT, 14, 0xFFFF);
    //    }
    
    

    if( strcmp(roc_name, "rocps2") == 0){
      printf(BOLDBLUE " Set Inverted Mode for TPOL fadcs \n" RESET);
      if( (FA_SLOT >= 13) && (FA_SLOT <= 16))
        faSetInvertedMode(FA_SLOT);      
    }


    /* FADC HitBit modes for PS, PSC, ST, and TOF  */
    if( (strcmp(roc_name, "rocps1") == 0)    ||  (strcmp(roc_name, "rocps2")  == 0)  || 
	(strcmp(roc_name, "rocst")  == 0)    ||  (strcmp(roc_name, "roctof1") == 0)  ||
	(strcmp(roc_name, "roctagm1") == 0)  ||  (strcmp(roc_name, "roctagh1") == 0) ){
      
      printf(BOLDMAGENTA " HitBitMode Enabled \n" RESET);
      faGSetHitbitsMode(1);      
    }
    
    
    faSetTriggerPathThreshold(FA_SLOT,  fa250[FA_SLOT].trig_thr);
    
    
    //   Not supported by the new firmware anymore
    //   faSetTriggerPathSamples(FA_SLOT, fa250[FA_SLOT].trig_nsb, fa250[FA_SLOT].trig_nsa);
    

    faSetTriggerPathSamples(FA_SLOT, fa250[FA_SLOT].trig_nsa, fa250[FA_SLOT].trig_nsat);
    
    faSampleConfig(FA_SLOT, fa250[FA_SLOT].mon_nped, fa250[FA_SLOT].mon_max_ped);

    
    //    faSetTriggerBusyCondition(FA_SLOT, 2);
    //    faSetTriggerStopCondition(FA_SLOT, 9);

    faSetTriggerBusyCondition(FA_SLOT, fa250[FA_SLOT].busy);
    faSetTriggerStopCondition(FA_SLOT, fa250[FA_SLOT].stop);


    /* Alex */ 
    //    printf(BOLDMAGENTA "SASCHA MASK = 0x%x \n" RESET,fa250[FA_SLOT].trigMask);
    //    faSetChannelDisableMask(FA_SLOT, fa250[FA_SLOT].trigMask );
    
    faSetChannelDisableMask(FA_SLOT, fa250[FA_SLOT].chDisMask );
    
    
    /* Bus errors to terminate block transfers (preferred) */
    faEnableBusError(FA_SLOT);
    
    /* Set the Block level to the initial value*/
    faSetBlockLevel(FA_SLOT,blockLevel);
      

    /*

    Set data format

      Data format   0   -  Standard
                    1   -  Intermediate compression
                    2   -  Full compression
    */

    faSetDataFormat(FA_SLOT, fa250[FA_SLOT].data_format);
    printf(BOLDMAGENTA " Set DATA FORMAT for FADC in slot  %d  to  %d \n" RESET, FA_SLOT, fa250[FA_SLOT].data_format);



    faPrintThreshold(FA_SLOT);
    faPrintDAC(FA_SLOT);
      
    faStatus(FA_SLOT, 0);

  }
  

  for(slot = 0; slot < NFADC_250; slot++){

    FA_SLOT = fadcID[slot];

    if(fa250[FA_SLOT].trigMask)
      printf(BOLDMAGENTA "FADC250:  Disable trigger mask for FADC in SLOT  %d   =  0x%x   \n"  RESET, FA_SLOT, fa250[FA_SLOT].trigMask);
    
    if(fa250[FA_SLOT].chDisMask)
      printf(BOLDMAGENTA "FADC250:  Disable channel mask for FADC in SLOT  %d   =  0x%x   \n"   RESET,
	     FA_SLOT,fa250[FA_SLOT].chDisMask );   

    printf(" Trigger Baseline Settings for FADC in slot %d: ", FA_SLOT );

    for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++) {
      if((ii%4)==0) printf("\n");
      printf("Chan %2d: %5d   ",(ii+1),faGetChannelPedestal(FA_SLOT, ii) );
    }
    printf("\n");
    
  }
  
  if(use_playback){
    fa250_init_playback();
  }
  
  faGStatus(0);


  /***************************************
   *   SD SETUP
   ***************************************/
  sdInit();   /* Initialize the SD library */
  
  printf(BOLDBLUE "fadcSlotMask =  0x%x  \n" RESET ,fadcSlotMask); 
  
  sdSetActiveVmeSlots(fadcSlotMask);    /* Use the fadcSlotMask to configure the SD */
  sdStatus();

  return(0);

}


int fa250_prestart(){

  int islot;
 /* Unlock the VME Mutex */
  vmeBusUnlock();
  

  /* FADC Perform some resets, status */
  for(islot = 0; islot < NFADC_250; islot++){
 
    FA_SLOT = fadcID[islot];
     
    //    faMGTReset(FA_SLOT,1);
 
    faSetClockSource(FA_SLOT,2);

    // Alex Alex
    //    taskDelay(1);
    //    faMGTReset(FA_SLOT,1);
    //    taskDelay(1);
    //    faMGTReset(FA_SLOT,0);
    //    taskDelay(1);









    faSoftReset(FA_SLOT,0);
    
    faResetToken(FA_SLOT);
    faResetTriggerCount(FA_SLOT);
    
    faEnableSyncSrc(FA_SLOT);

    //    faStatus(FA_SLOT,0);
    
    //    faMGTReset(FA_SLOT,1);
    //    taskDelay(1);
    //    faMGTReset(FA_SLOT,0);
    //    taskDelay(1);

    //      faSetMGTTestMode(FA_SLOT,0);
    //      faDisable(FA_SLOT,0);      
    //      faChanDisable(FA_SLOT,0xffff);
    //      faSetMGTTestMode(FA_SLOT,0);
    //      faEnable(FA_SLOT,0,0);

    //    faStatus(FA_SLOT,1);   
    
    //    printf(" AFTER PRESTART \n");  

  }
  
  printf(" AFTER PRESTART \n");  


  faGStatus(1);

  sdStatus();

  return(0);
  
}

int fa250_go(){
  int islot;

  int ii;

  for(islot = 0; islot < NFADC_250; islot++) {
    FA_SLOT = fadcID[islot];
    //       faChanDisable(FA_SLOT,0x0);
    //       faSetMGTTestMode(FA_SLOT,1);

    faSetBlockLevel(FA_SLOT,blockLevel);
    printf(BOLDBLUE  "rocGo: Set block level to %d\n" RESET, blockLevel);

    
    faEnable(FA_SLOT,0,0);

    if(use_playback){
      faPPGEnable(FA_SLOT);   
    }

    printf(" AFTER GO \n");


    // ALEX CHECK DACReset



    faPrintDAC(FA_SLOT);



  }
  
  

  faGStatus(1);
 

  return(0);

}

int fa250_end(){

  int islot;
  
  /* FADC Disable */
  for(islot = 0; islot < NFADC_250; islot++) {
    FA_SLOT = fadcID[islot];

    faDisable(FA_SLOT,0);
    faStatus(FA_SLOT,0);      

    if(use_playback)
      faPPGDisable(FA_SLOT);
        
  }

  sdStatus(1);

  return(0);
}




int fa250_cleanup(){
  int islot;

  for(islot = 0; islot < NFADC_250; islot++){
      FA_SLOT = fadcID[islot];
      faReset(FA_SLOT,1); /* Reset, and DO NOT restore A32 settings (1) */
    } 

  return(0);
}


int fa250_init_playback(){

  unsigned short adc_playback[32];
  unsigned short adc_playback_allchannels[512];
  int sample, ichan;

  int slot;

  for(slot = 0;slot < NFADC_250; slot++){

    FA_SLOT = fadcID[slot];
    
    for(ichan = 0; ichan < 16; ichan++) {
      for(sample = 0; sample < 32; sample++) {
        adc_playback_allchannels[ichan*32 + sample] = fa250[FA_SLOT].amp_pb[ichan][sample];
      }
    }
    
    faSetPPG(FA_SLOT,0,adc_playback_allchannels,512);
    taskDelay(5);
    
    printf("WWWWWWWWW SLOT = %d \n",FA_SLOT);
    for(ichan = 0; ichan < 16; ichan++) {
      printf(" CH = %d \n",ichan);
      for(sample = 0; sample < 32; sample++) {
        printf("  %5d  ",adc_playback_allchannels[ichan*32 + sample]);
      }
      printf("\n");
    }
    


  } 

  return(0);

}
