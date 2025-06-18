

/* These should already be included in the ROL */
#ifdef __cplusplus
extern "C" {
#endif

#include "jvme.h"
#include "fadcLib.h"
#include "fa125Lib.h"
#include "f1tdcLib.h"
//#include "c1190Lib.h"
#include "tsLib.h"
#include "tiLib.h"

#ifdef __cplusplus
}
#endif



// -- f250 --
extern int nfadc;
extern volatile struct fadc_struct *FAp[(FA_MAX_BOARDS+1)];
/* Mutex to guard flexio read/writes */
extern pthread_mutex_t   faMutex;
extern pthread_mutex_t   fasdcMutex;
#ifndef FALOCK
#define FALOCK      if(pthread_mutex_lock(&faMutex)<0) perror("pthread_mutex_lock");
#define FAUNLOCK    if(pthread_mutex_unlock(&faMutex)<0) perror("pthread_mutex_unlock");
#define FASDCLOCK   if(pthread_mutex_lock(&fasdcMutex)<0) perror("pthread_mutex_lock");
#define FASDCUNLOCK if(pthread_mutex_unlock(&fasdcMutex)<0) perror("pthread_mutex_unlock");
#endif /* FALOCK */

// -- f125 --
extern int nfa125;
extern unsigned long fa125A24Offset;
extern unsigned int fa125AddrList[FA125_MAX_BOARDS];
extern pthread_mutex_t    fa125Mutex;
#ifndef FA125LOCK
#define FA125LOCK     if(pthread_mutex_lock(&fa125Mutex)<0) perror("pthread_mutex_lock");
#define FA125UNLOCK   if(pthread_mutex_unlock(&fa125Mutex)<0) perror("pthread_mutex_unlock");
#endif /* FA125LOCK */

// -- F1TDC --
extern int nf1tdc;
extern int f1Nchips[(F1_MAX_BOARDS+1)];
extern volatile struct f1tdc_struct *f1p[(F1_MAX_BOARDS+1)];
extern unsigned int tdcAddrOffset;
extern pthread_mutex_t   f1Mutex;
extern pthread_mutex_t   f1sdcMutex;
#ifndef F1LOCK
#define F1LOCK      if(pthread_mutex_lock(&f1Mutex)<0) perror("pthread_mutex_lock");
#define F1UNLOCK    if(pthread_mutex_unlock(&f1Mutex)<0) perror("pthread_mutex_unlock");
#endif /* F1LOCK */

// -- CAEN1190 --
extern int Nc1190;
extern volatile struct v1190_struct *c1190p[V1190_MAX_MODULES];
extern pthread_mutex_t c1190_mutex;
#ifndef LOCK_1190
#define LOCK_1190    if(pthread_mutex_lock(&c1190_mutex)<0) perror("pthread_mutex_lock");
#define UNLOCK_1190  if(pthread_mutex_unlock(&c1190_mutex)<0) perror("pthread_mutex_unlock");
#endif /* LOCK_1190 */


//-------------------
// PrintSummary
//-------------------
void PrintSummary(ModulesConfigBOR *config)
{
	//------------------------------------------------------------------------------
	// Print summary
	printf("\n\n");
	printf("------- Summary ------\n");

	uint32_t i;
	for(i=0; i<config->Nf250; i++){
		f250config *c = &config->f250[i];
		printf("f250 in slot %d : (%x)\n", c->slot, c->version);
	}
	for(i=0; i<config->Nf125; i++){
		f125config *c = &config->f125[i];
		printf("f125 in slot %d : %x (%x)\n", c->slot, c->board_id, c->version);
	}
	for(i=0; i<config->NF1TDC; i++){
		F1TDCconfig *c = &config->F1TDC[i];
		printf("F1TDC%s in slot %d : (%x)\n", (c->nchips==8 ? "v2":"v3"), c->slot, c->version);
	}
	for(i=0; i<config->Ncaen1190; i++){
		caen1190config *c = &config->caen1190[i];
		printf("CAEN1190 in slot %d : (%x)\n", c->slot, c->firmwareRev);
	}
	printf("----------------------\n");
}

