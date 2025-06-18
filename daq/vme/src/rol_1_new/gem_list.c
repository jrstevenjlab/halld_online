/*
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ FADC125  689 Ev=290000
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 READ SRS dCnt= 3337 nframes= 13  
 srsReadBlock(0): timeout
**************************************************
No SRS data or error. ifec=0   dCnt = 0
**************************************************
srsReceiveData: ERROR: Timeout (-1)
srsSlowControl(10.0.3.2:6007): Failed to receive data
srsStatus: ERROR reading SYS port
srsReadBlock(0): timeout
**************************************************
No SRS data or error. ifec=0   dCnt = 0
**************************************************
srsReceiveData: ERROR: Timeout (-1)
srsSlowControl(10.0.3.2:6007): Failed to receive data
srsStatus: ERROR reading SYS port
srsReadBlock(0): timeout
*************************************************
ping 10.0.3.2
PING 10.0.3.2 (10.0.3.2) 56(84) bytes of data.
From 10.0.3.1 icmp_seq=2 Destination Host Unreachable
From 10.0.3.1 icmp_seq=3 Destination Host Unreachable
From 10.0.3.1 icmp_seq=4 Destination Host Unreachable

-------------------------------------------------------------------------------
                              System Summary

 Firmware Version      0xa078
 FEC IP                10.0.3.2
 DAQ IP:PORT           10.0.3.1:7000
 Clock Selection       0x00
   Status              0x00000011
   Main Clock Source   DTC
 Hardware FW Version   0x0000

 DTC Config
   Data over ETH       Enabled	   Flow Control        Enabled
   Padding Type        2	   Data Channel TrgID  Disabled
   Data Frame TrgID    Disabled	   Trailer word count  4
   Padding Byte        0xaa	   Trailer Byte        0xdd
--------------------------------------------------------------------------------
                        APV Application Summary

 APV Trigger Control
   Mode                4	   TrgBurst            6
   Trigger Seq Period  4	   Trigger Delay       2
   Test Pulse Delay    127	   RO Sync             112
   Status              0x03fffe

 Event Build
   Ch Enable Mask      0x0fff	   Capture Window      3500
   Mode                2	   Data Format         0
   Event Info Data     0xaa000bb8

 Readout is Disabled (0x0)
--------------------------------------------------------------------------------

*/

//#define USE_SRS


#define DO_SRS_READOUT  1

#ifdef USE_SRS

#include "../include/srsLib.h"
//#include <string.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>

int srsFD[MAX_FEC];
char FEC[MAX_FEC][100];
int nfec=1;
int roc_id=76;

static unsigned int tdcbuf[65536];


#endif

