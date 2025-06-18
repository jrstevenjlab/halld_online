// stty -tostop  // debugger gdb
// readelf --debug-dump=info Linux_CentOS5-i686-gcc4.1.2/dirc_srv.exe | grep -A2 'Compilation Unit @'

int Debug=0; //-- from shmem --
int sig_int, sig_hup;

int sspGSendScalers();

//#define Linux_vme


#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#if defined(Linux_vme) || defined(Linux_armv7l)

//#include "code.s/CrateMsgTypes.h"
//#include "ipc.h"

//#include "shmem_roc.h"

//#ifdef Linux_vme
#include "jvme.h"
#include "dsc2Lib.h"
#include "fadcLib.h"
//#include "vscmLib.h"
#include "sspLib.h"
//#endif


#define DW_SWAP(v)                                              ( ((v>>24) & 0x000000FF) |\
                                                                                  ((v<<24) & 0xFF000000) |\
                                                                                  ((v>>8)  & 0x0000FF00) |\
                                                                                  ((v<<8)  & 0x00FF0000) )

#define HW_SWAP(v)                                              ( ((v>>8) & 0x00FF) |\
                                                                                  ((v<<8) & 0xFF00) )
/*
#define LSWAP(v)                                                ( ((v>>24) & 0x000000FF) |\
                                                              ((v<<24) & 0xFF000000) |\
                                                              ((v>>8)  & 0x0000FF00) |\
                                                              ((v<<8)  & 0x00FF0000) )

*/
#define HSWAP(v)                                                ( ((v>>8) & 0x00FF) |\
                                                              ((v<<8) & 0xFF00) )



#define SCALER_THREAD


//#ifdef SCALER_THREAD

#define SCALER_TYPE_SSP 4

#define SCALER_LOCK   pthread_mutex_lock(&vmescalers_lock)
#define SCALER_UNLOCK pthread_mutex_unlock(&vmescalers_lock)

#define DIST_ADDR  0xEA00	  /*  base address of FADC signal distribution board  (A16)  */

#define MAXBOARDS  DIRC_MAX_SLOT // 21   /* max number od boards per crate */
#define MAXWORDS   DIRC_MAX_CHAN  // /*256*//*4096*/8192   /* max number of scaler words per board */

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

static int vmeScalersReadInterval = 4;

static pthread_mutex_t vmescalers_lock;


static int  nssp, ntd, rflag, rmode;
static int mssp=0, mvscm;

static unsigned int  vmescalersmap[MAXBOARDS+1];  /* crate map */

static unsigned int  vmescalerslen[MAXBOARDS];  /*scalers space (the number of words) */
static unsigned int *vmescalers[MAXBOARDS];     /*scalers memory space address*/

static unsigned int  vmedatalen[MAXBOARDS];  /*data space (the number of words) */
static unsigned int *vmedata[MAXBOARDS];     /*data memory space address*/


static unsigned int sspbuf[MAXWORDS];


static int init_boards;


//================================================================
/* parameter 'time' in seconds */
static int vmeSetScalersReadInterval(int time) {
  if(time < 0)       vmeScalersReadInterval = 0; /* 0 means no reads */
  else if(time > 10) vmeScalersReadInterval = 10;
  else               vmeScalersReadInterval = time;

  printf("vmeSetScalersReadInterval: vmeScalersReadInterval set to %d\n",vmeScalersReadInterval);

  return(vmeScalersReadInterval);
}
//================================================================
static int  vmeGetScalersReadInterval()  {
  return(vmeScalersReadInterval);
}

static int ScalersReadoutStart();
static int ScalersReadoutStop();

