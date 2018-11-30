#ifndef MEMORY_OPERATION_H
#define MEMORY_OPERATION_H

//============================================================================//

enum memory_operation_t {
	MEMORY_OPERATION_READ,
	MEMORY_OPERATION_WRITE,
	MEMORY_OPERATION_FREE,
};

//----------------------------------------------------------------------------//

const char *get_enum_memory_operation_char(memory_operation_t type);

//============================================================================//

#endif
