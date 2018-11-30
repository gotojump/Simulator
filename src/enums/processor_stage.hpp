#ifndef PROCESSOR_STAGE_H
#define PROCESSOR_STAGE_H

//============================================================================//

//PROCESSOR STAGES
enum processor_stage_t {
	PROCESSOR_STAGE_FETCH,
	PROCESSOR_STAGE_DECODE,
	PROCESSOR_STAGE_RENAME,
	PROCESSOR_STAGE_DISPATCH,
	PROCESSOR_STAGE_EXECUTION,
	PROCESSOR_STAGE_COMMIT
};

//----------------------------------------------------------------------------//

const char *get_enum_processor_stage_char(processor_stage_t type);

//============================================================================//

#endif
