#ifndef INSTRUCTION_OPERATION_H
#define INSTRUCTION_OPERATION_H

//============================================================================//

enum instruction_operation_t {
	// NOP
	INSTRUCTION_OPERATION_NOP,

	// INTEGERS
	INSTRUCTION_OPERATION_INT_ALU,
	INSTRUCTION_OPERATION_INT_MUL,
	INSTRUCTION_OPERATION_INT_DIV,

	// FLOAT POINT
	INSTRUCTION_OPERATION_FP_ALU,
	INSTRUCTION_OPERATION_FP_MUL,
	INSTRUCTION_OPERATION_FP_DIV,

	// BRANCHES
	INSTRUCTION_OPERATION_BRANCH,

	// MEMORY OPERATIONS
	INSTRUCTION_OPERATION_MEM_LOAD,
	INSTRUCTION_OPERATION_MEM_STORE,

	// NOT IDENTIFIED
	INSTRUCTION_OPERATION_OTHER,

	// SYNCHRONIZATION
	INSTRUCTION_OPERATION_BARRIER,

	// HMC
	INSTRUCTION_OPERATION_HMC_ROA,		 //#12 READ+OP +Answer
	INSTRUCTION_OPERATION_HMC_ROWA		 //#13 READ+OP+WRITE +Answer
};

//----------------------------------------------------------------------------//

const char* get_enum_instruction_operation_char(instruction_operation_t type);

//============================================================================//

#endif
