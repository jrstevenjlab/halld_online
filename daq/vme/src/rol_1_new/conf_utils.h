#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


#ifndef UINT16
#define UINT16 unsigned short
#endif

#define ROCLEN     80       /* length of ROC_name */
#define STRLEN    250       /* length of str_tmp */

#define NBD_FADC250        22
#define MAX_FADC250_CH     16

#define NBD_FADC125        22
#define MAX_FADC125_CH     72

#define NBD_DSC2           22
#define MAX_DSC2_CH        16

#define NBD_TDC1290        22
#define NBD_F1TDC          22

#define NBD_SSP            22

#define MAX_TRIG           16

#define MAX_EQ             11

#define MAX_PB_SAMP         32


#define F1_BLOCKERROR_NO_ERROR          0
#define F1_BLOCKERROR_TERM_ON_WORDCOUNT 1
#define F1_BLOCKERROR_UNKNOWN_BUS_ERROR 2
#define F1_BLOCKERROR_ZERO_WORD_COUNT   3
#define F1_BLOCKERROR_DMADONE_ERROR     4
#define F1_BLOCKERROR_NTYPES            5


const int fdc_ti_offset[14] = {20, 19, 20, 20, 20,
                               20, 20, 25, 26, 26,
                               24, 26, 26, 26};

const int cdc_ti_offset[14]  = {19, 20, 25, 25};

const int bcal_ti_offset[12]  = {28, 29, 29, 29, 29,
                                30, 26, 27, 27, 26,
                                27, 27};

const int fcal_ti_offset[12] = {87, 88, 88, 88, 89,
                                89, 88, 89, 89, 88,
                                89, 89};

const int ecal_ti_offset[7] = {96, 96, 96, 96, 96, 96, 96};


const int tof_ti_offset[2]  =  {93, 94};

const int ps_ti_offset[2]   =  {18, 20};

const int tagmh_ti_offset   =  143;

const int tagh1_ti_offset   =  141;

const int tagm1_ti_offset   =  145;

const int stpsc1_ti_offset  =  19;

const int st_ti_offset      =  18;

const int trig_ti_offset    =  3;

int  NFADC_250;
int  NFADC_125;  

char roc_name[ROCLEN];
char filename[STRLEN];

int CALIBRATION_FLAG = 0;

int use_gem = 0;

int nfadc125 = 0;

int use_trd  = 0;

int use_tabs = 0;

int use_muon = 0;

int bufferLevel = 1;

const int ts_rule_scale[4][2] = { {16, 480}, 
                                  {16, 960},
                                  {32, 2048},
                                  {64, 4096}};


const int FA125_NPEAK_MAX   = 63;

const int FA125_IE_MAX   = 1023;

const int FA125_IBIT_MAX = 7;
const int FA125_ABIT_MAX = 3;
const int FA125_PBIT_MAX = 3;

const int FA125_PG_MAX   = 7;
const int FA125_P1_MAX   = 7;
const int FA125_P2_MAX   = 7;

const int FA125_TH_MAX   = 511;
const int FA125_TL_MAX   = 255;


const int min_gtp_latency = 300;



typedef enum { 
  fcal          =    10,
  ecal          =    15,
  bcal          =    20,
  cdc           =    30,
  fdc           =    40,
  tof           =    50,
  st            =    60,
  ps            =    70,
  psc           =    80,
  tagm          =    90,
  tagh          =    100,
  trigger       =    200,
  tpol          =    300,
  trd           =    400,
  tabs          =    500, 
  muon          =    600,
  ccal          =    700,
  dirc          =    800,
  gem           =    900,
  global        =    1000,
  det_unknown   =   -1
} det_type;


typedef enum { 

  fadc250      =   5, 
  fadc125      =   15,
  f1tdc        =   6, 
  tdc1290      =   16,
  dsc2         =   10,
  tsp          =   30,
  ssp_mod      =   40,
  gtp          =   50, 
  bd_unknown   =  -1
} bd_type;



typedef struct {
  int  group;
  int  f_rev;
  int  b_rev;
  int  b_ID;
  char SerNum[80];

  char crate[80];

  char conf_common_dir[STRLEN];
  char conf_common_ver[STRLEN];

  char conf_user_dir[STRLEN];
  char conf_user_ver[STRLEN];

  char conf_pb_dir[STRLEN];
  char conf_pb_ver[STRLEN];

  int level;

  int          mode;
  unsigned int winOffset;
  unsigned int winWidth;
  int  nsb;
  unsigned int nsa;
  unsigned int npeak;

  unsigned int chDisMask;
  unsigned int trigMask;

  unsigned int dac[MAX_FADC250_CH];
  unsigned int read_thr[MAX_FADC250_CH];

  unsigned int nsat;

  unsigned int nped;
  unsigned int max_ped;

  unsigned int trig_bl[MAX_FADC250_CH];

  unsigned int trig_thr;
  unsigned int trig_nsb;
  unsigned int trig_nsa;
  unsigned int trig_nsat;

  /* Busy and stop processing conditions */
  unsigned int busy;
  unsigned int stop;

  unsigned int mon_nped;
  unsigned int mon_max_ped;

  unsigned int amp_pb[MAX_FADC250_CH][MAX_PB_SAMP];

  unsigned int data_format;


} FADC250_CONF;



typedef struct {
  int  group;
  int  f_rev;
  int  b_rev;
  int  p_rev;
  int  b_ID;
  char SerNum[80];

  char crate[80];

  char conf_common_dir[STRLEN];
  char conf_common_ver[STRLEN];

  char conf_user_dir[STRLEN];
  char conf_user_ver[STRLEN];

  int level;


  int          mode;
  unsigned int winOffset;
  unsigned int winWidth;
  unsigned int nsb;
  unsigned int nsa;
  unsigned int npeak;
  
  unsigned int PL;
  /* unsigned int NW;   This handled by legacy "winWidth" above" */
  /* unsigned int NPK;  This handled by legacy "npeak" above" */
  unsigned int P1;
  unsigned int P2;
  unsigned int PG;
  unsigned int IE;
  unsigned int H;
  unsigned int TH;
  unsigned int TL;
  unsigned int IBIT;
  unsigned int ABIT;
  unsigned int PBIT;

  unsigned int mask[4];

  unsigned int ch_disable[3];

  unsigned int dac[MAX_FADC125_CH];

  unsigned int TL_CH[MAX_FADC125_CH];
  unsigned int TH_CH[MAX_FADC125_CH];


  unsigned int read_thr[MAX_FADC125_CH];
  float bl[MAX_FADC125_CH];
  float sig[MAX_FADC125_CH];

  float thr_fact;

  /* Busy and stop processing conditions */
  unsigned int busy;
  unsigned int stop;
  
  unsigned int data_format;

} FADC125_CONF;


typedef struct {
  int    group;
  int    f_rev;
  int    b_ID;
  char   SerNum[12];

  UINT16 TDCChMask;
  UINT16 TRGChMask;
  UINT16 TDCORMask;
  UINT16 TRGORMask;

  int TDCWidth;
  int TRGWidth;
  int TDCThreshold[MAX_DSC2_CH];
  int TRGThreshold[MAX_DSC2_CH];
  int TRGDigitalWidth[MAX_DSC2_CH];
  int TRGDigitalDelay[MAX_DSC2_CH];

  char conf_common_dir[STRLEN];
  char conf_common_ver[STRLEN];
  
} DSC2_CONF;



