// 
// $Id: conf_output.c 21520 2016-10-27 15:45:23Z davidl $
// $HeadURL: https://halldsvn.jlab.org/repos/trunk/online/daq/vme/src/rol_1/conf_output.c $
//
//
// This file contains code used to write certain module configuration
// parameters into EVIO banks in the output data stream. It works by 
// reading the parameters that have been written into global variables by
// the routines in conf_utils.c. It forms the bank once at either download
// or prestart (your choice) and then writes it into every event during
// the trigger routine.
//
// Only 2 calls are needed:
//
//  conf_output_init()  -- called either in download or prestart 
//                         after configuration has been read in
//
//  conf_output_write() -- called in trigger, preferably before
//                         the board data is read to make offline
//                         event browsing easier, but can go after 
//

#include <stdint.h>

#define MAX_CONFIG_PARS 20

// globals holding complete EVIO config bank to be written event event
uint32_t CONFIGBANK[256];
uint32_t CONFIGBANK_SIZE = 0; // in bytes, includes header

// Routines defined in this file
void conf_output_init(bd_type  board_type);
void conf_output_write(void);
void config_output_add(uint32_t slot_mask, int Npar, uint16_t *partype, uint16_t *parvalue);
int config_output_params_changed(uint32_t slot, uint32_t Npar, uint16_t *partype, uint16_t *parvalue);
void config_output_update(uint32_t slot, uint32_t Npar, uint16_t *partype, uint16_t *parvalue);
void conf_output_write_to_file(void);
const char* conf_output_parname(uint16_t partype);


#ifndef EVIO_SWAP32
#define EVIO_SWAP32(x) ( (((x) >> 24) & 0x000000FF) | \
                         (((x) >> 8)  & 0x0000FF00) | \
                         (((x) << 8)  & 0x00FF0000) | \
                         (((x) << 24) & 0xFF000000) )
#endif // EVIO_SWAP32

