#include "ctpLib.h"

int ctp_download(){

  ctpInit(1);
  
  //      unsigned int ctp_vmemask = (1<<9) | (1<<10);
  
  unsigned int ctp_vmemask = fadcSlotMask;

  unsigned int bcal_mask   =  0;
  unsigned int fcal_mask   =  0;
  unsigned int tof_mask    =  0;
  unsigned int st_mask     =  0;
  unsigned int ccal_mask   =  0;

  unsigned int exclude_tabs = ~(1 << 20);

  // Exclude FADC in slot 19 in roctof1
  if(strcmp(roc_name, "roctof1") == 0){
    // Exclude fadc from slot 19
    tof_mask = ~(1 << 19);
    ctp_vmemask = (ctp_vmemask & tof_mask);
  } 


  // Exclude FADC in slot 19 in rocbcal7
  if(strcmp(roc_name, "rocbcal7") == 0){
    // Exclude fadc from slot 19
    bcal_mask = ~(1 << 19);
    ctp_vmemask = (ctp_vmemask & bcal_mask);
  } 

  // Exclude FADC in slot 20 in rocfcal4
  if(strcmp(roc_name, "rocfcal4") == 0){
    fcal_mask = ~(1 << 20);    
    if(use_tabs == 1){
      ctp_vmemask  = (ctp_vmemask & (1 << 20));
      printf(BOLDBLUE "\n CTP init:  Use TABS FADC in SLOT 20\n" RESET);
    }  else {
      ctp_vmemask  = (ctp_vmemask & fcal_mask);
      printf(BOLDBLUE "\n CTP init:  Exclude TABS FADC in SLOT 20\n" RESET);
    }  
  }

  // Exclude FADC in slot 15 in rocccal
  if(strcmp(roc_name, "rocccal") == 0){
    ccal_mask = ~(1 << 15);
    ctp_vmemask = (ctp_vmemask & ccal_mask);
    printf(BOLDBLUE "\n CTP init:  Exclude FADC in SLOT 15 \n" RESET);
  }  
  


  if(strcmp(roc_name, "rocps2") == 0){
    ctp_vmemask = (1<<6);
  }
  
  // Exclude FADC in slot 10 from the ST trigger, rocst
  if(strcmp(roc_name, "rocst") == 0){
    st_mask = ~(1 << 7);    
    ctp_vmemask  = (ctp_vmemask & st_mask);
    st_mask = ~(1 << 8);    
    ctp_vmemask  = (ctp_vmemask & st_mask);
    st_mask = ~(1 << 9);    
    ctp_vmemask  = (ctp_vmemask & st_mask);
    st_mask = ~(1 << 10);    
    ctp_vmemask  = (ctp_vmemask & st_mask);
    st_mask = ~(1 << 13);    
    ctp_vmemask  = (ctp_vmemask & st_mask);
    st_mask = ~(1 << 14);    
    ctp_vmemask  = (ctp_vmemask & st_mask);

    printf(BOLDBLUE "\nCTP init:  Exclude FADC in SLOT 10 (CCAL) \n" RESET);
  }  



  //  unsigned int test_mask = 0xFE7E8;
  // Slots 15 >
  //  unsigned int test_mask = 0x67F8;
  //  ctp_vmemask = 0;
  
  
  printf (BOLDBLUE "CTP_VMEMASK 0x%x \n" RESET, ctp_vmemask);

  ctpSetVmeSlotEnableMask(ctp_vmemask);
      
  printf(BOLDBLUE "ROCID = %d \n" RESET, ROCID);
  ctpSetCrateID(ROCID);
      
      
  /* PS, PSC, and ST trigger settings */
  if(strcmp(roc_name, "rocps1") == 0){
    if(ctp.HitWindowWidth > 0)
      ctpSetSpectrometerMode(1, ctp.HitWindowWidth );
    else ctpSetSpectrometerMode(1, 10 );
    printf(BOLDMAGENTA "SET CTP SPECTROMETER MODE TO  1    Window Width  =  %d \n" RESET, 
	   ctpGetSpectrometerWidth() );      
  }
  
  if(strcmp(roc_name, "rocps2") == 0 ){
    if(ctp.HitWindowWidth > 0)
      ctpSetSpectrometerMode(2, ctp.HitWindowWidth );
    else ctpSetSpectrometerMode(2, 10 );
    printf(BOLDMAGENTA "SET CTP SPECTROMETER MODE TO  2    Window Width  =  %d \n" RESET, 
	   ctpGetSpectrometerWidth() );
  }
  
  if(strcmp(roc_name, "rocst") == 0){
    if(ctp.HitWindowWidth > 0)
      ctpSetSpectrometerMode(0, ctp.HitWindowWidth );
    else ctpSetSpectrometerMode(0, 10 );
    printf(BOLDMAGENTA "SET CTP SPECTROMETER MODE TO  20   Window Width  =  %d \n" RESET, 
	   ctpGetSpectrometerWidth() );
  }

  /* TOF trigger settings */
  if(strcmp(roc_name, "roctof1") == 0){
    ctpSetTOFCoincWidth(6);
    printf(BOLDMAGENTA " TOF ccoincidence width is set to %d \n" RESET, ctpGetTOFCoincWidth() );
  }

  /* TAGH trigger settings */
  if(strcmp(roc_name, "roctagh1") == 0){
    // ALEX 
    //    ctpSetScopeTrigger(1, 12 );
    ctpSetScopeTrigger(1, 2 );
      printf(BOLDMAGENTA " Initialize TAGH trigger   %d \n" RESET, ctpGetScopeTrigger(1) );
  }

  /* TAGM trigger settings */
  if(strcmp(roc_name, "roctagm1") == 0){
    ctpSetScopeTrigger(0, 10 );
      printf(BOLDMAGENTA " Initialize TAGM trigger   %d \n" RESET, ctpGetScopeTrigger(1) );
  }


  /* BCAL cosmic settings */
  if( ctp.BCALThreshold > 0 ){
    ctpSetBCALThreshold(ctp.BCALThreshold);
    if( ctp.BCALWindowWidth > 5)
      ctpSetBCALWindowWidth(ctp.BCALWindowWidth);
    else {
      printf(BOLDRED " BCALWindowWidth too small %d, using the default value of 80 ns \n" RESET, 
	     ctp.BCALWindowWidth );
      ctpSetBCALWindowWidth(20);
    }
      
    printf(BOLDMAGENTA " CTP initialized for BCAL cosmic trigger. Window width = %d,  Energy threshold =  %d \n" RESET, 
	   ctpGetBCALWindowWidth(), ctpGetBCALThreshold() ) ;
    
  }

  /* Threshold on the BCAL total energy */
  if( ctp.FinalSumThreshold > 0){  
    ctpSetFinalSumThreshold( ctp.FinalSumThreshold ,0);
    printf(BOLDMAGENTA " CTP: Final Sum Threshold is set to  = %d \n" RESET, ctpGetFinalSumThreshold(0)); 
  }
      

#if 0
    
  printf("----------------------------- \n");
  ctpStatus(2);
  printf("----------------------------- \n");
      
    
  int iwait = 0;
  int allchanup = 0;
  while(allchanup  != (0x7) ) {
    iwait++;
    allchanup = ctpGetAllChanUp();
    if(iwait > 1000)
      {
	printf("iwait timeout   allchup - 0x%x\n",allchanup);
	break;
      }
  }    
#endif

  return(0);

    
}


