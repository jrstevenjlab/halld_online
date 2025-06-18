//--------------  RCM MONOTORING -------------------
//#include "shmem_roc.h"

static void shmem_get();
static int  semid,shmid;
//static roc_shmem *shmem_ptr;
FADC_Config* DAC_250;
FADC_Config* DAC_125;
int *HIST_FLAG;
int *RUN_NUM;
int *RUN_FLAG;

//-- timer --
//struct timeval tvA[10], tvB[10]; 
//SHM_HIST *hrol1;
int nevents=0;
int EVT_THR=100;
int DAC, DAC_old;


void monitor_download(){
  daLogMsg("INFO","ROL_1 SHMEM RCM ...");
  shmem_get();
  daLogMsg("INFO","ROL_1 SHMEM RCM attached ");
  HIST_FLAG=&shmem_ptr->EventBufferFLAG;
  RUN_NUM=&shmem_ptr->RUN_Number;
  
  DAC_250=&shmem_ptr->fadc_config[1];
  DAC_125=&shmem_ptr->fadc_config[0];
  
  RUN_FLAG=&shmem_ptr->RUN_Flag;
  *RUN_FLAG=0;
  if (CALIBRATION_FLAG>0)  {
    *RUN_FLAG=1;    //-- read from config file 
    daLogMsg("INFO","ROL_1:DL: Calibration flag = %d RUN_FLAG=%d ",CALIBRATION_FLAG,*RUN_FLAG);
  }
  DAC_250->Flag=0;
  DAC_125->Flag=0;
  
  hrol1 = shmem_ptr->H_rol1;
  hbook1(hrol1,1,"ROL_1 tiprimary, [us] ",500,0.,500.); 
  hbook1(hrol1,2,"ROL_1 trigger vme_list, [us] ",500,0.,500.);
  hbook1(hrol1,3,"ROL_1 trigger vme_list, [ms] ",100,0.,100.);
  hbook1(hrol1,4,"ROL_1 DMA wait, [us] ",500,0.,500.);
  hbook1(hrol1,5,"ROL_1 DMA wait, [ms] ",100,0.,100.);
  hbook1(hrol1,6,"ROL_1 monitor call, [us] ",500,0.,500.);
  hbook1(hrol1,7,"ROL_1 trigger vme_list, [us/100] ",100,0.,100.);
  hbook1(hrol1,11,"ROL_1 trigger vme_list.1, [us/100] ",100,0.,100.);
  hbook1(hrol1,12,"ROL_1 trigger vme_list.2, [us/100] ",100,0.,100.);
  hbook1(hrol1,13,"ROL_1 trigger vme_list.3, [us/100] ",100,0.,100.);
  hbook1(hrol1,14,"ROL_1 trigger vme_list.4, [us/100] ",100,0.,100.);
  hbook1(hrol1,15,"ROL_1 trigger vme_list.5, [us/100] ",100,0.,100.);
  hbook1(hrol1,16,"ROL_1 trigger vme_list.6, [us/100] ",100,0.,100.);
}