//-------------------
// ReadModuleConfigs
//-------------------
void ReadModuleConfigs(ModulesConfigBOR *config)
{
	int i;

	config->Nf250 = 0;
	config->Nf125 = 0;
	config->NF1TDC = 0;
	config->Ncaen1190 = 0;

	printf("\n");
	printf("---- Reading module parameters ...\n");

	//----  fa250  ----
	for(i = 0; i<nfadc; i++) {
		// Read configuration from module
		int slot = faSlot(i);
		ReadF250Config(ROCID, slot, &config->f250[config->Nf250++]);
	}

	//----  fa125  ----
	for(i = 0; i<nfa125; i++) {
		// Read configuration from module
		ReadF125Config(ROCID, i, &config->f125[config->Nf125++]);
	}

	//----  F1TDC  ----
	for(i = 0; i<nf1tdc; i++) {
		// Read configuration from module
		int slot = f1Slot(i);
		ReadF1TDCConfig(ROCID, slot, &config->F1TDC[config->NF1TDC++]);
	}

	//----  CAEN1190  ----
	for(i = 0; i<Nc1190; i++) {
		// Read configuration from module
		ReadCAEN1190Config(ROCID, i, &config->caen1190[config->Ncaen1190++]);
	}

}

//-------------------
// ReadF250Config
//-------------------
void ReadF250Config(uint32_t rocid, uint32_t slot, f250config *config)
{
	// Read module configuration parameters back from the module
	// in the specified slot and storing them in the given structure.
	// This assumes that faInit() has already been called so that 
	// the global FAp[] array in the fadcLib library has been filled
	// with the addresses of the modules in VME memory space. It also
	// assumes that there is a f250 module in "slot". The value of 
	// rocid is not used, but simply copied into the data structure.
	volatile struct fadc_struct *fa250 = FAp[slot];

	config->rocid = rocid;
	config->slot = slot;

	FALOCK;
	
	int i;

	config->version      = vmeRead32(&fa250->version);
	config->ctrl1        = vmeRead32(&fa250->ctrl1);
	config->ctrl2        = vmeRead32(&fa250->ctrl2);
	config->blk_level    = vmeRead32(&fa250->blk_level);
	config->delay        = vmeRead32(&fa250->delay);
	config->itrig_cfg    = vmeRead32(&fa250->itrig_cfg);
	for(i=0; i<16; i++)config->dac[i]           = vmeRead16(&fa250->dac[i]);
	config->trig21_delay = vmeRead32(&fa250->trig21_delay);
	for(i=0; i< 3; i++)config->serial_number[i] = vmeRead32(&fa250->serial_number[i]);

	for(i=0; i< 3; i++)config->adc_status[i]    = vmeRead32(&fa250->adc_status[i]);
	for(i=0; i< 4; i++)config->adc_config[i]    = vmeRead32(&fa250->adc_config[i]);
	config->adc_ptw      = vmeRead32(&fa250->adc_ptw);
	config->adc_pl       = vmeRead32(&fa250->adc_pl);
	config->adc_nsb      = vmeRead32(&fa250->adc_nsb);
	config->adc_nsa      = vmeRead32(&fa250->adc_nsa);
	for(i=0; i<16; i++)config->adc_thres[i]     = vmeRead16(&fa250->adc_thres[i]);
 	for(i=0; i<16; i++)config->adc_pedestal[i]  = vmeRead32(&fa250->adc_pedestal[i]);
	config->config6         = vmeRead32(&fa250->config6);
	config->config7         = vmeRead32(&fa250->config7);
	config->config3         = vmeRead32(&fa250->config3);
	
	FAUNLOCK;
}

