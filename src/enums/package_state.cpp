#include "simulator.hpp"

//============================================================================//

const char *get_enum_package_state_char(package_state_t type) {

	switch (type) {
		case PACKAGE_STATE_FREE:		 			return "FREE ";			break;
		case PACKAGE_STATE_READY:		 			return "READY ";		break;
		case PACKAGE_STATE_TRANSMIT:			return "TRANSMIT "; break;
		case PACKAGE_STATE_UNTREATED:			return "UNTRATED "; break;
		case PACKAGE_STATE_WAIT:		 			return "WAIT ";			break;
	};

	ERROR_PRINTF("Wrong PACKAGE_STATE\n");
	return "FAIL";
};

//============================================================================//