void monitor_prestart(){
  printf("===================================================================\n");
  printf("===  RUN_NUM=%d  RUN_FLAG=%d CALIBRATION_FLAG=%d nfadc=%d nfadc125=%d \n",*RUN_NUM,*RUN_FLAG,CALIBRATION_FLAG,nfadc,nfadc125);
  printf("===================================================================\n");

  *RUN_FLAG=0;
  if (CALIBRATION_FLAG>0)  {
    *RUN_FLAG=1;    //-- read from config file 
    daLogMsg("INFO","ROL_1:PRE: Calibration flag = %d RUN_FLAG=%d ",CALIBRATION_FLAG,*RUN_FLAG);
  }

  if (*RUN_FLAG>0) {
    EVT_THR=100;
    //----------------------------------------------------------------------
    if(nfadc!=0)  {
      DAC=3000;  //--  ?? also config ?
      shmem_ptr->FADC_TYPE=1;
      int slot,ii;
      for(ii=0; ii<MAX_CONF;ii++) { DAC_250->DAC[ii]=-1;  DAC_250->THR[ii]=-1; } //-- reset --
      for(slot = 0; slot < nfadc; slot++){
	FA_SLOT = fadcID[slot];
	printf("ROL_1:: rocPre():  F250::  Slot=%d,  mode=%d, set DAC=%d\n",FA_SLOT,fa250[FA_SLOT].mode,DAC);
	for(ii = 0; ii < MAX_FADC250_CH; ii++) {
	  faSetDAC(             FA_SLOT, DAC, (1<<ii));
	  faSetThreshold(       FA_SLOT, 5,(1<<ii));
	  //
	  int bin=FA_SLOT*MAX_FADC250_CH+ii; DAC_250->DAC[bin]=+1;  DAC_250->THR[bin]=+1; //-- mark active --
	}
      }
    }
    //----------------------------------------------------------------------
    if(nfadc125!=0)  {
      if (detector_type == cdc) {
        DAC=39000;  //--  ?? also config ?
      } else if (detector_type == st) {
        DAC=49000;  //--  ?? also config ?
      } else { 
        DAC=36000; 
      }
      shmem_ptr->FADC_TYPE=0;
      int slot,ii,ichan;
      for(ii=0; ii<MAX_CONF;ii++) { DAC_125->DAC[ii]=-1;  DAC_125->THR[ii]=-1; } //-- reset --
      for(slot = 0; slot <nfadc125 ; slot++){      
	FA_SLOT = fa125Slot(slot);
	printf("ROL_1:: rocPre():  F125::  Slot=%d,  mode=%d, set DAC=%d\n",FA_SLOT,fa250[FA_SLOT].mode,DAC);
	for(ichan = 0; ichan < 72; ichan++) {
	  fa125SetOffset(FA_SLOT,ichan,DAC);
	  fa125SetThreshold(FA_SLOT, ichan, 1);
	  //
	  int bin=FA_SLOT*MAX_FADC125_CH+ichan; DAC_125->DAC[bin]=+1;  DAC_125->THR[bin]=+1; //-- mark active --
          printf("ROL_1:: rocPre():  F125::  Slot=%d,  chan=%d, bin=%d  DAC/marker=%d \n",FA_SLOT,ichan,bin,DAC_125->DAC[bin]);
	}
      }
    }
    nevents=0;
  }
  dmaPStatsAll();
}

