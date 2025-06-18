
/*

	Beginning Of Run record utils
	
	This small library will read configuration parameters
	back from digitization modules and store them in the
	shared memory that is made available by the shared memory
	server in rcm. A remote process that is launched at 
	prestart will read this shared memory and package the
	module configuration information into a beginning of
	run (BOR) event that gets inserted into the data stream.

*/

#include <bor_roc.h>  /* from rcm/monitor */

void PrintSummary(ModulesConfigBOR *config);
void ReadModuleConfigs(ModulesConfigBOR *config);
void ReadF250Config(uint32_t rocid, uint32_t slot, f250config *config);
void ReadF125Config(uint32_t rocid, uint32_t idx, f125config *config);
void ReadF1TDCConfig(uint32_t rocid, uint32_t slot, F1TDCconfig *config);
void ReadCAEN1190Config(uint32_t rocid, uint32_t idx, caen1190config *config);