typedef struct {
  int  group;
  int  f_rev;
  char SerNum[80];

  int window_width;
  int window_offset;
  int window_extra;
  int window_reject;

  int n_Hits;
  int almostFullLevel;
  int outProgControl;
  int blt_Events;

  int cblt_not;
  int a24_a32;
  int sngl_blt_mblt;
  int berr_fifo;
  int sst_rate;

  int    edge;
  unsigned int mask;

  int rc_adjust[4][4];   // adjustment of R-C delay line.  Chip 0 - 3, Tap 0 - 3

  int inl;

} TDC1290_CONF;


typedef struct {
  int  group;
  int  f_rev;
  int  b_rev;
  int  b_ID;
  char SerNum[12];

  int version;

  float  bin_size;
  float  latency;
  float  window;
  int    clock;
  
  int hsdiv;     // calculated and filled when configuring F1
  int refclkdiv; // calculated and filled when configuring F1
  int refcnt;    // calculated and filled when configuring F1

  int global;

  unsigned long long int mask;

} F1TDC_CONF;


typedef struct {
  int  group;
  int  f_rev;
  int  b_rev;
  int  b_ID;
  char SerNum[12];

  int used;

  int version;

  
  int BCALThreshold;
  int BCALWindowWidth;
  unsigned int FinalSumThreshold;
  int HitWindowWidth;

} CTP_CONF;


// typedef struct {  
//  int slot;
//  unsigned int fiber_en;
//  int  sum_enable;
// } SSP_CONF;


typedef struct {
  char type[20];
  
  int   delay;
  int   int_width;  
  float gain_ecal;
  float gain_fcal;
  int   enable;

} GTP_EQUATION;

typedef struct {
  float gain_ecal;
  float gain_fcal;
} GTP_GAIN;


typedef struct {
  char type[20];
  
  int  latency;
  int  width;

  int  ecal_en;
  int  fcal_en;
  int  ccal_en;
  int  bcal_en;
  int  en_thr;

  int  gtp_lane;
  
  int  nhit;
  
  unsigned int mask;
  
  int ecal_min;
  int ecal_max;

  int fcal_min;
  int fcal_max;

  int ccal_min;
  int ccal_max;

  int bcal_min;
  int bcal_max;

} GTP_TRIGGER;


/** TI configuration parameters **/
typedef struct {
  unsigned int slot;
  unsigned int ver;         /* version */
  unsigned int rev;         /* revision */
  unsigned int crate_id;    /* crate_id: 1-15 */
  unsigned int trig_src;    /* trigger source: 0-5 */
  unsigned int holdoff_rule;     /* trigger holdoff rule: 1-5 */
  unsigned int holdoff_interval; /* trigger holdoff time interval (in time_step): 1-63 */
  unsigned int holdoff_step;     /* trigger holdoff time step: 0 = 16ns, 1 = 500ns */
  unsigned int sync_src;              /* sync source: 0-4 */
  unsigned int sync_delay;            /* sync delay: 1-127 */
  unsigned int sync_width;            /* sync width: 1-127 */
  unsigned int sync_units;            /* sync units: 0 = 4ns, 1 = 32ns */
  unsigned int fiber_delay;           /* fiber delay */
  unsigned int fiber_latency;         /* fiber offset */
  unsigned int busy_src;              /* busy source mask: bitmask: bits: 16-0 */
  unsigned int busy_flag;             /* busy source flag: 0-1 */
  unsigned int block_level;           /* block level: 1-15 */
  unsigned int event_format;          /* event format: 0-3 */
  unsigned int block_bflevel;         /* block buffer level: 0-65535 */
  unsigned int out_port_mask;         /* output port mask: bits: 3-0 (p4,p3,p2,p1) */
  unsigned int clk_src;               /* clock source: 0 = internal, 1 = external */
  unsigned int ti_master;             /* Stand alone runs with master TI - 1,  0 - TS is used */
  unsigned int ti_master_trig;        /* TI master trigger type: 1 - pulser; 2 - internal */
  unsigned int ti_fp_inputs;          /* select trigger source input: bitmask: bit5 bit4 bit3 bit2 bit1 bit0 (inp6, inp5, inp4, inp3, inp2, inp1) */
  unsigned int ti_fiber_en;           /* optical fiber ports enabled on master TI: port1 - port8 */
  unsigned int ti_soft_trig[4];       /* enable software trigger: [0] - trigger type 1 or 2 (playback)
                                                                  [1] - number of events to trigger
                                                                  [2] - period multiplier, depends on range (0-0x7FFF)
                                                                  [3] - small (0) or large (1)
                                                                        small: minimum of 120ns, increments of 30ns up to 983.13us
                                                                        large: minimum of 120ns, increments of 30.72us up to 1.007s */
} board_ti;


typedef struct {
  unsigned int slot;
  unsigned int ver;         /* version */
  unsigned int rev;         /* revision */
  unsigned int crate_id;    /* crate_id: 1-15 */

  unsigned int trig_type;             /*  TS trigger type: 1 - pulser; 2 - internal; 4 - GTP; 6 GTP + External */
  unsigned int ts_fp_inputs;          /*  TS front pannel inputs: input B1 - bit 0         */
  unsigned int ts_fp_delay[32];       /*  Delay triggers for TS FP  */ 
  unsigned int ts_td_slots;           /*  slots for TD boards in the crate: slot3 - slot20 */
  unsigned int ts_soft_trig[4];       /*  enable software trigger: [0] - trigger type 1 or 2 (playback)
					                           [1] - number of events to trigger
					                           [2] - period multiplier, depends on range (0-0x7FFF)
                                                                   [3] - small (0) or large (1)
                                                                         small: minimum of 120ns, increments of 30ns up to 983.13us
                                                                         large: minimum of 120ns, increments of 30.72us up to 1.007s  */
  unsigned  int gtp_pres[32];           /*  GTP trigger prescaling factor  */

  unsigned  int ts_coin_wind; 
  
  unsigned  int ts_trig_hold[4][2];     /*    * Trigger Holdoff rules  */
                                        /*      4 - rule
                                                rule 1:   16 ns / 480 ns step 
                                                rule 2:   16 ns / 960 ns step
                                                rule 3:   32 ns / 2 usec step 
                                                rule 4:   64 ns / 4 usec step     */

  unsigned int ts_sync_int;           /*    Sync. event interval           */
                                      /*        0 - sync events disabled   */
  unsigned int ts_sync_opt;           /*    Options for sync events        */


  unsigned  int  blocklevel;            /* block level */
  unsigned  int  bufferlevel;           /* buffer level */
} board_ts;





FADC250_CONF fa250[NBD_FADC250];

FADC125_CONF fa125[NBD_FADC125];

DSC2_CONF dsc[NBD_DSC2];

TDC1290_CONF caen1290[NBD_TDC1290];

F1TDC_CONF  f1[NBD_F1TDC];

CTP_CONF  ctp;

// SSP_CONF  ssp[NBD_SSP];

GTP_TRIGGER    gtp_trig[MAX_TRIG];

GTP_EQUATION   gtp_eq[MAX_EQ];