void monitor_trigger(){

  nevents++;
  //if (CALIBRATION_FLAG) printf("===================================================================\n");
  if (nevents<1000)     printf("===   NEVENTS = %d (%d) CALIBRATION_FLAG=%d,  RUN_FLAG=%d nfadc=%d nfadc125=%d \n",nevents,EVT_THR,CALIBRATION_FLAG,*RUN_FLAG,nfadc,nfadc125);
  //if (CALIBRATION_FLAG) printf("===================================================================\n");
  int slot;
  //=======================================================================================================
  if (*RUN_FLAG>0) {                        //--- calibration run
    if (*RUN_FLAG==1) {                  
      //if (nevents%100==0) { 
      if (nevents>EVT_THR) { //EVT_THR=nevents+100;
	//--------------------------------------------------------------------------------
	if(nfadc!=0) {  //-- F250 --
	  *RUN_FLAG=9;  //---- STOP fill adc
	  shmem_ptr->FADC_TYPE=1;
	  DAC_old=DAC;
          if (detector_type == st) {
            DAC+=300;  //--  ?? also config ?
          } else {
            DAC+=300;   //-- DAC step, config ?
          }
	  DAC_250->DAC_set=DAC;  
          int ii;
	  for(slot = 0; slot < nfadc; slot++) {
	    FA_SLOT = fadcID[slot];
	    for(ii = 0; ii < MAX_FADC250_CH; ii++) {
	      faSetDAC(             FA_SLOT, DAC, (1<<ii));
	      faSetThreshold(       FA_SLOT, 5,(1<<ii));
	    }
	  }
	  *RUN_FLAG=DAC_old;  //---  start fit
	} //-- nfadc!=0 
	//----------------------------------------------------------------------
	if(nfadc125!=0)  {
	  *RUN_FLAG=9;  //---- STOP fill adc
	  shmem_ptr->FADC_TYPE=0;
	  DAC_old=DAC; 
          if (detector_type == cdc) {
            DAC-=3000;  //--  ?? also config ?
          } else if (detector_type == st) {
            DAC-=10000;  //--  ?? also config ?
          } else { 
            DAC-=10000; 
          }
	  DAC_125->DAC_set=DAC;  
	  int ichan;
	  for(slot = 0; slot <nfadc125 ; slot++){      
	    FA_SLOT = fa125Slot(slot);
	    printf("ROL_1:: rocTrigger():  F125::  Slot=%d,  mode=%d, set DAC=%d\n",FA_SLOT,fa125[FA_SLOT].mode,DAC);
	    for(ichan = 0; ichan < 72; ichan++) {
	      fa125SetOffset(FA_SLOT,ichan,DAC);
	      fa125SetThreshold(FA_SLOT,  ichan , 1);
	    }
	  }
	  *RUN_FLAG=DAC_old; //---  start fit
	} //-- nfadc125!=0 
	printf("===================================================================\n");
	printf("===   SET new DACs = %d FADC_TYPE=%d \n",DAC,shmem_ptr->FADC_TYPE);
	printf("===================================================================\n");
        EVT_THR=nevents+100;
      }
      //===========================================================================================================
    } else if (*RUN_FLAG==2 || *RUN_FLAG==4 || *RUN_FLAG==6) {            //-- pedestals and THR calculated , load  and save 
      
      printf("===================================================================\n");
      printf("===   LOAD new F%d DACs ....      RUN_FLAG=%d    \n",125+125*shmem_ptr->FADC_TYPE,*RUN_FLAG);
      printf("===================================================================\n");
      
      FILE *DacFile=NULL;
      char DacName[256];
      char CalibPath[256];
      int FAt=125+125*shmem_ptr->FADC_TYPE;
      if (!strncmp(getenv("USER"),"hdops",5)) { //--- this is hdops 
	sprintf(CalibPath,"/gluex/CALIB/ALL");
      } else {  //-- any other user 
	sprintf(CalibPath,"%s/ROOT_HIST",getenv("HOME"));
      }
      if (*RUN_FLAG==2) {
	sprintf(DacName, "%s/fadc%d/%s_fadc%d_Run_%d.cnf",CalibPath,FAt,roc_name,FAt,rol->runNumber);
      } else {
	sprintf(DacName, "%s/fadc%d/%s_fadc%d_Run_%d_%d.cnf",CalibPath,FAt,roc_name,FAt,rol->runNumber,*RUN_FLAG);
      }
      printf("Open file  :%s:  \n",DacName);
      if((DacFile=fopen(DacName,"w")) == NULL) {   printf("Can not open file %s \n",DacName);  
      } else {  //---  write Calibration file ---
	fprintf(DacFile,"\nCRATE  %s \n\n\n\n",roc_name);  
	fprintf(DacFile,"#   slots:   1   2  3  4  ... \n\n\n\n");   
      }
      
      //--------------------------------------------------------------------------------
      for(slot = 0; slot < nfadc; slot++) {
	FA_SLOT = fadcID[slot];
	
	if(DacFile) {
	  fprintf(DacFile,"############################ \n");
	  fprintf(DacFile,"FADC250_SLOTS   %d \n",FA_SLOT);
	  fprintf(DacFile,"######################### \n\n");
	  fprintf(DacFile,"FADC250_ALLCH_DAC  ");
	}
	//--------------------------- DAC 250 -------------------------------------------
        int ii;
	for(ii = 0; ii < MAX_FADC250_CH; ii++) {
	  int bin=FA_SLOT*MAX_FADC250_CH+ii;
	  if (DAC_250->DAC[bin]<0) continue; 
          if (DAC_250->THR[bin]<=0) DAC_250->THR[bin]=1;
          if (*RUN_FLAG==2 && (DAC_250->DAC[bin]<100  || DAC_250->THR[bin]>120) ) {
            daLogMsg("ERROR","fa250 Pedestal ERROR: slot=%2d chan=%2d DAC=%d THR=%d\n", FA_SLOT,ii,DAC_250->DAC[bin],DAC_250->THR[bin]);
            DAC_250->DAC[bin]=4000;
          }
	  faSetDAC(             FA_SLOT, DAC_250->DAC[bin],(1<<ii));
	  //faSetThreshold(       FA_SLOT, DAC_250->THR[bin],(1<<ii));
	  faSetThreshold(       FA_SLOT, 5 ,(1<<ii));
	  if(DacFile) fprintf(DacFile," %d ",DAC_250->DAC[bin]);
	  printf("===   LOAD:: slot=%2d chan=%2d dac=%4d thr=%d \n",FA_SLOT,ii,DAC_250->DAC[bin],DAC_250->THR[bin]);
	}
	if(DacFile) fprintf(DacFile,"\n\n");
	//--------------------------- THR 250 -------------------------------------------
	if(DacFile) fprintf(DacFile,"FADC250_ALLCH_THR  ");
	for(ii = 0; ii < MAX_FADC250_CH; ii++) {
	  int bin=FA_SLOT*MAX_FADC250_CH+ii;
	  if (DAC_250->DAC[bin]<0) continue; 
	  if(DacFile) fprintf(DacFile," %d ",DAC_250->THR[bin]);
	}
	if(DacFile) fprintf(DacFile,"\n\n");
      }
      //----------------------------------------------------------------------    
      int ichan;
      for(slot = 0; slot <nfadc125 ; slot++){      
	FA_SLOT = fa125Slot(slot);
	if(DacFile) {
	  fprintf(DacFile,"############################ \n");
	  fprintf(DacFile,"FADC125_SLOTS   %d \n",FA_SLOT);
	  fprintf(DacFile,"######################### \n");
	}
	//--------------------------- DAC -------------------------------------------
	for(ichan = 0; ichan < 72; ichan++) {
	  if ((ichan%18)==0) { 
	    fprintf(DacFile,"\nFADC125_DAC_CH_%02d_%02d  ",ichan,ichan+17);
	  }
	  int bin=FA_SLOT*72+ichan;
	  if (DAC_125->DAC[bin]<0) continue; 
	  printf("===   LOAD:: slot=%2d chan=%2d dac=%4d THR=%3d\n" ,FA_SLOT, ichan, DAC_125->DAC[bin], DAC_125->THR[bin]);
          if (*RUN_FLAG==2 && ( DAC_125->DAC[bin]<100 || DAC_125->THR[bin]>255) ) 
            daLogMsg("ERROR","fa125 Pedestal ERROR: slot=%2d chan=%2d DAC=%d THR=%d\n",FA_SLOT,ichan,DAC_125->DAC[bin],DAC_125->THR[bin]);
	  fa125SetOffset(FA_SLOT,ichan,  DAC_125->DAC[bin]  );
	  // fa125SetThreshold(FA_SLOT, ichan,  DAC_125->THR[bin]);
	  fa125SetThreshold(FA_SLOT, ichan, 1);
	  
	  if(DacFile) fprintf(DacFile," %d",DAC_125->DAC[bin]);
	}
	if(DacFile) fprintf(DacFile,"\n\n");
	//--------------------------- THR -------------------------------------------
	for(ichan = 0; ichan < 72; ichan++) {
	  if ((ichan%18)==0) { 
	    fprintf(DacFile,"\nFADC125_THR_CH_%02d_%02d  ",ichan,ichan+17);
	  }
	  int bin=FA_SLOT*72+ichan;
	  if (DAC_125->DAC[bin]<0) continue; //--  active chan only --
	  printf("===   SAVE:: slot=%2d chan=%2d dac=%4d THR=%3d\n" ,FA_SLOT, ichan, DAC_125->DAC[bin], DAC_125->THR[bin]);
	  if(DacFile) fprintf(DacFile," %d",DAC_125->THR[bin]);
	}
	if(DacFile) fprintf(DacFile,"\n\n");
	//--------------------------- MASK -------------------------------------------
	
	if(DacFile) fprintf(DacFile,"FADC125_CH_ENB   0xFFFFFF  0xFFFFFF  0xFFFFFF\n");
	if(DacFile) fprintf(DacFile,"FADC125_CH_DIS   0x0  0x0  0x0 \n\n");
	
      }
      //----------------------------------------------------------------------
      if (DacFile) fclose(DacFile); DacFile=NULL; 
      //----------------------------------------------------------------------
      if (*RUN_FLAG==4) *RUN_FLAG=15;   //---- no reset hist, and continue, last run ---
      if (*RUN_FLAG==6) *RUN_FLAG=17;   //---- no reset hist, and continue, last run ---
      //-----
      if (*RUN_FLAG==2) {
	if (CALIBRATION_FLAG==1) *RUN_FLAG=0; //-- read from config file:  1 = stop run do NOT reset histograms 
	if (CALIBRATION_FLAG==2) *RUN_FLAG=3; //-- read from config file:  2 = continue run , reset histograms 
      }
      printf("===   CHECK  RUN_FLAG=%d \n",*RUN_FLAG);
      //---  end (RUN_FLAG==2 || RUN_FLAG==4)
      //===========================================================================================================
    }  else if (*RUN_FLAG==5) {  //--- calibration run for THR
      if (nevents%100==0) {
	*RUN_FLAG=101;
      }
    }  else if (*RUN_FLAG==7) {  //--- calibration run for THR
      if (nevents%100==0) {
	*RUN_FLAG=102;
      }
    }
  } //-- if RUN_FLAG>0
}

