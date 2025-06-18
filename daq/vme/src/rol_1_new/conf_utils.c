#include <stdio.h>
#include <assert.h>
#include <string.h>
//#include <unistd.h>
#include <stdlib.h>

#include <stdint.h>

//#include "conf_utils.h"

#include "fadcLib.h"
#include <fa125Lib.h>
#include "tsLib.h"

#include "dsc2Lib.h"
// ALEX
#include "sspLib.h"

extern pthread_mutex_t  faMutex;
#define FALOCK      if(pthread_mutex_lock(&faMutex)<0) perror("pthread_mutex_lock");
#define FAUNLOCK    if(pthread_mutex_unlock(&faMutex)<0) perror("pthread_mutex_unlock");


extern pthread_mutex_t   fa125Mutex ;//= PTHREAD_MUTEX_INITIALIZER;
#define FA125LOCK     if(pthread_mutex_lock(&fa125Mutex)<0) perror("pthread_mutex_lock");
#define FA125UNLOCK   if(pthread_mutex_unlock(&fa125Mutex)<0) perror("pthread_mutex_unlock");


extern pthread_mutex_t   dscMutex = PTHREAD_MUTEX_INITIALIZER;
#define DSCLOCK   if(pthread_mutex_lock(&dscMutex)<0) perror("pthread_mutex_lock");
#define DSCUNLOCK if(pthread_mutex_unlock(&dscMutex)<0) perror("pthread_mutex_unlock");



volatile struct fadc_struct *FAp[(FA_MAX_BOARDS+1)]; /* pointers to FADC memory map */
volatile struct fa125_a24 *fa125p[(FA_MAX_BOARDS+1)]; /* pointers to FA125 memory map */

extern  int fadcID[FA_MAX_BOARDS];



#if 0
int main(){

  //   char* roc_name = "roccdc2";
  //   char* filename     =  "/home/somov/conf/conf_examp.cnf";

  int rc;

  //  strcpy(filename, "/home/somov/conf/conf_examp.cnf");
  strcpy(filename, "/home/somov/conf/tagm_pulser.cnf");
  strcpy(roc_name, "rocps2");


  memset(fa250,     0, sizeof(fa250));
  memset(fa125,     0, sizeof(fa125));
  memset(&ti_bd,    0, sizeof(ti_bd));
  memset(&dsc,      0, sizeof(dsc));
  memset(&caen1290, 0, sizeof(caen1290));
  memset(&f1,       0, sizeof(f1));
  memset(&ctp,      0, sizeof(ctp));
  memset(&ssp,      0, sizeof(ssp));
  memset(&gtp_eq,   0, sizeof(gtp_eq));
  memset(&gtp_trig, 0, sizeof(gtp_trig));
  memser(&gtp_gain, 0, sizeof(gtp_gain));

  rc = read_conf_global(filename,roc_name);

  if(rc !=  0) return 0; 

  rc = read_conf_common(roc_name);

  if(rc == 0)
    read_conf_user(roc_name);

  print_ts_conf();
  print_ssp_conf();
  print_gtp_conf();

  //  print_fadc250_conf(3);
  //  print_fadc250_conf(4);
  //  print_fadc250_conf(5);
  //  print_fadc250_conf(6);
  //  print_fadc250_conf(7);


  //  print_fadc250_conf(4);

  //  print_ti_conf();

  //  printf(" Board type for crate %s  = %s \n", roc_name, get_board_type_name("rocbcal8"));


  //  FADC250_CONF fa250_test;
  //  fa250_test = get_conf_fadc250(3);
  //  printf(" get_conf_fadc250: group = %d,  Serial number %s \n", fa250_test.group, fa250_test.SerNum);

  
  //  FADC125_CONF fa125_test;
  //  fa125_test = get_conf_fadc125(3);
  //  printf(" get_conf_fadc125: group = %d,  Serial number %s \n", fa125_test.group, fa125_test.SerNum);


  //  print_fadc125_conf(3);
  //  print_fadc125_conf(4);

  //  print_ts_conf();

  //  print_ti_conf();

  //  print_le_conf();

  //  print_caen1290_conf();

  //  print_f1_conf();


  return 0;

}
#endif


FADC250_CONF get_conf_fadc250(int slot){

  FADC250_CONF fa250_tmp;

  fa250_tmp = fa250[slot];
  
  return fa250_tmp;
 
}


FADC125_CONF get_conf_fadc125(int slot){

  FADC125_CONF fa125_tmp;

  fa125_tmp = fa125[slot];
  
  return fa125_tmp;
 

}


DSC2_CONF get_conf_dsc2(int slot){

  DSC2_CONF dsc2_tmp;

  dsc2_tmp = dsc[slot];
  
  return dsc2_tmp;

}





det_type get_det_type(char* crate){

  det_type det;

  if(strncmp(crate, "rocecal", 7) == 0)        
    det = ecal;

  if(strncmp(crate, "rocfcal", 7) == 0)        
    det = fcal;

  if(strcmp(crate, "rocccal") == 0)
    det = ccal;

  if(strncmp(crate, "rocbcal", 7) == 0)        
    det = bcal;

  if(strncmp(crate, "roccdc",  6) == 0)        
    det = cdc;

  if(strncmp(crate, "rocfdc",  6) == 0)        
    det = fdc;

  if(strncmp(crate, "roctof",  6) == 0)        
    det = tof;

  if(strcmp(crate, "rocps1") == 0)
    det = ps;

  if(strcmp(crate, "rocps2") == 0)
    det = psc;

  if(strcmp(crate, "rocst")  == 0)        
    det = st;

  if(strncmp(crate, "rocstpsc", 8) == 0)   // Joint crates with LEs and F1TDCs belong to the ST
    det = st;

  if(strncmp(crate, "roctagm", 7) == 0)    // Joint crate with F1 tdc belongs to the microscope
    det = tagm;

  if(strncmp(crate, "roctagh", 7) == 0)        
    det = tagh;

  if(strncmp(crate, "roctrig", 7) == 0)        
    det = trigger;

  if(strcmp(crate, "rocdirc") == 0)        
    det = dirc;

  if(strcmp(crate, "rocfmwpc1") == 0)        
    det = muon;

  if(strcmp(crate, "roctrd1") == 0)        
    det = gem;

  return det;      
}



bd_type get_board_type(char* crate){ 

  bd_type board;

  /*   ECAL */ 
  if(strncmp(crate, "rocecal", 7) == 0)      
    board = fadc250;

  /*   FCAL */ 
  if(strncmp(crate, "rocfcal", 7) == 0)      
    board = fadc250;
  
  /*   CCAL */ 
  if(strcmp(crate, "rocccal") == 0)      
    board = fadc250;

  /*   BCAL */
  if( strcmp(crate, "rocbcal3") == 0 || strcmp(crate, "rocbcal6") == 0 || 
      strcmp(crate, "rocbcal9") == 0 || strcmp(crate, "rocbcal12") == 0 )
    board = f1tdc;
  
  else if( strcmp(crate, "rocbcal13") == 0 || strcmp(crate, "rocbcal14") == 0  ||
	   strcmp(crate, "rocbcal15") == 0 || strcmp(crate, "rocbcal16") == 0 )
    board = dsc2;

  else if(strncmp(crate, "rocbcal", 7) == 0 ) 
    board = fadc250;
    

  /*   CDC  */
  if(strncmp(crate, "roccdc", 6) == 0)      
    board = fadc125;


  /*   FDC  */
  if( strcmp(crate, "rocfdc1")  == 0  ||  strcmp(crate, "rocfdc4") == 0 || 
      strcmp(crate, "rocfdc13") == 0  ||  strcmp(crate, "rocfdc14") == 0 ){      
    board = f1tdc;
  } else  if( strncmp(crate, "rocfdc",6) == 0 )
    board = fadc125;


  /*   TOF  */ 
  if(strcmp(crate, "roctof1") == 0)      
    board = fadc250;

  if(strcmp(crate, "roctof2") == 0)      
    board = tdc1290;
  
  if(strncmp(crate, "roctof3", 7) == 0)      
    board = dsc2;
  
  /*   ST  */ 
  if(strcmp(crate, "rocst") == 0)      
    board = fadc250;

  if(strcmp(crate, "rocstpsc1") == 0)      
    board = f1tdc;

  if(strcmp(crate, "rocstpsc2") == 0)      
    board = dsc2;

  /*   PS  */
  if(strncmp(crate, "rocps", 5) == 0 )
    board = fadc250;


  /*  TAGH  */
  if(strcmp(crate, "roctagh1") == 0)        
    board = fadc250;
  
  if(strcmp(crate, "roctagh2") == 0)        
    board = dsc2;

  /*  TAGM  */
  if(strcmp(crate, "roctagm1") == 0)        
    board = fadc250;

  if(strcmp(crate, "roctagm2") == 0)        
    board = dsc2;


  if(strcmp(crate, "roctagmh") == 0)        
    board = f1tdc;
  
  if(strcmp(crate, "rocfmwpc1") == 0)        
    board = fadc125;

  /*  Trigger Supervisor  */
  if(strcmp(crate, "roctrig1") == 0)      
    board = tsp;

  /*   Global Trigger  */
  if(strcmp(crate, "roctrig2") == 0)      
    board = gtp;

  /*     DIRC     */
  if(strcmp(crate, "rocdirc") == 0)      
    board = ssp_mod;

  /* TRD_GEM TEST */
  if(strcmp(crate, "roctrd1") == 0)        
    board = fadc125;

  return board;
}

char *get_board_type_name(char* crate){

  char *bd_type;

  /*   ECAL */ 
  if(strncmp(crate, "rocecal", 7) == 0)      
    bd_type = "fadc250";

  /*   FCAL */ 
  if(strncmp(crate, "rocfcal", 7) == 0)      
    bd_type = "fadc250";

  /*   CCAL */ 
  if(strcmp(crate, "rocccal") == 0)      
    bd_type = "fadc250";

  /*   BCAL */
  if( strcmp(crate, "rocbcal3") == 0 || strcmp(crate, "rocbcal6")  == 0 || 
      strcmp(crate, "rocbcal9") == 0 || strcmp(crate, "rocbcal12") == 0 )
    bd_type = "f1tdc";
  
  else if( strcmp(crate, "rocbcal13") == 0 || strcmp(crate, "rocbcal14") == 0  ||
	   strcmp(crate, "rocbcal15") == 0 || strcmp(crate, "rocbcal16") == 0 )
    bd_type = "dsc2";

  else if(strncmp(crate, "rocbcal", 7) == 0 ) 
    bd_type = "fadc250";
    
  /*   CDC  */
  if(strncmp(crate, "roccdc", 6) == 0)      
    bd_type = "fadc125";

  /*   FDC  */
  if( strcmp(crate, "rocfdc1")  == 0  ||  strcmp(crate, "rocfdc4")  == 0 || 
      strcmp(crate, "rocfdc13") == 0  ||  strcmp(crate, "rocfdc14") == 0 )      
    bd_type = "f1tdc";
  else  if( strncmp(crate, "rocfdc",6) == 0 )
    bd_type = "fadc125";


  /*   TOF  */ 
  if(strcmp(crate, "roctof1") == 0)      
    bd_type = "fadc250";

  if(strcmp(crate, "roctof2") == 0)      
    bd_type = "tdc1290";
  
  if(strncmp(crate, "roctof3", 7) == 0)      
    bd_type = "dsc2";
  
  /*   ST  */ 
  if(strcmp(crate, "rocst") == 0)      
    bd_type = "fadc250";

  if(strcmp(crate, "rocstpsc1") == 0)      
    bd_type = "f1tdc";

  if(strcmp(crate, "rocstpsc2") == 0)      
    bd_type = "dsc2";
  
  /*   PS  */
  if(strncmp(crate, "rocps", 5) == 0 )
    bd_type = "fadc250";


  /*  TAGH  */
  if(strcmp(crate, "roctagh1") == 0)        
    bd_type = "fadc250";
  
  if(strcmp(crate, "roctagh2") == 0)        
    bd_type = "dsc2";

  /*  TAGM  */
  if(strcmp(crate, "roctagm1") == 0)        
    bd_type = "fadc250";

  if(strcmp(crate, "roctagm2") == 0)        
    bd_type = "dsc2";

  if(strcmp(crate, "roctagmh") == 0)        
    bd_type = "f1tdc";
  
  if(strcmp(crate, "rocfmwpc1") == 0)        
    bd_type = "fadc125";


  /*  Trigger Supervisor  */
  if(strcmp(crate, "roctrig1") == 0)      
    bd_type = "tsp";
  
  /*  Global Trigger  */
  if(strcmp(crate, "roctrig2") == 0)      
    bd_type = "gtp";
  
  /*  DIRC  */
  if(strcmp(crate, "rocdirc") == 0)      
    bd_type = "ssp_mod";
  
  /*  TRD_GEM TEST  */
  if(strcmp(crate, "roctrd1") == 0)      
    bd_type = "fadc125";


  return bd_type;
  

}