//#ifdef Linux_vme /* VME section */
//=======================================================================================
static int vmeScalersRead(roc_shmem *shmem_ptr) {
  int itype, id, ii, nw, nw_len, slot, fiber;
  unsigned int chmask = 0xFFFF;
  static long long  tm0, tm1, tm2;
  struct timeval tv0,tv1;
  
  printf("==========================================================> vmeScalersRead reached nssp=%d \n",nssp);
  
  SCALER_LOCK;
  
  /*for(id=0; id<nssp; id++)*/
  if(nssp>0)    {
    for(id=mssp; id<=mssp; id++) {
      unsigned int fibermask;
      slot = sspSlot(id);
      if (Debug>1) printf("slot=%d\n",slot);
      
      if(sspGetFirmwareType(slot) == SSP_CFG_SSPTYPE_HALLBRICH)  {
	vmeBusLock();
	sspRich_ScanFibers_NoInit(slot);
	vmeBusUnlock();
	
	sspRich_GetConnectedFibers(slot, (int*) &fibermask);
	if (Debug>1) printf("fibermask=0x%08x\n",fibermask);
	/* loop over fibers */
	nw = 0;
	nw_len = nw;
	sspbuf[nw++] = 0; /*reserve space for length*/
	for(fiber=0; fiber<32; fiber++)  {
	  unsigned int ref;
	  unsigned int maroc[RICH_CHAN_NUM];
	  
	  if(fibermask & (1<<fiber)) {
	    /*printf("befor lock\n");fflush(stdout);*/
	    vmeBusLock();
	    /*printf("in lock\n");fflush(stdout);*/
	    sspRich_ReadScalers(slot, fiber, &ref, maroc);
	    vmeBusUnlock();
	    //-----------------------------------------------------------
	    //                 rate normalization
	    //-----------------------------------------------------------
	    double maroc_hz[RICH_CHAN_NUM], chip_hz[3], scale;
	    unsigned int chip[3];
	    chip[0] = 0;       chip[1] = 0;       chip[2] = 0;
	    chip_hz[0] = 0.0;  chip_hz[1] = 0.0;  chip_hz[2] = 0.0;
	    if(ref>0) {
	      scale = 125000000.0 / ref;
	      if (Debug>1) printf("sspRich_ReadScalers:: reference scaler = %f \n",scale);
	    }  else   {
		printf("sspRich_ReadScalers:: ERROR - reference scaler invalid\n");
		scale = 1.0;
	    }
	    printf("------>  Read slot=%d fiber=%d ",slot,fiber);
	    int i; for(i=0;i<RICH_CHAN_NUM;i++)
	      {
		maroc_hz[i] = (float)maroc[i]*scale;
		chip_hz[i/64] += maroc_hz[i];
		chip[i/64] += maroc[i];
		printf(" | %d %.1f %.1f ",maroc[i],scale,(float)maroc[i]*scale);
	      }
	    printf("-----------------------------------------------------------------\n");
	    if (Debug>1) 
	      sspRich_PrintScalers(slot, fiber);
	    //------------------------------------------------------------

	    /*printf("after lock\n");fflush(stdout);*/
	    if (Debug>0) sspRich_PrintMonitor(slot, fiber);
	    
	    if (Debug>1) { 
	      printf("got fiber %d, nw=%d (%d)\n",fiber,nw,nw/(RICH_CHAN_NUM+3));
	      
	      printf("\n-------------slot = %d ----------------\n",slot);  int ii;  
	      for (ii=0; ii<RICH_CHAN_NUM; ii++) { 
		if (Debug>1) printf(" %d %f | ",maroc[ii],maroc_hz[ii]); 
	      }
	      printf("\n-----------------------------\n");
	    }
	    sspbuf[nw++] = fiber;
	    sspbuf[nw++] = ref;
	    
	    for(i=0;i<RICH_CHAN_NUM;i++) sspbuf[nw++]=maroc[i];
	    //memcpy(&sspbuf[nw], maroc, RICH_CHAN_NUM*sizeof(int)); nw += RICH_CHAN_NUM;

	  } //-- fiber mask check
	} //-- fibers loop --
	sspbuf[nw_len] = nw - nw_len; /*inclusive length in words*/
      } //-- firmware check --
      
      vmescalerslen[slot] = nw;
      if (DIRC_MAX_SCALER_FIBER < nw && nw < DIRC_MAX_CHAN)  for(ii=0; ii<nw; ii++) vmescalers[slot][ii] = sspbuf[ii];
      if (Debug>0) printf("vmeScalersRead: nw=%d, vmescalers[slot][nw-2]=%d, vmescalers[slot][nw-1]=%d\n",nw,vmescalers[slot][nw-2],vmescalers[slot][nw-1]);
    } //-- slots loop --
    
    
    /* set board id to be read on next itteration */
    mssp++;
    if(mssp>=nssp) mssp = 0;
  }
  
  SCALER_UNLOCK;
  
  return(0);
}


//=============================================================================
static int  ScalersReadoutStop()  {
  /* TODO: END TASK HERE !!!!!!!!!!!!!!!!!!!!!!!! */

  free(vmescalers);
  free(vmedata);

  pthread_mutex_unlock(&vmescalers_lock);
  pthread_mutex_destroy(&vmescalers_lock);
  return(0);
}
//#endif 
//-- end VME
//===============================================================================================================
//                              S C A N   
//===============================================================================================================

