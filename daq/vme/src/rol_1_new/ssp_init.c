
unsigned int sspMask   = 0;    /* bit=slot (starting from 0) */

int ssp_download(){

  int slot;

  // Make sspSlotMask according to modules found in the configuration

  for(slot = 3; slot < 21; slot++){
    if(ssp[slot].group == 1){
      sspMask |= (1 << slot); /* Add it to the mask */
      
    }
  }

  sdInit();   /* Initialize the SD library */

  printf(BOLDBLUE "sspMask =  0x%x  \n" RESET ,sspMask); 
  
  // ALEX SSP TESTS
  //  sdSetActiveVmeSlots(sspMask);    /* Use the sspMask to configure the SD */
  sdSetActiveVmeSlots(0); 

  sdStatus();
  

  return(0);    
}


int ssp_prestart(){


  int fiber;
  int asic;
  int ch;
  int connectedfibers;


  printf("SSP Prestart() starts =========================\n");
  
  /*****************
   *   SSP SETUP - must do sspInit() after master TI clock is stable, so do it in Prestart
   *****************/
  int iFlag = 0;
  
  iFlag  = SSP_INIT_MODE_DISABLED; /* Disabled, initially */
  iFlag |= SSP_INIT_SKIP_FIRMWARE_CHECK;
  iFlag |= SSP_INIT_MODE_VXS;
  //  iFlag |= SSP_INIT_FIBER0_ENABLE;         /* Enable hps1gtp fiber ports */
  //  iFlag |= SSP_INIT_FIBER1_ENABLE;         /* Enable hps1gtp fiber ports */
  //  iFlag |= SSP_INIT_GTP_FIBER_ENABLE_MASK; /* Enable all fiber port data to GTP */
  /*iFlag|= SSP_INIT_NO_INIT;*/ /* does not configure SSPs, just set pointers */
  
  int nssp = 0;
  vmeBusLock();
  nssp = sspInit(0, 0, 0, iFlag); /* Scan for, and initialize all SSPs in crate */
  vmeBusUnlock();
  printf("rol1: found %d SSPs (using iFlag=0x%08x)\n",nssp,iFlag);
  
  
  int id = 0, jj;
  
  //  ssp_init();
  
  for(id = 0; id < nssp; id++){

    int slot = sspSlot(id);


    // Readout window size and position    
    printf(BOLDBLUE "SSP Slot %d:  SetWindowWidth   %d \n" RESET, slot, ssp[slot].window_width);
    printf(BOLDBLUE "SSP Slot %d:  SetWindowOffset  %d \n" RESET, slot, ssp[slot].window_offset);

    sspSetWindowWidth(slot, ssp[slot].window_width);
    sspSetWindowOffset(slot, ssp[slot].window_offset);
    
    // Pulser
    if(ssp[slot].pulser_freq > 0){
      sspPulserSetup(slot, (float)ssp[slot].pulser_freq, 0.5, 0xFFFFFFFF);
      sspPulserStart(slot);
      printf("SSP PULSER STARTED, FREQ=%f\n", (float)ssp[slot].pulser_freq);
    } else{
      sspPulserSetup(slot, (float)0.0, 0, 0);
      printf("SSP PULSER STOPPED\n");
    }
    

    for(jj = 0; jj < SD_SRC_NUM; jj++)
      sspSetIOSrc(slot, jj, ssp[slot].ssp_io_mux[jj]);  
  
    sspRich_SetFiberEbDisable(slot, ssp[slot].dirc.disable_evtbuild);
    sspRich_SetFiberDisable(slot, ssp[slot].dirc.disable_fiber);

    printf(BOLDBLUE "Discabled fibers 0X%X \n\n", ssp[slot].dirc.disable_fiber);
    
    sspRich_GetConnectedFibers(slot, &connectedfibers);

    printf(BOLDBLUE "TEST TEST TES:  SLOT =  %d   Fibers in config = 0X%X  Connected fibers  =  0X%X  \n\n", 
           slot, ssp[slot].dirc.fiber_in_config, connectedfibers);


    if(ssp[slot].dirc.fiber_in_config != connectedfibers){
      
      printf(BOLDRED "FATAL: DIRC SPP in slot %d. Fibers NOT connected. Config =  0X%X  Connected = 0X%X  \n" RESET, slot, 
             ssp[slot].dirc.fiber_in_config, connectedfibers );
      
      daLogMsg("ERROR",     "FATAL: DIRC SPP in slot %d. Fibers NOT connected. Config =  0X%X  Connected = 0X%X  \n", slot, 
               ssp[slot].dirc.fiber_in_config, connectedfibers ); 
      ROL_SET_ERROR;
    }


    printf("\n");
    printf(BOLDBLUE "CONNECTED FIBERS = 0x%X  \n\n", connectedfibers);

    for(fiber = 0; fiber < DIRC_FIBER_NUM; fiber++){

      if(connectedfibers & (1<<fiber)){

        printf(BOLDBLUE "Set parameters for fiber  %d \n" RESET,fiber);

        for(asic = 0; asic < 3; asic++){
          
          
          sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_GLOBAL0,            0, ssp[slot].dirc.fiber[fiber].chip[asic].Global0.val);
          sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_GLOBAL1,            0, ssp[slot].dirc.fiber[fiber].chip[asic].Global1.val);
          
          sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_DAC0,               0, ssp[slot].dirc.fiber[fiber].chip[asic].DAC0);
          sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_DAC1,               0, ssp[slot].dirc.fiber[fiber].chip[asic].DAC1);
          

          for(ch = 0; ch < 64; ch++){

            int sum, ctest, maskor, gain;
              
            gain = ssp[slot].dirc.fiber[fiber].chip[asic].Gain[ch];
            
            if(ch < 32){
              sum = (ssp[slot].dirc.fiber[fiber].chip[asic].Sum[0] & (1 << ch)) ?        1 : 0;
              ctest = (ssp[slot].dirc.fiber[fiber].chip[asic].CTest[0] & (1 << ch)) ?    1 : 0;
              maskor = (ssp[slot].dirc.fiber[fiber].chip[asic].MaskOr[0] & (1 << ch)) ?  1 : 0;
            } else {
              sum = (ssp[slot].dirc.fiber[fiber].chip[asic].Sum[1] & (1<<(ch-32))) ?          1 : 0;
              ctest = (ssp[slot].dirc.fiber[fiber].chip[asic].CTest[1] & (1 << (ch-32))) ?    1 : 0;
              maskor = (ssp[slot].dirc.fiber[fiber].chip[asic].MaskOr[1] & (1 << (ch-32))) ?  1 : 0;
            }
            
            sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_GAIN,            ch, gain);
            sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SUM,             ch, sum);
            sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CTEST,           ch, ctest);
            sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_MASKOR,          ch, maskor);
          }         
 
        }  // Loop over ascis


        sspRich_UpdateMarocRegs(slot, fiber);
        
        sspRich_SetCTestAmplitude(slot, fiber, ssp[slot].dirc.fiber[fiber].ctest_dac);
        
        if(ssp[slot].dirc.fiber[fiber].ctest_enable)
          sspRich_SetCTestSource(slot, fiber, RICH_SD_CTEST_SRC_SEL_SSP);
        else
          sspRich_SetCTestSource(slot, fiber, RICH_SD_CTEST_SRC_SEL_0);
        
        
        sspRich_SetTDCEnableChannelMask(slot, fiber,
                                        ssp[slot].dirc.fiber[fiber].tdc_enable_mask[0],
                                        ssp[slot].dirc.fiber[fiber].tdc_enable_mask[1],
                                        ssp[slot].dirc.fiber[fiber].tdc_enable_mask[2],
                                        ssp[slot].dirc.fiber[fiber].tdc_enable_mask[3],
                                        ssp[slot].dirc.fiber[fiber].tdc_enable_mask[4],
                                        ssp[slot].dirc.fiber[fiber].tdc_enable_mask[5]
                                        );
        
        sspRich_SetLookback(slot, fiber, ssp[slot].dirc.fiber[fiber].window_offset);
        sspRich_SetWindow(slot, fiber, ssp[slot].dirc.fiber[fiber].window_width);
                
      }
    }
    
    
    printf(BOLDBLUE "SSP Slot %d:  SetBlockLevel  %d \n" RESET, slot, blockLevel);

    vmeBusLock();         
    sspSetBlockLevel(slot, blockLevel);
    sspGetBlockLevel(slot);
    vmeBusUnlock();
  }