GTP_GAIN      gtp_gain;

board_ti ti_bd;

board_ts ts_bd;

// Some global variables

unsigned int trig_delay = 0;

unsigned int dac_calib  = 1;

unsigned int use_playback = 0;


det_type  get_det_type(char *crate );
bd_type   get_board_type(char *crate );
char *get_board_type_name(char* crate);


int read_conf_global(char *filename, char *crate);
int read_conf_common(char *crate);
int read_conf_user(char *crate);
int read_conf_pb(char *crate);

void print_fadc250_conf(int slot);

void print_fadc250_pb(int slot);
void print_fadc250_pb_all();

int  get_nbd_fadc250();

FADC250_CONF get_conf_fadc250(int slot);
FADC125_CONF get_conf_fadc125(int slot);
DSC2_CONF  get_conf_dsc2(int slot);

int  get_nbd_fadc125();
void print_fadc125_conf(int slot);

void print_le_conf();

void print_caen1290_conf();

void print_f1_conf();


void print_ti_conf();

void print_ts_conf();

void print_ctp_conf();

void print_ssp_conf();

void print_gtp_conf();

void ssp_init();

void init_all();

int check_baseline(int *);

void ts_scalers();

void fa250_set_block_level(int level);
void fa125_set_block_level(int level);


int  fa250_pedestals();
void fa250_scalers();


int fa250_read_conf(int slot, FADC250_CONF *fa250_read_reg);
int fa250_check_conf();

int fa125_read_conf(int slot, FADC125_CONF *fa125_read_reg);
int fa125_check_conf();

int dsc2_read_conf(int slot, DSC2_CONF *dsc2_read_reg);
int dsc2_check_conf();



#define SCAN_MSK                                         \
  args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
                                     %d %d %d %d %d %d %d %d", \
                       &msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3],       \
                       &msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7],       \
                       &msk[ 8], &msk[ 9], &msk[10], &msk[11],             \
                       &msk[12], &msk[13], &msk[14], &msk[15])

#define SCAN_MSK1					       \
  args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
                                     %d %d %d %d %d %d %d %d %d %d", \
                       &msk1[ 0], &msk1[ 1], &msk1[ 2], &msk1[ 3],            \
		       &msk1[ 4], &msk1[ 5], &msk1[ 6], &msk1[ 7],	     \
		       &msk1[ 8], &msk1[ 9], &msk1[10], &msk1[11],	     \
		       &msk1[12], &msk1[13], &msk1[14], &msk1[15], &msk1[16], &msk1[17] )


#define SCAN_MSKF					       \
  args = sscanf (str_tmp, "%*s %f %f %f %f %f %f %f %f   \
                                     %f %f %f %f %f %f %f %f %f %f", \
		       &mskf[ 0], &mskf[ 1], &mskf[ 2], &mskf[ 3],	     \
		       &mskf[ 4], &mskf[ 5], &mskf[ 6], &mskf[ 7],	     \
		       &mskf[ 8], &mskf[ 9], &mskf[10], &mskf[11],	     \
		       &mskf[12], &mskf[13], &mskf[14], &mskf[15], &mskf[16], &mskf[17] )


#define SCAN_PB_MSK                                      \
  args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d  \
                               %d %d %d %d %d %d %d %d              \
                               %d %d %d %d %d %d %d %d              \
                               %d %d %d %d %d %d %d %d ", \
                 &mskpb[ 0], &mskpb[ 1], &mskpb[ 2], &mskpb[ 3], &mskpb[ 4], &mskpb[ 5], &mskpb[ 6], &mskpb[ 7], \
                 &mskpb[ 8], &mskpb[ 9], &mskpb[10], &mskpb[11], &mskpb[12], &mskpb[13], &mskpb[14], &mskpb[15], \
                 &mskpb[16], &mskpb[17], &mskpb[18], &mskpb[19], &mskpb[20], &mskpb[21], &mskpb[22], &mskpb[23], \
                 &mskpb[24], &mskpb[25], &mskpb[26], &mskpb[27], &mskpb[28], &mskpb[29], &mskpb[30], &mskpb[31])




#define GET_READ_MSK							\
  SCAN_MSK;								\
  ui1 = 0;								\
  for(jj = 0; jj < MAX_FADC250_CH; jj++){				\
  if((msk[jj] < 0) || (msk[jj] > 1)){                                   \
    msk[jj] = 1;                                                        \
  }									\
  if(msk[jj] == 3){                                                     \
    msk[jj] = 0;                                                        \
  }                                                                     \
  ui1 |= (msk[jj]<<jj);                                                 \
  }

//	printf("\nReadConfigFile: Wrong mask bit value, %d\n\n",msk[jj]); return(-6); 




#define MSK_EQ							       \
  args = sscanf (str_tmp, "%*s %s %d %d %d  ",			       \
		 (char *)&eq_msk_type, &eq_msk[ 0], &eq_msk[ 1], &eq_msk[ 2] ) \
    
#define MSK_TRIG							\
  args = sscanf (str_tmp, "%*s %s %d %d %d %d %d %d %d %d %d %d %d %x",		\
		 (char *)&trg_msk_type, &trg_msk[0], &trg_msk[1], &trg_msk[2], &trg_msk[3], &trg_msk[4], &trg_msk[5], &trg_msk[6], &trg_msk[7], &trg_msk[8], &trg_msk[9], &trg_msk[10], &trg_det_mask) \

#define MSK_TRIG_ECAL							\
  args = sscanf (str_tmp, "%*s %s %d %d %d %d %d %d %d %d %d %d %d %d %d",		\
		 (char *)&trg_msk_type, &trg_msk_ecal[0], &trg_msk_ecal[1], &trg_msk_ecal[2], &trg_msk_ecal[3], &trg_msk_ecal[4], &trg_msk_ecal[5], &trg_msk_ecal[6], &trg_msk_ecal[7], &trg_msk_ecal[8], &trg_msk_ecal[9], &trg_msk_ecal[10], &trg_msk_ecal[11], &trg_msk_ecal[12] ) \


#define MSK_RC								\
  args = sscanf (str_tmp, "%*s %d %d %d %d %d %d",			\
		 &msk_rc[0], &msk_rc[1], &msk_rc[2], &msk_rc[3], &msk_rc[4], &msk_rc[5] ) \


#define SCAN_RC(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    slot_rc = -1;							\
    MSK_RC;								\
    if(args == 6){	/* slot, chip, and 4 tap values have to be set */ \
      if(msk_rc[0] < 3 || msk_rc[0] > 8){				\
	printf(BOLDRED "\n Set CAEN TDC adjustments: Wrong slot number %d, %s\n" RESET, msk_rc[0],str_tmp); \
	return(-2);							\
      }									\
      if(msk_rc[1] < 0 || msk_rc[1] > 3){				\
	printf(BOLDRED "\n Set CAEN TDC adjustments: Wrong CHIP number for TDC in slot %d:  %d, %s\n" RESET, msk_rc[0], msk_rc[1], str_tmp); \
	return(-2);							\
      }									\
      slot_rc = msk_rc[0];						\
      for(jj = 2; jj < 6; jj++){					\
	if(msk_rc[jj] < 0 || msk_rc[jj] > 7){				\
	  printf(BOLDRED "\n Set CAEN TDC adjustments: Wrong TAP values for Slot: %d, Chip: %d  %d %s\n" RESET, msk_rc[0], msk_rc[1], msk_rc[jj], str_tmp); \
	  return(-2);							\
	} else {							\
	  (BSTRUCT)[msk_rc[1]][jj - 2] =  msk_rc[jj];			\
	}								\
      }									\
    }									\
  }									\
  