int read_conf_global(char *filename, char *crate){

  int debug = 0;

  FILE *pFile;
  
  int ch;  
  int ii = 0;
  int jj;
  int val; 
  int args_assigned = -1, args = -1;

  float fval;

  char   str_tmp[STRLEN];
  char   keyword[ROCLEN];
  
  char str[STRLEN]; 
  char str_val[80]; 
  
  int  msk[16];

  char eq_msk_type[20];
  char trg_msk_type[20];

  int  eq_msk[3];
  int  trg_msk[11];
  int  trg_msk_ecal[13];
  unsigned int trg_det_mask;

  int soft_trig[4];
  int ti_fiber[8];

  int val_ssp[3];

  int gtp_pres[2];

  unsigned int gtp_mask = 0;

  unsigned int coin_wind;
  
  unsigned int trig_hold[3];
  
  unsigned int trig_delay[2];

  float gain_ecal_tmp = -1.;
  float gain_fcal_tmp = -1.;


  int ti_master_trig = 0;
  int ti_master = 0;
  int fiber_latency  = 0;
  
  int i1 = 0, i2 = 0;

  int SL_MIN = 0;
  int SL_MAX = 0;

  int SL_MIN_CCAL_REF  = 0;
  int SL_MAX_CCAL_REF = 0;

  int SL_MIN_ECAL_REF  = 0;
  int SL_MAX_ECAL_REF = 0;
  

  int CH_MIN = 0;

  int SL_HR = 17;

  int ngtp_eq   = 0;
  int ngtp_trg  = 0;

  int msk_rc[6];

  int slot_rc = -1;

  int ssp_fiber = 0;
  int ssp_asic  = 0;
  int ssp_ch    = 0;

  init_all();


  memset(soft_trig,0,sizeof(soft_trig));
  memset(ti_fiber,0,sizeof(ti_fiber));
  


  det_type det_comp = det_unknown;


  det_type crate_det  =  get_det_type(crate);

  if(crate_det == det_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_global: Unknown detector type for crate  %s \n" RESET, crate);
    return(-2);
  }

  bd_type crate_board  =  get_board_type(crate);

  if(crate_board == bd_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_global: Unknown board type for crate  %s \n" RESET, crate);
    return(-2);
  }

  

  /*   Open Global Configuration File  */
  pFile = fopen(filename, "r");

  if(pFile == NULL)
  {
    printf(BOLDRED "\n FATAL: Read_conf_global: Cannot open config file   %s \n" RESET, filename);
    return(-2);
  }
  printf(BOLDBLUE "\n Reading  Global Configuration File    %s \n" RESET, filename);
  printf(BOLDBLUE " Crate =  %s,   Board  Type = %d,  Detector = %d  \n" RESET, crate, crate_board, crate_det);


  while ((ch = getc(pFile)) != EOF) {
    if ( ch == '#' || ch == '#' ||  ch  == '=' ) {
      while (getc(pFile) != '\n') {}
    } else if( ch == '\n' ) {}
    else  {	  
      ungetc(ch,pFile);
      if(fgets(str_tmp, sizeof(str_tmp), pFile)){


	if(debug) printf(" STR_TMP READ FROM FILE:   %s \n",str_tmp);


	memset(str,0,sizeof(str)); memset(keyword,0,sizeof(keyword));

	args_assigned = sscanf(str_tmp,"%s",str);
	if(args_assigned != 1) memset(str,0,sizeof(str));

	if(debug) printf("args_assigned:  %d  %d   STRING:  %s  \n", args_assigned, det_comp, str);

	/*    Read configuration for sub-detectors   */
	if(args_assigned == 1){

	  if(strncmp(str, "TRIGGER", 7) == 0){
	    printf(" TRIGGER config found \n");
	    det_comp = trigger;
	  }

	  if(strncmp(str, "GLOBAL", 6) == 0){
	    printf(" GLOBAL config found \n");
	    det_comp = global;
	  }

	  if(strncmp(str, "ECAL", 4) == 0){
	    printf(" ECAL config found \n");
	    det_comp = ecal;
	  }

	  if(strncmp(str, "FCAL", 4) == 0){
	    printf(" FCAL config found \n");
	    det_comp = fcal;
	  }	  

	  if(strcmp(str, "CCAL") == 0){
	    printf(" CCAL config found \n");
	    det_comp = ccal;
	  }

	  if(strncmp(str, "BCAL", 4) == 0){
	    printf(" BCAL config found \n");
	    det_comp = bcal;
	  }

	  if(strncmp(str, "CDC", 3) == 0){
	    printf(" CDC config found \n");
	    det_comp = cdc;
	  }

	  if(strncmp(str, "FDC", 3) == 0){
	    printf(" FDC config found \n");
	    det_comp = fdc;
	  }


	  if(strncmp(str, "TOF", 3) == 0){
	    printf(" TOF config found \n");
	    det_comp = tof;
	  }

	  if(strncmp(str, "ST", 2) == 0){
	    printf(" ST config found \n");
	    det_comp = st;
	  }

          if(strcmp(str, "PS") == 0){
            printf(" PS config found \n");
            det_comp = ps;
          }

          if(strcmp(str, "PSC") == 0){
            printf(" PSC config found \n");
            det_comp = psc;
          }	  

//          if(strcmp(str, "PSTEST") == 0){
//            printf(" PSTEST config found \n");
//            det_comp = pstest;
//          }   


	  if(strncmp(str, "TAGM", 4) == 0){
	    printf(" TAGM config found \n");
	    det_comp = tagm;
	  }	  

	  if(strncmp(str, "TAGH", 4) == 0){
	    printf(" TAGH config found \n");
	    det_comp = tagh;
	  }	  

	  if(strncmp(str, "TPOL", 4) == 0){
	    printf(" TPOL config found \n");
	    det_comp = tpol;
	  }

	  if(strncmp(str, "TABS", 4) == 0){
	    printf(" TABS config found \n");
	    det_comp = tabs;
	    use_tabs = 1;
	  }

	  if(strcmp(str, "DIRC") == 0){
	    printf(" DIRC config found \n");
	    det_comp = dirc;
	  }

	  if(strncmp(str, "MUON", 4) == 0){
	    printf(" MUON config found \n");
	    det_comp = muon;
	    use_muon = 2;
	  }


	  if(strcmp(str, "TRD") == 0){
	    printf(" TRD config found \n");
	    det_comp = trd;
            if(crate_det == st){
              use_trd = 1;
            }
	  }

          if(strncmp(str, "GEMTRD", 6) == 0){
	    printf(" GEMTRD config found \n");
	    det_comp = gem;
	  }  
          

	}
	
	/*  ==========  Trigger Settings  ==========  */
	if(det_comp == trigger){
	  
	  args = sscanf (str_tmp, "%s %s", keyword, str);


	  if(debug){
	    printf("\n");
	    printf(" STRTMP     %s \n",   str_tmp);
	    printf(" KEYWORD    %s \n",   keyword);
	    printf(" STR    %s \n",   str);
	    printf("\n");
	  }

	  
#ifdef SHM_BUF
	  if(strcmp("CALIBRATION",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &CALIBRATION_FLAG); 	    
	  }
#endif
	  

	  if(strcmp("TS_TRIG_TYPE",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &ts_bd.trig_type); 	    
	  }	  
	  

	  if(strcmp("TS_SYNC_INT",keyword) == 0){
            sscanf (str_tmp, "%*s  %d", &ts_bd.ts_sync_int);        
          }       
	  
          if(strcmp("TS_SYNC_OPT",keyword) == 0){
            sscanf (str_tmp, "%*s  %d", &ts_bd.ts_sync_opt);        
          }               


	  SCAN_TS_COIN_WIND("TS_COIN_WIND", ts_bd.ts_coin_wind);

	  SCAN_TS_TRIG_HOLD("TS_TRIG_HOLD", ts_bd.ts_trig_hold);	  

	  SCAN_TS_GTP_PRES("TS_GTP_PRES", ts_bd.gtp_pres);

  	  
	  if(strcmp("BLOCKLEVEL",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &ts_bd.blocklevel); 	    
	  }	  

	  if(strcmp("BUFFERLEVEL",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &ts_bd.bufferlevel); 	    
	  }	  


	  SCAN_SOFT_TRIG("TS_SOFT_TRIG",ts_bd.ts_soft_trig);
	  
	  
	  SCAN_TD_SLOTS("TS_TD_SLOTS",ts_bd.ts_td_slots);
	  
	  SCAN_FP_INPUTS("TS_FP_INPUTS",ts_bd.ts_fp_inputs);
	  
	  SCAN_TS_FP_DELAY("TS_FP_DELAY",ts_bd.ts_fp_delay);

	  if(strcmp("TI_MASTER_TRIG",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &ti_master_trig); 	    
	    ti_bd.ti_master_trig = ti_master_trig;
	  }

	  if(strcmp("TI_MASTER",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &ti_master); 	    
	    ti_bd.ti_master = ti_master;
	  }


	  SCAN_SOFT_TRIG("TI_SOFT_TRIG",ti_bd.ti_soft_trig);

	  SCAN_FP_INPUTS("TI_FP_INPUTS",ti_bd.ti_fp_inputs);
	  
	  SCAN_TI_FIBER("TI_FIBER_EN",ti_bd.ti_fiber_en);
	  
	  if(strcmp("TI_FIBER_LATENCY_OFFSET",keyword) == 0){
	    sscanf (str_tmp, "%*s  %x", &fiber_latency); 	    
	    ti_bd.fiber_latency = fiber_latency;
	  }
	  
	  if(strcmp("TRIG_DELAY",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &trig_delay); 	    
	  }	  
	  
	  if(strcmp("DAC_CALIB",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &dac_calib); 	    
	  }	  
	  
	  if(strcmp("USE_PLAYBACK",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &use_playback); 	    
	  }	 
	  

          if(crate_board == gtp){
            SCAN_SSP("SSP_SLOT",ssp);
          }


	  SCAN_EQ("TRIG_EQ",gtp_eq);

	  SCAN_TRIG("TRIG_TYPE",gtp_trig);

          SCAN_GTP_GAIN("TRIG_ECAL_FCAL_GAIN", gtp_gain);


	}   /*  ==========  End  Trigger  ==========  */ 
	


	/*  ==========  Global Settings  ==========  */
	if(det_comp == global){
	  
	  args = sscanf (str_tmp, "%s %s", keyword, str);

#ifdef SHM_BUF
	  if(strcmp("CALIBRATION",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &CALIBRATION_FLAG); 	    
	  }
	  printf("det_comp=%d keyword=%s  CALIBRATION_FLAG=%d \n",det_comp,keyword,CALIBRATION_FLAG);
#endif
  	  
	  if(strcmp("DAC_CALIB",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &dac_calib); 	    
	  }	  	  

	  if (crate_board == f1tdc){
	    SL_MIN = 3;
	    SL_MAX = NBD_F1TDC;
	    SCAN_B_FSETS("F1TDC_BIN_SIZE",  f1[jj].bin_size,SL_MIN,SL_MAX);
	    for(jj = SL_MIN; jj < SL_MAX; jj++)  f1[jj].global = 1;
	  } 


      	  if (crate_board ==  fadc250){
	    SL_MIN = 3;
	    SL_MAX = NBD_FADC250;
            
            SCAN_B_SSETS("PLAY_BACK_DIR",  fa250[jj].conf_pb_dir);
            SCAN_B_SSETS("PLAY_BACK_VER",  fa250[jj].conf_pb_ver);

            SCAN_B_SETS("FADC250_MON_NPED",     fa250[jj].mon_nped,SL_MIN,SL_MAX);
            SCAN_B_SETS("FADC250_MON_MAX_PED",  fa250[jj].mon_max_ped,SL_MIN,SL_MAX);

            SCAN_B_SETS("FADC250_BUSY",  fa250[jj].busy, SL_MIN, SL_MAX);
            SCAN_B_SETS("FADC250_STOP",  fa250[jj].stop, SL_MIN, SL_MAX);

            SCAN_B_SETS("FADC250_FORMAT",  fa250[jj].data_format, SL_MIN, SL_MAX);

	  }     


      	  if (crate_board ==  fadc125){
            SL_MIN = 3;
	    SL_MAX = NBD_FADC125;

            SCAN_B_SETS("FADC125_BUSY",   fa125[jj].busy, SL_MIN, SL_MAX);
            SCAN_B_SETS("FADC125_STOP",   fa125[jj].stop, SL_MIN, SL_MAX); 

            SCAN_B_SETS("FADC125_FORMAT", fa125[jj].data_format, SL_MIN, SL_MAX); 

          }
          


	}  /*  ==========  End  Global  ==========  */

        // DIRC

	if( (det_comp == dirc)  &&  (crate_board == ssp_mod)  &&  (crate_det == dirc) ){

	  args = sscanf (str_tmp, "%s %s", keyword, str);

          SL_MIN = 3;
          SL_MAX = NBD_SSP;

          SCAN_B_SETS("SSP_W_OFFSET", ssp[jj].window_offset, SL_MIN,SL_MAX);          
	  SCAN_B_SETS("SSP_W_WIDTH",  ssp[jj].window_width,  SL_MIN,SL_MAX);
          SCAN_B_SETS("SSP_PULSER",   ssp[jj].pulser_freq,   SL_MIN,SL_MAX);

	  SCAN_B_SSETS("SSP_COM_DIR",  ssp[jj].conf_common_dir);
	  SCAN_B_SSETS("SSP_COM_VER",  ssp[jj].conf_common_ver);
          
          
          SCAN_SSP_REG_GLOB_ALL("SSP_REG_GLOBAL0",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Global0.val);
          SCAN_SSP_REG_GLOB_ALL("SSP_REG_GLOBAL1",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Global1.val);
          
          SCAN_SSP_REG_DAC_ALL("SSP_MAROC_REG_DAC0",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].DAC0);
          SCAN_SSP_REG_DAC_ALL("SSP_MAROC_REG_DAC1",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].DAC1);
          

          SCAN_SSP_REG_GAIN_ALL("SSP_MAROC_REG_GAIN_0_15",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
          SCAN_SSP_REG_GAIN_ALL("SSP_MAROC_REG_GAIN_16_31",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
          SCAN_SSP_REG_GAIN_ALL("SSP_MAROC_REG_GAIN_32_47",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
          SCAN_SSP_REG_GAIN_ALL("SSP_MAROC_REG_GAIN_48_63",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
          
          SCAN_SSP_FIB_SETS("SSP_CTEST_ENABLE",ssp[jj].dirc.fiber[ssp_fiber].ctest_enable);
          SCAN_SSP_FIB_SETS("SSP_CTEST_DAC",ssp[jj].dirc.fiber[ssp_fiber].ctest_dac);

          SCAN_SSP_FIB_SETS("SSP_FIBER_W_WIDTH",ssp[jj].dirc.fiber[ssp_fiber].window_width);
          SCAN_SSP_FIB_SETS("SSP_FIBER_W_OFFSET",ssp[jj].dirc.fiber[ssp_fiber].window_offset);


          SCAN_SSP_TDC_EN_ALL("SSP_TDC_ENABLE",ssp[jj].dirc.fiber[ssp_fiber].tdc_enable_mask);
          
          SCAN_SSP_CTEST("SSP_MAROC_REG_SUM",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].Sum);
          SCAN_SSP_CTEST("SSP_MAROC_REG_CTEST",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].CTest);
          SCAN_SSP_CTEST("SSP_MAROC_REG_MASKOR",ssp[jj].dirc.fiber[ssp_fiber].chip[ssp_asic].MaskOr);


        } 

	//	printf(" DET COMP = %d  CRATE BOARD = %d CRATE DET = %d \n",det_comp,crate_board,crate_det);
	
	
	/* ==========  FADC250 for FCAL, BCAL, TOF, and  ST  ==========  */
	if( ((det_comp == ecal)  &&  (crate_board == fadc250)  &&  (crate_det == ecal))   ||
            ((det_comp == fcal)  &&  (crate_board == fadc250)  &&  (crate_det == fcal))   || 
            ((det_comp == ecal)  &&  (crate_board == fadc250)  &&  (crate_det == ecal))   ||
            ((det_comp == ccal)  &&  (crate_board == fadc250)  &&  (crate_det == ccal))   ||
	    ((det_comp == bcal)  &&  (crate_board == fadc250)  &&  (crate_det == bcal))   || 
	    ((det_comp == tof)   &&  (crate_board == fadc250)  &&  (crate_det == tof))    || 
	    ((det_comp == st)    &&  (crate_board == fadc250)  &&  (crate_det == st))     || 
            ((det_comp == ps)    &&  (crate_board == fadc250)  &&  (crate_det == ps))     ||
	    ((det_comp == ps)    &&  (crate_board == fadc250)  &&  (crate_det == psc))    ||
            ((det_comp == psc)   &&  (crate_board == fadc250)  &&  (crate_det == psc))    ||
	    ((det_comp == tpol)  &&  (crate_board == fadc250)  &&  (crate_det == psc))    ||
            ((det_comp == tagm)  &&  (crate_board == fadc250)  &&  (crate_det == tagm))   ||
            ((det_comp == tagh)  &&  (crate_board == fadc250)  &&  (crate_det == tagh))   ||
	    ((det_comp == tabs)  &&  (crate_board == fadc250)  &&  (crate_det == fcal))   || 
	    ((det_comp == dirc)  &&  (crate_board == fadc250)  &&  (crate_det == tof))   || 
            ((det_comp == muon)  &&   (use_muon == 1)          &&  (crate_det == muon)) ){  
          
	  
          
	  SL_MIN  =  3;
	  SL_MAX  =  NBD_FADC250;

	  if( (det_comp == ccal) && (crate_det == ccal) ){
	    SL_MIN  =  3;
	    SL_MAX  =  16; 

            SL_MIN_CCAL_REF = 15;
            SL_MAX_CCAL_REF = 16;

	  }


	  if( (det_comp == ps) && (crate_det == psc) ){
	    SL_MIN  =  3;
	    SL_MAX  =  20; 
	  }
	  
	  if( (det_comp == ps) && (crate_det == psc) ){
	    SL_MIN  =  3;
	    SL_MAX  =  6; 
	  }

	  if( (det_comp == psc) && (crate_det == psc) ){
	    SL_MIN  =  6;
	    SL_MAX  =  7; 
	  }
	  
	  if( (det_comp == tpol) && (crate_det == psc) ){
	    SL_MIN  =  13;
	    SL_MAX  =  17; 
	  }

          if( (det_comp == st) && (crate_det == st) ){
	    SL_MIN  =  3;
            //            SL_MAX  =  5;
            //            SL_MAX  =  11;
            SL_MAX  =  15;

	  } 

	  if( (det_comp == muon) && (crate_det == muon) ){
	    SL_MIN  =  15;
	    SL_MAX  =  16; 
	  }

	  if( (det_comp == tabs) && (crate_det == fcal) && (strcmp(crate, "rocfcal4") == 0) ){
	    SL_MIN = 20;
	    SL_MAX = 21;
	  }	    

          
          // DIRC reference SiPM, was moved from Slot 16 to Slot 19 of the roctof1 crate
	  if( (det_comp == dirc) && (crate_det == tof) ){
	    SL_MIN  =  19;
	    SL_MAX  =  20; 
	  }


	  args = sscanf (str_tmp, "%s %s", keyword, str);

          SCAN_B_SETS("FADC250_BUSY",  fa250[jj].busy, SL_MIN, SL_MAX);
          SCAN_B_SETS("FADC250_STOP",  fa250[jj].stop, SL_MIN, SL_MAX);
          

	  SCAN_B_SETS("FADC250_MODE",     fa250[jj].mode,SL_MIN,SL_MAX);

	  SCAN_B_SETS("FADC250_W_OFFSET", fa250[jj].winOffset,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC250_W_WIDTH",  fa250[jj].winWidth,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC250_NSB",      fa250[jj].nsb,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC250_NSA",      fa250[jj].nsa,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC250_NPEAK",    fa250[jj].npeak,SL_MIN,SL_MAX);

	  SCAN_B_SETS("FADC250_NSAT",     fa250[jj].nsat,SL_MIN,SL_MAX);

          SCAN_B_SETS("FADC250_TRIG_NSAT",     fa250[jj].trig_nsat,SL_MIN,SL_MAX);

          SCAN_B_SETS("FADC250_NPED",     fa250[jj].nped,SL_MIN,SL_MAX);
          SCAN_B_SETS("FADC250_MAX_PED",  fa250[jj].max_ped,SL_MIN,SL_MAX);


	  SCAN_B_SETS("FADC250_TRIG_THR", fa250[jj].trig_thr,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC250_TRIG_NSB", fa250[jj].trig_nsb,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC250_TRIG_NSA", fa250[jj].trig_nsa,SL_MIN,SL_MAX);


	  SCAN_B_XSETS("FADC250_F_REV",   fa250[jj].f_rev,SL_MIN,SL_MAX);
	    
	  SCAN_B_XSETS("FADC250_B_REV",   fa250[jj].b_rev,SL_MIN,SL_MAX);
	  
	  SCAN_B_XSETS("FADC250_TYPE",    fa250[jj].b_ID,SL_MIN,SL_MAX);

	  
	  SCAN_TDP("FADC250_TRIG_BL",fa250[jj].trig_bl,SL_MIN,SL_MAX);
	  SCAN_TDP("FADC250_READ_THR",fa250[jj].read_thr,SL_MIN,SL_MAX);

	  SCAN_B_SSETS("FADC250_COM_DIR",  fa250[jj].conf_common_dir);
	  SCAN_B_SSETS("FADC250_COM_VER",  fa250[jj].conf_common_ver);

	  SCAN_B_SSETS("FADC250_USER_DIR", fa250[jj].conf_user_dir);
	  SCAN_B_SSETS("FADC250_USER_VER", fa250[jj].conf_user_ver);


          //  CCAL REFERENCE PMTs
	  SCAN_B_SETS("FADC250_MODE_REF",     fa250[jj].mode,      SL_MIN_CCAL_REF, SL_MAX_CCAL_REF);

	  SCAN_B_SETS("FADC250_W_OFFSET_REF", fa250[jj].winOffset, SL_MIN_CCAL_REF, SL_MAX_CCAL_REF);
	  
	  SCAN_B_SETS("FADC250_W_WIDTH_REF",  fa250[jj].winWidth,  SL_MIN_CCAL_REF, SL_MAX_CCAL_REF);

	  SCAN_TDP("FADC250_READ_THR_REF",    fa250[jj].read_thr,  SL_MIN_CCAL_REF, SL_MAX_CCAL_REF);


	  if( (det_comp == ecal) && (crate_det == ecal)  && (strcmp(crate, "rocecal4") == 0) ){
	    
            SL_MIN_ECAL_REF    =  20;
            SL_MAX_ECAL_REF   =  21;
	    
	    SCAN_B_SETS("FADC250_MODE_EREF",         fa250[jj].mode,          SL_MIN_ECAL_REF,   SL_MAX_ECAL_REF);
	    
	    SCAN_B_SETS("FADC250_W_OFFSET_EREF",  fa250[jj].winOffset,   SL_MIN_ECAL_REF,   SL_MAX_ECAL_REF);
	    
	    SCAN_B_SETS("FADC250_W_WIDTH_EREF",   fa250[jj].winWidth,   SL_MIN_ECAL_REF,   SL_MAX_ECAL_REF);
	     
	    SCAN_TDP("FADC250_READ_THR_EREF",       fa250[jj].read_thr,     SL_MIN_ECAL_REF,   SL_MAX_ECAL_REF);
	    
	  }




          if(strcmp("CTP_BCAL_THR",keyword) == 0)
            sscanf (str_tmp, "%*s  %d", &ctp.BCALThreshold);        
          
          if(strcmp("CTP_BCAL_WID",keyword) == 0)
            sscanf (str_tmp, "%*s  %d", &ctp.BCALWindowWidth);     

          if(strcmp("CTP_THR",keyword) == 0)
            sscanf (str_tmp, "%*s  %d", &ctp.FinalSumThreshold); 
	  
          if(strcmp("CTP_HIT_WID",keyword) == 0)
            sscanf (str_tmp, "%*s  %d", &ctp.HitWindowWidth); 	  

          if(strcmp("CTP_USE",keyword) == 0)
            sscanf (str_tmp, "%*s  %d", &ctp.used); 	  	  


	}  //  FADC250



	/* ========== FADC125 for CDC and FDC   ========== */
	if( ((det_comp == cdc)  && (crate_board == fadc125) && (crate_det == cdc)) || 
	    ((det_comp == fdc)  && (crate_board == fadc125) && (crate_det == fdc)) || 
	    ((det_comp == trd)  && (crate_det == st)) || 
            ((det_comp == muon) && (crate_det == muon)) ||
	    ((det_comp == gem)  && (crate_board == fadc125) && (crate_det == gem)) ){
	  
	  SL_MIN  =  3;
	  SL_MAX  =  NBD_FADC125;  
	  
	  if( (det_comp == trd) && (crate_det == st) ){
	    SL_MIN  =  13;
	    SL_MAX  =  16; 
	  }

	  if( (use_muon == 1) && (det_comp == muon) && (crate_det == muon) ){
	    SL_MIN  =  3;
	    SL_MAX  =  5; 
	  }

	  if( (det_comp == gem) && (crate_det == gem) ){
	    SL_MIN  =  3;
	    SL_MAX  =  20; 
	  }
          

	  args = sscanf (str_tmp, "%s %s", keyword, str);

 	  if(strcmp("USE_GEM",keyword) == 0){
	    sscanf (str_tmp, "%*s  %d", &use_gem); 	    
	  }

	  SCAN_B_SETS("FADC125_MODE",     fa125[jj].mode,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC125_W_OFFSET", fa125[jj].winOffset,SL_MIN,SL_MAX);

	  SCAN_B_SETS("FADC125_W_WIDTH",  fa125[jj].winWidth,SL_MIN,SL_MAX);

	  SCAN_B_SETS("FADC125_NSB",      fa125[jj].nsb,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC125_NSA",      fa125[jj].nsa,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("FADC125_NPEAK",    fa125[jj].npeak,SL_MIN,SL_MAX);
	  	  
	  SCAN_B_SETS("FADC125_PG",fa125[jj].PG,SL_MIN,SL_MAX); 

	  SCAN_B_SETS("FADC125_IE",fa125[jj].IE,SL_MIN,SL_MAX);  	  

	  SCAN_B_SETS("FADC125_P1",fa125[jj].P1,SL_MIN,SL_MAX);  

	  SCAN_B_SETS("FADC125_P2",fa125[jj].P2,SL_MIN,SL_MAX);  

	  SCAN_B_SETS("FADC125_TH",fa125[jj].TH,SL_MIN,SL_MAX);  

	  SCAN_B_SETS("FADC125_TL",fa125[jj].TL,SL_MIN,SL_MAX);  


	  SCAN_B_SETS("FADC125_IBIT",fa125[jj].IBIT,SL_MIN,SL_MAX);  

	  SCAN_B_SETS("FADC125_ABIT",fa125[jj].ABIT,SL_MIN,SL_MAX);  

	  SCAN_B_SETS("FADC125_PBIT",fa125[jj].PBIT,SL_MIN,SL_MAX);  
	  

	  SCAN_B_XSETS("FADC125_F_REV",   fa125[jj].f_rev,SL_MIN,SL_MAX);
	    
	  SCAN_B_XSETS("FADC125_B_REV",   fa125[jj].b_rev,SL_MIN,SL_MAX);

	  SCAN_B_XSETS("FADC125_P_REV",   fa125[jj].p_rev,SL_MIN,SL_MAX);	  

	  SCAN_B_XSETS("FADC125_TYPE",    fa125[jj].b_ID,SL_MIN,SL_MAX);
	  	  

	  SCAN_B_FSETS("FADC125_THR_FACT", fa125[jj].thr_fact ,SL_MIN,SL_MAX);


	  SCAN_TDP_125("FADC125_THR",fa125[jj].read_thr);
	  SCAN_TDP_125("FADC125_DAC",fa125[jj].dac);

          
          SCAN_B_SETS("FADC125_BUSY",  fa125[jj].busy, SL_MIN, SL_MAX);
          SCAN_B_SETS("FADC125_STOP",  fa125[jj].stop, SL_MIN, SL_MAX); 


	  SCAN_B_SSETS("FADC125_COM_DIR",  fa125[jj].conf_common_dir);

	  SCAN_B_SSETS("FADC125_COM_VER",  fa125[jj].conf_common_ver);

	  SCAN_B_SSETS("FADC125_USER_DIR", fa125[jj].conf_user_dir);

	  SCAN_B_SSETS("FADC125_USER_VER", fa125[jj].conf_user_ver);
	  

	}    //  End CDC and FDC
	
	
	/* ==========   DSC2 for BCAL, TOF, ST, and PSC   ==========  */
	if( ((det_comp == tof)    && (crate_board == dsc2) && (crate_det == tof))     ||
            ((det_comp == dirc)   && (crate_board == dsc2) && (crate_det == tof))     ||
	    ((det_comp == psc)    && (crate_board == dsc2) && (crate_det == st))      ||   //PSC settings;  rocstpsc2 belongs to PS
	    ((det_comp == st)     && (crate_board == dsc2) && (crate_det == st))      ||
	    ((det_comp == tagm)   && (crate_board == dsc2) && (crate_det == tagm))    ||
	    ((det_comp == tagh)   && (crate_board == dsc2) && (crate_det == tagh))    ||
	    ((det_comp == bcal)   && (crate_board == dsc2) && (crate_det == bcal))    || 
	    ((det_comp == tabs)   && (crate_board == dsc2) && (crate_det == tof))     || 
            ((det_comp == muon)   && (use_muon == 1)      && (crate_det == muon)) ){

	  SL_MIN  =  3;
	  SL_MAX  =  NBD_DSC2; 
	  
	  if( (det_comp == psc)  && (crate_det == st) ){
	    SL_MIN  =  6;
	    SL_MAX  =  7;  
	  }	  
	  
	  if( (det_comp == st)  && (crate_det == st) ){
	    SL_MIN  =  4;
	    SL_MAX  =  6;  
	  }
	  
	  if( (det_comp == tabs)  && (crate_det == tof) ){
	    SL_MIN  =  15;
	    SL_MAX  =  16;  
	  }

          if( (det_comp == muon)  && (crate_det == muon) ){
	    SL_MIN  =  14;
	    SL_MAX  =  15;  
	  }


	  args = sscanf (str_tmp, "%s %s", keyword, str);

	  if(strcmp(keyword,"DSC2_THRESHOLD") == 0){
	    sscanf (str_tmp, "%*s %d %d", &i1, &i2);
	    for(ii = SL_MIN; ii < SL_MAX; ii++)  for(jj = 0; jj < MAX_DSC2_CH; jj++){
	      dsc[ii].TDCThreshold[jj] = i1;
	      dsc[ii].TRGThreshold[jj] = i2;
	    }
	  }
	  

          printf("\n\n\n\n --------------  MY TEST  %d    %d  \n\n\n  ",det_comp, crate_det);


          // DIRC reference SiPM, which is temporary positioned in Slot 15, Ch 16 of the roctof3 crate
	  if( (det_comp == dirc) && (crate_det == tof) ){
            if(strcmp(keyword,"DSC2_THRESHOLD") == 0){
              sscanf (str_tmp, "%*s %d %d", &i1, &i2);
              dsc[19].TDCThreshold[15] = i1;
              dsc[19].TRGThreshold[15] = i2;
            }                
	  }
          
          printf("\n\n\n\n --------------  MY TEST  %d    %d    %d \n\n\n  ",det_comp, crate_det, dsc[19].TDCThreshold[19]);


	  if(strcmp(keyword,"DSC2_WIDTH") == 0) {
	    sscanf (str_tmp, "%*s %d %d", &i1, &i2);
	    for(ii = SL_MIN; ii < SL_MAX; ii++){
	      dsc[ii].TDCWidth = i1;
	      dsc[ii].TRGWidth = i2;
	    } 	    
	  }	

	  SCAN_DSC_SSETS("DSC2_COM_DIR",  dsc[jj].conf_common_dir);
	  SCAN_DSC_SSETS("DSC2_COM_VER",  dsc[jj].conf_common_ver);

  
	}    //  TOF DSC2





	/* ==========  F1TDC for BCAL, ST, PSC, TAGM, and TAGH   ==========  */
	if( ((det_comp == bcal)  && (crate_board == f1tdc) && (crate_det == bcal))  || 
	    ((det_comp == psc)   && (crate_board == f1tdc) && (crate_det == st))    ||
	    ((det_comp == st)    && (crate_board == f1tdc) && (crate_det == st))    ||
	    ((det_comp == tagm)  && (crate_board == f1tdc) && (crate_det == tagm))  ||
	    ((det_comp == tagh)  && (crate_board == f1tdc) && (crate_det == tagm)) ){
	  
	  
	  printf(" I AM HERE \n");
	  
	  // High-resolution, synchronous
	  for(ii = 3; ii < NBD_F1TDC; ii++) f1[ii].version = 2;
	  
	  SL_MIN  =  3;
	  SL_MAX  =  NBD_F1TDC;  	  
	  
	  if( (det_comp == psc)  && (crate_det == st) ){
	    SL_MIN  =  4;
	    SL_MAX  =  5;  	  	    
	  }

	  if( (det_comp == st)   && (crate_det == st) ){
	    SL_MIN  =  3;
	    SL_MAX  =  4;  	  	    
	  }

	  if( (det_comp == tagm)   && (crate_det == tagm) ){
	    SL_MIN  =  3;
	    SL_MAX  =  7;  	  	    
	  }

	  if( (det_comp == tagh)   && (crate_det == tagm) ){
	    SL_MIN  =  7;
	    SL_MAX  =  17;  	  	    
	  }

	  
	  args = sscanf (str_tmp, "%s %s", keyword, str);	  


	  SCAN_B_FSETS("F1TDC_BIN_SIZE",  f1[jj].bin_size,SL_MIN,SL_MAX);
	  // Rewrite Global bin size for F1TDC
	  if(strcmp(keyword,"F1TDC_BIN_SIZE") == 0){
	    for(jj =  SL_MIN; jj <  SL_MAX; jj++)
	      f1[jj].global = 0;
	  }

	  SCAN_B_FSETS("F1TDC_LATENCY",   f1[jj].latency,SL_MIN,SL_MAX);
	  SCAN_B_FSETS("F1TDC_WINDOW",    f1[jj].window,SL_MIN,SL_MAX);
	  SCAN_B_SETS("F1TDC_CLOCK",      f1[jj].clock,SL_MIN,SL_MAX);
	  
	  
	}    //  High-Resolution F1TDCs
	

	/* ==========  F1TDC for FDC   ==========  */
	if( (det_comp == fdc)  && (crate_board == f1tdc) && (crate_det == fdc)){   
	  
	  // Maximum F1 slot 16, high-resolution board in slot 17 of rocfdc1
	  SL_MIN  =  3;
	  SL_MAX  =  17;  
	  
	  // Low-resolution, synchronous
	  for(ii = 3; ii < 17; ii++) f1[ii].version = 3;
	  
	  sscanf (str_tmp, "%s %s", keyword, str);	  
	  
	  SCAN_B_FSETS("F1TDC_BIN_SIZE",  f1[jj].bin_size,SL_MIN,SL_MAX);
	  SCAN_B_FSETS("F1TDC_LATENCY",   f1[jj].latency,SL_MIN,SL_MAX);
	  SCAN_B_FSETS("F1TDC_WINDOW",    f1[jj].window,SL_MIN,SL_MAX);
	  SCAN_B_SETS("F1TDC_CLOCK",      f1[jj].clock,SL_MIN,SL_MAX);

	  // Rewrite Global bin size for F1TDC
	  if(strcmp(keyword,"F1TDC_BIN_SIZE") == 0){
	    for(jj =  SL_MIN; jj <  SL_MAX; jj++)
	      f1[jj].global = 0;
	  }

	  // High-resolution board
	  SCAN_B_FSETS("F1TDC_HR_BIN_SIZE",  f1[SL_HR].bin_size, SL_HR, SL_HR + 1);
	  SCAN_B_FSETS("F1TDC_HR_LATENCY",   f1[SL_HR].latency,  SL_HR, SL_HR + 1);
	  SCAN_B_FSETS("F1TDC_HR_WINDOW",    f1[SL_HR].window,   SL_HR, SL_HR + 1);

	  // Rewrite Global bin size for F1TDC
	  if(strcmp(keyword,"F1TDC_BIN_SIZE") == 0){
	    for(jj =  SL_HR; jj <  SL_HR + 1; jj++)
	      f1[jj].global = 0;
	  }
	  
	  f1[SL_HR].version = 2;
	  	  
	  
	}    //  Low-Resolution F1TDCs


       	
	/* ==========  TOF  CAEN1290 TDC  ==========  */
	if( ((det_comp == tof)  && (crate_board == tdc1290) && (crate_det == tof)) || 
	    ((det_comp == tabs) && (crate_board == tdc1290) && (crate_det == tof))
	    ){
	  
	  //  Cannot run TABS and TOF in parallel; initialize either one or another.
	  
	  SL_MIN  =  3;
	  SL_MAX  =  NBD_FADC250;

	  sscanf (str_tmp, "%s %s", keyword, str);
	  
	  SCAN_B_SETS("TDC1290_W_WIDTH",   caen1290[jj].window_width,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_W_OFFSET",  caen1290[jj].window_offset,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_W_EXTRA",   caen1290[jj].window_extra,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_W_REJECT",  caen1290[jj].window_reject,SL_MIN,SL_MAX);
	  
	  SCAN_B_SETS("TDC1290_BLT_EVENTS", caen1290[jj].blt_Events,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_N_HITS",     caen1290[jj].n_Hits,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_ALMOSTFULL", caen1290[jj].almostFullLevel,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_OUT_PROG",   caen1290[jj].outProgControl,SL_MIN,SL_MAX);

	  SCAN_B_SETS("TDC1290_A24_A32",    caen1290[jj].a24_a32,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_SNGL_BLT",   caen1290[jj].sngl_blt_mblt,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_SST_RATE",   caen1290[jj].sst_rate,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_BERR_FIFO",  caen1290[jj].berr_fifo,SL_MIN,SL_MAX);
	  SCAN_B_SETS("TDC1290_EDGE",       caen1290[jj].edge,SL_MIN,SL_MAX);

          SCAN_B_SETS("TDC1290_INL",        caen1290[jj].inl,SL_MIN,SL_MAX);
          

	  SCAN_RC("TDC1290_RC", caen1290[slot_rc].rc_adjust);

	}    //  TOF CAEN1290 TDC


      }     
    }      
  }
  
  fclose(pFile);

  return 0;
  
}




int read_conf_common(char *crate){
  
  int debug = 0;
 
  FILE *pFile;
  
  int ch;
  int ii = 0;
  int jj;
  int val;

  int args,  msk[16], msk1[18];
  
  int ch_dis_mask[3];

  float mskf[18];

  int slot = -1;

  int ssp_fiber = -1;
  int ssp_asic  = -1;
  int ssp_ch    = -1;
  int ssp_dac0  = -1;

  int first = -1, last = -1;

  char   sn[NBD_FADC250][12];

  char   str[STRLEN], str2[STRLEN], str_tmp[STRLEN];
  char   keyword[ROCLEN];
  
  int line = 0;

  int SL_MIN  =  0;
  int SL_MAX  =  0;  
  int CH_MIN  =  0;


  det_type crate_det  =  get_det_type(crate );

  if(crate_det == det_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_common: Unknown detector type for crate  %s \n" RESET, crate);
    return(-2);
  }

  bd_type crate_board  =  get_board_type(crate );

  printf(BOLDMAGENTA "USE_TRD  =  %d \n" RESET,use_trd);
  if((use_trd == 1) && (crate_det == st) && (crate_board == fadc250)) crate_board = fadc125;

  printf(BOLDMAGENTA "USE_MUON  =  %d \n" RESET,use_muon);
  if((use_muon == 0) && (crate_det == muon) && (crate_board == fadc125)) crate_board = fadc250;

  if(crate_board == bd_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_common: Unknown board type for crate  %s \n" RESET, crate);
    return(-2);
  }



  /*  Take user configuration file name from the flobal config file */
  char myfile[STRLEN]="";


  switch(crate_board){
    
  case fadc250: 

    if(debug){
      printf(" STRING LENGTH = %d  %s \n", strlen(fa250[3].conf_common_dir), fa250[3].conf_common_dir);
      printf(" STRING LENGTH = %d  %s \n", strlen(fa250[3].conf_common_ver), fa250[3].conf_common_ver);
    }

    if((strlen(fa250[3].conf_common_dir) != 0)  && (strlen(fa250[3].conf_common_ver) != 0) )
      sprintf(myfile,"%s/%s_fadc250_%s.cnf", fa250[3].conf_common_dir,crate,fa250[3].conf_common_ver);
    else {
      printf(BOLDRED "\n FATAL: Have to set  FADC250_COM_DIR and FADC250_COM_VER in the global configuration file %s %s  \n" RESET,
	     fa250[3].conf_common_dir, fa250[3].conf_common_ver );
      return(-2);
    }
    
    break;
    
  case fadc125: 

    if(debug){
      printf(" STRING LENGTH = %d  %s \n", strlen(fa125[3].conf_common_dir), fa125[3].conf_common_dir); 
      printf(" STRING LENGTH = %d  %s \n", strlen(fa125[3].conf_common_ver), fa125[3].conf_common_ver); 
    }
    
    if((strlen(fa125[3].conf_common_dir) != 0)  && (strlen(fa125[3].conf_common_ver) != 0) )
      sprintf(myfile,"%s/%s_fadc125_%s.cnf", fa125[3].conf_common_dir,crate,fa125[3].conf_common_ver);
    else {
      printf(BOLDRED "\n FATAL: Have to set FADC125_COM_DIR and FADC125_COM_VER in the global configuration file %s %s  \n" RESET,
	     fa125[3].conf_common_dir, fa125[3].conf_common_ver );
      return(-2);
    }
     
    break;

  case tdc1290:
    return(0);
    break;

  case dsc2:
    
    if(debug){
      printf(" STRING LENGTH = %d  %s \n", strlen(dsc[3].conf_common_dir), dsc[3].conf_common_dir);
      printf(" STRING LENGTH = %d  %s \n", strlen(dsc[3].conf_common_ver), dsc[3].conf_common_ver);
    }

    //  Common configuration file for DSC is not required. If DSC_COM_DIR or DSC_COM_VER are not set, 
    //  thresholds from the Global Config file will be used by default

    if((strlen(dsc[3].conf_common_dir) != 0)  && (strlen(dsc[3].conf_common_ver) != 0) )
      sprintf(myfile,"%s/%s_dsc_%s.cnf", dsc[3].conf_common_dir,crate,dsc[3].conf_common_ver);
    else {
      printf(BOLDBLUE "\n No common configuration file is used \n" RESET );
      return 0;
    }

    break;
    
  case f1tdc:
    return(0);
    break;
    
  case ssp_mod: 
    
    if(debug){
      printf(" STRING LENGTH = %d  %s \n", strlen(ssp[3].conf_common_dir), ssp[3].conf_common_dir);
      printf(" STRING LENGTH = %d  %s \n", strlen(ssp[3].conf_common_ver), ssp[3].conf_common_ver);
    }

    if((strlen(ssp[3].conf_common_dir) != 0)  && (strlen(ssp[3].conf_common_ver) != 0) )
      sprintf(myfile,"%s/%s_ssp_%s.cnf", ssp[3].conf_common_dir,crate,ssp[3].conf_common_ver);
    else {
      printf(BOLDRED "\n FATAL: Have to set  SSP_COM_DIR and SSP_COM_VER in the global configuration file %s %s  \n" RESET,
	     ssp[3].conf_common_dir, ssp[3].conf_common_ver );
      return(-2);
    }
    
    break;
    
    
  default: break;    
    
  }
  

  if(strlen(myfile) == 0){
    printf(BOLDBLUE "\n No user configuration file is used \n" RESET );
    return 0;    
  }
  
  
  /*   Open Global Configuration File  */
  pFile = fopen(myfile, "r");

  if(pFile == NULL)
  {
    printf(BOLDRED "\n FATAL: Read_conf_common: Cannot open config file   %s \n" RESET, myfile);
    return(-2);
  }
  printf(BOLDBLUE "\n Reading  Common  Parameters from    %s \n" RESET, myfile);


  while ((ch = getc(pFile)) != EOF) {
    if(debug) printf(" %d %d %d \n",ch,' ','=');
    if ( ch == '#' || ch == '#' ||  ch  == '=' ) {
      while (getc(pFile) != '\n') {}
    } else if( ch == '\n' ) {}
    else  {	  
      ungetc(ch,pFile);
      if(fgets(str_tmp, sizeof(str_tmp), pFile)){
	
	memset(str,0,sizeof(str)); memset(keyword,0,sizeof(keyword));

	switch(crate_board){
	  
	case fadc250: 
	  
	  sscanf (str_tmp, "%s %s", keyword, str);
	  	  
	  if(strcmp(keyword,"CRATE") == 0){
	    if(strcmp(crate, str) != 0){
	      printf(BOLDRED "\nRead_config_common: Wrong crate name in config file, %s\n" RESET, str);
	      return(-2);
	    }
	  }

	  SL_MIN  =  3;
	  SL_MAX  =  NBD_FADC250;  


	  SCAN_TDP("FADC250_DAC",fa250[jj].dac,SL_MIN,SL_MAX);

	  SCAN_SLOT("FADC250_SLOTS",fa250[slot].group);

	  if( (slot >= 3) && (slot != 11) && (slot != 12) && (slot < 21)){
	    
	    SCAN_TDP("FADC250_DAC",fa250[slot].dac,SL_MIN,SL_MAX);
	    
	    SCAN_TDP_ALLCH("FADC250_ALLCH_DAC",fa250[slot].dac);
	  }

	  
	  if(strcmp(keyword,"FADC250_ALLSN") == 0) {
	    args = sscanf (str_tmp, "%*s %s %s %s %s %s %s %s %s %s %s \
                                     %s %s %s %s %s %s %s %s %s %s %s",
			   &sn[ 1][0], &sn[ 2][0], &sn[ 3][0], &sn[ 4][0], &sn[ 5][0], &sn[ 6][0], &sn[ 7][0],
			   &sn[ 8][0], &sn[ 9][0], &sn[10][0], &sn[11][0], &sn[12][0], &sn[13][0], &sn[14][0],
			   &sn[15][0], &sn[16][0], &sn[17][0], &sn[18][0], &sn[19][0], &sn[20][0], &sn[21][0]);
	    if(args != 21) {
	      printf(BOLDRED "\n Read_conf_common: Wrong number of SNs =  %d, should be 21 \n\n" RESET, args);
	      printf(" Read string:  %s \n",str_tmp);
	      return(-2);
	    }
	    for(ii = 1; ii < NBD_FADC250; ii++) sprintf((char *)fa250[ii].SerNum, "%s", sn[ii]);
	  }
	  
	  break;
	  
	case fadc125: 
	  
	  sscanf (str_tmp, "%s %s", keyword, str);
	  
	  if(strcmp(keyword,"CRATE") == 0){
	    if(strcmp(crate, str) != 0){
	      printf(BOLDRED "\nRead_config_common: Wrong crate name in config file, %s\n" RESET, str);
	      return(-2);
	    }
	  }

	  if(strcmp(keyword,"FADC125_ALLSN") == 0) {
	    args = sscanf (str_tmp, "%*s %s %s %s %s %s %s %s %s %s %s \
                                     %s %s %s %s %s %s %s %s %s %s %s",
			   &sn[ 1][0], &sn[ 2][0], &sn[ 3][0], &sn[ 4][0], &sn[ 5][0], &sn[ 6][0], &sn[ 7][0],
			   &sn[ 8][0], &sn[ 9][0], &sn[10][0], &sn[11][0], &sn[12][0], &sn[13][0], &sn[14][0],
			   &sn[15][0], &sn[16][0], &sn[17][0], &sn[18][0], &sn[19][0], &sn[20][0], &sn[21][0]);
	    if(args != 21) {
	      printf("\n Read_conf_common: Wrong number of SNs %d, should be 21 :: %s\n\n",args,str_tmp);
	      return(-5);
	    }
	    for(ii = 1; ii < NBD_FADC125; ii++) sprintf((char *)fa125[ii].SerNum, "%s", sn[ii]);
	  }



	  SCAN_SLOT("FADC125_SLOTS", fa125[slot].group);

	  
	  //	  printf("AA  %s   BB   %s \n",keyword, str);


	  SCAN_FA125_MASK("FADC125_CH_DIS",fa125[slot].ch_disable);

	  SCAN_FA125_DAC_THR("FADC125_DAC_CH_", fa125[slot].dac);

	  SCAN_TDP_125("FADC125_DAC",fa125[slot].dac);

	  SCAN_FA125_DAC_THR("FADC125_THR_CH_", fa125[slot].read_thr);

	  SCAN_TDP_125("FADC125_THR",fa125[slot].read_thr);
          
	  SCAN_FA125_SIG("FADC125_SIG_CH_", fa125[slot].sig);
          
	  SCAN_FA125_BL("FADC125_BL_CH_", fa125[slot].bl);

	  break;
	  

	case dsc2: 
	  
	  sscanf (str_tmp, "%s %s", keyword, str);
	  	  
	  if(strcmp(keyword,"CRATE") == 0){
	    if(strcmp(crate, str) != 0){
	      printf(BOLDRED "\nRead_config_common: Wrong crate name in config file, %s\n" RESET, str);
	      return(-2);
	    }
	  }

	  SL_MIN  =  3;
	  SL_MAX  =  NBD_DSC2;  
          
	  SCAN_DSC_SLOT("DSC2_SLOTS",dsc[slot].group);
          
	  if( (slot >= 2) && (slot < 22)){
            
	    SCAN_TDP_ALLCH_THR("DSC2_ALLCH_THR",dsc[slot].TDCThreshold);

	  }


	  break;

	case ssp_mod: 
	  
	  sscanf (str_tmp, "%s %s", keyword, str);
	  
	  if(strcmp(keyword,"CRATE") == 0){
	    if(strcmp(crate, str) != 0){
	      printf(BOLDRED "\nRead_config_common: Wrong crate name in config file, %s\n" RESET, str);
	      return(-2);
	    }
	  }
          
	  SL_MIN  =  3;
	  SL_MAX  =  NBD_SSP;  
          
          SCAN_SLOT("SSP_SLOTS",ssp[slot].group);

	  if( (slot >= 3) && (slot != 11) && (slot != 12) && (slot < 21)){
	    SCAN_TDP("SSP_FIBER",fa250[slot].dac,SL_MIN,SL_MAX);

            SCAN_SSP_FIBER("SSP_FIBER",ssp[slot].dirc.fiber_in_config);
            
            SCAN_SSP_ASIC("SSP_ASIC");
            
            if(strcmp(keyword,"SSP_MAROC_REG_DAC0") == 0){	
              sscanf (str_tmp, "%*s %d", &val);
              ssp[slot].dirc.fiber[ssp_fiber].chip[ssp_asic].DAC0 = val;
              printf("FIBER = %d  ASIC = %d   VAL = %d \n",ssp_fiber, ssp_asic, val);
            }
            
            SCAN_SSP_REG_GAIN("SSP_MAROC_REG_GAIN_0_15",ssp[slot].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
            SCAN_SSP_REG_GAIN("SSP_MAROC_REG_GAIN_16_31",ssp[slot].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
            SCAN_SSP_REG_GAIN("SSP_MAROC_REG_GAIN_32_47",ssp[slot].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
            SCAN_SSP_REG_GAIN("SSP_MAROC_REG_GAIN_48_63",ssp[slot].dirc.fiber[ssp_fiber].chip[ssp_asic].Gain);
            
	  }
          
          break;


	default:  printf(BOLDRED "read_conf_common:  Unknown board type %d. Exit" RESET, crate_board);  return(-3);  break;	  
	  	  
	}

      }     
    }      
  }
  
  fclose(pFile);

  return 0;
  
}



int read_conf_user(char *crate){
  
  int debug = 1;

  FILE *pFile;
  
  int ch;  

  int ii = 0;
  int jj;

  int args,  msk[16], msk1[18];

  int ch_dis_mask[3];

  int slot = -1;

  int first = -1, last = -1;

  unsigned int  ui1;

  char   str_tmp[STRLEN];
  char   keyword[ROCLEN];
  
  char str[150],  str2[STRLEN];

  int line = 0;

  det_type crate_det  =  get_det_type(crate );

  int my_bd, my_ch;

  if(crate_det == det_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_user: Unknown detector type for crate  %s \n" RESET, crate);
    return(-2);
  }

  bd_type crate_board  =  get_board_type(crate );

  if(crate_board == bd_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_user: Unknown board type for crate  %s \n" RESET, crate);
    return(-2);
  }
  
  
  /*  Take user configuration file name from the flobal config file */
  char myfile[STRLEN]="";;
  char myfile1[STRLEN]="";;

  switch(crate_board){
    
  case fadc250: 
    if(debug){
      printf(" STRING LENGTH = %d  %s \n", strlen(fa250[3].conf_user_dir), fa250[3].conf_user_dir);
      printf(" STRING LENGTH = %d  %s \n", strlen(fa250[3].conf_user_ver), fa250[3].conf_user_ver);
    }    

    // SASHA
    if((strlen(fa250[3].conf_user_dir) != 0)  && (strlen(fa250[3].conf_user_ver) != 0) ){
      sprintf(myfile,"%s/%s_%s.cnf", fa250[3].conf_user_dir,crate,fa250[3].conf_user_ver);
      sprintf(myfile1,"%s/%s_%s.cnf", fa250[3].conf_user_dir,crate,fa250[3].conf_user_ver);

      if(crate_det == ecal){
        char my_name1[100] = "mask9.0";
        char my_name[100] = "/gluonfs1/gluex/CALIB/ECAL/fadc250/user/spring_2025";
        //        char my_name[100] = "/gluonfs1/gluex/Subsystems/PS/test";
        //			char my_name[100] = "/home/somov/test";

        if(strcmp(fa250[3].conf_user_ver,"ring5_hot_v1") != 0){
          
          for(my_bd = 3; my_bd <  NBD_FADC250; my_bd++)
            for(my_ch = 0; my_ch <  MAX_FADC250_CH; my_ch++)
              fa250[my_bd].trig_bl[my_ch] = 108;
          

          //          gtp_trig[0].en_thr = 8000; gtp_trig[1].en_thr = 8000;

          //          printf("SSSSS -------------  = %d\n ",crate_det);
 
        }

        sprintf(myfile,"%s/%s_%s.cnf",my_name,crate,my_name1);
        //        printf("SSSSS -------------  = %d\n ",crate_det); 

      }
    }
    
    break;

  case fadc125: 
    if(debug){
      printf(" STRING LENGTH = %d  %s \n", strlen(fa125[3].conf_user_dir), fa125[3].conf_user_dir); 
      printf(" STRING LENGTH = %d  %s \n", strlen(fa125[3].conf_user_ver), fa125[3].conf_user_ver); 
    }

    if((strlen(fa125[3].conf_user_dir) != 0)  && (strlen(fa125[3].conf_user_ver) != 0) )
      sprintf(myfile,"%s/%s_%s.cnf", fa125[3].conf_user_dir,crate,fa125[3].conf_user_ver);

    break;

  default: break;    

  }

  if(strlen(myfile) == 0){
    printf(BOLDBLUE "\n No user configuration file is used \n" RESET );
    return 0;

  }



  /*    Open Global Configuration File   */
  pFile = fopen(myfile, "r");



  if(pFile == NULL)
  {
    printf(BOLDRED "\n  No  user configuration file found for crate  %s. Proceed with the default configuration \n" RESET, crate);
    return(0);
  }

  // SASHA
  printf(BOLDMAGENTA "\n Read User Configuration file  %s \n" RESET,  myfile1);
  //  printf(BOLDMAGENTA "\n Read User Configuration file  %s \n" RESET,  myfile);
  printf(BOLDMAGENTA "  Crate  = %s  Board type  = %d,  Detector  = %d  \n" RESET, crate, crate_board, crate_det);


  while ((ch = getc(pFile)) != EOF) {
    if(debug) printf(" %d %d %d \n",ch,' ','=');
    if ( ch == '#' || ch == '#' ||  ch  == '=' ) {
      while (getc(pFile) != '\n') {}
      line++;
    } else if( ch == '\n' ) {line++;}
    else  {	  
      ungetc(ch,pFile);
      if(fgets(str_tmp, sizeof(str_tmp), pFile)){
	line++;

	if(debug) printf(" Board type = %d \n", crate_board);

	memset(str,0,sizeof(str)); memset(keyword,0,sizeof(keyword));

	switch(crate_board){

	case fadc250: 

	  args = sscanf (str_tmp, "%s %s", keyword, str);
	  if(args != 2) break;	  

	  if(strcmp(keyword,"FADC250_NOT_USED") == 0)
	    {
	      slot = -1;
	      SCAN_MSK;
	      if(debug) printf("\nReadConfigFile:   args = %d \n",args);
	      
	      for(jj=0; jj<args; jj++) {

		slot = msk[jj];
		if(slot<3 || slot==11 || slot==12 || slot>20)
		  {
		    printf("\n Read_conf_user: Wrong slot number %d, %s\n",slot,str_tmp);
		    continue;
		  }
		fa250[slot].group = 0;
	      }
	      slot = -1;
	    }
	  
	  
	  SCAN_SLOT("FADC250_SLOTS", fa250[slot].group);
          
          
	  if( (slot >= 3) && (slot != 11) && (slot != 12) &&  (slot < 21)){
            
            SCAN_TDP_ALLCH("FADC250_ALLCH_THR",fa250[slot].read_thr);
            
	    SCAN_B_MSKS("FADC250_ADC_MASK", fa250[slot].chDisMask);
	    SCAN_B_MSKS("FADC250_TRG_MASK", fa250[slot].trigMask);

	    debug = 1;
	    if(debug){
	      printf(" CHECK CHANNEL DISABLE MASK  NEW  = %d  %x \n", slot, fa250[slot].chDisMask );
	      printf(" TRIGGER MASK   =   %d  %x \n", slot, fa250[slot].trigMask );
	    }
	    debug = 0;

	  }
	  
	  break;
	  
	case fadc125: 


	  args = sscanf (str_tmp, "%s %s", keyword, str);
	  if(args != 2) break;
	  

	  if(strcmp(keyword,"FADC125_NOT_USED") == 0) {
	    slot = -1;
	    SCAN_MSK;
	    if(debug) printf("\nReadConfigFile:   args = %d  %s \n",args, str_tmp);
	    
	    for(jj = 0; jj < args; jj++) {
	      
	      slot = msk[jj];
	      if(slot<3 || slot==11 || slot==12 || slot>20)
		{
		  printf("\n Read_conf_user: Wrong slot number %d, %s\n",slot,str_tmp);
		  continue;
		}
	      fa125[slot].group = 0;
	    }
	    slot = -1;
	  }
	  

	  if(strcmp(keyword,"FADC125_SLOTS") == 0) {
	    slot = -1;							
	    SCAN_MSK;							
	    if(args == 1){	/* Don't allow groups. Each Slot should have an entry in the file */ 
	      slot = msk[0];						
	      if(slot < 3 || slot == 11 || slot == 12 || slot > 20){	
		printf(BOLDRED "\nRead_conf: Wrong slot number %d, %s\n" RESET, slot,str_tmp); 
		return(-2);						
	      }							
	    } else {							
	      printf(BOLDRED "\n Read_conf:  Wrong number of slots in  FADC_SLOTS  %d. Only one slot is allowed. Exit \n"  RESET, args); 
	      return(-2);						
	    }								  
	  }
	  
	  
	  

	  SCAN_FA125_DAC_THR("FADC125_DAC_CH_", fa125[slot].dac);

	  SCAN_FA125_DAC_THR("FADC125_THR_CH_", fa125[slot].read_thr);

	  SCAN_FA125_MASK("FADC125_CH_DIS",fa125[slot].ch_disable);


	  if(strncmp(keyword,"FADC125_MASK_",12) == 0){
	    
	    SCAN_MSK1;
	    
	    ui1 = 0;
	    for(jj = 0; jj < 18; jj++) {
	      if((msk1[jj] < 0) || (msk1[jj] > 1)) {
		printf("\nReadConfigFile: Wrong mask bit value, %d\n\n",msk1[jj]); return(-9);
	      }
	      ui1 |= (msk1[jj]<<jj);
	    }
	    
	    str2[0] = keyword[13];
	    str2[1] = '\0';
	    jj = atoi(str2);
	    /* printf("mask jj = %d    str2 = %s \n",jj,str2); */
	    
	    if(      jj == 1) jj = 1;
	    else if (jj == 3) jj = 2;
	    else if (jj == 5) jj = 3;
	    
	    if(jj < 0 || jj > 3) {
	      printf("ERROR: mask jj = %d, must be between 0 and 3\n",jj);
	      return(-9);
	    }
	    fa125[slot].mask[jj] = ui1;	      	      
	  }

    
	  break;
	  
	default: break;	  
	  
	  
	}
      }     
    }      
  }
  
  fclose(pFile);

  return 0;

}



int get_nbd_fadc250(int print){
  
  int bd;
  int nbd = 0;

  for(bd = 3; bd < NBD_FADC250; bd++){
    
    if(fa250[bd].group > 0) nbd++;
    if(print ) printf(" Group for board in slot %d   =    %d \n", bd, fa250[bd].group);
  }

  return nbd;

}

int get_nbd_fadc125(int print){
  
  int bd;
  int nbd = 0;

  for(bd = 3; bd < NBD_FADC125; bd++){
    
    if(fa125[bd].group > 0) nbd++;
    if(print ) printf(" Group for board in slot %d   =    %d \n", bd, fa125[bd].group);
  }

  return nbd;

}


void print_fadc250_pb(int slot){
  
  int bd_ch, samp_pb, amp_set;
  
  printf(BOLDBLUE "\n Play Back Amplitudes for FADC250 in Slot %d\n\n" RESET, slot);
  
  for(bd_ch = 0; bd_ch < MAX_FADC250_CH; bd_ch++){

    amp_set = 0;

    for(samp_pb = 0; samp_pb < MAX_PB_SAMP; samp_pb++){
      if(fa250[slot].amp_pb[bd_ch][samp_pb] != 100) amp_set = 1;
    }

    if(amp_set == 1){
      printf(BOLDBLUE "CH %3d " RESET, bd_ch);
      for(samp_pb = 0; samp_pb < MAX_PB_SAMP; samp_pb++){
        printf("  %5d ", fa250[slot].amp_pb[bd_ch][samp_pb]);
      }
      
      printf("\n");
    }

  }

  printf("\n");
  
 
}

void print_fadc250_pb_all(){
  
  int bd_ch, samp_pb, bd_slot;

  int use_bd, amp_set;

  printf(BOLDBLUE "\n Play Back Amplitudes for FADC250 in Slot \n" RESET);
  
  for(bd_slot = 3; bd_slot <  NBD_FADC250; bd_slot++){

    use_bd = 0;

    for(bd_ch = 0; bd_ch < MAX_FADC250_CH; bd_ch++){
            
      for(samp_pb = 0; samp_pb < MAX_PB_SAMP; samp_pb++){
        if(fa250[bd_slot].amp_pb[bd_ch][samp_pb] != 100){
          use_bd = 1;
          break;
        }
      }

      if(use_bd == 1) break;
      
    }

    if(use_bd == 1)  printf(BOLDBLUE "\n\n SLOT %d \n\n" RESET, bd_slot);




    //  for(bd_slot = 3; bd_slot <  NBD_FADC250; bd_slot++){
    
    //    printf(BOLDBLUE "\n\n SLOT %d \n\n" RESET, bd_slot);

    for(bd_ch = 0; bd_ch < MAX_FADC250_CH; bd_ch++){
      
      amp_set = 0;
      
      for(samp_pb = 0; samp_pb < MAX_PB_SAMP; samp_pb++){
        if(fa250[bd_slot].amp_pb[bd_ch][samp_pb] != 100) amp_set = 1;
      }
      
      if(amp_set == 1){
        
        printf(BOLDBLUE "CH %3d " RESET, bd_ch);
        for(samp_pb = 0; samp_pb < MAX_PB_SAMP; samp_pb++){
          printf("  %5d ", fa250[bd_slot].amp_pb[bd_ch][samp_pb]);
        }
        
        printf("\n");
      }
      
    }
  }

  printf("\n");
  
 
}




void print_fadc250_conf(int slot){

  int ch;


  if(fa250[slot].group <= 0) 
    printf(" print_fadc250_conf: FADC250 in slot %d is not used in the configuration \n", slot);

  printf("\n===================================================\n");

  printf("Slot  =  %d \n", slot);
  printf("Used  =  %d \n", fa250[slot].group);
  printf("Processing firmware  =  0x%x \n", fa250[slot].f_rev);
  printf("Control    firmware  =  0x%x \n", fa250[slot].b_rev); 
  printf(" FADC250_TYPE        =  0x%x \n", fa250[slot].b_ID); 
  printf(" FADC250_SN          =  %s \n", fa250[slot].SerNum); 
  printf(" FADC250_MODE      =  %d  \n", fa250[slot].mode); 
  printf(" FADC250_W_OFFSET  =  %d  \n", fa250[slot].winOffset); 
  printf(" FADC250_W_WIDTH   =  %d  \n", fa250[slot].winWidth); 
  printf(" FADC250_NSB       =  %d  \n", fa250[slot].nsb); 
  printf(" FADC250_NSA       =  %d  \n", fa250[slot].nsa); 
  printf(" FADC250_NPEAK     =  %d  \n", fa250[slot].npeak);
  printf(" FADC250_NSAT      =  %d  \n", fa250[slot].nsat);
  printf(" \n");
  printf(" FADC250_NPED      =  %d  \n", fa250[slot].nped);
  printf(" FADC250_MAX_PED   =  %d  \n", fa250[slot].max_ped);
  printf(" \n");
  printf(" FADC250_BUSY      =  %d  \n", fa250[slot].busy);
  printf(" FADC250_STOP      =  %d  \n", fa250[slot].stop);
  printf(" \n");
  printf(" FADC250_FORMAT    =  %d  \n", fa250[slot].data_format);
  printf(" \n");
  printf(" FADC250_ADC_MASK  =  0x%x  \n", fa250[slot].chDisMask);
  printf(" \n");

  printf(" FADC250_DAC  =  ");
  for(ch = 0; ch <  MAX_FADC250_CH; ch++)
    printf("  %d", fa250[slot].dac[ch]);
  printf("\n");


  printf(" FADC250_READ_THR  =  ");
  for(ch = 0; ch <  MAX_FADC250_CH; ch++)
    printf("  %d", fa250[slot].read_thr[ch]);
  printf("\n");
  

  printf("\n");

  printf(" FADC250_TRIG_THR     =  %d  \n",   fa250[slot].trig_thr);
  printf(" FADC250_TRIG_NSB     =  %d  \n",   fa250[slot].trig_nsb);
  printf(" FADC250_TRIG_NSA     =  %d  \n",   fa250[slot].trig_nsa);
  printf(" FADC250_TRIG_NSAT    =  %d  \n",   fa250[slot].trig_nsat);
  printf(" FADC250_TRIG_MASK    =  0x%x  \n", fa250[slot].trigMask);

  printf(" FADC250_TRIG_BL  =  ");
  for(ch = 0; ch <  MAX_FADC250_CH; ch++)
    printf("  %d", fa250[slot].trig_bl[ch]);
  printf("\n");

  printf("\n");

  printf(" FADC250_COM_DIR    =  %s  \n", fa250[slot].conf_common_dir);
  printf(" FADC250_COM_VER    =  %s  \n", fa250[slot].conf_common_ver);

  printf(" FADC250_USER_DIR   =  %s  \n", fa250[slot].conf_user_dir);
  printf(" FADC250_USER_VER   =  %s  \n", fa250[slot].conf_user_ver);

  printf(" PLAY_BACK_DIR   =  %s  \n", fa250[slot].conf_pb_dir);
  printf(" PLAY_BACK_VER   =  %s  \n", fa250[slot].conf_pb_ver);

  printf(" Block Level        =  %d  \n", fa250[slot].level);

  printf("\n");
  printf("           MONITORING          \n");
  printf(" FADC250_MON_NPED      =  %d  \n",   fa250[slot].mon_nped);
  printf(" FADC250_MON_MAX_PED   =  %d  \n",   fa250[slot].mon_max_ped);

}


void print_fadc125_conf(int slot){

  int ch;
  
  if(fa125[slot].group <= 0){
    printf("\n");
    printf(" print_fadc125_conf: FADC125 in slot %d is not used in the configuration \n", slot);
  }
  
  printf("========================================================================\n");

  printf("Slot  =  %d \n", slot);
  printf("Used  =  %d \n", fa125[slot].group);
  printf("Firmware    version  =  0x%x \n", fa125[slot].f_rev);
  printf("FrontEnd    version  =  0x%x \n", fa125[slot].b_rev); 
  printf("Processing  version  =  0x%x \n", fa125[slot].b_rev); 

  printf(" FADC125_TYPE          =  0x%x \n", fa125[slot].b_ID); 
  printf(" FADC125_SN            =  %s  \n",   fa125[slot].SerNum); 
  printf(" FADC125_MODE          =  %d  \n",    fa125[slot].mode); 
  printf(" FADC125_W_OFFSET      =  %d  \n",    fa125[slot].winOffset); 
  printf(" FADC125_W_WIDTH       =  %d  \n",    fa125[slot].winWidth);
  printf(" FADC125_IE            =  %d  \n",    fa125[slot].IE); 
  printf(" FADC125_PG            =  %d  \n",    fa125[slot].PG); 
  printf(" FADC125_NPEAK         =  %d  \n",    fa125[slot].npeak);
  printf(" FADC125_P1            =  %d  \n",    fa125[slot].P1);
  printf(" FADC125_P2            =  %d  \n",    fa125[slot].P2);
  
  printf("\n");

  printf(" FADC125_TH            =  %d  \n",    fa125[slot].TH);
  printf(" FADC125_TL            =  %d  \n",    fa125[slot].TL);
 
  printf("\n");

  printf(" FADC125_IBIT          =  %d  \n",    fa125[slot].IBIT);
  printf(" FADC125_ABIT          =  %d  \n",    fa125[slot].ABIT);
  printf(" FADC125_PBIT          =  %d  \n",    fa125[slot].PBIT);


  printf("\n");
  printf(" FADC125_BUSY          =  %d  \n",    fa125[slot].busy);
  printf(" FADC125_STOP          =  %d  \n",    fa125[slot].stop);
  printf("\n");
  printf(" FADC125_FORMAT        =  %d  \n",    fa125[slot].data_format);

  //  printf(" FADC125_NSB  Not used     =  %d  \n",    fa125[slot].nsb); 
  //  printf(" FADC125_NSA  Not used     =  %d  \n",    fa125[slot].nsa); 


  printf("\n");
  printf(" FADC125_THR_FACT     =  %f  \n",    fa125[slot].thr_fact);

  printf("\n");
  
  printf("\n FADC125_DAC_CH_00_17  ");
  for(ch = 0; ch < 18; ch++) printf(" %d  ", fa125[slot].dac[ch]);
  
  printf("\n FADC125_DAC_CH_18_35  ");
  for(ch = 18; ch < 36; ch++) printf(" %d  ", fa125[slot].dac[ch]);

  printf("\n FADC125_DAC_CH_36_53  ");
  for(ch = 36; ch < 54; ch++) printf(" %d  ", fa125[slot].dac[ch]);

  printf("\n FADC125_DAC_CH_54_71  ");
  for(ch = 54; ch < 72; ch++) printf(" %d  ", fa125[slot].dac[ch]);

  printf("\n");
  printf("\n");

  
  printf(" FADC125_THR_CH_00_17  ");
  for(ch = 0; ch < 18; ch++) printf(" %d  ",  fa125[slot].read_thr[ch]);
  
  printf("\n FADC125_THR_CH_18_35  ");
  for(ch = 18; ch < 36; ch++) printf(" %d  ", fa125[slot].read_thr[ch]);
  
  printf("\n FADC125_THR_CH_36_53  ");
  for(ch = 36; ch < 54; ch++) printf(" %d  ", fa125[slot].read_thr[ch]);
  
  printf("\n FADC125_THR_CH_54_71  ");
  for(ch = 54; ch < 72; ch++) printf(" %d  ", fa125[slot].read_thr[ch]);
  

  printf("\n");
  printf("\n");

  
  printf(" FADC125_BL_CH_00_17  ");
  for(ch = 0; ch < 18; ch++)  printf(" %.1f  ", fa125[slot].bl[ch]);
  
  printf("\n FADC125_BL_CH_18_35  ");
  for(ch = 18; ch < 36; ch++) printf(" %.1f  ", fa125[slot].bl[ch]);
  
  printf("\n FADC125_BL_CH_36_53  ");
  for(ch = 36; ch < 54; ch++) printf(" %.1f  ", fa125[slot].bl[ch]);
  
  printf("\n FADC125_BL_CH_54_71  ");
  for(ch = 54; ch < 72; ch++) printf(" %.1f  ", fa125[slot].bl[ch]);


  printf("\n");
  printf("\n");

  
  printf(" FADC125_SIG_CH_00_17  ");
  for(ch = 0; ch < 18; ch++)  printf(" %5.1f  ", fa125[slot].sig[ch]);
  
  printf("\n FADC125_SIG_CH_18_35  ");
  for(ch = 18; ch < 36; ch++) printf(" %5.1f  ", fa125[slot].sig[ch]);
  
  printf("\n FADC125_SIG_CH_36_53  ");
  for(ch = 36; ch < 54; ch++) printf(" %5.1f  ", fa125[slot].sig[ch]);
  
  printf("\n FADC125_SIG_CH_54_71  ");
  for(ch = 54; ch < 72; ch++) printf(" %5.1f  ", fa125[slot].sig[ch]);




  printf("\n");
  
  printf("\n FADC125_MASK_00_17  ");
  for(ch = 0; ch < 18; ch++)  
    if(  (fa125[slot].mask[0]  &  (1 << ch)) == 0)  printf(" 0"); else printf(" 1");
    
  printf("\n FADC125_MASK_18_35  ");
  for(ch = 0; ch < 18; ch++) 
    if(  (fa125[slot].mask[1]  &  (1 << ch)) == 0)  printf(" 0"); else printf(" 1");
  
  printf("\n FADC125_MASK_36_53  ");
  for(ch = 0; ch < 18; ch++) 
    if(  (fa125[slot].mask[2]  &  (1 << ch)) == 0)  printf(" 0"); else printf(" 1");
  
  printf("\n FADC125_MASK_54_71  ");
  for(ch = 0; ch < 18; ch++) 
    if(  (fa125[slot].mask[3]  &  (1 << ch)) == 0)  printf(" 0"); else printf(" 1");



  printf("\n");

  printf("\n");


  printf(" FADC125_CH_DIS:   0x%x   0x%x   0x%x \n", fa125[slot].ch_disable[0], fa125[slot].ch_disable[1], 
	 fa125[slot].ch_disable[2]);




  printf(" FADC125_COM_DIR    =  %s  \n",  fa125[slot].conf_common_dir);
  printf(" FADC125_COM_VER    =  %s  \n",  fa125[slot].conf_common_ver);

  printf(" FADC125_USER_DIR   =  %s  \n",  fa125[slot].conf_user_dir);
  printf(" FADC125_USER_VER   =  %s  \n",  fa125[slot].conf_user_ver);

}



void print_ti_conf(){

  int opt_port, port;

  printf("\n========================================================================\n");
  printf("==================    TI CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");


  printf("TI_MASTER:  %d \n", ti_bd.ti_master );
  printf("\n");


  printf("TI_MASTER_TRIG:  %d \n", ti_bd.ti_master_trig );
  printf("\n");

  printf(" TI_SOFT_TRIG: \n");
  printf("    Trigger type (type 1 or 2 for playback):          %d \n", ti_bd.ti_soft_trig[0] );
  printf("    Number of events to trigger:                      %d \n", ti_bd.ti_soft_trig[1] );
  printf("    Period multiplier (depends on range 0-0x7FFF):    %d \n", ti_bd.ti_soft_trig[2] );
  printf("    Range:                                            %d \n", ti_bd.ti_soft_trig[3] );
  printf("\n");


  printf("TI_FP_INPUTS     =  ");
  
  for(port = 0; port < 16; port++){
    if( ti_bd.ti_fp_inputs & (1 << port)) printf("  %d", port+1); 
  }
  printf("\n");
  printf("    0x%x \n",ti_bd.ti_fp_inputs);




  printf("TI_FIBER_EN: \n");
  for(opt_port = 2; opt_port < 7; opt_port++)
    if(ti_bd.ti_fiber_en & (1 << (opt_port - 1)))
      printf("  %d ", opt_port);
  printf("\n");

  printf("\n");
  printf("TI_FIBER_LATENCY_OFFSET:  0x%x \n", ti_bd.fiber_latency);


}


void print_ts_conf(){

  int slot;

  int port, lane, rule;

  printf("\n========================================================================\n");
  printf("==================    TS CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");

  printf("TS_TD_SLOTS     =  ");

  for(slot = 3; slot < 21; slot++){
    if( ts_bd.ts_td_slots & (1 << slot)) printf("  %d", slot); 
  }
  printf("\n");
  printf("    0x%x \n",ts_bd.ts_td_slots);
  printf("\n");


  printf("TS_FP_INPUTS: \n ");
  
  for(port = 0; port < 16; port++){
    if( ts_bd.ts_fp_inputs & (1 << port)){
      printf("Input =   %d    Delay  = %d \n", port+1, ts_bd.ts_fp_delay[port]); 
    }
  }
  printf("\n");
  printf("    0x%x \n",ts_bd.ts_fp_inputs);
  printf("\n");


  
  printf("TRIG_TYPE    =  %d\n", ts_bd.trig_type);

  printf("BLOCKLEVEL      =  %d\n", ts_bd.blocklevel);
  printf("BUFFERLEVEL     =  %d\n", ts_bd.bufferlevel);

  printf("TS_SYNC_INT     =  %d\n", ts_bd.ts_sync_int);
  printf("TS_SYNC_OPT     =  %d\n", ts_bd.ts_sync_opt);


  printf("TS_COIN_WIND    = %d\n", ts_bd.ts_coin_wind);


  for(rule = 0; rule < 4; rule++){
    printf("TS_TRIG_HOLD: Rule  %d,   value =  %d,  range =  %d\n", rule, ts_bd.ts_trig_hold[rule][0], ts_bd.ts_trig_hold[rule][1]);
  }

  
  printf("TS_SOFT_TRIG: \n");
  printf("    Trigger type (type 1 or 2 for playback):          %d \n", ts_bd.ts_soft_trig[0] );
  printf("    Number of events to trigger:                      %d \n", ts_bd.ts_soft_trig[1] );
  printf("    Period multiplier (depends on range 0-0x7FFF):    %d \n", ts_bd.ts_soft_trig[2] );
  printf("    Range:                                            %d \n", ts_bd.ts_soft_trig[3] );

  printf("\n");
  printf("TS_GTP_PRES: \n");
  for(lane = 0; lane < 32; lane++){
    if(ts_bd.gtp_pres[lane] > 0){
      printf("Lane:   %d     Factor:   %d \n", lane, ts_bd.gtp_pres[lane] );
    }
  }
  

}


void print_le_conf(){

  int slot, ch;


  printf("\n========================================================================\n");
  printf("==================    LE CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");

  
  for(slot = 3; slot < NBD_DSC2; slot++){

    printf(" Slot = %d \n",slot);

    printf(" TDCWidth  = %d    TRGWidth = %d  \n", dsc[slot].TDCWidth,  dsc[slot].TRGWidth);

    
    printf(" TDCThreshold   =  "); 
    for(ch = 0; ch < MAX_DSC2_CH; ch++){
      printf("  %d", dsc[slot].TDCThreshold[ch]);
    }
    printf(" \n"); 

    printf(" TRGThreshold   =  "); 
    for(ch = 0; ch < MAX_DSC2_CH; ch++){
      printf("  %d", dsc[slot].TRGThreshold[ch]);
    }
    printf(" \n"); 

    printf(" \n"); 

  }

}


void print_caen1290_conf(){

  int slot, chip, tap;


  printf("\n========================================================================\n");
  printf("==================    CAEN 1290 TDC  CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");

  
  for(slot = 3; slot < NBD_TDC1290; slot++){

    printf(" Slot = %d \n",slot);

    printf(" TDC1290_W_WIDTH   = %d  \n", caen1290[slot].window_width);
    printf(" TDC1290_W_OFFSET  = %d  \n", caen1290[slot].window_offset);
    printf(" TDC1290_W_EXTRA   = %d  \n", caen1290[slot].window_extra);
    printf(" TDC1290_W_REJECT  = %d  \n", caen1290[slot].window_reject);

    printf("\n");

    printf(" TDC1290_BLT_EVENTS  =  %d \n",   caen1290[slot].blt_Events);
    printf(" TDC1290_N_HITS      =  %d \n",   caen1290[slot].n_Hits);
    printf(" TDC1290_ALMOSTFULL  =  %d \n",   caen1290[slot].almostFullLevel);
    printf(" TDC1290_OUT_PROG    =  %d \n",   caen1290[slot].outProgControl);

    printf("\n");

    printf(" TDC1290_A24_A32     =  %d \n",   caen1290[slot].a24_a32);
    printf(" TDC1290_SNGL_BLT    =  %d \n",   caen1290[slot].sngl_blt_mblt);
    printf(" TDC1290_SST_RATE    =  %d \n",   caen1290[slot].sst_rate);
    printf(" TDC1290_BERR_FIFO   =  %d \n",   caen1290[slot].berr_fifo);
    printf(" TDC1290_EDGE        =  %d \n",   caen1290[slot].edge);
    printf(" TDC1290_INL         =  %d \n",   caen1290[slot].inl);

    printf(" \n"); 


    printf(" TDC1290_RC     SLOT      CHIP    TAP1    TAP2    TAP3    TAP4\n");
    for(chip = 0; chip < 4; chip++){      
      printf("                %2d        %2d      %2d      %2d      %2d       %2d\n",  slot,  chip,  
	       caen1290[slot].rc_adjust[chip][0], caen1290[slot].rc_adjust[chip][1], 
	       caen1290[slot].rc_adjust[chip][2], caen1290[slot].rc_adjust[chip][3] );	
    }
    
  }
  
}


void print_f1_conf(){

  int slot;


  printf("\n========================================================================\n");
  printf("==================     F1TDC  CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");

  
  for(slot = 3; slot < NBD_F1TDC; slot++){

    printf(" Slot = %d \n",slot);

    printf(" F1TDC_VERSION   = %d  \n",  f1[slot].version);
    printf(" F1TDC_CLOCK     = %d  \n",  f1[slot].clock);
    printf("\n");
    printf(" F1TDC_WINDOW    = %f  \n",  f1[slot].window);
    printf(" F1TDC_BIN_SIZE  = %f  \n",  f1[slot].bin_size);
    printf(" F1TDC_LATENCY   = %f  \n",  f1[slot].latency);

    printf(" Global Init     = %d  \n",  f1[slot].global);

    printf("\n");

  }

}


void fa250_set_block_level(int level){

  int bd_slot;

  for(bd_slot = 3; bd_slot < NBD_FADC250; bd_slot++){    
    fa250[bd_slot].level  =  level;
  }
}

void fa125_set_block_level(int level){

  int bd_slot;

  for(bd_slot = 3; bd_slot < NBD_FADC125; bd_slot++){    
    fa125[bd_slot].level  =  level;
  }
}


void init_all(){

  int bd_slot, bd_ch, range, ntr, lane, input;
  int chip, tap, samp_pb;

  // FADC250
  for(bd_slot = 3; bd_slot < NBD_FADC250; bd_slot++){
    
    fa250[bd_slot].group      = -1;
    fa250[bd_slot].mode       =  1;
    fa250[bd_slot].winOffset  =  500;
    fa250[bd_slot].winWidth   =  10;
    fa250[bd_slot].nsb    =  3;
    fa250[bd_slot].nsa    =  5;
    fa250[bd_slot].npeak  =  1; 
    
    fa250[bd_slot].nsat      =   1; 
    fa250[bd_slot].nped      =   4; 
    fa250[bd_slot].max_ped   =   512; 

    fa250[bd_slot].level   =   1; 
    
    fa250[bd_slot].trig_thr    =  4095;
    
    fa250[bd_slot].trig_nsb    =   3;
    fa250[bd_slot].trig_nsa    =   5;

    fa250[bd_slot].trig_nsat   =   1;

    fa250[bd_slot].busy        =   2;
    fa250[bd_slot].stop        =   9;

    fa250[bd_slot].data_format  =  0;
    
    fa250[bd_slot].trigMask    =   0;
    fa250[bd_slot].chDisMask   =   0;

    for(bd_ch = 0; bd_ch < MAX_FADC250_CH; bd_ch++){
      fa250[bd_slot].dac[bd_ch]       =  3200;
      fa250[bd_slot].read_thr[bd_ch]  =  4095;
      fa250[bd_slot].trig_bl[bd_ch]   =  4095;	

      for(samp_pb = 0; samp_pb < MAX_PB_SAMP; samp_pb++){
        fa250[bd_slot].amp_pb[bd_ch][samp_pb] = 100;
      }
    }

    fa250[bd_slot].mon_nped     =  4;
    fa250[bd_slot].mon_max_ped  =  1023;


  }


  // DSC
  for(bd_slot = 3; bd_slot < NBD_DSC2; bd_slot++){

    dsc[bd_slot].group       =  -1;

    dsc[bd_slot].TDCChMask   =   0;
    dsc[bd_slot].TRGChMask   =   0;
    dsc[bd_slot].TDCWidth    =   40;
    dsc[bd_slot].TRGWidth    =   40;
    
    for(bd_ch = 0; bd_ch < MAX_DSC2_CH; bd_ch++){
      
      dsc[bd_slot].TDCThreshold[bd_ch] = 1023;
      dsc[bd_slot].TRGThreshold[bd_ch] = 1023;
      
    }
  }


  // FADC125
  for(bd_slot = 3; bd_slot < NBD_FADC125; bd_slot++){

    fa125[bd_slot].group     = -1;
    fa125[bd_slot].mode      =  1;
    fa125[bd_slot].winOffset =  500;
    fa125[bd_slot].winWidth  =  100;
    fa125[bd_slot].nsb       =  3; 
    fa125[bd_slot].nsa       =  7;
    fa125[bd_slot].npeak     =  1;
    
    fa125[bd_slot].IE        =  80;
    fa125[bd_slot].PG        =  4;
    
    fa125[bd_slot].P1        =  4;
    fa125[bd_slot].P2        =  4;

    fa125[bd_slot].TH        =  100;
    fa125[bd_slot].TL        =  25;
    
    fa125[bd_slot].IBIT      =  4;
    fa125[bd_slot].ABIT      =  3;
    fa125[bd_slot].PBIT      =  0;
    
    
    fa125[bd_slot].thr_fact  =  0;
    
    fa125[bd_slot].busy      =  2;
    fa125[bd_slot].stop      =  6;

    fa125[bd_slot].data_format  =  0;

    for(range = 0; range < 3; range++) fa125[bd_slot].ch_disable[range] = 0;
    
    for(bd_ch = 0; bd_ch < MAX_FADC125_CH; bd_ch++){
      fa125[bd_slot].dac[bd_ch]      =  0x9000;
      fa125[bd_slot].read_thr[bd_ch] =  0xFFFF;
      
      fa125[bd_slot].sig[bd_ch]  =  0;
      fa125[bd_slot].bl[bd_ch]   =  0;          
      
    }
  }
  
  // F1TDC
  for(bd_slot = 3; bd_slot < NBD_F1TDC; bd_slot++){
    
    f1[bd_slot].bin_size  =   0.056;
    f1[bd_slot].latency   =   900;
    f1[bd_slot].window    =   100;
    f1[bd_slot].version   =   2;
    f1[bd_slot].global    =  -1;
  }

  // CAEN1290  TDC
  for(bd_slot = 3; bd_slot < NBD_TDC1290; bd_slot++){
      
    caen1290[bd_slot].window_width     =  100;
    caen1290[bd_slot].window_offset    =  2000;
    caen1290[bd_slot].window_extra     =  25;
    caen1290[bd_slot].window_reject    =  50;
    
    caen1290[bd_slot].blt_Events       =  1;
    caen1290[bd_slot].n_Hits           =  64;
    caen1290[bd_slot].almostFullLevel  =  16384;
    caen1290[bd_slot].outProgControl   =  2;
    
    caen1290[bd_slot].a24_a32          =  2;
    caen1290[bd_slot].sngl_blt_mblt    =  3;
    caen1290[bd_slot].sst_rate         =  0;
    caen1290[bd_slot].berr_fifo        =  1;
    caen1290[bd_slot].edge             =  2;    

    caen1290[bd_slot].inl              =  1;    

    for(chip = 0; chip < 4; chip++){
      for(tap = 0; tap < 4; tap++){
	caen1290[bd_slot].rc_adjust[chip][tap]  =  -1;
      }
    }
    
  }

  // CTP
  ctp.BCALThreshold      =   0;
  ctp.BCALWindowWidth    =   0;
  ctp.FinalSumThreshold  =   0;
  ctp.HitWindowWidth     =   0;

  ctp.used               =   1;

  // SSP
  //  for(bd_slot = 3; bd_slot < NBD_SSP; bd_slot++){
  //    ssp[bd_slot].slot     = 0;
  //    ssp[bd_slot].fiber_en = 0;
  //    ssp[bd_slot].sum_enable = 0;
  //  }

  ssp_init();

  // GTP EQ
  for(ntr = 0; ntr < MAX_EQ; ntr++){
    gtp_eq[ntr].enable    =  0;
    gtp_eq[ntr].delay     =  0;
    gtp_eq[ntr].int_width =  0;
  }

  // GTP TRIG
  for(ntr = 0; ntr < MAX_TRIG; ntr++){
    gtp_trig[ntr].gtp_lane   = -1;
    gtp_trig[ntr].ecal_en    =  0;
    gtp_trig[ntr].fcal_en    =  0;
    gtp_trig[ntr].bcal_en    =  0;
    gtp_trig[ntr].en_thr     =  0;
    gtp_trig[ntr].latency    =  0;
    gtp_trig[ntr].nhit       =  0;
    gtp_trig[ntr].width      =  0;
    gtp_trig[ntr].ecal_min   =  0;
    gtp_trig[ntr].ecal_max   =  65535;
    gtp_trig[ntr].fcal_min   =  0;
    gtp_trig[ntr].fcal_max   =  65535;
    gtp_trig[ntr].bcal_min   =  0;
    gtp_trig[ntr].bcal_max   =  65535;
    gtp_trig[ntr].mask  =  0xFFFFFFFF;
  }

  // TS
  for(lane = 0; lane < 32; lane++){
    ts_bd.gtp_pres[lane] = 0;
  }
  ts_bd.ts_coin_wind = 5;

  // Set large default value for rule 1
  ts_bd.ts_trig_hold[0][0]  =  62;
  ts_bd.ts_trig_hold[0][1]  =  1;

  ts_bd.ts_trig_hold[1][0]  =  2;
  ts_bd.ts_trig_hold[1][1]  =  0;

  ts_bd.ts_trig_hold[2][0]  =  3;
  ts_bd.ts_trig_hold[2][1]  =  0;

  // rule 4, 2.048 usec
  ts_bd.ts_trig_hold[3][0]  =  32;
  ts_bd.ts_trig_hold[3][1]  =  0;


  ts_bd.ts_fp_inputs = 0;

  ts_bd.ts_sync_int  =  0;
  ts_bd.ts_sync_opt  =  0;

  for(input = 0; input < 32;  input++){
    ts_bd.ts_fp_delay[input] = 0;
  }

}

void print_ctp_conf(){

  printf("\n========================================================================\n");
  printf("==================   CTP  CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");

  printf("\n");
  printf(" CTP_BCAL_THR  = %d \n ", ctp.BCALThreshold);
  printf("\n"); 
  printf(" CTP_THR  = %d \n ", ctp.FinalSumThreshold);
  printf("\n");
  printf(" CTP_BCAL_WID  = %d \n ", ctp.BCALWindowWidth);
  printf("\n");
  printf(" CTP_HIT_WID  = %d \n ", ctp.HitWindowWidth);
  printf("\n");

}


void print_ssp_conf(){

  int slot, fiber, asic, ch;

  printf("\n========================================================================\n");
  printf("==================   SSP  CONFIGURATION    ==============================\n");
  printf("========================================================================\n\n");

  printf(" \n ");

  for(slot = 3; slot < NBD_SSP; slot++){    

    if(ssp[slot].group == 1){
      printf("SLOT  %d  \n\n", slot); 
      printf("SSP_W_OFFSET  =  %d  \n", ssp[slot].window_offset); 
      printf("SSP_W_WIDTH   =  %d  \n", ssp[slot].window_width); 
      printf("\n");
      printf("SSP_PULSER   =  %d  \n", ssp[slot].pulser_freq); 

      printf("Fiber found in config file 0x%X  \n",ssp[slot].dirc.fiber_in_config);


      printf("\n");
      printf("                  DAC0                    DAC1                          Global0                                Global1                 CTEST_EN  CTEST_DAC  W_WIDTH   W_OFFSET  \n\n");  


      for(fiber = 0; fiber < 32; fiber++){
        printf("FIBER %2d  ",fiber);
        for(asic = 0; asic < 3; asic++){
          printf("%5d  ", ssp[slot].dirc.fiber[fiber].chip[asic].DAC0);
        }
        printf("   ");
        for(asic = 0; asic < 3; asic++){
          printf("%5d  ", ssp[slot].dirc.fiber[fiber].chip[asic].DAC1);
        }
        printf("   ");
        for(asic = 0; asic < 3; asic++){
          printf("0x%08X  ", ssp[slot].dirc.fiber[fiber].chip[asic].Global0.val);
        }
        printf("   ");
        for(asic = 0; asic < 3; asic++){
          printf("0x%08X  ", ssp[slot].dirc.fiber[fiber].chip[asic].Global1.val);
        }

        printf("   %4d   ", ssp[slot].dirc.fiber[fiber].ctest_enable); 
        printf("   %4d   ", ssp[slot].dirc.fiber[fiber].ctest_dac); 

        printf("    %5d  ", ssp[slot].dirc.fiber[fiber].window_width);        
        printf("    %5d  ", ssp[slot].dirc.fiber[fiber].window_offset);

        printf("\n");        

      }  

      printf("\n");


      /* Print TDC mask and CTest[0]  */
      printf("\n");
      printf("           T DC_EN_MASK[0]  TDC_EN_MASK[1]  TDC_EN_MASK[2]  TDC_EN_MASK[3]  TDC_EN_MASK[4]  TDC_EN_MASK[4]                  CTest      \n\n"); 

      for(fiber = 0; fiber < 32; fiber++){
        printf("FIBER %2d  ",fiber);
        
        printf("    0x%08X  ", ssp[slot].dirc.fiber[fiber].tdc_enable_mask[0]); 
        printf("    0x%08X  ", ssp[slot].dirc.fiber[fiber].tdc_enable_mask[1]); 
        printf("    0x%08X  ", ssp[slot].dirc.fiber[fiber].tdc_enable_mask[2]); 
        printf("    0x%08X  ", ssp[slot].dirc.fiber[fiber].tdc_enable_mask[3]); 
        printf("    0x%08X  ", ssp[slot].dirc.fiber[fiber].tdc_enable_mask[4]); 
        printf("    0x%08X  ", ssp[slot].dirc.fiber[fiber].tdc_enable_mask[5]); 
        
        for(asic = 0; asic < 3; asic++){
          printf("  0x%08X  ",ssp[slot].dirc.fiber[fiber].chip[asic].CTest[0] ); 
        }
        
        printf("\n");
        
      }        
      


#if 1
      /* Print gains */

      printf("\n");
      for(fiber = 0; fiber < 32; fiber++){

        for(asic = 0; asic < 3; asic++){

          printf("                  FIBER  =  %d     ASIC  =  %d",fiber, asic);
          
          for(ch = 0; ch < 64; ch++){
        
            if((ch % 16) == 0) printf("\n");
            if(ch ==  0) printf("SSP_MAROC_REG_GAIN_0_15   ");
            if(ch == 16) printf("SSP_MAROC_REG_GAIN_16_31  ");
            if(ch == 32) printf("SSP_MAROC_REG_GAIN_32_47  ");
            if(ch == 48) printf("SSP_MAROC_REG_GAIN_48_63  ");
            printf("%4d  ", ssp[slot].dirc.fiber[fiber].chip[asic].Gain[ch]);
            
          }
          
          printf("\n");
          
        }        
        
      }
      



      //      printf("\n");        
#endif     
 
      
    }
  }
  
  printf("cmd_fsu       =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.cmd_fsu);
  printf("cmd_ss        =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.cmd_ss);
  printf("cmd_fsb       =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.cmd_fsb);
  printf("swb_buf_250f  =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.swb_buf_250f);
  printf("swb_buf_500f  =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.swb_buf_500f);
  printf("swb_buf_1p    =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.swb_buf_1p);
  printf("swb_buf_2p    =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.swb_buf_2p);
  printf("ONOFF_ss      =  %d\n",ssp[3].dirc.fiber[0].chip[0].Global0.bits.ONOFF_ss);

  printf("\n");
  printf("\n");

}



void print_gtp_conf(){

  int ii;

  printf("\n========================================================================\n");
  printf("==================   GTP  CONFIGURATION    =============================\n");
  printf("========================================================================\n\n");

  int ntrig = 0;

  printf(" \n ");

  printf("GTP EQUATIONS  \n");

  printf(" \n ");

  printf("Number   TYPE    DELAY   INT_WIDTH   ENABLE   \n");

  printf(" \n ");

  for(ii = 0; ii < MAX_EQ; ii++) {
    if(gtp_eq[ii].enable ==  1){
      ntrig++;
      printf("%2d    %7s     %3d       %4d       %2d     \n ",ntrig, gtp_eq[ii].type,
	     gtp_eq[ii].delay, gtp_eq[ii].int_width, gtp_eq[ii].enable );
    }
  }
  
  printf("\n");
  printf("\n");


  ntrig = 0;

  printf("Number    TYPE    LATENCY   WIDTH    FCAL_E    BCAL_E    EN_THR    NHIT     LANE      FMIN      FMAX          BMIN       BMAX      MASK \n");


  printf(" \n ");

  for(ii = 0; ii < MAX_TRIG; ii++) {
    if(gtp_trig[ii].gtp_lane >= 0){
      ntrig++;
      printf("%2d   %7s     %4d      %3d      %5d     %5d      %5d     %2d      %2d      %6d      %6d      %6d      %6d     %8X    \n ", ntrig, 
	     gtp_trig[ii].type, gtp_trig[ii].latency, gtp_trig[ii].width, gtp_trig[ii].fcal_en, 
	     gtp_trig[ii].bcal_en, gtp_trig[ii].en_thr, gtp_trig[ii].nhit, gtp_trig[ii].gtp_lane, 
	     gtp_trig[ii].fcal_min, gtp_trig[ii].fcal_max, 
	     gtp_trig[ii].bcal_min, gtp_trig[ii].bcal_max, 
	     gtp_trig[ii].mask);
    }
  }
    
  printf("\n");
  printf("\n");
  

  ntrig = 0;

  printf("Number    TYPE    LATENCY   WIDTH    ECAL_E    FCAL_E    BCAL_E    EN_THR    LANE      EMIN      EMAX     FMIN      FMAX      BMIN       BMAX \n");


  printf(" \n ");

  for(ii = 0; ii < MAX_TRIG; ii++) {
    if(gtp_trig[ii].gtp_lane >= 0){
      ntrig++;
      printf("%2d   %7s     %4d      %3d      %5d     %5d      %5d     %5d      %2d      %6d      %6d      %6d      %6d     %6d      %6d    \n ", ntrig, 
	     gtp_trig[ii].type, gtp_trig[ii].latency, gtp_trig[ii].width, gtp_trig[ii].ecal_en, gtp_trig[ii].fcal_en, gtp_trig[ii].bcal_en, 
             gtp_trig[ii].en_thr, gtp_trig[ii].gtp_lane,  gtp_trig[ii].ecal_min, gtp_trig[ii].ecal_max, gtp_trig[ii].fcal_min, gtp_trig[ii].fcal_max, 
	     gtp_trig[ii].bcal_min, gtp_trig[ii].bcal_max);
    }
  }
    
  printf("\n");
  printf("\n");


}



int check_baseline(int *fail_baseline){

  int debug_base = 1;

  //  int nevent = 1000;
  int nevent = 10;

  int iboard;
  int slot = -1;
  int ii, ch, samp;

  int ret = -1;

  int slot_min = -1;
  int slot_max = -1;

  unsigned int  ped_buff[8]; 
  
  int ped_overflow[21][16];
  int ped_underflow[21][16];
  
  float  ped_all[21][16];
  int    ped_ent[21][16];
  int    ped_value[21][16][120];
  
  int ped_min = 80;
  int ped_max = 110;

  int status = 0;

  int nsamples = -1;

  if( strcmp(roc_name, "rocps1") == 0){
    slot_min = 3;
    slot_max = 20;
  }

  if( strcmp(roc_name, "rocps2") == 0){
    slot_min = 3;
    slot_max = 6;
  }

  if( strcmp(roc_name, "roctagh1") == 0){
    slot_min = 3;
    slot_max = 19;
  }
  
  if(strncmp(roc_name, "rocbcal", 7) == 0){
    slot_min = 3;
    slot_max = 16;
  }

  if(strncmp(roc_name, "rocecal", 7) == 0){
    slot_min = 3;
    slot_max = 20;
  }

  if(strncmp(roc_name, "rocfcal", 7) == 0){
    slot_min = 3;
    slot_max = 19;
  }


  memset(ped_buff,0,sizeof(ped_buff));

  memset(ped_overflow,0,sizeof(ped_overflow));
  memset(ped_underflow,0,sizeof(ped_underflow));

  memset(ped_all,0,sizeof(ped_all));
  memset(ped_ent,0,sizeof(ped_ent));
  memset(ped_value,0,sizeof(ped_value));


  if(NFADC_250 <= 0) return(-1);
  
  for(ii = 0; ii < nevent; ii++){
        
    for(iboard = 0; iboard < NFADC_250; iboard++){
      
      slot = fadcID[iboard];
      
      if( fa250[slot].mon_nped > 0){
        nsamples = fa250[slot].mon_nped;
      } else {

        printf(BOLDRED    "Wrong number of samples for pedestal monitoring  %d  \n" RESET, fa250[slot].mon_nped);        
        daLogMsg("ERROR", "Wrong number of samples for pedestal monitoring  %d  \n" RESET, fa250[slot].mon_nped);
        return(-1);
      }

      if(debug_base > 1)
          printf("Slot = %d\n ", slot);

      if( (slot >= slot_min) && (slot <= slot_max)){
        
        ret = faReadAllChannelSamples(slot, ped_buff);
        


        for(ch  = 0; ch < 8; ch++) {
          
	  int chan1 = 2*ch;
	  int chan2 = 2*ch + 1;

          unsigned int ped1 =   ( ped_buff[ch] & 0x3FFF ) / nsamples;
          unsigned int ped2 =  ((ped_buff[ch] & 0x3FFF0000) >> 16) / nsamples;
          
          //          if(slot == 3){
          //            printf("CH = %d Values = 0x%x \n",chan1, ped1);
          //          }


          if( (ped1 >= 80) && (ped1 < 120) ){
            ped_value[slot][chan1][ped1]++;
          } else if(ped1 >= 120){
	    ped_overflow[slot][chan1];
	  } else if(ped1 < 80){
	    ped_underflow[slot][chan1];
	  }
          
          if( (ped2 >= 80) && (ped2 < 120) ){
            ped_value[slot][chan2][ped2]++;
          } else if(ped2 >= 120){
	    ped_overflow[slot][chan2];
	  } else if(ped2 < 80){
	    ped_underflow[slot][chan2];
	  }

          ped_all[slot][chan1] += ped1;
          ped_all[slot][chan2] += ped2;

          if(ped1 > 0){
            ped_ent[slot][chan1]++;
          }
          if(ped2 > 0){
            ped_ent[slot][chan2]++;
          }


            if(debug_base > 1){
              printf("ch = %d   %d     ch = %d    %d  %d  \n", 2*ch, ( ped_buff[ch] & 0xFFFF ) , 2*ch+1, ((ped_buff[ch] & 0xFFFF0000) >> 16) , 
                     nsamples);
            }

	  ped_buff[ch] = 0; /* clear buffer */
	    
        }
         
      }

    }  // Loop over boards
    
  }    // Nevent
  


  for(iboard = 0; iboard < NFADC_250; iboard++){
        
    slot = fadcID[iboard];

    if( (slot >= slot_min) && (slot <= slot_max)){

      printf("Slot = %d\n\n ", slot);

      for(ch  = 0; ch < 16; ch++){
        
	// Find pedestal peak	
        
	int ped_peak  = 0;
        int peak_samp = 0;
	for(samp  = 80; samp < 120; samp++){
	  if(ped_value[slot][ch][samp] > ped_peak){
	    ped_peak = ped_value[slot][ch][samp];
            peak_samp = samp;
	  }
	}

        float ped_sum_tmp =  0;
        float ped_mean = 0;
        int entries  = 0;

        if( (peak_samp > 83) && (peak_samp < 117)){
          for(samp  = peak_samp - 2; samp < peak_samp + 2; samp++){
            if(ped_value[slot][ch][samp] > 0){
              entries += ped_value[slot][ch][samp];
              ped_sum_tmp += ped_value[slot][ch][samp]*samp;
            }
          }
        }
        
        if(entries > 0){
          ped_mean = ped_sum_tmp/entries;
        }

        
        float ped_mean_all = 0;
        
        //        printf(" Entries = %d \n",ped_ent[slot][ch]);

        if(ped_ent[slot][ch] > 0) ped_mean_all = ped_all[slot][ch]/ped_ent[slot][ch];
        
        
        if( (ped_mean < ped_min) || (ped_mean > ped_max)){
          
          status++;
          
          *fail_baseline |=  (1 << (slot - 1));

          printf(BOLDRED "Wrong pedestal in Slot %d  Ch %d   Ped %f   Ped Mean   %f \n" RESET, slot, ch, ped_mean, 
                 ped_mean_all);


          // Current exceptions for showing pedestal problems in the run control
          if(  (strcmp(roc_name, "roctagh1")   == 0)  ||  
               ( (strcmp(roc_name, "rocps2")   == 0) &&  (slot == 5)   && (ch == 7))  || 
               // Alex, Spring 2018, one dead channel in fadc
               ( (strcmp(roc_name, "rocfcal7") == 0) &&  (slot == 4)  && (ch == 11))  || 
               // Alex, April 7 2018, noisy channel

               ( (strcmp(roc_name, "rocfcal3") == 0) &&  (slot == 8)  && (ch ==  0))  ||
               // Aex, April 15 2018
               ( (strcmp(roc_name, "rocps1") == 0) &&  (slot == 3)   && (ch == 13))  ||
               ( (strcmp(roc_name, "rocps1") == 0) &&  (slot == 14)  && (ch == 15)) ){  
            
            //               ( (strcmp(roc_name, "rocfcal6") == 0) &&  (slot == 19)  && (ch == 11)) || 
            //               ( (strcmp(roc_name, "rocfcal4") == 0) &&  (slot == 14)  && (ch == 10)) ){
            
            
            status--;
            
            //            daLogMsg("WARN", "Wrong pedestal in Slot %d  Ch %d   Ped %f   Ped Mean   %f \n", slot, ch, ped_mean, 
            //                     ped_mean_all ); 
          } else {
            
            daLogMsg("CONSOLE", "Wrong pedestal in Slot %d  Ch %d   Ped %f   Ped Mean   %f \n", slot, ch, ped_mean, 
                     ped_mean_all );          
          }

        }
        

        if(debug_base > 0)
          printf("Slot = %d   Channel = %d   Peak =  %d    Peak_samp = %d   Mean  = %f   Mean all =  %f\n",slot, ch, ped_peak, 
                 peak_samp, ped_mean, ped_mean_all );
        
        
      }
      
    }    
  }
  

  return(status);



}

void ts_scalers(){

  int debug_scalers = 1;

  static  uint32_t nsync_evt = 0;

  static  uint32_t ts_sc_prev[66];

  int sc;

  uint32_t ts_sc[66];
  uint32_t ts_rate[66];
  float ts_rate_float[66];

  float tot_time;

  memset(ts_sc,0,sizeof(ts_sc));
  memset(ts_rate,0,sizeof(ts_rate));
  memset(ts_rate_float,0,sizeof(ts_rate_float));

  nsync_evt++;


  tsReadScalersMon(ts_sc);
    
  if(debug_scalers){
    for(sc = 0; sc < 66; sc++) {
      if(sc == 0)
        printf("TS scaler %d   =   %d  \n", sc, ts_sc[sc]);
    }
  }
  
  uint32_t live_time  =  ts_sc[64];
  uint32_t busy_time  =  ts_sc[65];

  if( (ts_sc_prev[64] > live_time) || (ts_sc_prev[65] > busy_time) ){
    ts_sc_prev[64] = ts_sc_prev[65] = 0;
  }

  tot_time = (live_time - ts_sc_prev[64] + busy_time - ts_sc_prev[65])*7.68e-6;

  if(tot_time > 0){
    for(sc = 0; sc < 64; sc++){
      ts_rate_float[sc] = (float) (ts_sc[sc] - ts_sc_prev[sc])/ tot_time;
      ts_sc_prev[sc]  =  ts_sc[sc];          
      ts_rate[sc] = ts_rate_float[sc];
    }               
    ts_rate_float[64] = (float) (live_time - ts_sc_prev[64])*7.68e-6/ tot_time; 
  } 
  
  ts_sc_prev[64] = live_time;
  ts_sc_prev[65] = busy_time;
  
  uint32_t inst_live = ts_rate_float[64]*1000;
  
  if(debug_scalers)
    printf("Sync = %d  IntCount =  %d   Rate = %f %f \n",nsync_evt, tsGetIntCount(), ts_rate_float[0], tot_time);
  
  BANKOPEN(0xEE02,BT_UI4,blockLevel);
  
  *dma_dabufp++ = LSWAP(nsync_evt);          //  number of sync events
  *dma_dabufp++ = LSWAP(tsGetIntCount());    //  trigger number
  *dma_dabufp++ = LSWAP(live_time);          //  integrated live time
  *dma_dabufp++ = LSWAP(busy_time);          //  integrated busy time 
  *dma_dabufp++ = LSWAP(inst_live);          //  instantaneous live time 
  *dma_dabufp++ = LSWAP(time(NULL));         //  unix timestamp
  
  // GTP scalers
  for(sc = 0; sc < 32; sc++){
    *dma_dabufp++ = LSWAP(ts_sc[sc]);
  }
  // FP scalers
  for(sc = 32; sc < 48; sc++){
    *dma_dabufp++ = LSWAP(ts_sc[sc]);
  }
  // GTP rate
  for(sc = 0; sc < 32; sc++){    
    *dma_dabufp++ = LSWAP(ts_rate[sc]);
  }
  // FP rate
  for(sc = 32; sc < 48; sc++){
    *dma_dabufp++ = LSWAP(ts_rate[sc]);
  }
  
  BANKCLOSE;
  
}


void fa250_scalers(){
  
  uint32_t version  =  1;
  
  int debug_sc  =  0;
  
  static  uint32_t nsync_sc  =  0;
  
  int fsc_flag  =  1;    /*  latch scalers, 3 - latch and reset */
  
  unsigned int  sclr_buff[17];
  
  int iboard, slot, ch;
  
  static  unsigned int  fadc_time_prev[16];
  
  static  unsigned int  fadc_sc_prev[16][16];
  
  uint32_t  fadc_rate[16][16];
  
  float time, rate_tmp;

  int ret = -1;


  memset(fadc_rate,0,sizeof(fadc_rate));
  memset(sclr_buff,0,sizeof(sclr_buff));


  nsync_sc++;
   

  if(NFADC_250 <= 0) return(-1);

  BANKOPEN(0xEE10,BT_UI4,blockLevel);
  
  *dma_dabufp++ = LSWAP(nsync_sc);           //  number of sync events
  *dma_dabufp++ = LSWAP(tiGetIntCount());    //  trigger number
  *dma_dabufp++ = LSWAP(version);            //  format version number
  

  for(iboard = 0; iboard < NFADC_250; iboard++){    

    slot = fadcID[iboard];
    
    if( (slot < 3) || (slot > 20) ){
      daLogMsg(      "ERROR","%s Wrong slot in fa250_scalers %d  \n", roc_name, slot);  
      printf(BOLDRED "ERROR:     Wrong slot in fa250_scalers %d  \n " RESET, slot);
      continue;
    }

    uint32_t slot_write = (0x80000000 | slot);
    *dma_dabufp++ = LSWAP(slot_write);
    

    ret = faReadScalers(slot, sclr_buff, 0xFFFF, fsc_flag);
    

#if 0
    if( (slot == 3) || (slot == 4)){
      for(ch = 0; ch < 17; ch++) printf(" %d ",sclr_buff[ch]);
      for(ch = 0; ch < 17; ch++) printf(" %d ",fadc_sc_prev[iboard][ch]);
      printf("\n");
    }

    if( (slot == 3) || (slot == 4)){
      for(ch = 0; ch < 17; ch++) printf(" %d ",fadc_sc_prev[iboard][ch]);
      printf("\n");
    }
#endif

    time =  ((float) (sclr_buff[16] - fadc_time_prev[iboard] ))*2048/1.e9;
    
    //    printf(" Time =  %f \n", time);

    

    if(time > 0){
      
      for(ch = 0; ch < 16; ch++) {
	if( (sclr_buff[ch] - fadc_sc_prev[iboard][ch]) <= 0){
	  fadc_rate[iboard][ch]   =  0;
	}  else {
	  rate_tmp = (float) ((sclr_buff[ch] - fadc_sc_prev[iboard][ch]))/time;        /*rate in Hz */ 
	  fadc_rate[iboard][ch]    = rate_tmp;
	} 

	fadc_sc_prev[iboard][ch] =  sclr_buff[ch];
	uint32_t value = fadc_rate[iboard][ch] & 0xFFFFFFFF;
	*dma_dabufp++ = LSWAP(value);     
      }
    
    } else {
      for(ch = 0; ch < 16; ch++){
	fadc_sc_prev[iboard][ch] =  sclr_buff[ch];
	*dma_dabufp++ = LSWAP(0); 
      }
    }
    
    fadc_time_prev[iboard]   =  sclr_buff[16];

    memset(sclr_buff,0,sizeof(sclr_buff));

    if(debug_sc){
      printf("Slot  =  %d  \n",slot);
      for(ch = 0; ch < 16; ch++) {
	printf("%d   ", fadc_rate[iboard][ch]);	
      }
      printf("\n ");

    }

  }  // Loop over fadc250 boards
  
  BANKCLOSE;
  
}




int fa250_pedestals(){
  
  static  uint32_t nsync_ped = 0;
  
  int debug_pedestal = 0;  
  
  int iboard, ch;
  unsigned int slot = 0;
  
  int ret = -1;
    
  int slot_min = -1;
  int slot_max = -1;
  
  unsigned int  ped_buff[8]; 

  uint32_t ped;

  memset(ped_buff,0,sizeof(ped_buff));

  nsync_ped++;


#if 0
  if( strcmp(roc_name, "rocps1") == 0){
    slot_min = 3;
    slot_max = 20;
  }

  if( strcmp(roc_name, "rocps2") == 0){
    slot_min = 3;
    slot_max = 6;
  }

  if(strncmp(roc_name, "rocbcal", 7) == 0){
    slot_min = 3;
    slot_max = 16;
  }
#endif


  if(NFADC_250 <= 0) return(-1);
    
  BANKOPEN(0xEE05,BT_UI4,blockLevel);


  *dma_dabufp++ = LSWAP(nsync_ped);          //  number of sync events
  *dma_dabufp++ = LSWAP(tiGetIntCount());    //  trigger number

  for(iboard = 0; iboard < NFADC_250; iboard++){    

    slot = fadcID[iboard];
    
    if( (slot < 3) || (slot > 20) ){
      daLogMsg("ERROR","%s ERROR: Wrong slot  %d in fa250_pedestals \n", roc_name, slot);  
      printf(BOLDRED " ERROR: Wrong slot  %d in fa250_pedestals \n " RESET, slot);
      continue;
    }

    
    memset(ped_buff,0,sizeof(ped_buff));

    ret = faReadAllChannelSamples(slot, ped_buff);
      
    uint32_t slot_write = (0x80000000 | slot);
    *dma_dabufp++ = LSWAP(slot_write);

    for(ch  = 0; ch < 8; ch++) {
      
      ped =  (uint32_t) ped_buff[ch]; 
      *dma_dabufp++ = LSWAP(ped);
      
      if(debug_pedestal){
        
        int chan1 = 2*ch;
        int chan2 = 2*ch + 1;
        
        uint32_t ped1 =   ped_buff[ch] & 0xFFFF;
        uint32_t ped2 =  (ped_buff[ch] & 0xFFFF0000) >> 16;
        
        printf("ch = %d   %d     ch = %d    %d   \n", 2*ch, ped1, 2*ch+1, ped2);
      }
    }
  }     // Loop over boards
  
  
  BANKCLOSE;
  
  
  return(0);    

}

int  fa250_read_conf(int slot, FADC250_CONF *fa250_read_reg){

  unsigned int nsb_reg, nsa_reg;

  unsigned int adcConf[3];
  
  unsigned int config6 = 0, config7 = 0;

  unsigned short dval[FA_MAX_ADC_CHANNELS];
  unsigned short tval[FA_MAX_ADC_CHANNELS];

  int ii;

  if(slot == 0) slot = fadcID[0];
  
  if((slot <= 0) || (slot > 21) || (FAp[slot] == NULL)) {
    printf("%s: ERROR : ADC in slot %d is not initialized \n",
           __FUNCTION__, slot);
    return(-1);

  }
  
  FALOCK;

  for(ii = 0; ii < 3;ii++) {
    adcConf[ii] = (vmeRead32(&(FAp[slot]->adc_config[ii]))&0xFFFF);
  }    

  config6 = vmeRead32(&FAp[slot]->config6);
  config7 = vmeRead32(&FAp[slot]->config7);


  fa250_read_reg->level       =   (vmeRead32(&(FAp[slot]->blk_level)))&FA_BLOCK_LEVEL_MASK;     // Block Level
  fa250_read_reg->mode        =   (adcConf[0]&FA_ADC_PROC_MASK)>>8 ? 10 : 9;                    //  MODE
  fa250_read_reg->winWidth    =   (vmeRead32(&(FAp[slot]->adc_ptw))&0xFFFF) + 1;                //  PTW
  fa250_read_reg->winOffset   =   (vmeRead32(&(FAp[slot]->adc_pl))&0xFFFF);                     //  PL
  nsb_reg  =  (vmeRead32(&(FAp[slot]->adc_nsb))&0xFFFF);
  nsa_reg  =  (vmeRead32(&(FAp[slot]->adc_nsa))&0xFFFF);

  fa250_read_reg->nsb         =   (nsb_reg&FA_ADC_NSB_READBACK_MASK);                           //  NSB
  if( fa250_read_reg->nsb & 0x8 ) fa250_read_reg->nsb = - (fa250_read_reg->nsb & 0x3);

  fa250_read_reg->nsa         =   (nsa_reg&FA_ADC_NSA_READBACK_MASK);                           //  NSA

  fa250_read_reg->npeak       =   (( (adcConf[0] & FA_ADC_PEAK_MASK ) >> 4) + 1);               //  NP
  
  fa250_read_reg->trig_nsa    =   ( nsa_reg&FA_ADC_TNSA_MASK )>>9;                              //  NSA_tp  
  fa250_read_reg->trig_thr    =   vmeRead32(&FAp[slot]->config3) & FA_ADC_TPT_MASK;             //  threshold_tp;

  fa250_read_reg->nsat        =        (( (adcConf[0] & FA_ADC_CONFIG1_NSAT_MASK ) >>10) + 1);  //  NSAT;

  fa250_read_reg->trig_nsat   =   (( (adcConf[0] & FA_ADC_CONFIG1_TNSAT_MASK ) >>12) + 1);      //  TNSAT;



  fa250_read_reg->nped        =   (((config7 & FA_ADC_CONFIG7_NPED_MASK)>>10) + 1);             //  NPED

  fa250_read_reg->max_ped     =   (config7 & FA_ADC_CONFIG7_MAXPED_MASK);                       //  MAX_PED


  fa250_read_reg->mon_nped       =   (((config6 & FA_ADC_CONFIG7_NPED_MASK)>>10) + 1);          //  MON_NPED
  fa250_read_reg->mon_max_ped    =   (config6 & FA_ADC_CONFIG7_MAXPED_MASK);                    //  MON_MAX_PED



  // DAC
  for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
    fa250_read_reg->dac[ii]       =  vmeRead16(&(FAp[slot]->dac[ii])) & FA_DAC_VALUE_MASK;  
    fa250_read_reg->read_thr[ii]  =  vmeRead16(&(FAp[slot]->adc_thres[ii]));
    fa250_read_reg->trig_bl[ii]   =  vmeRead32(&FAp[slot]->adc_pedestal[ii]) & FA_ADC_PEDESTAL_MASK;
  }


  FAUNLOCK;
  
}

int fa250_check_conf(){

  FADC250_CONF  fa250_set_reg;
  FADC250_CONF  fa250_read_reg;  

  int debug  = 1;

  int ii;

  int iboard, slot;


  for(iboard = 0; iboard < NFADC_250; iboard++){    
    
    slot = fadcID[iboard];
    
    printf("Inside fa250_check_conf() slot = %d\n",slot);
    
    if( (slot < 3) || (slot > 20) ){
      daLogMsg("ERROR","%s ERROR: Wrong slot  %d in fa250_check_conf \n", roc_name, slot);  
      printf(BOLDRED "     ERROR: Wrong slot  %d in fa250_check_conf \n " RESET, slot);
      continue;
    }
    
    fa250_set_reg = get_conf_fadc250(slot);
    fa250_read_conf(slot, &fa250_read_reg);


    if(fa250_set_reg.level != fa250_read_reg.level){
      daLogMsg("ERROR","Check fa250 config. Wrong Block Level  in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.level, fa250_read_reg.level );
      printf(BOLDRED   "Check fa250 config. Wrong Block Level  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.level, fa250_read_reg.level );
      return(1);
    }  

    if(fa250_set_reg.mode != fa250_read_reg.mode){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_MODE  in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.mode, fa250_read_reg.mode );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_MODE  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.mode, fa250_read_reg.mode );
      return(1);
    }  

    if(fa250_set_reg.winOffset != fa250_read_reg.winOffset){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_W_OFFSET in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.winOffset, fa250_read_reg.winOffset );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_W_OFFSET in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.winOffset, fa250_read_reg.winOffset );
      return(1);
    }

    if(fa250_set_reg.winWidth != fa250_read_reg.winWidth){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_W_WIDTH in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.winWidth, fa250_read_reg.winWidth );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_W_WIDTH in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.winWidth, fa250_read_reg.winWidth );
      return(1);
    }

    // This has to be checked 
    //    if(fa250_set_reg.nsb  == 0)  fa250_set_reg.nsb = 1;
    if(fa250_set_reg.nsb != fa250_read_reg.nsb){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_NSB in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.nsb, fa250_read_reg.nsb );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_NSB in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.nsb, fa250_read_reg.nsb );
      return(1);
    }

    if(fa250_set_reg.nsa != fa250_read_reg.nsa){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_NSA in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.nsa, fa250_read_reg.nsa );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_NSA in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.nsa, fa250_read_reg.nsa );
      return(1);
    }  

    if(fa250_set_reg.npeak != fa250_read_reg.npeak){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_NPEAK in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.npeak, fa250_read_reg.npeak );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_NPEAK in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.npeak, fa250_read_reg.npeak );
      return(1);
    }  

    if(fa250_set_reg.nped != fa250_read_reg.nped){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_NPED in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.nped, fa250_read_reg.nped );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_NPED in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.nped, fa250_read_reg.nped );
      return(1);
    }  

    if(fa250_set_reg.max_ped != fa250_read_reg.max_ped){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_MAX_PED in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.max_ped, fa250_read_reg.max_ped );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_MAX_PED in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.max_ped, fa250_read_reg.max_ped );
      return(1);
    }  


    // DAC settings
    int bad_dac = 0;
    for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
      if(fa250_set_reg.dac[ii] !=  fa250_read_reg.dac[ii]){
        daLogMsg("ERROR","Check fa250 config. Wrong FADC250_DAC in Slot %d  Chan = %d  Config  %d   Set  %d   \n",        slot, ii, fa250_set_reg.dac[ii], fa250_read_reg.dac[ii] );
        printf(BOLDRED   "Check fa250 config. Wrong FADC250_DAC in Slot %d  Chan = %d  Config  %d   Set  %d   \n" RESET,  slot, ii, fa250_set_reg.dac[ii], fa250_read_reg.dac[ii] );
        bad_dac++;
      }
    }
    if(bad_dac){
      return(1);
    }

    // Read thresholds
    int bad_thr = 0;
    for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
      if(fa250_set_reg.read_thr[ii] !=  fa250_read_reg.read_thr[ii]){
        daLogMsg("ERROR","Check fa250 config. Wrong FADC250_READ_THR in Slot %d  Chan = %d  Config  %d   Set  %d   \n",        slot, ii, fa250_set_reg.read_thr[ii], 
                 fa250_read_reg.read_thr[ii] );
        printf(BOLDRED   "Check fa250 config. Wrong FADC250_READ_THR in Slot %d  Chan = %d  Config  %d   Set  %d   \n" RESET,  slot, ii, fa250_set_reg.read_thr[ii], 
               fa250_read_reg.read_thr[ii] );
        bad_thr++;
      }
    }
    if(bad_thr){
      return(1);
    }

    // Check trigger settings
    //    if(fa250_set_reg.trig_nsb != fa250_read_reg.trig_nsb){
    //      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_TRIG_NSB in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.trig_nsb, fa250_read_reg.trig_nsb );
    //      printf(BOLDRED   "Check fa250 config. Wrong FADC250_TRIG_NSB in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.trig_nsb, fa250_read_reg.trig_nsb );
    //      return(1);
    //    }  

    if(fa250_set_reg.trig_nsa != fa250_read_reg.trig_nsa){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_TRIG_NSA in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.trig_nsa, fa250_read_reg.trig_nsa );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_TRIG_NSA in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.trig_nsa, fa250_read_reg.trig_nsa );
      return(1);
    }  
    
    if(fa250_set_reg.trig_thr != fa250_read_reg.trig_thr){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_TRIG_THR in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.trig_thr, fa250_read_reg.trig_thr );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_TRIG_THR in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.trig_thr, fa250_read_reg.trig_thr );
      return(1);
    }  

    if(fa250_set_reg.nsat != fa250_read_reg.nsat){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_NSAT in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.nsat, fa250_read_reg.nsat );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_NSAT in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.nsat, fa250_read_reg.nsat );
      return(1);
    }    

    if(fa250_set_reg.trig_nsat != fa250_read_reg.trig_nsat){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_TRIG_NSAT in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.trig_nsat, fa250_read_reg.trig_nsat );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_TRIG_NSAT in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.trig_nsat, fa250_read_reg.trig_nsat );
      return(1);
    }    
    

    // Read thresholds
    int bad_trig_bl = 0;

    for(ii = 0; ii < FA_MAX_ADC_CHANNELS; ii++){

      // Don't check channels masked for the trigger and readout 

      if( ( fa250_set_reg.trigMask & (1 << ii )) != 0) {
        //        printf(BOLDBLUE "Slot  %d  Chan %d \n" RESET, slot, ii);
        continue;
      }
      
      if(fa250_set_reg.trig_bl[ii] !=  fa250_read_reg.trig_bl[ii]){

        daLogMsg("ERROR","Check fa250 config. Wrong FADC250_TRIG_BL in Slot %d  Chan = %d  Config  %d   Set  %d   \n",        slot, ii, fa250_set_reg.trig_bl[ii], 
                 fa250_read_reg.trig_bl[ii] );
        printf(BOLDRED   "Check fa250 config. Wrong FADC250_TRIG_BL in Slot %d  Chan = %d  Config  %d   Set  %d   \n" RESET,  slot, ii, fa250_set_reg.trig_bl[ii], 
               fa250_read_reg.trig_bl[ii] );
        bad_trig_bl++;
      }
    }
    if(bad_trig_bl){
      return(1);
    }

    if(fa250_set_reg.mon_nped != fa250_read_reg.mon_nped){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_MON_NPED in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.mon_nped, fa250_read_reg.mon_nped );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_MON_NPED in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.mon_nped, fa250_read_reg.mon_nped );
      return(1);
    }  

    if(fa250_set_reg.mon_max_ped != fa250_read_reg.mon_max_ped){
      daLogMsg("ERROR","Check fa250 config. Wrong FADC250_MON_MAX_PED in Slot %d   Config  %d   Set  %d   \n",       slot, fa250_set_reg.mon_max_ped, fa250_read_reg.mon_max_ped );
      printf(BOLDRED   "Check fa250 config. Wrong FADC250_MON_MAX_PED in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa250_set_reg.mon_max_ped, fa250_read_reg.mon_max_ped );
      return(1);
    }  



    if(debug){
      
      if(slot == 3){      
        printf(BOLDBLUE " Set Block Level =  %d \n" RESET ,fa250_set_reg.level);
        printf(BOLDBLUE " Set Mode        =  %d \n"       RESET ,fa250_set_reg.mode);
        printf(BOLDBLUE " Set winOffset   =  %d \n"  RESET ,fa250_set_reg.winOffset);
        printf(BOLDBLUE " Set winWidth    =  %d \n"  RESET ,fa250_set_reg.winWidth);
        printf(BOLDBLUE " Set nsb         =  %d \n"        RESET ,fa250_set_reg.nsb);
        printf(BOLDBLUE " Set nsa         =  %d \n"        RESET ,fa250_set_reg.nsa);
        printf(BOLDBLUE " Set npeak       =  %d \n"      RESET ,fa250_set_reg.npeak);
        printf(BOLDBLUE " Set nsat        =  %d \n"      RESET ,fa250_set_reg.nsat);
        printf(BOLDBLUE " Set nped        =  %d \n"      RESET ,fa250_set_reg.nped);
        printf(BOLDBLUE " Set max_ped     =  %d \n"      RESET ,fa250_set_reg.max_ped);

        printf("\n");
        printf(BOLDBLUE " Set trig_nsa    =  %d \n"   RESET ,fa250_set_reg.trig_nsa);
        printf(BOLDBLUE " Set trig_thr    =  %d \n"   RESET ,fa250_set_reg.trig_thr);
        printf(BOLDBLUE " Set trig_nsat   =  %d \n"   RESET ,fa250_set_reg.trig_nsat);

        printf("\n");
        printf(BOLDBLUE " Set mon_nped    =  %d \n"   RESET ,fa250_set_reg.mon_nped);
        printf(BOLDBLUE " Set mon_max_ped =  %d \n"   RESET ,fa250_set_reg.mon_max_ped);


        printf("\n");
        printf(BOLDBLUE "DAC Settings for FADC in slot %d:" RESET, slot);
        for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
          if((ii % 4)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa250_set_reg.dac[ii]);
        }
        
        printf("\n\n");
        
        printf(BOLDBLUE " Threshold Settings for FADC in slot %d:" RESET,slot);
        for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
          if((ii % 4)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa250_set_reg.read_thr[ii]);
        }
        
        printf("\n\n");
        
        printf(BOLDBLUE " Trigger Baseline Settings for FADC in slot %d:" RESET,slot);
        for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
          if((ii % 4)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa250_set_reg.trig_bl[ii]);
        }
        printf("\n\n");

        printf(BOLDMAGENTA " Read Block Level =  %d  \n"      RESET ,fa250_read_reg.level);
        printf(BOLDMAGENTA " Read Mode        =  %d  \n"      RESET ,fa250_read_reg.mode);
        printf(BOLDMAGENTA " Read winOffset   =  %d  \n"      RESET ,fa250_read_reg.winOffset);
        printf(BOLDMAGENTA " Read winWidth    =  %d  \n"      RESET ,fa250_read_reg.winWidth);
        printf(BOLDMAGENTA " Read nsb         =  %d  \n"      RESET ,fa250_read_reg.nsb);
        printf(BOLDMAGENTA " Read nsa         =  %d  \n"      RESET ,fa250_read_reg.nsa);
        printf(BOLDMAGENTA " Read npeak       =  %d  \n"      RESET ,fa250_read_reg.npeak);
        printf(BOLDMAGENTA " Read nsat        =  %d  \n"      RESET ,fa250_read_reg.nsat);
        printf(BOLDMAGENTA " Read nped        =  %d  \n"      RESET ,fa250_read_reg.nped);
        printf(BOLDMAGENTA " Read max_ped     =  %d  \n"      RESET ,fa250_read_reg.max_ped);

        printf("\n");

        printf(BOLDMAGENTA " Read trig_nsa    =  %d  \n"      RESET ,fa250_read_reg.trig_nsa);
        printf(BOLDMAGENTA " Read trig_thr    =  %d  \n"      RESET ,fa250_read_reg.trig_thr);
        printf(BOLDMAGENTA " Read trig_nsat   =  %d  \n"      RESET ,fa250_read_reg.trig_nsat);

        printf("\n");
        printf(BOLDMAGENTA " Read mon_nped    =  %d \n"       RESET ,fa250_read_reg.mon_nped);
        printf(BOLDMAGENTA " Read mon_max_ped =  %d \n"       RESET ,fa250_read_reg.mon_max_ped);


        printf("\n");
        printf(BOLDMAGENTA "DAC Settings for FADC in slot %d:" RESET, slot);
        for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
          if((ii%4)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa250_read_reg.dac[ii]);
        }
        
        printf("\n\n");
        
        printf(BOLDMAGENTA " Threshold Settings for FADC in slot %d:" RESET,slot);
        for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
          if((ii % 4)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa250_set_reg.read_thr[ii]);
        }
        
        printf("\n\n");
        
        printf(BOLDMAGENTA " Trigger Baseline Settings for FADC in slot %d:" RESET,slot);
        for(ii = 0;ii < FA_MAX_ADC_CHANNELS;ii++){
          if((ii % 4)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa250_read_reg.trig_bl[ii]);
        }
        printf("\n\n");
        
        
      }
    }         // debug
  }

  return(0);

}