//=================================================================
int  DIRC_INIT(void)  {
  int id, iFlag;
  int ii, jj, slot, interval;
  
  // fsv daqConfig("");
  interval = 5; // fsv daqGetExternalVmeReadoutInterval();
  vmeSetScalersReadInterval(interval);
  interval = 5; //vmeGetScalersReadInterval();
  printf("Set scalers readout interval to %d seconds\n\n",interval);
  
  for(ii=0; ii<MAXBOARDS; ii++)   {
    if( (vmescalers[ii] = (unsigned int *) calloc(MAXWORDS,sizeof(int))) <= 0)  {
      printf("ERROR in ScalerThread: cannot allocate memory for vmescalers[]\n");
      return 1;
    }
    printf("ScalerThread: Allocated vmescalers[]: 0x%08x words for slot %d at address 0x%p\n",MAXWORDS,ii,vmescalers[ii]);
    
    if( (vmedata[ii] = (unsigned int *) calloc(MAXWORDS,sizeof(int))) <= 0) {
      printf("ERROR in ScalerThread: cannot allocate memory for vmedata[]\n");
      return 1;
    }
    printf("ScalerThread: Allocated vmedata[]: 0x%08x words for slot %d at address 0x%p\n",MAXWORDS,ii,vmedata[ii]);
  }
  
  for(ii=0; ii<(MAXBOARDS+1); ii++) {
    vmescalersmap[ii] = -1;
  }
  /* dma */
  //usrVmeDmaSetConfig(2,3,0); /*A32,MBLT*/
  // vmeDmaConfig(2,3,0);
  // printf("!!!!!!!!! set DMA as A32,MBLT\n");

 nssp = sspGetNssp();


  /* fill map array with FADC's found */
  for(ii=0; ii<nssp; ii++) if( (slot=sspSlot(ii)) > 0) vmescalersmap[slot] = SCALER_TYPE_SSP;

  printf("Finished  initialization, nssp=%d\n",nssp);
  return 0;
}



