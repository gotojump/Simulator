#include "simulator.hpp"

//============================================================================//

const char *get_enum_memory_operation_char(memory_operation_t type) {

		switch (type) {
			case MEMORY_OPERATION_READ:			return "MEMORY_OPERATION_READ ";	break;
			case MEMORY_OPERATION_WRITE:		return "MEMORY_OPERATION_WRITE ";	break;
			case MEMORY_OPERATION_FREE:			return "MEMORY_OPERATION_FREE ";	break;
		};

		ERROR_PRINTF("Wrong MEMORY_OPERATION\n");

		return "FAIL";
};

//============================================================================//