int read_conf_pb(char *crate){
  
  int debug = 0;
 
  FILE *pFile;
  
  int ch;
  int ii = 0;
  int jj;
  int val;

  int args,  msk[16], msk1[18];
  
  int ch_dis_mask[3];

  float mskf[18];

  int slot = -1;

  int first = -1, last = -1;

  char   sn[NBD_FADC250][12];

  char   str[STRLEN], str2[STRLEN], str_tmp[STRLEN];
  char   keyword[ROCLEN];
  
  int line = 0;

  int SL_MIN  =  0;
  int SL_MAX  =  0;  

  int crate_found = 0;

  int slot_pb = -1;
  int chan_pb = -1;

  int mskpb[32];

  det_type crate_det  =  get_det_type(crate );

  if(crate_det == det_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_pb: Unknown detector type for crate  %s \n" RESET, crate);
    return(-2);
  }

  bd_type crate_board  =  get_board_type(crate );


  if(crate_board == bd_unknown) {
    printf(BOLDRED "\n FATAL: Read_conf_pb: Unknown board type for crate  %s \n" RESET, crate);
    return(-2);
  }


  /*  Take user configuration file name from the flobal config file */
  char myfile[STRLEN]="";


  if(crate_board != fadc250){
    printf(BOLDRED "\n WARNING: Read_conf_pb: No fadc250 modules in the crate  %s \n" RESET, crate);
    return(0);
  }


  if(debug){
    printf(" STRING LENGTH = %d  %s \n", strlen(fa250[3].conf_pb_dir), fa250[3].conf_pb_dir);
    printf(" STRING LENGTH = %d  %s \n", strlen(fa250[3].conf_pb_ver), fa250[3].conf_pb_ver);
  }
  
  if((strlen(fa250[3].conf_pb_dir) != 0)  && (strlen(fa250[3].conf_pb_ver) != 0) )
    sprintf(myfile,"%s/%s.cnf", fa250[3].conf_pb_dir,fa250[3].conf_pb_ver);
  else {
    printf(BOLDRED "\n FATAL: Have to set  PLAY_BACK_DIR and PLAY_BACK_VERS in the global configuration file %s %s  \n" RESET,
           fa250[3].conf_pb_dir, fa250[3].conf_pb_ver );
    return(-2);
  }
  
  
  /*   Open Global Configuration File  */

  pFile = fopen(myfile, "r");

  if(pFile == NULL) {
    printf(BOLDRED "\n FATAL: Read_conf_pb: Cannot open config file   %s \n" RESET, myfile);
    return(-2);
  }
  
  printf(BOLDBLUE "\n Reading Play Back parameters from    %s \n" RESET, myfile);
  
  while ((ch = getc(pFile)) != EOF) {
    if(debug) printf(" %d %d %d \n",ch,' ','=');
    if ( ch == '#' || ch == '#' ||  ch  == '=' ) {
      while (getc(pFile) != '\n') {}
    } else if( ch == '\n' ) {}
    else  {	  
      ungetc(ch,pFile);
      if(fgets(str_tmp, sizeof(str_tmp), pFile)){
        
	memset(str,0,sizeof(str)); memset(keyword,0,sizeof(keyword));
        
        sscanf (str_tmp, "%s %s", keyword, str);
	
        printf("  %s\n ",str_tmp); 
        
        if(strcmp(keyword,"CRATE") == 0){
          if(strcmp(crate, str) == 0){
            printf(BOLDBLUE "\n Read_conf_pb: Crate found %s\n" RESET, str);
            crate_found = 1;
          }
        }

        if(crate_found){
          
          SCAN_PB_SLOT("SLOT",slot_pb);
          SCAN_PB_CHAN("CHAN",chan_pb);
          
          printf("SASCHA %d  %d\n  ", slot_pb,  chan_pb); 
          
          SCAN_PB_AMP("AMP",fa250[slot_pb].amp_pb[chan_pb]);
        }
        
        
      }

    }
  }
  
  fclose(pFile);
  
  return 0;
  
}