//-------------------
// ReadF125Config
//-------------------
void ReadF125Config(uint32_t rocid, uint32_t idx, f125config *config)
{
	// Read module configuration parameters back from the module
	// in the specified slot and storing them in the given structure.
	// This assumes that fa125Init() has already been called so that 
	// the globals fa125AddrList and fa125A24Offset in the fa125Lib
	// library has been filled with the addresses of the modules in
	// VME memory space. It also assumes that there is a f125 module
	// in the slot corresponding to "idx". The value of rocid is not
	// used, but simply copied into the data structure.
	int slot = fa125Slot(idx);
	volatile struct fa125_a24 *fa125 = (volatile struct fa125_a24 *)(fa125AddrList[idx]+fa125A24Offset);

	config->rocid = rocid;
	config->slot = slot;

	FA125LOCK;
	
	int i;

	config->board_id = vmeRead32(&fa125->main.id);
	config->version = vmeRead32(&fa125->main.version);
	config->clock   = vmeRead32(&fa125->main.clock);
	for(i=0; i<4; i++)config->serial[i] = vmeRead32(&fa125->main.serial[i]);
	for(i=0; i<2; i++)config->temperature[i] = vmeRead32(&fa125->main.temperature[i]);
	config->ctrl1   = vmeRead32(&fa125->main.ctrl1);
	
	for(i=0; i<12; i++){
		int ii;

		config->fe[i].version = vmeRead32(&fa125->fe[i].version);
		config->fe[i].nw      = vmeRead32(&fa125->fe[i].nw);
		config->fe[i].pl      = vmeRead32(&fa125->fe[i].pl);
		for(ii=0; ii<6; ii++)config->fe[i].threshold[ii] = vmeRead32(&fa125->fe[i].threshold[ii]);
		config->fe[i].config1 = vmeRead32(&fa125->fe[i].config1);
		config->fe[i].config2 = vmeRead32(&fa125->fe[i].config2);
		config->fe[i].ped_sf  = vmeRead32(&fa125->fe[i].ped_sf);
		for(ii=0; ii<3; ii++)config->fe[i].timing_thres_lo[ii] = vmeRead32(&fa125->fe[i].timing_thres_lo[ii]);
		config->fe[i].ie      = vmeRead32(&fa125->fe[i].ie);
		for(ii=0; ii<2; ii++)config->fe[i].timing_thres_hi[ii] = vmeRead32(&fa125->fe[i].timing_thres_hi[ii]);
	}
	
	config->proc_version    = vmeRead32(&fa125->proc.version);
	config->proc_csr        = vmeRead32(&fa125->proc.csr);
	config->proc_trigsrc    = vmeRead32(&fa125->proc.trigsrc);
	config->proc_ctrl2      = vmeRead32(&fa125->proc.ctrl2);
	config->proc_blocklevel = vmeRead32(&fa125->proc.blocklevel);
	
	FA125UNLOCK;
}

//-------------------
// ReadF1TDCConfig
//-------------------
void ReadF1TDCConfig(uint32_t rocid, uint32_t slot, F1TDCconfig *config)
{
	// Read module configuration parameters back from the module
	// in the specified slot and storing them in the given structure.
	// This assumes that faInit() has already been called so that 
	// the global FAp[] array in the fadcLib library has been filled
	// with the addresses of the modules in VME memory space. It also
	// assumes that there is a f250 module in "slot". The value of 
	// rocid is not used, but simply copied into the data structure.
	volatile struct f1tdc_struct *F1TDC = f1p[slot];

	config->rocid = rocid;
	config->slot = slot;

	F1LOCK;

	config->version     = vmeRead32(&F1TDC->version);
	config->ctrl        = vmeRead32(&F1TDC->ctrl);
	config->blocklevel  = vmeRead32(&F1TDC->blocklevel);

	F1UNLOCK;

	// The F1 chip itself maintains registers that are read-only.
	// The JLab firmware was supposed to keep a copy of the register
	// settings. but it looks like that may not have been implemented.
	// Instead, they keep a copy in the library. This means we cannot
	// find these settings using an external program reading them from
	// the module. We MUST access the copy the library keeps from within
	// the ROL. (Or, reproduce them from the configuration files which
	// seems even more sketchy.)
	//
	// The f1ConfigRead() call below locks the F1LOCK so we must do this
	// outside of having that lock.
	//
	// We copy all registers here, but don't interpret them at this stage.
	// Also note that some F1TDC modules have 8 chips and others
	// only 6. We record that number here so we know later how many
	// of the "f1registers" elements are valid.
	int ichip, ireg;
	config->nchips = f1Nchips[slot];
	for(ichip = 0; ichip < config->nchips; ichip++){
		for(ireg = 0; ireg < 16; ireg++){
			///config->f1registers[ireg][ichip] = f1ReadChip(slot, ichip, ireg); // f1ReadChip is static
			f1ConfigRead(slot, &config->f1registers[ichip][0], ichip);
		}
	}

}

//-------------------
// ReadCAEN1190Config
//-------------------
void ReadCAEN1190Config(uint32_t rocid, uint32_t idx, caen1190config *config)
{
	volatile struct v1190_struct *caen1190 = c1190p[idx];
	int slot = tdc1190GetGeoAddress(idx);

	config->rocid = rocid;
	config->slot = slot;

//	config->edge_resolution       = tdc1190GetEdgeResolution(idx);
//	config->double_hit_resolution = tdc1190GetDoubleHitResolution(idx);
	config->max_hits_per_event    = tdc1190GetMaxNumberOfHitsPerEvent(idx);
	config->almostFullLevel       = tdc1190GetAlmostFullLevel(idx);
	config->bltEventNumber        = tdc1190GetBLTEventNumber(idx);

	LOCK_1190
	
	config->firmwareRev           = vmeRead16(&caen1190->firmwareRev);
	
	UNLOCK_1190
}