//=============================================================================
//                 M A I N  
//=============================================================================
void sig_handler(int signo);
int dirc_scan()  {
  int stat;
 char DircName[256];
 char CalibPath[256];

  if(signal(SIGINT, sig_handler) == SIG_ERR)
  {
	perror("signal");
	exit(0);
  }

  //#ifdef Linux_vme

  if (DIRC_INIT()) { printf("Error DIRC_INIT() \n"); return 1; }
  printf("done.\n");


  sspRich_PrintConnectedAsic_All();
 
  //------------------------------------------------------------------
  if (!strncmp(getenv("USER"),"hdops",5)) { //--- this is hdops 
    sprintf(CalibPath,"/gluex/CALIB/ALL/DIRC");
  } else {  //-- any other user 
    sprintf(CalibPath,"%s/ROOT_HIST",getenv("HOME"));
  }
  sprintf(DircName, "%s/dirc_thr_%d.dat",CalibPath,rol->runNumber);
  printf("Open file %s \n",DircName);
  //-------------------------------------------------------------------

 sleep(5);


  FILE *fp = fopen(DircName, "w");
  int slot,fiber,asic,thr,id;
  

  for(id=0; id<nssp; id++) {  //----  start from slots 4(1) or 3(0) !!!  ----------------- loop over slots  ---------------
    
    mssp=id;
    slot = sspSlot(mssp); 

    if (Debug>1) printf("slot=%d\n",slot);

    for (thr=170; thr<220; thr+=1) {  //-- 0 ... 1023        <-------------------------- THR -----------------------
    
    //  if(nssp>0)    {


	unsigned int fibermask;

#if 0
	sspRich_SetAllThreshold(slot,thr);
#else 
	if(sspGetFirmwareType(slot) == SSP_CFG_SSPTYPE_HALLBRICH)  {
	  vmeBusLock();
	  sspRich_ScanFibers_NoInit(slot);
	  vmeBusUnlock();
	  
	  sspRich_GetConnectedFibers(slot, (int*) &fibermask);
	  if (Debug>1) printf("fibermask=0x%08x\n",fibermask);
	  /* loop over fibers */
	  for(fiber=0; fiber<32; fiber++)  {
	    unsigned int ref;
	    unsigned int maroc[RICH_CHAN_NUM];
	    
	    //printf("----------------------------------------------------------->  set: slot=%d \n",slot);
	    if(fibermask & (1<<fiber)) {
	      /*printf("befor lock\n");fflush(stdout);*/
	      for (asic=0; asic<2; asic++) { 
		//printf("next:  slot=%d, fiber=%d, asic=%d, thr = %d ",slot,fiber,asic,thr);
		sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_DAC0, 0, thr);
	      } //-- asic loop
	      sspRich_UpdateMarocRegs(slot, fiber);
	    } //-- if active
	  } //-- fiber loop
	}
#endif

	//} //-- slot loop
	//} else { printf("No SSP found \n"); return 1; };
	
	
	//==================================================================
	//  readout one slot 
	//==================================================================
	
	printf("SET THR = %d  slot = %d \n",thr, slot);

	sleep(5); //=====  !!!!  sleep !!!  ====

	printf("vmeReadTask: reading scalers ...\n");
	vmeScalersRead(shmem_ptr);    mssp=id;

	printf("vmeReadTask: ... scalers read\n");
	//---------------------------------------------------------------------
	//vmescalerslen[slot] = nw;
	//if (DIRC_MAX_SCALER_FIBER < nw && nw < DIRC_MAX_CHAN)  for(ii=0; ii<nw; ii++) vmescalers[slot][ii] = sspbuf[ii];
	
	int nw=vmescalerslen[slot];
	printf("Read slot=%d: nw=%d \n",slot,nw);
	int ii;
	for(ii=0; ii<nw; ii++) { 
	  if (slot==4) printf(" %d ",vmescalers[slot][ii]);
	}
	//-------------------------------------------------------------------
	double maroc_hz[RICH_CHAN_NUM], chip_hz[3], scale;
	unsigned int chip_sc[3];
	//-------------------------------------------------------------------
	ii=0;
	int len=vmescalers[slot][ii++];
	if (nw!=len) printf("************>>  ERROR   nw=%d != len=%d <<*****************\n",nw,len);
	while(ii<len) {
	  int  i,fiber, ref, chip, chip_old=-1;
	  fiber=vmescalers[slot][ii++];
	  ref=vmescalers[slot][ii++];
	  
	  if(ref>0) {
	    scale = 125000000.0 / ref;
	    //printf("sspRich_ReadScalers:: reference scaler = %f \n",scale);
	  }  else   {
	    printf("sspRich_ReadScalers:: ERROR - reference scaler invalid\n");
	    scale = 1.0;
	  }
	  
	  chip_sc[0] = 0;    chip_sc[1] = 0;    chip_sc[2] = 0;
	  chip_hz[0] = 0.0;  chip_hz[1] = 0.0;  chip_hz[2] = 0.0;
	  for(chip=0;chip<3;chip++) {
	    fprintf(fp," %4d  %d  %2d  %d  ",thr,slot,fiber,chip);
	    printf(" %4d  %d  %2d  %d  ",thr,slot,fiber,chip);	  
	    for(i=0;i<64;i++) {
	      maroc_hz[chip*64+i] = (float)vmescalers[slot][ii++]*scale;
	      fprintf(fp,"%.1f ",maroc_hz[chip*64+i]);
	      if (i<10) printf("%.1f ",maroc_hz[chip*64+i]);
	      chip_hz[chip] += maroc_hz[chip*64+i];
	    } //-- chan loop 
	    fprintf(fp,"    %.1f \n",chip_hz[chip]/64.);
	    printf("    %.1f \n",chip_hz[chip]/64.);
	    //printf("    %.1f ",chip_hz[chip]);
	  } //-- chip loop
	  
	  //-------------------------------------------------------------------
	} //-- while (ii) loop 
	

	//=======================================
	
    }//-- THR loop --
      // } else { printf("No SSP found \n"); return 1; };
    
    //=======================================
    
  } //-- slot loop
  
  if (fp) fclose(fp); fp=NULL; 	 //-- close dirc scan threshold file 
  
  sleep(2);
  
  // while(1) sleep(1);

	
CLOSE:

  //#ifdef Linux_vme
  //vmeCloseDefaultWindows();
  //#endif
	
  return 0;
}
//==========================  E N D   M A I N   ================================
//#endif
void closeup() {
#ifdef Linux_vme
  //  vmeCloseDefaultWindows();
#endif
  printf("DiagGUI server closed...\n");
}

//======================================
void sig_handler(int signo) {
	printf("%s: signo = %d\n",__FUNCTION__,signo);
	switch(signo)
	{
		case SIGINT:
			closeup();
			exit(1);  /* exit if CRTL/C is issued */
	}
}
//=============================================================================
//                 T H E   E N D
//=============================================================================