int gem_download() {

#ifdef USE_SRS

  if(strncmp(roc_name, "roctrd1", 7) || use_gem==0 ) return 0;
  

  printf("gem_download(): Executed\n");

  /*************************************************************
   * Setup SRS 
   */

  nfec=0;

  /* testing before run
  strncpy(FEC[nfec++], "10.0.0.2",100);
  strncpy(FEC[nfec++], "10.0.3.2",100);
  strncpy(FEC[nfec++], "10.0.8.2",100);
  */

  /* from Bryan on May 13, 2016 */


  if (roc_id == ROCID) /* roctrd1 */
  {
    strncpy(FEC[nfec++], "10.0.3.2",100);
  }
  else
  {
    printf("ERROR1: SRS HAS WRONG PID=%d\n",roc_id);
    exit(0);
  }

  char hosts[MAX_FEC][100];
  int ifec=0;
  int starting_port = 7000;

  memset(&srsFD, 0, sizeof(srsFD));

  /* Associate FECs with Specific Host IPs and ports */

  if (roc_id == ROCID) /* roctrd1 */
  {
    for(ifec=0; ifec<nfec; ifec++)
    {
      printf("SRS:: Associate FECs with Specific Host IPs and ports ifec=%d nfec=%d port=%d starting_port=%d \n",ifec,nfec,ifec+2,starting_port+ifec);
      sprintf(hosts[ifec],"10.0.3.%d",1);
      srsSetDAQIP(FEC[ifec], hosts[ifec], starting_port+ifec);


      printf("B: ifec=%d daq_hosts=%s srsFD=%d srs_host=%s\n",ifec,hosts[ifec],srsFD[ifec],FEC[ifec]);

      srsConnect((int*)&srsFD[ifec], hosts[ifec], starting_port+ifec);

      printf("A: ifec=%d hosts=%s srsFD=%d \n",ifec,hosts[ifec],srsFD[ifec]);

    }
  }
  else
  {
    printf("ERROR2: SRS HAS WRONG PID=%d\n",roc_id);
    exit(0);
  }


  /* Configure FEC */
  for(ifec=0; ifec<nfec; ifec++)
  {

    printf("SRS configure ifec = %d roc_id=%d\n",ifec,roc_id);
    usleep(100000);
    /* Same as call to 
    srsExecConfigFile("config/set_IP10012.txt"); */
    srsSetDTCC(FEC[ifec], 
		 1, // int dataOverEth
		 0, // int noFlowCtrl 
		 2, // int paddingType
		 0, // int trgIDEnable
		 0, // int trgIDAll
		 4, // int trailerCnt
		 0xaa, // int paddingByte
		 0xdd  // int trailerByte
		 );

    /* Same as call to 
	srsExecConfigFile("config/adc_IP10012.txt"); */
    usleep(100000);
    srsConfigADC(FEC[ifec],
		   0xffff, // int reset_mask
		   0, // int ch0_down_mask
		   0, // int ch1_down_mask
		   0, // int eq_level0_mask
		   0, // int eq_level1_mask
		   0, // int trgout_enable_mask
		   0xffff // int bclk_enable_mask
		   );

    /* Same as call to 
	srsExecConfigFile("config/fecCalPulse_IP10012.txt"); */
    usleep(100000);
    srsSetApvTriggerControl(FEC[ifec],
			    4+0, // int mode  // 4+0 , 3+2 fsv
			    2, // int trgburst (x+1)*3 time bins  // 8 // 6 // 4 // 2 // 1 // 0
			    0x4, // int freq
			    // 0x6C, // int trgdelay 61(tage total sum) 5f(scintillator trigger) 60(MASTER OR)  
			    //0x36 , /// DEC=54, // int trgdelay for 21 samples and PStrig lat=1.84 us
			    //0x26 , /// DEC=38, // int trgdelay for 6 samples and PStrig lat=1.84 us;    tiSetTSInputDelay(2, 160); 
			    0x9, //0x2 , /// DEC=4, // int trgdelay for 6 samples and PStrig lat=1.84 us;       tiSetTSInputDelay(2, 370); 
			    //0x40 , /// DEC=64, // int trgdelay for 6 samples and PStrig lat=1.84 us
			    //0x36 , ///    // int trgdelay for 27 samples and PStrig lat=1.84 us and  tiSetTSInputDelay(2, 160); 
			    //^^^^ LATENCY !!! 0123456789ABCDEF
			    0x7f, // int tpdelay 
			    0x70 // i22nt rosync  // before 9f / 0x80 ! fsv //-- left window position
			    );
    usleep(100000);

    if(roc_id == ROCID)  
      {
	srsSetEventBuild(FEC[ifec],
			 0x0f00, // int chEnable // 0xffff=16APV(4GEMs) 0xfff=12apv(3GEMs) 0xff=8apv(2GEMs)
			         // 0x3000/GEMTRD-APV ;  0x0f00/GEM2 ;  0x00f0/GEM1 ; 0x000f/uWell ;
			 1500, // int dataLength  // 4000 // 3500  // 2000 // 1500  // 550 // 250
			 2, // int mode
			 0, // int eventInfoType
			 0xaa000bb8 | ((ifec)<<16) //  ((ifec+4)<<16) !! fsv unsigned int eventInfoData !! 
			 );
      }
    else /* roc_id == 7 */
      {
	srsSetEventBuild(FEC[ifec],
			 0x1ff, // int chEnable
			 550, // int dataLength
			 2, // int mode
			 0, // int eventInfoType
			 0xaa000bb8 | ((ifec)<<16) // unsigned int eventInfoData
			 );
      }
	
    /* Same as call to 
	srsExecConfigFile("config/apv_IP10012.txt"); */
    usleep(100000);
    srsAPVConfig(FEC[ifec], 
		   0xffff, // int channel_mask, 
		   0x03, // int device_mask,
		   0x19, // int mode, 
		   0x80, // int latency, 
		   0x02, // int mux_gain, 
		   0x62, // int ipre, 
		   0x34, // int ipcasc, 
		   0x22, // int ipsf, 
		   0x22, // int isha, 
		   0x22, // int issf, 
		   0x37, // int ipsp, 
		   0x28, // int imuxin, 0x10,  ! fsv
		   0x64, // int ical, 
		   0x28, // int vsps,
		   0x3c, // int vfs, 
		   0x1e, // int vfp, 
		   0xef, // int cdrv, 
		   0xf7 // int csel
		   );

    /* Same as call to 
	srsExecConfigFile("/daqfs/home/moffit/work/SRS/test/config/fecAPVreset_IP10012.txt"); */
     usleep(100000);
     srsAPVReset(FEC[ifec]);

    /* Same as call to 
	srsExecConfigFile("config/pll_IP10012.txt"); */
     usleep(100000);
     srsPLLConfig(FEC[ifec], 
		   0xffff, // int channel_mask,
		   0x00, // int fine_delay, 0x0 ! fsv
		   0 // int trg_delay
		   );
      
  }

  for(ifec=0; ifec<nfec; ifec++)
  {
    srsStatus(FEC[ifec],0);
  }
  
#endif


} //--- download ----