int  fa125_read_conf(int slot, FADC125_CONF *fa125_read_reg){

  unsigned int nsb_reg, nsa_reg;

  unsigned int adcConf[3];
  
  unsigned int config6 = 0, config7 = 0;

  //  unsigned short dval[FA_MAX_ADC_CHANNELS];
  //  unsigned short tval[FA_MAX_ADC_CHANNELS];

  int ii;

  
  if((slot <= 0) || (slot > 21) || (fa125p[slot] == NULL)) {
    printf("%s: ERROR : ADC in slot %d is not initialized \n",
           __FUNCTION__, slot);
    return(-1);    
  }
  
  
  FA125LOCK;


  //  a24addr[slot]    = (unsigned int)fa125p[slot] - fa125A24Offset;

  fa125_read_reg->level      =   vmeRead32(&fa125p[slot]->proc.blocklevel) &  FA125_PROC_BLOCKLEVEL_MASK;                 //   Block Level
  fa125_read_reg->mode       =  (vmeRead32(&fa125p[slot]->fe[0].config1)  & FA125_FE_CONFIG1_MODE_MASK ) + 1;             //   MODE
  fa125_read_reg->winOffset  =   vmeRead32(&fa125p[slot]->fe[0].pl) & FA125_FE_PL_MASK;                                   //   PL
  fa125_read_reg->winWidth   =   vmeRead32(&fa125p[slot]->fe[0].nw) & FA125_FE_NW_MASK;                                   //   PTW
  fa125_read_reg->IE         =   vmeRead32(&fa125p[slot]->fe[0].ie) & FA125_FE_IE_INTEGRATION_END_MASK;                   //   IE
  
  fa125_read_reg->npeak      =   ((vmeRead32(&fa125p[slot]->fe[0].config1) & FA125_FE_CONFIG1_NPULSES_MASK) >> 4);        //   NPEAK
  
  fa125_read_reg->PG         =   ((vmeRead32(&fa125p[slot]->fe[0].ie) & FA125_FE_IE_PEDESTAL_GAP_MASK) >> 12);            //   PG
  
  fa125_read_reg->P1         =   ((vmeRead32(&fa125p[slot]->fe[0].ped_sf) &  FA125_FE_PED_SF_NP_MASK));                   //   P1
  fa125_read_reg->P2         =   ((vmeRead32(&fa125p[slot]->fe[0].ped_sf) &  FA125_FE_PED_SF_NP2_MASK) >> 8);             //   P2
  

  fa125_read_reg->IBIT       =   ((vmeRead32(&fa125p[slot]->fe[0].ped_sf) &  FA125_FE_PED_SF_IBIT_MASK) >> 16);           //   IBIT
  fa125_read_reg->ABIT       =   ((vmeRead32(&fa125p[slot]->fe[0].ped_sf) &  FA125_FE_PED_SF_ABIT_MASK) >> 19);           //   ABIT
  fa125_read_reg->PBIT       =   ((vmeRead32(&fa125p[slot]->fe[0].ped_sf) &  FA125_FE_PED_SF_PBIT_MASK) >> 22);           //   PBIT 


  for(ii = 0;ii < MAX_FADC125_CH; ii++){
    
    fa125_read_reg->read_thr[ii] = vmeRead32(&fa125p[slot]->fe[ii/6].threshold[ii%6]) & FA125_FE_THRESHOLD_MASK;          //   Threshold

    if((ii%2)==0){                                                                                                         //  TH and TL
      fa125_read_reg->TL_CH[ii] =  (vmeRead32(&fa125p[slot]->fe[ii/6].timing_thres_lo[(ii/2)%3]) & 
                                    FA125_FE_TIMING_THRES_LO_MASK(ii))>>8;
    } else {
      fa125_read_reg->TL_CH[ii]  = (vmeRead32(&fa125p[slot]->fe[ii/6].timing_thres_lo[(ii/2)%3]) & 
                                    FA125_FE_TIMING_THRES_LO_MASK(ii))>>24;
    }     
    
    if((ii%3)==0) {
      fa125_read_reg->TH_CH[ii]  = vmeRead32(&fa125p[slot]->fe[ii/6].timing_thres_hi[(ii/3)%2]) & 
	FA125_FE_TIMING_THRES_HI_MASK(ii);

    } else if((ii%3)==1) {
      fa125_read_reg->TH_CH[ii] = (vmeRead32(&fa125p[slot]->fe[ii/6].timing_thres_hi[(ii/3)%2]) & 
                                     FA125_FE_TIMING_THRES_HI_MASK(ii))>>9;
    } else {
      fa125_read_reg->TH_CH[ii]  = (vmeRead32(&fa125p[slot]->fe[ii/6].timing_thres_hi[(ii/3)%2]) & 
                                    FA125_FE_TIMING_THRES_HI_MASK(ii))>>18;
    }    
    
  }


  FA125UNLOCK;

}