//----------------------------
// conf_output_init
//----------------------------
void conf_output_init(bd_type  board_type)
{
	/// Initialze CONFIGBANK using the given board type. The
	/// current conf_utils code assumes a single board type for
	/// all modules in the crate so we do the same here. This
	/// will overwrite the contents of the global array 
	/// CONFIGBANK and the global variable CONFIGBANK_SIZE.
	/// After calling this, CONFIGBANK will contain a complete
	/// EVIO bank and CONFIGBANK_SIZE will contain its size 
	/// in bytes. This can then be written to the event stream
	/// using conf_output_write(). 
	
	// Initilize bank lengths.
	// CONFIGBANK_SIZE will be set to non-zero only if there's
	// interesting data.
	CONFIGBANK[0] = 1;    // 1 for header word
	CONFIGBANK_SIZE = 0;
	
	// Bank header: conf. bank |   uint32_t   |   1
	CONFIGBANK[1] = (0x55<<16) |   (0x1<<8)   | (0x1);
	
	// Specify which parameters will be written based on board type
	uint32_t Npar = 0;
	uint16_t partype[MAX_CONFIG_PARS];
	uint16_t parvalue[MAX_CONFIG_PARS];
	switch(board_type){
		case fadc250:
			partype[Npar]  = kPARAM250_NPED;      parvalue[Npar++] = 0; // kPARAM250_NPED (will be overwritten below)
			partype[Npar]  = kPARAM250_NSA_NSB;   parvalue[Npar++] = 0; // kPARAM250_NSA_NSB (will be overwritten below)
			break;
		case fadc125:
			partype[Npar]  = kPARAM125_NPED;      parvalue[Npar++] = 1; // kPARAM125_NPED hardwired in 1st gen. firmware
			partype[Npar]  = kPARAM125_NW;        parvalue[Npar++] = 0; // kPARAM125_NW (will be overwritten below)
			partype[Npar]  = kPARAM125_P2;        parvalue[Npar++] = 0; // kPARAM125_P2 (will be overwritten below)
			partype[Npar]  = kPARAM125_IE;        parvalue[Npar++] = 0; // kPARAM125_IE (will be overwritten below)
			partype[Npar]  = kPARAM125_IBIT;      parvalue[Npar++] = 0; // kPARAM125_IBIT (will be overwritten below)
			partype[Npar]  = kPARAM125_ABIT;      parvalue[Npar++] = 0; // kPARAM125_ABIT (will be overwritten below)
			partype[Npar]  = kPARAM125_PBIT;      parvalue[Npar++] = 0; // kPARAM125_PBIT (will be overwritten below)
			break;
		case f1tdc: 
			partype[Npar]  = kPARAMF1_REFCNT;     parvalue[Npar++] = 0; // kPARAMF1_REFCNT (will be overwritten below)
			partype[Npar]  = kPARAMF1_HSDIV;      parvalue[Npar++] = 0; // kPARAMF1_HSDIV (will be overwritten below)
			partype[Npar]  = kPARAMF1_REFCLKDIV;  parvalue[Npar++] = 0; // kPARAMF1_REFCLKDIV (will be overwritten below)
			break;
		case tdc1290:
			// No parameters need to be written out
			break;
		default:
			// no other modules of interest here
			break;
	}
	
	// Loop over all slots, checking parameters for ones that actually have a module
	uint32_t slot_mask = 0x0;
	uint32_t slot;
	uint32_t Nconfigs = 0;
	uint32_t Nmodules = 0;
	for(slot=0; slot<NBD_FADC250; slot++){ // NBD_FADC250=NBD_FADC125=NBD_TDC1290=NBD_F1TDC
	
		// has_module will be zero if no modules of interest in this slot 
		int has_module = 0;
		if(fa250[slot].group    ==1) has_module = 1;
		if(fa125[slot].group    ==1) has_module = 1;
		if(f1[slot].group       ==1) has_module = 1;
		if(caen1290[slot].group ==1) has_module = 1;
		if(has_module == 0) continue; // no module in slot (at least of type we care about)
		Nmodules++;

		// Check if any parameters have changed and if so, write them into CONFIGBANK. 
		if(config_output_params_changed(slot, Npar, partype, parvalue)){
			config_output_add(slot_mask, Npar, partype, parvalue);
			if(slot_mask!=0x0 && Npar>0) Nconfigs++;
			slot_mask = 0x0;
		}

		// Update slot_mask and copy parameter(s) into parvalue
		slot_mask |= 1 << slot;
		config_output_update(slot, Npar, partype, parvalue);		
	}

	// Write config parameters to CONFIGBANK that are not yet written
	config_output_add(slot_mask, Npar, partype, parvalue);
	if(slot_mask!=0x0 && Npar>0) Nconfigs++;
	
	// Only set CONFIGBANK_SIZE if there's data words
	if(CONFIGBANK[0] > 1) CONFIGBANK_SIZE = (CONFIGBANK[0] + 1) * sizeof(uint32_t); // bank size in bytes (includes length word)

	// OK, this is a little confusing. Empirically, it seems that the length
	// and header words are OK, but the actual data content needs to be byte
	//swapped. We do this here (though I'm not sure why I need to!)
	uint32_t i;
	for(i=2; i< (CONFIGBANK[0]+1); i++) CONFIGBANK[i] = EVIO_SWAP32(CONFIGBANK[i]);
	
	printf("DAQ configuration output initialized:\n");
	printf("   Bank size: %d bytes\n", CONFIGBANK_SIZE);
	printf("    Nmodules: %d\n", Nmodules);
	printf("    Nconfigs: %d\n", Nconfigs);
	
	conf_output_write_to_file();
}

//----------------------------
// conf_output_write
//----------------------------
void conf_output_write(void)
{
	/// Write module configuration information (if any) for this crate 
	/// into the output bank pointed to by dma_dabufp. This assumes
	/// dma_dabufp is already pointing to the position we should start
	/// writing to and it will increment dma_dabufp to point to the position
	/// immediately following the configuration bank when done. Prior to
	/// calling this, a call to conf_output_init should have been made.
	/// If no configuration data is available for this crate, then
	/// dma_dabufp is left untouched.

	// Do nothing if config bank is empty
	if(CONFIGBANK_SIZE == 0) return; 

	// The dma_dabufp global should be available due to us being compiled
	// into a ROL. Simply copy existing bank into output buffer.
	memcpy(dma_dabufp, CONFIGBANK, CONFIGBANK_SIZE);
	dma_dabufp += CONFIGBANK[0]+1;
}

//=======================================================================

//----------------------------
// config_output_add
//----------------------------
void config_output_add(uint32_t slot_mask, int Npar, uint16_t *partype, uint16_t *parvalue)
{
	/// This will add a section to the output bank in CONFIGBANK that applies
	/// to the given slot_mask. The Npar variables contained in partype and parvalue
	/// will be written to the section. If slot_mask is empty or Npar=0, then nothing
	/// is written.

	// If the slot_mask is empty or there are no parameters, then don't do anything
	if( (slot_mask==0x0) || (Npar==0) ) return;

	// Index of next word to write to (+2 is for length and EVIO header words)
	uint32_t N = CONFIGBANK[0] + 1;
	
	CONFIGBANK[N++] = (Npar<<24) | slot_mask;

	// Copy parameters into CONFIGBANK
	uint32_t i;
	for(i=0; i<Npar; i++){
		uint32_t ptype = (uint32_t)partype[i];
		uint32_t pval  = (uint32_t)parvalue[i];
		CONFIGBANK[N++] = (ptype<< 16) | (pval);
	}
	
	// Update bank length
	CONFIGBANK[0] = N-1; // evio bank length in words (not including length word itself)
}

