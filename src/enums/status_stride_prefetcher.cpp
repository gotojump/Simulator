#include "simulator.hpp"

//============================================================================//

const char *get_enum_status_stride_prefetcher_char(status_stride_prefetcher_t type) {

	switch (type){
		case INVALID:			return "INVALID";		break;
		case TRAINING:		return "TRAINING";	break;
		case ACTIVE:			return "ACTIVE";		break;
	};

	ERROR_PRINTF("Wrong Status Prefetcher STRIDE\n");

	return "FAIL";
};

//============================================================================//