#define SCAN_SLOT(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    slot = -1;								\
    SCAN_MSK;								\
    if(args == 1){	/* Don't allow groups. Each Slot should have an entry in the file */ \
      for(jj = 0; jj < args; jj++){					\
	slot = msk[jj];							\
	if(slot < 3 || slot == 11 || slot == 12 || slot > 20){		\
	  printf(BOLDRED "\nRead_conf: Wrong slot number %d, %s\n" RESET, slot,str_tmp); \
	  return(-2);							\
	}								\
	(BSTRUCT) = 1;                                                  \
      }									\
    } else {								\
      printf(BOLDRED "\n Read_conf:  Wrong number of slots in the FADC_SLOTS card  %d. Only one slot is allowed. Exit \n"  RESET, args); \
      return(-2);							\
    }									\
  }									


#define SCAN_DSC_SLOT(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    slot = -1;								\
    SCAN_MSK;								\
    if(args == 1){	/* Don't allow groups. Each Slot should have an entry in the file */ \
      for(jj = 0; jj < args; jj++){					\
	slot = msk[jj];							\
	if(slot < 2 || slot > 21){		\
	  printf(BOLDRED "\nRead_conf: Wrong slot number %d, %s\n" RESET, slot,str_tmp); \
	  return(-2);							\
	}								\
	(BSTRUCT) = 1;                                                  \
      }									\
    } else {								\
      printf(BOLDRED "\n Read_conf:  Wrong number of slots in the DSC2_SLOTS card  %d. Only one slot is allowed. Exit \n"  RESET, args); \
      return(-2);							\
    }									\
  }		



#define SCAN_PB_SLOT(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    args =  sscanf (str_tmp, "%*s %d", &val);                           \
    if(args == 1){	/*  Each Slot should have an entry in the file */ \
      if(val < 3 || val == 11 || val == 12 || val > 20){                \
        printf(BOLDRED "\nRead_conf_pb: Wrong slot number %d, %s\n" RESET, val, str_tmp); \
        return(-2);                                                     \
      }                                                                 \
      (BSTRUCT) = val;                                                  \
    } else {								\
      printf(BOLDRED "\n Read_conf_pb:  Wrong number of slots in  FADC_SLOTS  %d. Only one slot is allowed, %s\n"  RESET, args, str_tmp); \
      return(-2);                                                       \
    }									\
  }		


#define SCAN_PB_CHAN(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    args =  sscanf (str_tmp, "%*s %d", &val);                           \
    if(args == 1){	/*  Each Slot should have an entry in the file */ \
      if(val < 0 || val > 15){                                          \
        printf(BOLDRED "\nRead_conf_pb: Wrong channel number %d, %s\n" RESET, val, str_tmp); \
        return(-2);                                                     \
      }                                                                 \
      (BSTRUCT) = val;                                                  \
    } else {								\
      printf(BOLDRED "\n Read_conf_pb:  Wrong number of arguments for Play Back channel %d, %s\n"  RESET, args, str_tmp); \
      return(-2);                                                       \
    }									\
  }		

#define SCAN_PB_AMP(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    SCAN_PB_MSK;                                                        \
    if(args == MAX_PB_SAMP){                                                     \
      for(jj = 0; jj < MAX_PB_SAMP; jj++){                                       \
        if( (mskpb[jj] >= 0) && (mskpb[jj] <= 4096) ){                   \
          (BSTRUCT)[jj] = mskpb[jj];                                    \
        } else {                                                        \
          printf(BOLDRED "\n Read_conf_pb:  Wrong Play Back amplitude %d, %s\n"  RESET, mskpb[jj], str_tmp); \
        }                                                               \
      }                                                                 \
    } else {								\
      printf(BOLDRED "\n Read_conf_pb:  Wrong number of arguments for Play Back amplitude %d, %s \n"  RESET, args, str_tmp); \
      return(-2);                                                       \
    }									\
  }	





#define SCAN_B_SETS(BKEYWORD,BSTRUCT,SL_MIN,SL_MAX)                     \
  if(strcmp(keyword,(BKEYWORD)) == 0)                                   \
    {									\
      sscanf (str_tmp, "%*s %d", &val);					\
      for(jj = SL_MIN; jj < SL_MAX; jj++) (BSTRUCT) = val;		\
    }



#define SCAN_B_XSETS(BKEYWORD,BSTRUCT,SL_MIN,SL_MAX)					\
  if(strcmp(keyword,(BKEYWORD)) == 0)					\
    {									\
      sscanf (str_tmp, "%*s %x", &val);					\
      for (jj = SL_MIN; jj < SL_MAX; jj++) (BSTRUCT) = val;			\
    }


#define SCAN_B_SSETS(BKEYWORD,BSTRUCT)		\
  if(strcmp(keyword,(BKEYWORD)) == 0)		\
    {									\
      args = sscanf (str_tmp, "%*s %s", &str_val[0]);			\
      if(args == 1)							\
	{								\
	  for(jj = 3; jj < NBD_FADC250; jj++) sprintf( (char *) (BSTRUCT), "%s", str_val); \
	} else printf(" Warning: no values set to either FADC250_COMMON or FADC250_USER \n"); \
    }

#define SCAN_DSC_SSETS(BKEYWORD,BSTRUCT)		\
  if(strcmp(keyword,(BKEYWORD)) == 0)		\
    {									\
      args = sscanf (str_tmp, "%*s %s", &str_val[0]);			\
      if(args == 1)							\
	{								\
	  for(jj = 3; jj < NBD_DSC2; jj++) sprintf( (char *) (BSTRUCT), "%s", str_val); \
	} else printf(" Warning: no values set to DSC_COMMON \n"); \
    }


#define SCAN_TDP(TDP_K, TDP_S,SL_MIN,SL_MAX) \
  if(strcmp(keyword,(TDP_K)) == 0)		\
    {						\
      sscanf (str_tmp, "%*s %d", &val);			      \
      for(jj = SL_MIN; jj < SL_MAX; jj++)		      \
	for(ii = 0; ii < MAX_FADC250_CH; ii++)   (TDP_S)[ii] = val;	      \
    }


#define SCAN_TDP_125(TDP_K,TDP_S) \
  if(strcmp(keyword,(TDP_K)) == 0)		\
    {						\
      sscanf (str_tmp, "%*s %d", &val);			      \
      for(jj = 3; jj < NBD_FADC125; jj++)		      \
	for(ii = 0; ii < MAX_FADC125_CH; ii++)   (TDP_S)[ii] = val;	      \
    }