//----------------------------
// config_output_params_changed
//----------------------------
int config_output_params_changed(uint32_t slot, uint32_t Npar, uint16_t *partype, uint16_t *parvalue)
{
	/// This compares all of the parameters given by Npar, partype, and parvalue
	/// against the settings for the given slot. If any parameter differs, then
	/// "true" is returned. Otherwise, "false" is returned. The idea is to pass
	/// in a small list of parameter types and from those, we know which module
	/// type/param to compare against.

	// Check if any parameter has changed
	int param_changed = 0;
	uint32_t ipar;
	for(ipar=0; ipar<Npar; ipar++){
		switch(partype[ipar]){
			case kPARAM250_NSA_NSB        : param_changed |= (parvalue[ipar]!=(fa250[slot].nsa+fa250[slot].nsb));    break;
			case kPARAM250_NSA            : param_changed |= (parvalue[ipar]!=fa250[slot].nsa);                      break;
			case kPARAM250_NSB            : param_changed |= (parvalue[ipar]!=fa250[slot].nsb);                      break;
			case kPARAM250_NPED           : param_changed |= (parvalue[ipar]!=fa250[slot].nped);                     break;

			case kPARAM125_NSA_NSB        : param_changed |= (parvalue[ipar]!=(fa125[slot].nsa+fa125[slot].nsb));    break;
			case kPARAM125_NSA            : param_changed |= (parvalue[ipar]!=fa125[slot].nsa);                      break;
			case kPARAM125_NSB            : param_changed |= (parvalue[ipar]!=fa125[slot].nsb);                      break;
			case kPARAM125_WINWIDTH       : param_changed |= (parvalue[ipar]!=fa125[slot].winWidth);                 break;

			case kPARAM125_PL             : param_changed |= (parvalue[ipar]!=fa125[slot].PL);                       break;
			case kPARAM125_NW             : param_changed |= (parvalue[ipar]!=fa125[slot].winWidth);                 break;
			case kPARAM125_NPK            : param_changed |= (parvalue[ipar]!=fa125[slot].npeak);                    break;
			case kPARAM125_P1             : param_changed |= (parvalue[ipar]!=fa125[slot].P1);                       break;
			case kPARAM125_P2             : param_changed |= (parvalue[ipar]!=fa125[slot].P2);                       break;
			case kPARAM125_PG             : param_changed |= (parvalue[ipar]!=fa125[slot].PG);                       break;
			case kPARAM125_IE             : param_changed |= (parvalue[ipar]!=fa125[slot].IE);                       break;
			case kPARAM125_H              : param_changed |= (parvalue[ipar]!=fa125[slot].H);                        break;
			case kPARAM125_TH             : param_changed |= (parvalue[ipar]!=fa125[slot].TH);                       break;
			case kPARAM125_TL             : param_changed |= (parvalue[ipar]!=fa125[slot].TL);                       break;
			case kPARAM125_IBIT           : param_changed |= (parvalue[ipar]!=fa125[slot].IBIT);                     break;
			case kPARAM125_ABIT           : param_changed |= (parvalue[ipar]!=fa125[slot].ABIT);                     break;
			case kPARAM125_PBIT           : param_changed |= (parvalue[ipar]!=fa125[slot].PBIT);                     break;

			case kPARAMF1_BINSIZE         : param_changed |= (parvalue[ipar]!=(uint16_t)(1000.0*f1[slot].bin_size)); break;
			case kPARAMF1_REFCNT          : param_changed |= (parvalue[ipar]!=f1[slot].refcnt);                      break;
			case kPARAMF1_HSDIV           : param_changed |= (parvalue[ipar]!=f1[slot].hsdiv);                       break;
			case kPARAMF1_REFCLKDIV       : param_changed |= (parvalue[ipar]!=f1[slot].refclkdiv);                   break;

			case kPARAMCAEN1290_WINWIDTH  : param_changed |= (parvalue[ipar]!=caen1290[slot].window_width);          break;
			case kPARAMCAEN1290_WINOFFSET : param_changed |= (parvalue[ipar]!=caen1290[slot].window_offset);         break;

			default: break;  // not interested in other parameters
		}
	}

	return param_changed;
}