void shmem_get() {
    /*---------------------------------------------------------------------*/
    /*      Shared memory                                                  */
    /*---------------------------------------------------------------------*/
    
    if ( (semid=semget( SEM_ID1, 1, 0) ) < 0 ) {
        printf("shmem_ROL_2: can not get semaphore \n"); 
	fflush(stdout);
    }

    if ( (shmid=shmget(SHM_ID1,sizeof(roc_shmem), 0))<0) {
      printf("==> shmem_ROL_2: shared memory 0x%x, size=%d get error=%d\n",SHM_ID1,sizeof(roc_shmem),shmid);
      fflush(stdout);
      exit(1);
    }
    
    if ( (shmem_ptr=(roc_shmem *) shmat (shmid, 0, 0))<0 ) {
        printf("==> shmem_ROL_2: shared memory attach error\n");
	fflush(stdout);
    }

    printf("==> shmem_ROL_2: shared memory attached OK ptr=%p\n",shmem_ptr); 
    fflush(stdout);

    
    printf("===== ROL_2 wait semaphore  ======  \n"); 
    fflush(stdout);
 
    //---   add new shmem client here -----------


    //-- access to shmem --
    //sem_wait(semid);
    
    if (shmem_ptr) shmem_ptr->rol2_hist=1;
    
    //sem_post(semid);
}
/* -----------------------------------------------------------------------------------------------------

SET:
===

monitor.c:                          *RUN_FLAG=0;
monitor.c:                          *RUN_FLAG=1;  //-- read from config file 

hist_mon_srv.cc:                    *RUN_FLAG=1;  //--- continue fill ADC hist 
hist_mon_srv.cc:                    *RUN_FLAG=2;  //--- all done load and stop.  //--- for RUN_FLAG=102

monitor.c: if (CALIBRATION_FLAG==2) *RUN_FLAG=3;  //-- read from config file:  2 = continue run , reset histograms 

hist_mon_srv.cc:                    *RUN_FLAG=4;  //--- set DAC and start run to check pedestals.
hist_mon_srv.cc:                    *RUN_FLAG=5;  //--- reset done, continue

monitor.c: if (CALIBRATION_FLAG==3) *RUN_FLAG=6;  //-- read from config file:  2 = continue run , reset histograms 
monitor.c:      if (*RUN_FLAG==4)   *RUN_FLAG=15; //---- no reset hist, and continue, last run ---
monitor.c:                          *RUN_FLAG=101;


CHECK:
======

hist_mon_srv.cc:      if (*RUN_FLAG==0 || *RUN_FLAG==1 || *RUN_FLAG==5) {   //--  RUN_FLAG==0,1,5  etc... 
hist_mon_srv.cc:      } else if (*RUN_FLAG==15) {  //--- 
hist_mon_srv.cc:      } else if (*RUN_FLAG==3)  {  //--- DACs loaded with interpolated values, THR is not good yet 
hist_mon_srv.cc:      } else if (*RUN_FLAG>100) {                    //--   fit ADC hist, RUN_FLAG=3000 , ....

monitor.c:  if (*RUN_FLAG>0) {                        //--- calibration run
monitor.c:    if (*RUN_FLAG==1) {                  
monitor.c:    } else if (*RUN_FLAG==2 || *RUN_FLAG==4) {            //-- pedestals and THR calculated , load  and save 
monitor.c:      if (*RUN_FLAG==2) {
monitor.c:          if (*RUN_FLAG==2 && (DAC_250->DAC[bin]<100  || DAC_250->THR[bin]>120) ) 
monitor.c:          if (*RUN_FLAG==2 && ( DAC_125->DAC[bin]<100 || DAC_125->THR[bin]>255) ) 
monitor.c:      if (*RUN_FLAG==4) *RUN_FLAG=15;   //---- no reset hist, and continue, last run ---
monitor.c:      if (*RUN_FLAG==2) {
monitor.c:      //---  end (RUN_FLAG==2 || RUN_FLAG==4)
monitor.c:    }  else if (*RUN_FLAG==5) {  //--- calibration run for THR


----------------------------------------------------------------------------------------------------- */