#define SCAN_SSP(TDP_K,TDP_S)						\
  if(strcmp(keyword,(TDP_K)) == 0)					\
    {									\
      args = sscanf (str_tmp, "%*s %d %x %d", &val_ssp[0], &val_ssp[1], &val_ssp[2]); \
      if(args == 3){							\
	if(val_ssp[0] < 3 || val_ssp[0] > 20 || val_ssp[0] == 11 || val_ssp[0] == 12){ \
	  printf(BOLDRED " SSP_SLOT: Wrong SLOT  %d  \n" RESET, val_ssp[0] ); \
	  return(-8);							\
	}								\
	(TDP_S)[val_ssp[0]].slot = val_ssp[0];				\
	(TDP_S)[val_ssp[0]].fiber_en = val_ssp[1];			\
        (TDP_S)[val_ssp[0]].sum_enable = val_ssp[2];			\
      } else {								\
	printf(BOLDRED " SSP_SLOT: Argument is missing  \n" RESET );	\
	return(-8);							\
      }									\
    }

#define SCAN_TS_GTP_PRES(TDP_K,TDP_S)				       	\
  if(strcmp(keyword,(TDP_K)) == 0) {					\
    args = sscanf (str_tmp, "%*s %d %d", &gtp_pres[0], &gtp_pres[1] );	\
    if(args == 2){							\
      if(gtp_pres[0] >= 0  && gtp_pres[0] < 32) {			\
	if(gtp_pres[1] < 0 || gtp_pres[1] > 0xF){			\
	  printf(BOLDRED "TS_GTP_PRES: Wrong prescaling range %d \n" RESET, gtp_pres[1] ); \
	  return(-8);							\
	} else								\
	  (TDP_S)[gtp_pres[0]]  = gtp_pres[1];				\
      } else {								\
	printf(BOLDRED "TS_GTP_PRES: Wrong GTP line selected %d \n" RESET, gtp_pres[0] ); \
	return(-8);							\
      }									\
    } else {								\
      printf(BOLDRED "TS_GTP_PRES: Wrong number of arguments %d \n" RESET, args ); \
      return(-8);							\
    }									\
  }


#define SCAN_TS_COIN_WIND(TDP_K,TDP_S)				       	\
  if(strcmp(keyword,(TDP_K)) == 0) {					\
    args = sscanf (str_tmp, "%*s  %d", &coin_wind);			\
    if(args == 1){							\
      if(coin_wind > 0  && coin_wind < 256) {				\
	(TDP_S)  = coin_wind;						\
      } else {								\
	printf(BOLDRED "TS_COIN_WIND: Wrong value of TS_COIN_WIND %d \n" RESET, coin_wind ); \
	return(-8);							\
      }									\
    } else {								\
      printf(BOLDRED "TS_COIN_WIND: Wrong number of arguments %d \n" RESET, args ); \
      return(-8);							\
    }									\
  }


#define SCAN_TS_TRIG_HOLD(TDP_K,TDP_S)				       	\
  if(strcmp(keyword,(TDP_K)) == 0) {                                    \
    args = sscanf (str_tmp, "%*s  %d  %d   %d",  &trig_hold[0],  &trig_hold[1], &trig_hold[2] ); \
    if(args == 3){                                                      \
      if(trig_hold[0] < 1 || trig_hold[0] > 4){                         \
        printf(BOLDRED "TS_TRIG_HOLD: Wrong range = %d \n" RESET, trig_hold[0] ); \
        return(-8);                                                     \
      }									\
      if(trig_hold[1] < 0 ||  trig_hold[1] > 127){                      \
        printf(BOLDRED "TS_TRIG_HOLD: Wrong value = %d \n" RESET, trig_hold[1] ); \
        return(-8);							\
      }                                                                 \
      if(trig_hold[2] < 0 ||  trig_hold[2] > 1){                        \
        printf(BOLDRED "TS_TRIG_HOLD: Wrong range = %d \n" RESET, trig_hold[2] ); \
        return(-8);							\
      }                                                                 \
      (TDP_S)[trig_hold[0] - 1][0]  = trig_hold[1];                         \
      (TDP_S)[trig_hold[0] - 1][1]  = trig_hold[2];                         \
    } else {								\
      printf(BOLDRED "TS_TRIG_HOLD: Wrong number of arguments %d \n" RESET, args ); \
      return(-8);                                                       \
    }									\
  }


#define SCAN_TS_FP_DELAY(TDP_K,TDP_S)				       	\
  if(strcmp(keyword,(TDP_K)) == 0) {					\
    args = sscanf (str_tmp, "%*s  %d   %d", &trig_delay[0], &trig_delay[1]); \
    if(args == 2){							\
      if(trig_delay[0] <= 0 || trig_delay[0] > 32){			\
	printf(BOLDRED "TS_TRIG_DELAY: Wrong TS FP input = %d \n" RESET, trig_delay[0] ); \
	return(-8);							\
      }									\
      if(trig_delay[1] < 0 || trig_delay[1] > 0x1FF){				\
	printf(BOLDRED "TS_TRIG_DELAY: Wrong TS FP range = %d \n" RESET, trig_delay[1] ); \
	return(-8);							\
      }									\
      (TDP_S)[trig_delay[0]-1]  = trig_delay[1];			\
    } else {								\
      printf(BOLDRED "TS_TRIG_DELAY: Wrong number of arguments %d \n" RESET, args ); \
      return(-8);							\
    }									\
  }



#define SCAN_EQ(TDP_K,TDP_S)						\
  if(strcmp(keyword,(TDP_K)) == 0){					\
    if(ngtp_eq >= MAX_EQ){						\
      printf(BOLDRED " TRIG_EQ:  Too many GTP equations defined  %d  \n" RESET, ngtp_eq ); \
      return(-8);							\
    }									\
    MSK_EQ;								\
    sprintf((char *)(TDP_S)[ngtp_eq].type, "%s", eq_msk_type);		\
    (TDP_S)[ngtp_eq].delay     = eq_msk[0];				\
    (TDP_S)[ngtp_eq].int_width = eq_msk[1];				\
    (TDP_S)[ngtp_eq].enable    = eq_msk[2];				\
    ngtp_eq++;								\
 }


#define SCAN_GTP_GAIN(TDP_K,TDP_S)				       	\
  if(strcmp(keyword,(TDP_K)) == 0) {					\
    args = sscanf (str_tmp, "%*s  %f  %f", &gain_ecal_tmp, &gain_fcal_tmp ); \
    if(args == 2){							\
      if(gain_ecal_tmp < 0   ||  gain_ecal_tmp > 10 ||			\
         gain_fcal_tmp < 0   ||  gain_fcal_tmp > 10){			\
        printf(BOLDRED "SCAN_GTP_GAIN: Wrong gain range %f  %f  \n" RESET, gain_ecal_tmp, gain_fcal_tmp ); \
        return(-8);							\
      } else{								\
        (TDP_S).gain_ecal  = gain_ecal_tmp;				\
        (TDP_S).gain_fcal  = gain_fcal_tmp;                             \
      }                                                                 \
    } else {								\
      printf(BOLDRED "SCAN_GTP_GAIN: Wrong number of arguments %d \n" RESET, args ); \
      return(-8);							\
    }									\
  }