//----------------------------
// config_output_update
//----------------------------
void config_output_update(uint32_t slot, uint32_t Npar, uint16_t *partype, uint16_t *parvalue)
{
	/// For all parameters listed in partype, update the values in parvalue
	/// based on the parameters for the module in the given slot. This will
	/// overwrite any existing values in parvalue. It is assumed that this
	/// is only called for slots that actually have a module in them. 
	uint32_t ipar;
	for(ipar=0; ipar<Npar; ipar++){

		switch(partype[ipar]){
			case kPARAM250_NSA_NSB        : parvalue[ipar] = (int)fa250[slot].nsa + fa250[slot].nsb;break;
			case kPARAM250_NSA            : parvalue[ipar] = fa250[slot].nsa;                       break;
			case kPARAM250_NSB            : parvalue[ipar] = abs(fa250[slot].nsb) + (fa250[slot].nsb<0 ? 0x8:0x0); break;
			case kPARAM250_NPED           : parvalue[ipar] = fa250[slot].nped;                      break;

			case kPARAM125_NSA            : parvalue[ipar] = fa125[slot].nsa;                       break;
			case kPARAM125_NSB            : parvalue[ipar] = fa125[slot].nsb;                       break;
			case kPARAM125_NSA_NSB        : parvalue[ipar] = fa125[slot].nsa + fa125[slot].nsb;     break;
			case kPARAM125_NPED           : parvalue[ipar] = (1 + (1<<fa125[slot].P2))/(1<<fa125[slot].PBIT);  break;		
			case kPARAM125_WINWIDTH       : parvalue[ipar] = fa125[slot].winWidth;                  break;

			case kPARAM125_PL             : parvalue[ipar] = fa125[slot].PL;                        break;
			case kPARAM125_NW             : parvalue[ipar] = fa125[slot].winWidth;                  break;
			case kPARAM125_NPK            : parvalue[ipar] = fa125[slot].npeak;                     break;
			case kPARAM125_P1             : parvalue[ipar] = fa125[slot].P1;                        break;
			case kPARAM125_P2             : parvalue[ipar] = fa125[slot].P2;                        break;
			case kPARAM125_PG             : parvalue[ipar] = fa125[slot].PG;                        break;
			case kPARAM125_IE             : parvalue[ipar] = fa125[slot].IE;                        break;
			case kPARAM125_H              : parvalue[ipar] = fa125[slot].H;                         break;
			case kPARAM125_TH             : parvalue[ipar] = fa125[slot].TH;                        break;
			case kPARAM125_TL             : parvalue[ipar] = fa125[slot].TL;                        break;
			case kPARAM125_IBIT           : parvalue[ipar] = fa125[slot].IBIT;                      break;
			case kPARAM125_ABIT           : parvalue[ipar] = fa125[slot].ABIT;                      break;
			case kPARAM125_PBIT           : parvalue[ipar] = fa125[slot].PBIT;                      break;

			case kPARAMF1_BINSIZE         : parvalue[ipar] = (uint16_t)(1000.0*f1[slot].bin_size);  break;
			case kPARAMF1_REFCNT          : parvalue[ipar] = f1[slot].refcnt;                       break;
			case kPARAMF1_HSDIV           : parvalue[ipar] = f1[slot].hsdiv;                        break;
			case kPARAMF1_REFCLKDIV       : parvalue[ipar] = f1[slot].refclkdiv;                    break;

			case kPARAMCAEN1290_WINWIDTH  : parvalue[ipar] = caen1290[slot].window_width;           break;
			case kPARAMCAEN1290_WINOFFSET : parvalue[ipar] = caen1290[slot].window_offset;          break;

			default: break;  // not interested in other parameters
		}
	}
}