int fa125_check_conf(){

  FADC125_CONF  fa125_set_reg;
  FADC125_CONF  fa125_read_reg;  

  int debug  = 1;

  int ii;

  int iboard, slot;

  int bad_thr = 0;

  for(iboard = 0; iboard < NFADC_125; iboard++){    
    
    slot  =  fa125Slot(iboard);

    printf("Inside fa125_check_conf() slot = %d\n",slot);
    
    if( (slot < 3) || (slot > 20) ){
      daLogMsg(      "ERROR","%s ERROR: Wrong slot  %d in fa125_check_conf   \n", roc_name, slot);  
      printf(BOLDRED "ERROR:            Wrong slot  %d in fa125_check_conf   \n " RESET, slot);
      continue;
    }

    
    fa125_set_reg = get_conf_fadc125(slot);
    fa125_read_conf(slot, &fa125_read_reg);


    if(fa125_set_reg.level != fa125_read_reg.level){
      daLogMsg("ERROR","Check fa125 config. Wrong Block Level  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.level, fa125_read_reg.level );
      printf(BOLDRED   "Check fa125 config. Wrong Block Level  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.level, fa125_read_reg.level );
      return(1);
    }  

    if(fa125_set_reg.mode != fa125_read_reg.mode){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_MODE  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.mode, fa125_read_reg.mode );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_MODE  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.mode, fa125_read_reg.mode );
      return(1);
    }  
    
    if(fa125_set_reg.winOffset != fa125_read_reg.winOffset){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_W_OFFSET  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.winOffset, fa125_read_reg.winOffset );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_W_OFFSET  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.winOffset, fa125_read_reg.winOffset );
      return(1);
    }  
    
    if(fa125_set_reg.winWidth != fa125_read_reg.winWidth){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_W_WIDTH  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.winWidth, fa125_read_reg.winWidth );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_W_WIDTH  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.winWidth, fa125_read_reg.winWidth );
      return(1);
    }     

    if(fa125_set_reg.IE != fa125_read_reg.IE){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_IE  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.IE, fa125_read_reg.IE );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_IE  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.IE, fa125_read_reg.IE );
      return(1);
    }     
 

    if(fa125_set_reg.npeak != fa125_read_reg.npeak){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_NPEAK  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.npeak, fa125_read_reg.npeak );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_NPEAK  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.npeak, fa125_read_reg.npeak );
      return(1);
    }     

    if(fa125_set_reg.PG != fa125_read_reg.PG){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_PG  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.PG, fa125_read_reg.PG );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_PG  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.PG, fa125_read_reg.PG );
      return(1);
    }     

    if(fa125_set_reg.P1 != fa125_read_reg.P1){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_P1  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.P1, fa125_read_reg.P1 );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_P1  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.P1, fa125_read_reg.P1 );
      return(1);
    }     

    if(fa125_set_reg.P2 != fa125_read_reg.P2){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_P2  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.P2, fa125_read_reg.P2 );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_P2  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.P2, fa125_read_reg.P2 );
      return(1);
    }     


    if(fa125_set_reg.IBIT != fa125_read_reg.IBIT){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_IBIT  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.IBIT, fa125_read_reg.IBIT );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_IBIT  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.IBIT, fa125_read_reg.IBIT );
      return(1);
    }     

    if(fa125_set_reg.ABIT != fa125_read_reg.ABIT){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_ABIT  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.ABIT, fa125_read_reg.ABIT );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_ABIT  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.ABIT, fa125_read_reg.ABIT );
      return(1);
    }     

    if(fa125_set_reg.PBIT != fa125_read_reg.PBIT){
      daLogMsg("ERROR","Check fa125 config. Wrong FADC125_PBIT  in Slot %d   Config  %d   Set  %d   \n",       slot, fa125_set_reg.PBIT, fa125_read_reg.PBIT );
      printf(BOLDRED   "Check fa125 config. Wrong FADC125_PBIT  in Slot %d   Config  %d   Set  %d   \n" RESET, slot, fa125_set_reg.PBIT, fa125_read_reg.PBIT );
      return(1);
    }     


 

    // Read thresholds
    
    for(ii = 0;ii < MAX_FADC125_CH; ii++){

      unsigned short val_tmp = fa125_set_reg.read_thr[ii];
      if(val_tmp > 0x1FF) val_tmp = 0x1FF;                  //   Maximum threshold values is 511

      if(val_tmp !=  fa125_read_reg.read_thr[ii]){
        daLogMsg("ERROR","Check fa125 config. Wrong FADC125_THR in Slot %d  Chan = %d  Config  %d   Set  %d   \n",        slot, ii, fa125_set_reg.read_thr[ii], 
                 fa125_read_reg.read_thr[ii] );
        printf(BOLDRED   "Check fa125 config. Wrong FADC125_THR in Slot %d  Chan = %d  Config  %d   Set  %d   \n" RESET,  slot, ii, fa125_set_reg.read_thr[ii], 
               fa125_read_reg.read_thr[ii] );
        bad_thr++;
      }
    }


    // TH thresholds

    for(ii = 0;ii < MAX_FADC125_CH; ii++){
      
      if( fa125_set_reg.TH !=  fa125_read_reg.TH_CH[ii]){

        daLogMsg("ERROR","Check fa125 config. Wrong FADC125_TH in Slot %d  Chan = %d  Config  %d   Set  %d   \n",        slot, ii, fa125_set_reg.TH, 
                 fa125_read_reg.TH_CH[ii] );
        printf(BOLDRED   "Check fa125 config. Wrong FADC125_TH in Slot %d  Chan = %d  Config  %d   Set  %d   \n" RESET,  slot, ii, fa125_set_reg.TH, 
               fa125_read_reg.TH_CH[ii] );
        bad_thr++;
      }
    }





    if(debug){
      
      if(slot == 3){      

        printf(BOLDBLUE " Set Block Level =  %d \n"  RESET ,fa125_set_reg.level);
        printf(BOLDBLUE " Set Mode        =  %d \n"  RESET ,fa125_set_reg.mode);
        printf(BOLDBLUE " Set PL          =  %d \n"  RESET ,fa125_set_reg.winOffset);
        printf(BOLDBLUE " Set PTW         =  %d \n"  RESET ,fa125_set_reg.winWidth);
        printf(BOLDBLUE " Set IE          =  %d \n"  RESET ,fa125_set_reg.IE);
        printf("\n\n");
        
        printf(BOLDBLUE " Threshold Settings for FADC in slot %d:" RESET,slot);
        for(ii = 0;ii < MAX_FADC125_CH; ii++){
          if((ii % 8)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa125_set_reg.read_thr[ii]);
        }


        printf("\n\n");

        printf(BOLDMAGENTA " Read Block Level =  %d  \n"      RESET ,fa125_read_reg.level);
        printf(BOLDMAGENTA " Read Mode        =  %d  \n"      RESET ,fa125_read_reg.mode);
        printf(BOLDMAGENTA " Set  PL          =  %d  \n"      RESET ,fa125_read_reg.winOffset);
        printf(BOLDMAGENTA " Set  PTW         =  %d  \n"      RESET ,fa125_read_reg.winWidth);
        printf(BOLDMAGENTA " Set  IE          =  %d \n"       RESET ,fa125_read_reg.IE);

        printf("\n\n");
        
        printf(BOLDMAGENTA " Threshold Settings for FADC in slot %d:" RESET,slot);
        for(ii = 0;ii < MAX_FADC125_CH; ii++){
          if((ii % 8)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1),fa125_read_reg.read_thr[ii]);
        }
        
        printf("\n\n");

      }
    }

  }

  if(bad_thr){
    return(1);
  }


  return(0);

}