int gem_prestart() {

#ifdef USE_SRS

  if(strncmp(roc_name, "roctrd1", 7) || use_gem==0 ) return 0;
  printf("gem_prestart(): Enter, nfec=%d \n",nfec);

  // trigger output from the TI is through OUTPUT: OT#2.  
  /* (arg + 2) * 4ns.  arg = 13 -> width = 60ns; 26=110ns*/  
  tiSetPromptTriggerWidth(26); 


 int ifec=0, nframes=0, dCnt=0;
 for(ifec=0; ifec<nfec; ifec++)
   {
     srsStatus(FEC[ifec],0);
   }
 
 /* Check SRS buffers and clear them */
 dCnt = srsCheckAndClearBuffers((int *)&srsFD, nfec,
				(volatile unsigned int *)tdcbuf,
				99000, 1, &nframes);
 if(dCnt>0)
    {
      printf("SYNC ERROR: SRS had extra data at SyncEvent.\n");
    }
 
#endif /* USE_SRS */

} //--- prestart ---



int gem_end() {

  int ifec=0;


#ifdef USE_SRS

  if(strncmp(roc_name, "roctrd1", 7) || use_gem==0 ) return 0;
  printf("gem_end(): Executed\n");


  for(ifec=0; ifec<nfec; ifec++)
  {
    srsTrigDisable(FEC[ifec]);
    srsStatus(FEC[ifec],0);
  }
#endif



} //--- end ----


int gem_go() {


#ifdef USE_SRS

  if(strncmp(roc_name, "roctrd1", 7) || use_gem==0 ) return 0;
  printf("gem_go(): Execute nfec=%d\n",nfec);

  int ifec=0;

  for(ifec=0; ifec<nfec; ifec++)
  {
    printf("gem_go(): Trig enabled for nfec=%d\n",nfec);
    srsTrigEnable(FEC[ifec]);
    srsStatus(FEC[ifec],0);
  }

#endif



}


int gem_trigger() {

  int dCnt;

#ifdef USE_SRS
  if(strncmp(roc_name, "roctrd1", 7) == 0 && use_gem==1 ) {

    if (CALIBRATION_FLAG>0) {  usleep(1000000); printf("CALIBRATION_FLAG=%d\n",CALIBRATION_FLAG); }
    //printf("enter SRS readout trigger");
    dCnt=0;
    /************************************************************
     * GEM / SRS READOUT
     */
    int ifec=0;
    int nframes=0;
    nfec=1;
    for(ifec=0; ifec<nfec; ifec++)  {
      dCnt=0;
      BANKOPEN(17,BT_UI4,blockLevel);
      //usleep(250);      
      dCnt = srsReadBlock(srsFD[ifec],(volatile UINT32 *)dma_dabufp,99000,blockLevel, &nframes); //--  399620 Byte max ?
      if(dCnt<=0)    {
        printf("**************************************************\n");
        printf("No SRS data or error. ifec=%d   dCnt = %d\n",ifec,dCnt);
        printf("**************************************************\n");
        srsStatus(FEC[ifec],0);
	daLogMsg("ERROR",     "GEM SRS Error \n"); 
	ROL_SET_ERROR;
      }  else  {
        /* Bump data buffer by the amount of data received */
        dma_dabufp += dCnt;
	int tic=tiGetIntCount();
        if(tic<1000 || tic % 1000 == 0)
          printf(" READ SRS dCnt= %d nframes= %d  \n",dCnt,nframes);
	if (CALIBRATION_FLAG>0) {  usleep(1000000); printf("CALIBRATION_FLAG=%d dCnt= %d nframes= %d \n",CALIBRATION_FLAG,dCnt,nframes); }

//#define CHECKFRAMES
#ifdef CHECKFRAMES
        if(nframes!=10)    {
          int idata=0, len = dCnt;
          unsigned int *SRSdata = dma_dabufp; // rol->dabufp;
          int icounter=0;
          int samples=0;
          unsigned int tmpData=0;
                   
          for(idata=0;idata<(len);idata++) {
            if ((SRSdata[idata]&0xffffff)==0x434441)  icounter=0;
            if((icounter%8)==0)               printf("\n%4d\t",icounter);
            if(icounter==1)                 samples = (LSWAP(SRSdata[idata])) & 0xffff;
            if (((icounter-2)*2)==samples)  {
              icounter=0;
              printf("\n");
            }
            if(icounter<2)
              printf("  0x%08x ",(unsigned int)LSWAP(SRSdata[idata]));
            else
              {
                tmpData = (unsigned int)LSWAP(SRSdata[idata]);
                printf("   %04x %04x ",
                       (tmpData&0xFF000000)>>24 | (tmpData&0x00FF0000)>>8, 
                       (tmpData&0x0000FF00)>>8 | (tmpData&0x000000FF)<<8
                       );
              }
            icounter++;
          }
          // printf("paused\n");  getchar(); 
        }
#endif
      }
      BANKCLOSE;
    } // - ifec loop
  } //-- if use_gem
#endif
  //---- end GEM/SRS readout

}
