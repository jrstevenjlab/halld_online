// #include "dsc2Lib.h"


int dsc_download(){

  int kk, slot = -1;

  int Ndsc = 0;
  
  int chan;

  UINT16  selMask,  bypMask;

  UINT16 defTDCChMask = 0xFFFF;
  
  dsc2Init(0x100000,0x80000,20,0);
  
  
  Ndsc = dsc2GetNdsc();
  
  for(kk = 0; kk < Ndsc; kk++) {
    
    slot = dsc2Slot(kk);
    
    printf(BOLDBLUE "TDCWidth =  %d  TRGWidth =  %d  \n" RESET, dsc[slot].TDCWidth, dsc[slot].TRGWidth);


    printf(BOLDRED "SLOT = %d \n" RESET, slot);

    dsc2SetPulseWidth(slot, dsc[slot].TDCWidth, 1);
    //    dsc2SetPulseWidth(slot, dsc[slot].TRGWidth, 2);

    for(chan = 0; chan < MAX_DSC2_CH; chan++){
      if( (dsc[slot].TDCThreshold[chan] > -4095 ) && (dsc[slot].TDCThreshold[chan] < 4095)){
        // Invert values from the config file

        printf(BOLDRED " %d " RESET, dsc[slot].TDCThreshold[chan]);

        INT16 set_thr = dsc[slot].TDCThreshold[chan];
        dsc2SetBipolarThreshold(slot, chan, set_thr, 1);
        //    dsc2SetThreshold(slot,chan, set_thr, 1); 
        
      } else {
        printf(BOLDRED     "Warning: Wrong DSC2 Threshold range %d. Set Thr to -4095   \n" RESET, dsc[slot].TDCThreshold[chan] );
        daLogMsg("WARNING","Warning  Wrong DSC2 Threshold range %d. Set Thr to -4095   \n \n",    dsc[slot].TDCThreshold[chan]);    
        dsc2SetBipolarThreshold(slot, chan, 4095, 1);
      }
      //dsc2SetThreshold(slot,chan,dsc[slot].TRGThreshold[chan],2); // SF:: TRG set by EPICS: 
    }
    
    selMask = 0xFFFF; /* by default, trigger output will use TRG discriminator */
    bypMask = 0xFFFF; /* by default, TRG discriminator output goes directly to trigger output */
    
    for(chan = 0; chan < MAX_DSC2_CH; chan++) {

      printf("SASCHA: TRGDigitalWidth = %d   TRGDigitalDelay = %d  \n",dsc[slot].TRGDigitalWidth[chan], dsc[slot].TRGDigitalDelay[chan]);
      

      dsc2SetTRGOut(slot, chan, dsc[slot].TRGDigitalWidth[chan], dsc[slot].TRGDigitalDelay[chan]);
      
      /* if TRGDigitalWidth>0, set trigger output to the digital */
      if(dsc[slot].TRGDigitalWidth[chan]>0) {
        /*printf("[%2d][%2d] befor 0x%04x\n",slot,chan,bypMask);*/
        bypMask = bypMask ^ (1<<chan);
        /*printf("[%2d][%2d] after 0x%04x\n",slot,chan,bypMask);*/
      }
    }
    

    dsc2SetTRGOutSource(slot, selMask, bypMask);
    
    dsc2Status(slot, 1);
    
    printf(" TDC thresholds = ");
    
    for(chan = 0; chan < MAX_DSC2_CH; chan++)
      //      printf("  %d ", dsc2GetThreshold(slot, chan, 1));
      printf("  %d ", dsc2GetBipolarThreshold(slot, chan, 1));

    printf("\n");
    
    
  }
  
  
  return(0);
  
}

int dsc_prestart(){
  
  return(0);
  
}