int  dsc2_read_conf(int slot, DSC2_CONF *dsc2_read_reg){
  
  int rval = 0;
  
  int ii;
  
  //  if( (slot > 21) ||  (dscp[slot] == NULL) ) {
  if( (slot > 21) ) {
    printf("%s: ERROR : DSC in slot %d is not initialized \n",
           __FUNCTION__, slot);
    return(-1);    
  }
  
  //  DSCLOCK;

  for(ii = 0;ii < MAX_DSC2_CH; ii++){

    //    rval = ( vmeRead32(&dscp[slot]->threshold[ii]) & DSC_THRESHOLD_TDC_MASK ) ;
    //    if(rval & 0x800) rval |= 0xFFFFF000;

    //    dsc2_read_reg->TDCThreshold[ii] = rval;

    dsc2_read_reg->TDCThreshold[ii] = dsc2GetBipolarThreshold(slot, ii, 1);
  }

  //  DSCUNLOCK;


}




int dsc2_check_conf(){

  DSC2_CONF  dsc2_set_reg;
  DSC2_CONF  dsc2_read_reg;  

  int debug  = 1;

  int ii;

  int iboard, slot;

  int bad_thr = 0;


  int nboards = dsc2GetNdsc();

  //  for(iboard = 0; iboard < Ndsc; iboard++){    
  for(iboard = 0; iboard < nboards; iboard++){    

    slot = dsc2Slot(iboard);

    printf("Inside dsc2_check_conf() slot = %d \n",slot);
    
    if( slot > 21 ){
      daLogMsg(      "ERROR","%s ERROR: Wrong slot  %d in dsc2_check_conf   \n", roc_name, slot);  
      printf(BOLDRED "ERROR:            Wrong slot  %d in dsc2_check_conf   \n " RESET, slot);
      continue;
    }

    
    dsc2_set_reg = get_conf_dsc2(slot);
    dsc2_read_conf(slot, &dsc2_read_reg);


    //  Check thresholds
    
    for(ii = 0;ii < MAX_DSC2_CH; ii++){

      int val_tmp = dsc2_set_reg.TDCThreshold[ii];

      if(abs(val_tmp) > 4095) val_tmp = -4095;                  //   Maximum threshold values is +- 4095

      if(val_tmp !=  dsc2_read_reg.TDCThreshold[ii]){
        daLogMsg("ERROR", "Check DSC2 config. Wrong DSC2_THRESHOLD  in Slot %d  Chan = %d  Config  %d   Set  %d  \n",        slot, ii, dsc2_set_reg.TDCThreshold[ii], 
                 dsc2_read_reg.TDCThreshold[ii]);
        printf(BOLDRED    "Check fa125 config. Wrong DSC2_THRESHOLD in Slot %d  Chan = %d  Config  %d   Set  %d  \n" RESET,  slot, ii, dsc2_set_reg.TDCThreshold[ii], 
               dsc2_read_reg.TDCThreshold[ii]);
        bad_thr++;
      }
    }



    if(debug){
      
      if( (slot == 4) || (slot == 5) ){      

        printf("\n\n");
        
        printf(BOLDBLUE " Threshold Settings for DSC2 in slot %d:" RESET,slot);

        for(ii = 0;ii < MAX_DSC2_CH; ii++){
          if((ii % 8)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1), dsc2_set_reg.TDCThreshold[ii]);
        }

        printf("\n\n");
        
        printf(BOLDMAGENTA " Threshold Settings for DSC2 in slot %d:" RESET,slot);

        for(ii = 0;ii < MAX_DSC2_CH; ii++){
          if((ii % 8)==0) printf("\n");
          printf("Chan %2d: %5d   ",(ii+1), dsc2_read_reg.TDCThreshold[ii]);
        }
        
        printf("\n\n");
        
      }
    }

  }

  if(bad_thr){
    return(1);
  }


  return(0);
  
}


