// 
// $Id: conf_output.h 19639 2015-11-13 21:03:18Z davidl $
// $HeadURL: https://halldsvn.jlab.org/repos/trunk/online/daq/vme/src/rol_1/conf_output.h $
//
//
// (See comments in conf_output.c )

// ==== IMPORTANT =====
// The way ROLs are compiled, they traditionally #include other
// implmentation files such as "conf_utils.c" and "conf_output.c"
// This means we don't need to #include "conf_utils.h" since it 
// will already have been included. (In fact, doing so would result 
// in multiply defined errors since the globals are actually defined
// in the header file!) The following are the important globals:
//
// FADC250_CONF fa250[NBD_FADC250];
// FADC125_CONF fa125[NBD_FADC125];
// TDC1290_CONF caen1290[NBD_TDC1290];
// F1TDC_CONF  f1[NBD_F1TDC];


// This is copied from the file DAQ/daq_param_type.h from the offline.
// It *must* be kept in alignment so if you change anything here, change
// it there as well or notify me at davidl@jlab.org and I'll take care of
// it.

#ifndef __CONF_OUTPUT__
#define __CONF_OUTPUT__


enum daq_param_type{

	kPARAM250_NSA             = 0x0501,
	kPARAM250_NSB             = 0x0502,
	kPARAM250_NSA_NSB         = 0x0503,  // NSA+NSB
	kPARAM250_NPED            = 0x0504,

	kPARAM125_NSA             = 0x0F01,
	kPARAM125_NSB             = 0x0F02,
	kPARAM125_NSA_NSB         = 0x0F03,  // NSA+NSB
	kPARAM125_NPED            = 0x0F04,  // (1 + 2^P2)/(2^PBIT)
	kPARAM125_WINWIDTH        = 0x0F05,
	kPARAM125_PL              = 0x0F06,
	kPARAM125_NW              = 0x0F07,
	kPARAM125_NPK             = 0x0F08,
	kPARAM125_P1              = 0x0F09,
	kPARAM125_P2              = 0x0F0A,
	kPARAM125_PG              = 0x0F0B,
	kPARAM125_IE              = 0x0F0C,
	kPARAM125_H               = 0x0F0D,
	kPARAM125_TH              = 0x0F0E,
	kPARAM125_TL              = 0x0F0F,
	kPARAM125_IBIT            = 0x0F10,
	kPARAM125_ABIT            = 0x0F11,
	kPARAM125_PBIT            = 0x0F12,

	kPARAMF1_REFCNT           = 0x0601,
	kPARAMF1_TRIGWIN          = 0x0602,
	kPARAMF1_TRIGLAT          = 0x0603,
	kPARAMF1_HSDIV            = 0x0604,
	kPARAMF1_BINSIZE          = 0x0605,  // in picoseconds
	kPARAMF1_REFCLKDIV        = 0x0606,
	
	kPARAMCAEN1290_WINWIDTH   = 0x1001,
	kPARAMCAEN1290_WINOFFSET  = 0x1002,

	kPARAM_NONE               = 0x0000
};


void conf_output_init(bd_type  board_type);
void conf_output_write(void);
void conf_output_write_to_file(void);
const char* conf_output_parname(uint16_t partype);


#endif // __CONF_OUTPUT__