//----------------------------
// conf_output_write_to_file
//----------------------------
void conf_output_write_to_file(void)
{
	/// This is called at the end of download to write the
	/// parameters to a text file on gluonfs1 so that it
	/// can be read into the automated e-log entry later.
	/// The filename will be based on the hostname and ROCID.
	/// It will overwrite any existing file with the same
	/// name. If it can't open/create the file for any reason,
	/// a small message will be printed and it will simply
	/// return.
	
	// Determine output filename and open file
	const char* DAQ_HOME = getenv("DAQ_HOME");
	if(DAQ_HOME == NULL){
		printf("DAQ_HOME environment variable not set. Config. \n");
		printf("   params will not be written to file\n");
		printf("   (and consequently not into the e-log)\n");
		return;
	}
	char fname[512];
	sprintf(fname, "%s/../work/DAQ_CONFIG_OUT/daq_config_%s_rocid%d.dat", DAQ_HOME, roc_name, ROCID);
	FILE *f = fopen(fname, "w");
	if( f == NULL ){
		printf("Unable to open \"%s\" for writing.\n", fname);
		printf("   params will not be written to file\n");
		printf("   (and consequently not into the e-log)\n");
		return;
	}
	
	printf("Writing select config parameters to: %s\n", fname);
	
	// Write header
	char tstr[256]="";
	time_t t = time(NULL);
	sprintf(tstr, ctime(&t));
	if(strlen(tstr)>0) tstr[strlen(tstr)-1] = 0;
	fprintf(f, "-- %s -- rocid:%d -- %s --\n", roc_name, ROCID, tstr);
	
	uint32_t *istart = &CONFIGBANK[2];
	uint32_t *iend = &CONFIGBANK[CONFIGBANK[0]+1];
	uint32_t *iptr = istart;
	while(iptr < iend){
		uint32_t w = EVIO_SWAP32(*iptr);
		uint32_t Npar = w >> 24;
		uint32_t slot_mask = w & 0xFFFFFF;
		fprintf(f, "Npar:%d  slot_mask:0x%x ", Npar, slot_mask);
		
		int32_t slot;
		fprintf(f, "(");
		for(slot=23; slot>=0; slot--){
			fprintf(f, "%d", (slot_mask>>slot)&0x01);
			if(slot==23 || slot==13 || slot==11 || slot==3) fprintf(f, " ");
		}
		fprintf(f, ")");
		fprintf(f, "\n");
		
		uint32_t ipar;
		for(ipar=0; ipar<Npar; ipar++){
			iptr++;
			if(iptr >= iend) break;
			uint32_t w = EVIO_SWAP32(*iptr);
			uint16_t partype = (w>>16)&0xFFFF;
			uint16_t parval  = w&0xFFFF;
			
			fprintf(f, "%s = %d\n", conf_output_parname(partype), parval);
		}
		iptr++; // advance to next parameter header
	}
	
	fclose(f);
}

//----------------------------
// conf_output_parname
//----------------------------
const char* conf_output_parname(uint16_t partype)
{
	switch(partype){
		case kPARAM250_NSA:              return "f250_NSA";
		case kPARAM250_NSB:              return "f250_NSB";
		case kPARAM250_NSA_NSB:          return "f250_NSA_NSB";
		case kPARAM250_NPED:             return "f250_NPED";

		case kPARAM125_NSA:              return "f125_NSA";
		case kPARAM125_NSB:              return "f125_NSB";
		case kPARAM125_NSA_NSB:          return "f125_NSA_NSB";
		case kPARAM125_NPED:             return "f125_NPED";
		case kPARAM125_WINWIDTH:         return "f125_WINWIDTH";

		case kPARAM125_PL:               return "f125_PL";
		case kPARAM125_NW:               return "f125_NW";
		case kPARAM125_NPK:              return "f125_NPK";
		case kPARAM125_P1:               return "f125_P1";
		case kPARAM125_P2:               return "f125_P2";
		case kPARAM125_PG:               return "f125_PG";
		case kPARAM125_IE:               return "f125_IE";
		case kPARAM125_H:                return "f125_H";
		case kPARAM125_TH:               return "f125_TH";
		case kPARAM125_TL:               return "f125_TL";
		case kPARAM125_IBIT:             return "f125_IBIT";
		case kPARAM125_ABIT:             return "f125_ABIT";
		case kPARAM125_PBIT:             return "f125_PBIT";

		case kPARAMF1_REFCNT:            return "F1_REFCNT";
		case kPARAMF1_TRIGWIN:           return "F1_TRIGWIN";
		case kPARAMF1_TRIGLAT:           return "F1_TRIGLAT";
		case kPARAMF1_HSDIV:             return "F1_HSDIV";
		case kPARAMF1_BINSIZE:           return "F1_BINSIZE";
		case kPARAMF1_REFCLKDIV:         return "F1_REFCLKDIV";

		case kPARAMCAEN1290_WINWIDTH :   return "CAEN1290_WINWIDTH";
		case kPARAMCAEN1290_WINOFFSET :  return "CAEN1290_WINOFFSET";

		case kPARAM_NONE :               return "none";
		default:                         return "unknown";
	}
}