void ssp_init(){
  
  int ii, jj, fiber, asic, ch;
  
  printf("\n ----  SSP  init ------ \n"); 

  for(jj = 3; jj < NBD_SSP; jj++){

    ssp[jj].slot        =  0;
    ssp[jj].fiber_en    =  0;
    ssp[jj].sum_enable  =  0;

    ssp[jj].fw_rev = 0;
    ssp[jj].fw_type = 0;
  
    ssp[jj].window_width  = 100;
    ssp[jj].window_offset = 825;
    ssp[jj].pulser_freq   = 1000000;
    
    for(ii = 0; ii < SD_SRC_NUM; ii++)
      ssp[jj].ssp_io_mux[ii] = SD_SRC_SEL_0;
    
    ssp[jj].ssp_io_mux[SD_SRC_TRIG] = SD_SRC_SEL_TRIG1;
    ssp[jj].ssp_io_mux[SD_SRC_SYNC] = SD_SRC_SEL_SYNC;

    ssp[jj].ssp_io_mux[SD_SRC_TRIG2] = SD_SRC_SEL_PULSER;
    //    ssp[jj].ssp_io_mux[SD_SRC_TRIG2] = SD_SRC_SEL_TRIG2;


    ssp[jj].dirc.disable_evtbuild   =  0;
    ssp[jj].dirc.disable_fiber      =  0;
    ssp[jj].dirc.fiber_in_config    =  0;

    for(fiber = 0; fiber < 32; fiber++)
      {
        ssp[jj].dirc.fiber[fiber].ctest_dac    =  1000;
        ssp[jj].dirc.fiber[fiber].ctest_enable =  1;
        ssp[jj].dirc.fiber[fiber].tdc_enable_mask[0] = 0xFFFFFFFF;
        ssp[jj].dirc.fiber[fiber].tdc_enable_mask[1] = 0xFFFFFFFF;
        ssp[jj].dirc.fiber[fiber].tdc_enable_mask[2] = 0xFFFFFFFF;
        ssp[jj].dirc.fiber[fiber].tdc_enable_mask[3] = 0xFFFFFFFF;
        ssp[jj].dirc.fiber[fiber].tdc_enable_mask[4] = 0xFFFFFFFF;
        ssp[jj].dirc.fiber[fiber].tdc_enable_mask[5] = 0xFFFFFFFF;
        ssp[jj].dirc.fiber[fiber].window_width  = 1000;
        ssp[jj].dirc.fiber[fiber].window_offset = 1000;
        for(asic = 0; asic < 3; asic++)
          {
            ssp[jj].dirc.fiber[fiber].chip[asic].Sum[0]      =  0;
            ssp[jj].dirc.fiber[fiber].chip[asic].Sum[1]      =  0;
            ssp[jj].dirc.fiber[fiber].chip[asic].CTest[0]    =  0x1;
            ssp[jj].dirc.fiber[fiber].chip[asic].CTest[1]    =  0;
            ssp[jj].dirc.fiber[fiber].chip[asic].MaskOr[0]   =  0;
            ssp[jj].dirc.fiber[fiber].chip[asic].MaskOr[1]   =  0;
            ssp[jj].dirc.fiber[fiber].chip[asic].Global0.val =  0x69306B87;
            ssp[jj].dirc.fiber[fiber].chip[asic].Global1.val =  0;
            ssp[jj].dirc.fiber[fiber].chip[asic].DAC0        =  500;
            ssp[jj].dirc.fiber[fiber].chip[asic].DAC1        =  0;
            for(ch = 0; ch < 64; ch++)
              ssp[jj].dirc.fiber[fiber].chip[asic].Gain[ch]  =  64;
          }
      } 
  }
}