#ifdef SHM_BUF
  printf("======================  save SSP config =============================\n");
  FILE *Sspfile=NULL;
  char Sspname[256];
  char SSPPath[256];
  if (!strncmp(getenv("USER"),"hdops",5)) { //--- this is hdops 
    sprintf(SSPPath,"/gluex/CALIB/ALL/ssp/");
  } else {  //-- any other user 
    sprintf(SSPPath,"%s/ROOT_HIST",getenv("HOME"));
  }
  sprintf(Sspname,"%s/SSP_ASIC.config",SSPPath);
  sspRich_SaveConfig(Sspname);
  printf("===   save SSP config: %s \n",Sspname);

  if (CALIBRATION_FLAG>0) {
    printf("===>   START DIRC Theshold scan , CALIBRATION_FLAG=%d \n",CALIBRATION_FLAG);
    shmem_ptr->dirc_scalers.slots[DIRC_MAX_SLOT]=0; //--- disable rates readout 
    dirc_scan();                                    //--- dirc threshold scan 
    shmem_ptr->dirc_scalers.slots[DIRC_MAX_SLOT]=1; //--- enable rates readout 
  }

  printf("===================================================================\n");
#endif

  //--------------------------------------------------------------------------------

  return(0);
}

int ssp_go(){
  
  return(0);
}
