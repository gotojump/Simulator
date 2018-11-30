#include "simulator.hpp"

//============================================================================//

const char *get_enum_processor_stage_char(processor_stage_t type) {

	switch (type) {
		case PROCESSOR_STAGE_FETCH:			return "FETCH		";	break;
		case PROCESSOR_STAGE_DECODE:		return "DECODE	 ";	break;
		case PROCESSOR_STAGE_RENAME:		return "RENAME	 ";	break;
		case PROCESSOR_STAGE_DISPATCH:	return "DISPATCH ";	break;
		case PROCESSOR_STAGE_EXECUTION: return "EXECUTION";	break;
		case PROCESSOR_STAGE_COMMIT:		return "COMMIT	 ";	break;
	};

	ERROR_PRINTF("Wrong PROCESSOR_STAGE\n");

	return "FAIL";
};

//============================================================================//