void ti_check_latency(char *crate){

  int rc = 0;

  int wrong_ti_dt = 0;
  
  int ti_delta_t  = -10; 

  long cr_ind  = 0;

  int max_dt = 0;

  char *p = crate;

  while (*p){
    if(isdigit(*p)){
      cr_ind = strtol(p, &p, 10); // Read a number
      printf( BOLDBLUE   "Crate Index = %ld  \n" RESET,cr_ind); 
    } else {
      p++;
    }
  }

  // Check FDC fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "rocfdc",  6) == 0) {    

    if(cr_ind > 0 && cr_ind < 15){
      ti_delta_t = tiGetFiberLatencyMeasurement() - fdc_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED     "Wrong Fiber Latency Measurement in the FDC crate. Continue, %d,  %d  \n" RESET,  fdc_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the FDC crate. Continue, %d,  %d  \n", fdc_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        //        ROL_SET_ERROR; 
      }
    } else {
      printf( BOLDBLUE   "Wrong FDC crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong FDC crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }    

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(fdc_ti_offset[cr_ind - 1],152);

      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in FDC crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in FDC crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;

    }
  }
  
 // Check BCAL fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "rocbcal",  7) == 0) {    
    if(cr_ind > 0 && cr_ind < 13){
      ti_delta_t = tiGetFiberLatencyMeasurement() - bcal_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED       "Wrong Fiber Latency Measurement in the BCAL crate. Continue, %d,  %d  \n" RESET,  bcal_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the BCAL crate. Continue, %d,  %d  \n", bcal_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
      }
    } else {
      printf( BOLDBLUE   "Wrong BCAL crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong BCAL crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(bcal_ti_offset[cr_ind - 1],152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in BCAL crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in BCAL crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;

    }        
  }

 // Check CDC fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "roccdc",  6) == 0) {
    if(cr_ind > 0 && cr_ind < 5){
      ti_delta_t = tiGetFiberLatencyMeasurement() - cdc_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED       "Wrong Fiber Latency Measurement in the CDC crate. Continue, %d,  %d  \n" RESET,  cdc_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the CDC crate. Continue, %d,  %d  \n", cdc_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
      }      
    } else {
      printf( BOLDBLUE   "Wrong CDC crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong CDC crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }    
    
    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(cdc_ti_offset[cr_ind - 1],152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in CDC crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in CDC crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;

    }    
  }


 // Check ECAL fiber latency first and write to the fiberDelay register if it fails the measurement test


  if(strncmp(roc_name, "rocecal",  7) == 0) {    
    if(cr_ind > 0 && cr_ind < 13){
      ti_delta_t = tiGetFiberLatencyMeasurement() - ecal_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED       "Wrong Fiber Latency Measurement in the ECAL crate. Continue, %d,  %d  \n" RESET,  ecal_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the ECAL crate. Continue, %d,  %d  \n", ecal_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
      }
    } else {
      printf( BOLDBLUE   "Wrong ECAL crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong ECAL crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }    

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(ecal_ti_offset[cr_ind - 1],152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in ECAL crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in ECAL crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }      
  }


  // Check FCAL fiber latency first and write to the fiberDelay register if it fails the measurement test


  if(strncmp(roc_name, "rocfcal",  7) == 0) {    
    if(cr_ind > 0 && cr_ind < 13){
      ti_delta_t = tiGetFiberLatencyMeasurement() - fcal_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED       "Wrong Fiber Latency Measurement in the FCAL crate. Continue, %d,  %d  \n" RESET,  fcal_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the FCAL crate. Continue, %d,  %d  \n", fcal_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
      }
    } else {
      printf( BOLDBLUE   "Wrong FCAL crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong FCAL crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }    

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(fcal_ti_offset[cr_ind - 1],152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in FCAL crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in FCAL crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }      
  }

   // Check TOF fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "roctof",  6) == 0) {    
    if(cr_ind > 0 && cr_ind < 3){
      ti_delta_t = tiGetFiberLatencyMeasurement() - tof_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED       "Wrong Fiber Latency Measurement in the TOF crate. Continue, %d,  %d  \n" RESET,  tof_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the TOF crate. Continue, %d,  %d  \n", tof_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
      }
    } else {
      printf( BOLDBLUE   "Wrong TOF crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong TOF crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }    

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(tof_ti_offset[cr_ind - 1],152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in TOF crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in TOF crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }     
  }

  // Check PS fiber latency first and write to the fiberDelay register if it fails the measurement test
  
  if(strncmp(roc_name, "rocps",  5) == 0) {    
    if(cr_ind > 0 && cr_ind < 3){
      ti_delta_t = tiGetFiberLatencyMeasurement() - ps_ti_offset[cr_ind - 1];
      if(abs(ti_delta_t) > max_dt){
        wrong_ti_dt = 1;
        printf(BOLDRED       "Wrong Fiber Latency Measurement in the PS crate. Continue, %d,  %d  \n" RESET,  ps_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
        daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the PS crate. Continue, %d,  %d  \n", ps_ti_offset[cr_ind - 1], tiGetFiberLatencyMeasurement() );
      }
    } else {
      printf( BOLDBLUE   "Wrong PS crate   %ld   \n" RESET, cr_ind);
      daLogMsg("ERROR",  "Wrong PS crate   %ld   \n", cr_ind);
      ROL_SET_ERROR; 
    }    
    
    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(ps_ti_offset[cr_ind - 1],152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in PS crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in PS crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }      
  }

  // Check TAGMH fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "roctagmh",  8) == 0) {    
    ti_delta_t = tiGetFiberLatencyMeasurement() - tagmh_ti_offset;
    if(abs(ti_delta_t) > max_dt){
      wrong_ti_dt = 1;
      printf(BOLDRED       "Wrong Fiber Latency Measurement in the TAGMH crate. Continue, %d,  %d  \n" RESET, tagmh_ti_offset, tiGetFiberLatencyMeasurement() );
      daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the TAGMH crate. Continue, %d,  %d  \n", tagmh_ti_offset , tiGetFiberLatencyMeasurement() );
    }

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(tagmh_ti_offset,152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in TAGMH crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in TAGMH crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }         
  }
  
  // Check TAGH1 fiber latency first and write to the fiberDelay register if it fails the measurement test
  
  if(strncmp(roc_name, "roctagh1",  8) == 0) {    
    ti_delta_t = tiGetFiberLatencyMeasurement() - tagh1_ti_offset;
    if(abs(ti_delta_t) > max_dt){
      wrong_ti_dt = 1;
      printf(BOLDRED       "Wrong Fiber Latency Measurement in the TAGH1 crate. Continue, %d,  %d  \n" RESET, tagh1_ti_offset, tiGetFiberLatencyMeasurement() );
      daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the TAGH1 crate. Continue, %d,  %d  \n", tagh1_ti_offset , tiGetFiberLatencyMeasurement() );
    }
    
    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(tagh1_ti_offset,152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in TAGH1 crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in TAGH1 crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }         
  }

  // Check TAGM1 fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "roctagm1",  8) == 0) {    
    ti_delta_t = tiGetFiberLatencyMeasurement() - tagm1_ti_offset;
    if(abs(ti_delta_t) > max_dt){
      wrong_ti_dt = 1;
      printf(BOLDRED       "Wrong Fiber Latency Measurement in the TAGM1 crate. Continue, %d,  %d  \n" RESET, tagm1_ti_offset, tiGetFiberLatencyMeasurement() );
      daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the TAGM1 crate. Continue, %d,  %d  \n", tagm1_ti_offset , tiGetFiberLatencyMeasurement() );
    }

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(tagm1_ti_offset,152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in TAGM1 crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in TAGM1 crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }         
  }


  // Check STPSC1 fiber latency first and write to the fiberDelay register if it fails the measurement test

  if(strncmp(roc_name, "rocstpsc1",  9) == 0) {    
    ti_delta_t = tiGetFiberLatencyMeasurement() - stpsc1_ti_offset;
    if(abs(ti_delta_t) > max_dt){
      wrong_ti_dt = 1;
      printf(BOLDRED       "Wrong Fiber Latency Measurement in the STPSC1 crate. Continue, %d,  %d  \n" RESET, stpsc1_ti_offset, tiGetFiberLatencyMeasurement() );
      daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the STPSC1 crate. Continue, %d,  %d  \n", stpsc1_ti_offset , tiGetFiberLatencyMeasurement() );       
    }

    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(stpsc1_ti_offset,152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in STPSC1 crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in STPSC1 crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }             
  }


  // Check ST fiber latency first and write to the fiberDelay register if it fails the measurement test

  if((strncmp(roc_name, "rocst",  5) == 0) && (strlen(roc_name) == 5)) {  
    ti_delta_t = tiGetFiberLatencyMeasurement() - st_ti_offset;
    if(abs(ti_delta_t) > max_dt){
      wrong_ti_dt = 1;
      printf(BOLDRED       "Wrong Fiber Latency Measurement in the ST crate. Continue, %d,  %d  \n" RESET, st_ti_offset, tiGetFiberLatencyMeasurement() );
      daLogMsg("CONSOLE",  "Wrong Fiber Latency Measurement in the ST crate. Continue, %d,  %d  \n", st_ti_offset , tiGetFiberLatencyMeasurement() );       
    }
    
    if(wrong_ti_dt == 1){
      
      rc = tiSetFiberDelay(st_ti_offset,152);
      
      if(rc != 0){
        printf(BOLDRED "  Wrong content of TI fiberSyncDelay register in ST crate  %d \n" RESET, rc);
        daLogMsg("ERROR","Wrong content of TI fiberSyncDelay register in ST crate  %d  \n", rc);
        ROL_SET_ERROR;
      } else wrong_ti_dt = 0;
      
    }             

  }


  if(wrong_ti_dt == 1){
    printf( BOLDRED   "Wrong TI FIBER MEASUREMENT: DELTA T  =  %d \n\n" RESET, ti_delta_t);
    daLogMsg("ERROR", "Wrong TI FIBER MEASUREMENT: DELTA T  =  %d   \n",ti_delta_t);
    ROL_SET_ERROR;
  }



}