int ctp_prestart(){

  ctpFiberReset();
  ctpPayloadReset();
  ctpAlignAtSyncReset(1);
  

  taskDelay(10);
  ctpStatus(1);
  ctpResetScalers();
  
  printf("CTP sync counter = %d\n",ctpGetSyncScaler());

  printf("CTP sync counter = %d\n",ctpGetSyncScaler());


  taskDelay(10);

  // Alex    
  int iwait = 0;
  int allchanup = 0;
  while(allchanup  != (0x7) ) {
    iwait++;
    allchanup = ctpGetAllChanUp();

    taskDelay(2);

    if(iwait > 1000)
      {
	printf("iwait timeout   allchup - 0x%x\n",allchanup);
	break;
      }
  }    


  printf("----------------------------- \n");
  ctpStatus(2);
  printf("----------------------------- \n");
  




  return(0);
}

int ctp_go(){
  
  int rval = -1;

  //    ctpAlignAtSyncReset(0);
  //    ctpGetAlignmentStatus(pflag,ntries)
  rval = ctpGetAlignmentStatus(1,10);
  
  if(rval == 0){
    daLogMsg("ERROR"," CTP alignment failed. Exit \n");  
    printf(BOLDRED "ERROR: CTP alignment failed. Exit \n" RESET );
    ROL_SET_ERROR;
  }

  ctpStatus(1);
  
  return(0);
}