#define SCAN_ECAL_TRIG(TDP_K,TDP_S)                                     \
  if(strcmp(keyword,(TDP_K)) == 0){					\
    if(ngtp_trg >= MAX_TRIG){						\
      printf(BOLDRED " ECAL_TRIG_TYPE:  Too many GTP equations defined  %d  \n" RESET, ngtp_trg ); \
      return(-8);							\
    }									\
    MSK_TRIG_ECAL;                                                      \
    sprintf((char *)(TDP_S)[ngtp_trg].type, "%s", trg_msk_type);	\
    (TDP_S)[ngtp_trg].latency     = trg_msk[0];				\
    (TDP_S)[ngtp_trg].width       = trg_msk[1];				\
    (TDP_S)[ngtp_trg].ecal_en     = trg_msk[2];				\
    (TDP_S)[ngtp_trg].fcal_en     = trg_msk[3];				\
    (TDP_S)[ngtp_trg].bcal_en     = trg_msk[4];				\
    (TDP_S)[ngtp_trg].en_thr      = trg_msk[5];				\
    (TDP_S)[ngtp_trg].gtp_lane    = trg_msk[6];				\
    (TDP_S)[ngtp_trg].ecal_min    = trg_msk[7];                         \
    (TDP_S)[ngtp_trg].ecal_max    = trg_msk[8];                         \
    (TDP_S)[ngtp_trg].fcal_min    = trg_msk[9];                         \
    (TDP_S)[ngtp_trg].fcal_max    = trg_msk[10];			\
    (TDP_S)[ngtp_trg].bcal_min    = trg_msk[11];			\
    (TDP_S)[ngtp_trg].bcal_max    = trg_msk[12];			\
    ngtp_trg++;								\
  }


#define SCAN_TRIG(TDP_K,TDP_S)						\
  if(strcmp(keyword,(TDP_K)) == 0){					\
    if(ngtp_trg >= MAX_TRIG){						\
      printf(BOLDRED " TRIG_TYPE:  Too many GTP equations defined  %d  \n" RESET, ngtp_trg ); \
      return(-8);							\
    }									\
    MSK_TRIG;								\
    sprintf((char *)(TDP_S)[ngtp_trg].type, "%s", trg_msk_type);	\
    (TDP_S)[ngtp_trg].latency     = trg_msk[0];				\
    (TDP_S)[ngtp_trg].width       = trg_msk[1];				\
    (TDP_S)[ngtp_trg].fcal_en     = trg_msk[2];				\
    (TDP_S)[ngtp_trg].bcal_en     = trg_msk[3];				\
    (TDP_S)[ngtp_trg].en_thr      = trg_msk[4];				\
    (TDP_S)[ngtp_trg].nhit        = trg_msk[5];				\
    (TDP_S)[ngtp_trg].gtp_lane    = trg_msk[6];				\
    if( (args >= 9) && (args <= 12)){					\
      (TDP_S)[ngtp_trg].fcal_min        = trg_msk[7];			\
      (TDP_S)[ngtp_trg].fcal_max        = trg_msk[8];			\
      (TDP_S)[ngtp_trg].bcal_min        = trg_msk[9];			\
      (TDP_S)[ngtp_trg].bcal_max        = trg_msk[10];			\
    }									\
    if(args == 13){							\
      (TDP_S)[ngtp_trg].mask      = trg_det_mask;			\
      printf(" Trigger Mask  Set to 0x%x \n", trg_det_mask);		\
    }									\
    ngtp_trg++;								\
  }


#define SCAN_TDP_ALLCH(TDP3_K,TDP3_S)					\
  if(strcmp(keyword,(TDP3_K)) == 0)					\
    {									\
      SCAN_MSK;								\
      if(args != 16)							\
	{								\
	  printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n",args); \
          return(-8);							\
        }								\
	for(ii = 0; ii < MAX_FADC250_CH; ii++)   (TDP3_S)[ii] = msk[ii]; \
    }


#define SCAN_TDP_ALLCH_THR(TDP3_K,TDP3_S)                               \
  if(strcmp(keyword,(TDP3_K)) == 0)					\
    {									\
      SCAN_MSK;								\
      if(args != 16)							\
	{								\
	  printf("\nReadConfigFile: Wrong number of arguments in DSC2_ALLCH_THR  %d, should be 16\n\n",args); \
          return(-8);							\
        }								\
      for(ii = 0; ii < MAX_DSC2_CH; ii++)   (TDP3_S)[ii] = msk[ii];     \
    }



#define SCAN_B_MSKS(BKEYWORD,BSTRUCT)					\
  if(strcmp(keyword,(BKEYWORD)) == 0)					\
    {									\
      GET_READ_MSK;							\
      (BSTRUCT) = ui1;							\
    }


#define SCAN_FA125_DAC_THR(BKEYWORD,BSTRUCT)					\
  if(strncmp(keyword,(BKEYWORD),15) == 0){				\
    SCAN_MSK1;								\
    for(jj = 0; jj < 18; jj++){						\
      if((msk1[jj] < 0) || (msk1[jj] > 65000)){   /* SP: threshold limits should be verified later */ \
	printf(BOLDRED "\n FATAL: Read FADC125 DAC or THR: Wrong  value, %d, jj = %d\n\n" RESET, msk1[jj],jj); \
	return(-2); \
      } \
    } \
    str2[0] = keyword[15];						\
    str2[1] = keyword[16];						\
    str2[2] = '\0';							\
    first = atoi(str2);							\
    last = first + 18;							\
    if(first < 0 || last > 72){						\
      printf(BOLDRED "\n FATAL: Read FADC125 DAC or THR: Wrong  channel range: first = %d  last = %d. Line = %d \n" RESET,first, last, line); return(-2); \
    }   \
    for(jj = first; jj < last; jj++){		\
      (BSTRUCT)[jj] = msk1[jj - first ];	\
    }							\
  }			


#define SCAN_FA125_BL(BKEYWORD,BSTRUCT)					\
  if(strncmp(keyword,(BKEYWORD),14) == 0){				\
    SCAN_MSKF;								\
    for(jj = 0; jj < 18; jj++){						\
      if((mskf[jj] < 0) || (mskf[jj] > 500)){   /* SP: threshold limits should be verified later */ \
	printf(BOLDRED "\n WARNING: Read FADC125:  BL  is larger than  500,  value =  %f, jj = %d\n\n" RESET, mskf[jj],jj); \
      } \
    } \
    str2[0] = keyword[14];						\
    str2[1] = keyword[15];						\
    str2[2] = '\0';							\
    first = atoi(str2);							\
    last = first + 18;							\
    if(first < 0 || last > 72){						\
      printf(BOLDRED "\n FATAL: Read FADC125 BL: Wrong  channel range: first = %d  last = %d. Line = %d \n" RESET,first, last, line); return(-2); \
    }   \
    for(jj = first; jj < last; jj++){		\
      (BSTRUCT)[jj] = mskf[jj - first ];	\
    }							\
  }			


#define SCAN_FA125_SIG(BKEYWORD,BSTRUCT)					\
  if(strncmp(keyword,(BKEYWORD),15) == 0){				\
    SCAN_MSKF;								\
    for(jj = 0; jj < 18; jj++){						\
      if((mskf[jj] < 0) || (mskf[jj] > 4096)){   /* SP: threshold limits should be verified later */ \
	printf(BOLDRED "\n WARNING: Read FADC125:  SIG is larger than 100, value, %f, jj = %d\n\n" RESET, mskf[jj],jj); \
      } \
    } \
    str2[0] = keyword[15];						\
    str2[1] = keyword[16];						\
    str2[2] = '\0';							\
    first = atoi(str2);							\
    last = first + 18;							\
    if(first < 0 || last > 72){						\
      printf(BOLDRED "\n FATAL: Read FADC125 SIG: Wrong  channel range: first = %d  last = %d. Line = %d \n" RESET,first, last, line); return(-2); \
    }   \
    for(jj = first; jj < last; jj++)	\
      (BSTRUCT)[jj] = mskf[jj - first ];	\
  }		


#define SCAN_FA125_MASK(TDP_K, TDP_S) \
  if(strcmp(keyword,(TDP_K)) == 0)		\
    {						\
      sscanf (str_tmp, "%*s   %x  %x  %x", &ch_dis_mask[0], &ch_dis_mask[1], &ch_dis_mask[2]); \
      for(jj = 0; jj < 3; jj++)	 (TDP_S)[jj] = ch_dis_mask[jj];		\
    }



#define SCAN_SOFT_TRIG(BKEYWORD,TI_SOFT)  \
if(strcmp(keyword,(BKEYWORD)) == 0){					\
  sscanf (str_tmp, "%*s %d %d %x %d", &soft_trig[0], &soft_trig[1], &soft_trig[2], &soft_trig[3]); \
  for(jj=0; jj < 4; jj++)(TI_SOFT)[jj] = soft_trig[jj];			\
 }


#define SCAN_TI_FIBER(BKEYWORD,TI_FIBER)				\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d", &ti_fiber[0], &ti_fiber[1], &ti_fiber[2], &ti_fiber[3], \
		   &ti_fiber[4], &ti_fiber[5], &ti_fiber[6], &ti_fiber[7]); \
    if(args > 7){							\
      printf(" TI_FIBER too many ports: EXIT %d \n",args);		\
      return(-7);							\
    }									\
    for(jj=0; jj < args; jj++){						\
      if(ti_fiber[jj] > 0 && ti_fiber[jj] < 9){				\
	(TI_FIBER) =  (TI_FIBER) | (1 << (ti_fiber[jj]-1));		\
      } else {								\
	printf(" TI_FIBER out of range %d \n",ti_fiber[jj]);		\
	return(-2);							\
      }									\
    }									\
  }


