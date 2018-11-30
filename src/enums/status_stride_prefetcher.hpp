#ifndef STATUS_STRIDE_PREFETCHER_H
#define STATUS_STRIDE_PREFETCHER_H

//============================================================================//

enum status_stride_prefetcher_t{
	INVALID,
	TRAINING,
	ACTIVE
};

//----------------------------------------------------------------------------//

const char *get_enum_status_stride_prefetcher_char(status_stride_prefetcher_t type);

//============================================================================//

#endif
