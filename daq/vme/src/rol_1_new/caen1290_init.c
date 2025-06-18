#include "c1190Lib.h"

extern int Nc1190;

/* CAEN 1190/1290 specific definitions */

int caen1290_download(){

  int ii, chip;
  int Nc1190 = 7;

  int res = 1;

  int tdc_offset = 0;

  unsigned int adjust = 0;
  unsigned int adjust_set = 0;

  int caen_slot = 0;

  //  UINT32 list[6] = {0x11180000, 0x11200000, 0x11280000, 0x11300000, 0x11380000, 0x11400000};
  //  tdc1190InitList(list,6,1);
  
  vmeDmaAllocLLBuffer();

  tdc1190Init(0x11180000,0x80000,Nc1190,0);


  printf(BOLDBLUE " Initialize Multi-block (CBLT) readout for CAEN TDCs. Block Level =  %d \n" RESET,  blockLevel);

  printf("\n");
  printf(BOLDBLUE "Number of CAEN1290 TDC initialized = %d \n" RESET, Nc1190);
  printf("\n");
  
   
  for(ii = 0; ii < Nc1190; ii++) tdc1190Clear(ii);
  for(ii = 0; ii < Nc1190; ii++) tdc1190Reset(ii);

  
  tdc1190InitMCST(0x11180000);    
  tdc1190ConfigureGReadout(0);     //  CBLT



  tdc1190GSetTriggerMatchingMode();
  tdc1190GSetWindowWidth(caen1290[3].window_width);

  tdc_offset = caen1290[3].window_offset  - trig_delay*4;

  tdc1190GSetWindowOffset(tdc_offset);

  tdc1190GSetExtraSearchMargin(caen1290[3].window_extra);

  tdc1190GSetRejectMargin(caen1290[3].window_reject);

  tdc1190GEnableTriggerTimeSubtraction();

  tdc1190GSetEdgeDetectionConfig(caen1290[3].edge);

  tdc1190GEnableAllChannels();

  printf(BOLDMAGENTA "Set Block Lebel to %d  \n" RESET, blockLevel);
  tdc1190GSetBLTEventNumber(blockLevel);

  tdc1190GSetMaxNumberOfHitsPerEvent(caen1290[3].n_Hits);

  for(ii = 0; ii < Nc1190; ii++) {
    /* program TDC for trigger matching mode */
    //    tdc1190SetTriggerMatchingMode(ii);
    
    /* set Trigger Window Width (ns) */
    //    tdc1190SetWindowWidth(ii, caen1290[3].window_width);
    

    //    tdc_offset = caen1290[3].window_offset  - trig_delay*4;

    //    printf(BOLDMAGENTA " Global Trigger Delay = %d (ns).  Latency = %d  \n" RESET, trig_delay, 
    //	   tdc_offset );
            
    /* set Trigger Window Offset (ns) */
    //    tdc1190SetWindowOffset(ii, tdc_offset);

    /* set Extra Search Margin (after window) (ns) */
    //    tdc1190SetExtraSearchMargin(ii,  caen1290[3].window_extra);

   /* set Reject Margin (before window) (ns) */
    //    tdc1190SetRejectMargin(ii,  caen1290[3].window_reject);

    /* enable subtraction of trigger time */
    //    tdc1190EnableTriggerTimeSubtraction(ii);

    /* set Edge Detection */
    //    tdc1190SetEdgeDetectionConfig(ii, caen1290[3].edge);


    // NOT USED
    /* set Channels Enable Mask */
    //    tdc1290EnableChannels(ii, tdc[ii].mask);

    //    tdc1190EnableChannels(ii, 0xFFFFFFFF);
    // NOT USED

    //    tdc1190EnableAllChannels(ii);
    /* set Readout settings */

   /* enable 64-bit alignment */
    /*  NOTE: 64-bit alignment must be enabled for any DMA readout,
       not only for MBLT(D64) but for BLT(D32) as well; some DMA engines
       (for example the one on mv5100) will cut off last 32-bit word if
       it is not even, for example if event size is equal to 137 D32 words,
       only 136 D32 words will be transfered */
    /* in addition universe library contains check for 64-bit alignment
       in dmastart procedure, it will return error if ... */

    
      // Don't need to set for mult-block readout
      //      tdc1190EventFifo(ii, caen1290[3].berr_fifo);
      //      tdc1190BusError(ii, 0);
      //      tdc1190Align64(ii, 1);
    

    /* set BLT Event Number Register */
    //    printf(BOLDMAGENTA "Set Block Lebel to %d  \n" RESET, blockLevel);
    //    tdc1190SetBLTEventNumber(ii, blockLevel);

    /* set Max Number of Hits per Event */
    //    tdc1190SetMaxNumberOfHitsPerEvent(ii, caen1290[3].n_Hits);

    // NOT USED
    //    if(blockLevel == 1){
    //      tdc1190ResetMCST(ii);
    //    }
    // NOT USED

    
    /* set 'almost full' level */
    tdc1190SetAlmostFullLevel(ii, caen1290[3].almostFullLevel);

    /* program output connector to signal on it */
    tdc1190SetOutProg(ii,caen1290[3].outProgControl);
 

    // Set R-C chip adjustment
    caen_slot = tdc1190GetGeoAddress(ii);

    if( (caen_slot < 3) || (caen_slot > 20) || (caen_slot == 11) || (caen_slot == 12)){
      printf(BOLDRED " FATAL: Wrong slot for CAEN TDC %d \n" RESET, caen_slot);
      daLogMsg("ERROR"," Wrong slot for CAEN TDC %d\n", caen_slot );
      ROL_SET_ERROR;
    }

    // Enable the use of the Compensation table

    if(caen1290[3].inl > 0){
      printf(BOLDMAGENTA "Enable Non-Linearity Corrections \n" RESET);
      tdc1190Compensation(ii, 1);
    } else if(caen1290[3].inl == 0){
      printf(BOLDMAGENTA "Disable Non-Linearity Corrections \n" RESET);
      tdc1190Compensation(ii, 0);
    } else {
      printf(BOLDRED "FATAL: Wrong value of TDC1290_INL  %d \n" RESET, caen1290[3].inl );
      printf(BOLDRED "Enable Non-Linearity Corrections \n" RESET);
      tdc1190Compensation(ii, 1);
      daLogMsg("ERROR"," Wrong value of  TDC1290_INL  %d\n", caen1290[3].inl );
    }
      

    for(chip = 0; chip < 4; chip++){
      if( (caen1290[caen_slot].rc_adjust[chip][0] >= 0) && (caen1290[caen_slot].rc_adjust[chip][1] >= 0) &&
	  (caen1290[caen_slot].rc_adjust[chip][2] >= 0) && (caen1290[caen_slot].rc_adjust[chip][3] >= 0)){
	
	adjust = ((caen1290[caen_slot].rc_adjust[chip][0] & 0x7) | 
	       ((caen1290[caen_slot].rc_adjust[chip][1] & 0x7) << 3) |
	       ((caen1290[caen_slot].rc_adjust[chip][2] & 0x7) << 6) |
	       ((caen1290[caen_slot].rc_adjust[chip][3] & 0x7) << 9) );
	
	printf(BOLDBLUE " SetRCAdjust for SLOT = %d, CHIP = %d,  ADJUST = 0x%x\n" RESET, caen_slot, chip, adjust);

	tdc1190SetRCadjust(ii, chip, adjust);

	adjust_set = tdc1190GetRCadjust(ii, chip);
	
	if(adjust != adjust_set){
	  printf(BOLDRED " FATAL: CAEN TDC RC_ADJUST is not set for module in slot %d.  Config value  =  0x%x, Register  =  0x%x\n" RESET, 
		 caen_slot, adjust, adjust_set );
	  daLogMsg("ERROR",  " FATAL: CAEN TDC RC_ADJUST is not set for module in slot %d.  Config value  =  0x%x, Register  =  0x%x\n", 
		   caen_slot, adjust, adjust_set ); 

	}


	printf(BOLDMAGENTA " GetRCAdjust for SLOT = %d, CHIP = %d, ADJUST = 0x%x\n" RESET, caen_slot, chip, 
	       adjust_set );

      }
    }
    
  }


   // Print out settings

  printf("\n");
  printf(" SASCHA Print out settings \n");
  printf("\n");
  
  tdc1190ReadTriggerConfiguration(5);
  printf("\n");
  tdc1190ReadEdgeDetectionConfig(5);
  
  res = tdc1190GetEdgeResolution(5);
  printf(" Resolution = %d ps\n",res);

  tdc1190Status(5);

  res = tdc1190ReadAcquisitionMode(5);
  if(res==0)
    printf(" Acquisition Mode = Continuous Storage\n");
  else
    printf(" Acquisition Mode = Trigger Matching\n");
  
  tdc1190GStatus(1);

  //  tdc1190RCStatus();

  return(0);
}