#define SCAN_TD_SLOTS(BKEYWORD,TD_SLOTS)				\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    SCAN_MSK;								\
    if(args > 16){							\
      printf(" TS_TD_SLOTS too many slots: EXIT %d \n",args);		\
      return(-2);							\
    }									\
    for(jj=0; jj < args; jj++){						\
      if( (msk[jj] > 2 && msk[jj] < 21) && (msk[jj] != 11) && (msk[jj] != 12)) { \
	(TD_SLOTS) =  (TD_SLOTS) | (1 << msk[jj]);			\
      } else {								\
	printf(BOLDRED "TS_TD_SLOTS out of range %d \n" RESET, msk[jj]);			\
	return(-2);							\
      }									\
    }									\
  }

#define SCAN_FP_INPUTS(BKEYWORD,FP_INPUTS)				\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    SCAN_MSK;								\
    if(args > 16){							\
      printf(" TS_FP_INPUTS too many inputs: EXIT %d \n",args);		\
      return(-2);							\
    }									\
    for(jj=0; jj < args; jj++){						\
      if( (msk[jj] > 0 && msk[jj] < 32) ) {				\
	(FP_INPUTS) =  (FP_INPUTS) | (1 << (msk[jj]-1));		\
      } else {								\
	printf(BOLDRED "TS_FP_INPUT out of range %d \n" RESET, msk[jj]); \
	return(-2);							\
      }									\
    }									\
  }


#define SCAN_B_FSETS(BKEYWORD,BSTRUCT,SL_MIN,SL_MAX)   			\
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s %f", &fval);					\
    printf(" %f \n",fval);						\
    for(jj = SL_MIN; jj < SL_MAX; jj++) (BSTRUCT) = fval;			\
  }

/* ------------------- DIRC SSP Configuration -----------------------*/

#define DIRC_FIBER_NUM            32
#define SD_SRC_NUM                18
#define SD_SRC_TRIG               15
#define SD_SRC_TRIG2              17
#define SD_SRC_SYNC               16
#define SD_SRC_SEL_TRIG1           3
#define SD_SRC_SEL_SYNC            2
#define SD_SRC_SEL_0               0

#define SCAN_SSP_REG_GLOB_ALL(BKEYWORD,BSTRUCT)                         \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s 0x%X", &val);					\
    for(jj = SL_MIN; jj < SL_MAX; jj++){                                \
      for(ssp_fiber = 0; ssp_fiber < DIRC_FIBER_NUM; ssp_fiber++){      \
        for(ssp_asic = 0; ssp_asic < 3; ssp_asic++){                    \
          (BSTRUCT) = val;                                              \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }    

#define SCAN_SSP_REG_DAC_ALL(BKEYWORD,BSTRUCT)                          \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s %d", &val);					\
    for(jj = SL_MIN; jj < SL_MAX; jj++){                                \
      for(ssp_fiber = 0; ssp_fiber < DIRC_FIBER_NUM; ssp_fiber++){      \
        for(ssp_asic = 0; ssp_asic < 3; ssp_asic++){                    \
          (BSTRUCT) = val;                                              \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }    



#define SCAN_SSP_REG_GAIN_ALL(BKEYWORD,BSTRUCT)                         \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    SCAN_MSK;                                                           \
    if(strcmp("SSP_MAROC_REG_GAIN_0_15",(BKEYWORD))  == 0) CH_MIN = 0;  \
    else if(strcmp("SSP_MAROC_REG_GAIN_16_31",(BKEYWORD)) == 0) CH_MIN = 16; \
    else if(strcmp("SSP_MAROC_REG_GAIN_32_47",(BKEYWORD)) == 0) CH_MIN = 32; \
    else if(strcmp("SSP_MAROC_REG_GAIN_48_63",(BKEYWORD)) == 0) CH_MIN = 48; \
    else{                                                               \
      printf(BOLDRED "SSP_MAROC_REG_GAIN invalid range \n" RESET);      \
      return(-2);                                                       \
    }                                                                   \
    for(jj = SL_MIN; jj < SL_MAX; jj++){                                \
      for(ssp_fiber = 0; ssp_fiber < DIRC_FIBER_NUM; ssp_fiber++){      \
        for(ssp_asic = 0; ssp_asic < 3; ssp_asic++){                    \
          for(ssp_ch = 0; ssp_ch < 16; ssp_ch++){                       \
            (BSTRUCT)[ssp_ch + CH_MIN] = msk[ssp_ch];                   \
          }                                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }    


#define SCAN_SSP_REG_GAIN(BKEYWORD,BSTRUCT)                         \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    SCAN_MSK;                                                           \
    if(strcmp("SSP_MAROC_REG_GAIN_0_15",(BKEYWORD))  == 0) CH_MIN = 0;  \
    else if(strcmp("SSP_MAROC_REG_GAIN_16_31",(BKEYWORD)) == 0) CH_MIN = 16; \
    else if(strcmp("SSP_MAROC_REG_GAIN_32_47",(BKEYWORD)) == 0) CH_MIN = 32; \
    else if(strcmp("SSP_MAROC_REG_GAIN_48_63",(BKEYWORD)) == 0) CH_MIN = 48; \
    else{                                                               \
      printf(BOLDRED "SSP_MAROC_REG_GAIN invalid range \n" RESET);      \
      return(-2);                                                       \
    }                                                                   \
    for(ssp_ch = 0; ssp_ch < 16; ssp_ch++){                             \
      (BSTRUCT)[ssp_ch + CH_MIN] = msk[ssp_ch];                         \
    }                                                                   \
  }                                                                     \
  


#define SCAN_SSP_FIB_SETS(BKEYWORD,BSTRUCT)                                \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s %d", &val);					\
    for(jj = SL_MIN; jj < SL_MAX; jj++){                                \
      for(ssp_fiber = 0; ssp_fiber < DIRC_FIBER_NUM; ssp_fiber++){      \
        (BSTRUCT) = val;                                                \
      }                                                                 \
    }                                                                   \
  }    


#define SCAN_SSP_TDC_EN_ALL(BKEYWORD,BSTRUCT)                           \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",               \
            &msk[0],&msk[1],&msk[2],&msk[3],&msk[4],&msk[5]);           \
    for(jj = SL_MIN; jj < SL_MAX; jj++){                                \
      for(ssp_fiber = 0; ssp_fiber < DIRC_FIBER_NUM; ssp_fiber++){      \
        for(ssp_ch = 0; ssp_ch < 6; ssp_ch++){                          \
          (BSTRUCT)[ssp_ch] = msk[ssp_ch];                              \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }    


#define SCAN_SSP_CTEST(BKEYWORD,BSTRUCT)                                \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s 0x%X 0x%X", &msk[0], &msk[1]);                \
    for(jj = SL_MIN; jj < SL_MAX; jj++){                                \
      for(ssp_fiber = 0; ssp_fiber < DIRC_FIBER_NUM; ssp_fiber++){      \
        for(ssp_asic = 0; ssp_asic < 3; ssp_asic++){                    \
          (BSTRUCT)[0] = msk[0];                                        \
          (BSTRUCT)[1] = msk[1];                                        \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }    



#define SCAN_SSP_FIBER(BKEYWORD,BSTRUCT)                                \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s %d", &val);					\
    ssp_fiber = val;                                                    \
    (BSTRUCT) = ((BSTRUCT) | (1 << val));                               \
    if(ssp_fiber < 0 && ssp_fiber >= 32 ){                              \
      printf(BOLDRED "\n Read_conf: Wrong SSP fiber  %d in SLOT %d \n" RESET, slot,ssp_fiber); \
      return(-2);                                                       \
    }                                                                   \
  }                                                                     \
  
#define SCAN_SSP_ASIC(BKEYWORD)                                         \
  if(strcmp(keyword,(BKEYWORD)) == 0){					\
    sscanf (str_tmp, "%*s %d", &val);					\
    ssp_asic = val;                                                     \
    if(ssp_asic < 0 && ssp_asic >= 4 ){                                 \
      printf(BOLDRED "\n Read_conf: Wrong SSP asic  %d in  fiber  %d  of the SLOT %d \n" RESET, slot,ssp_fiber,ssp_asic); \
      return(-2);                                                       \
    }                                                                   \
  }          


typedef struct
{
  union
  {
    unsigned int val;
    struct
    {
      unsigned int cmd_fsu        : 1;  //bit 0
      unsigned int cmd_ss         : 1;  //bit 1
      unsigned int cmd_fsb        : 1;  //bit 2
      unsigned int swb_buf_250f   : 1;  //bit 3
      unsigned int swb_buf_500f   : 1;  //bit 4
      unsigned int swb_buf_1p     : 1;  //bit 5
      unsigned int swb_buf_2p     : 1;  //bit 6
      unsigned int ONOFF_ss       : 1;  //bit 7
      unsigned int sw_ss_300f     : 1;  //bit 8
      unsigned int sw_ss_600f     : 1;  //bit 9
      unsigned int sw_ss_1200f    : 1;  //bit 10
      unsigned int EN_ADC         : 1;  //bit 11
      unsigned int H1H2_choice    : 1;  //bit 12
      unsigned int sw_fsu_20f     : 1;  //bit 13
      unsigned int sw_fsu_40f     : 1;  //bit 14
      unsigned int sw_fsu_25k     : 1;  //bit 15
      unsigned int sw_fsu_50k     : 1;  //bit 16
      unsigned int sw_fsu_100k    : 1;  //bit 17
      unsigned int sw_fsb1_50k    : 1;  //bit 18
      unsigned int sw_fsb1_100k   : 1;  //bit 19
      unsigned int sw_fsb1_100f   : 1;  //bit 20
      unsigned int sw_fsb1_50f    : 1;  //bit 21
      unsigned int cmd_fsb_fsu    : 1;  //bit 22
      unsigned int valid_dc_fs    : 1;  //bit 23
      unsigned int sw_fsb2_50k    : 1;  //bit 24
      unsigned int sw_fsb2_100k   : 1;  //bit 25
      unsigned int sw_fsb2_100f   : 1;  //bit 26
      unsigned int sw_fsb2_50f    : 1;  //bit 27
      unsigned int valid_dc_fsb2  : 1;  //bit 28
      unsigned int ENb_tristate   : 1;  //bit 29
      unsigned int polar_discri   : 1;  //bit 30
      unsigned int inv_discriADC  : 1;  //bit 31
    } bits;
  } Global0;

  union
  {
    unsigned int val;
    struct
    {
      unsigned int d1_d2              : 1;  //bit 0
      unsigned int cmd_CK_mux         : 1;  //bit 1
      unsigned int ONOFF_otabg        : 1;  //bit 2
      unsigned int ONOFF_dac          : 1;  //bit 3
      unsigned int small_dac          : 1;  //bit 4
      unsigned int enb_outADC         : 1;  //bit 5
      unsigned int inv_startCmptGray  : 1;  //bit 6
      unsigned int ramp_8bit          : 1;  //bit 7
      unsigned int ramp_10bit         : 1;  //bit 8
      unsigned int Reserved0          : 23;  //bit 9-31
    } bits;
  } Global1;

  int DAC0;
  int DAC1;
  int Gain[64];
  unsigned int Sum[2];
  unsigned int CTest[2];
  unsigned int MaskOr[2];
} dirc_maroc_reg;


typedef struct
{
  dirc_maroc_reg chip[3];
  int ctest_dac;
  int ctest_enable;
  int tdc_enable_mask[6];
  int window_width;
  int window_offset;
} dirc_fiber;


/** SSP configuration parameters **/
typedef struct {

  int group;
  int fw_rev;
  int fw_type;

  int slot;

  char conf_common_dir[STRLEN];
  char conf_common_ver[STRLEN];


  unsigned int fiber_en;
  int  sum_enable;

  int window_width;
  int window_offset;

  int pulser_freq;
  int ssp_io_mux[SD_SRC_NUM];
  
  struct
  {
    dirc_fiber fiber[DIRC_FIBER_NUM];
    int disable_evtbuild;
    int disable_fiber;
    int fiber_in_config;
  } dirc;
  
} SSP_CONF;

SSP_CONF  ssp[NBD_SSP];


void ti_check_latency(char *crate);
