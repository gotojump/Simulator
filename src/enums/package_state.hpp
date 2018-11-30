#ifndef PACKAGE_STATE_H
#define PACKAGE_STATE_H

//============================================================================//

enum package_state_t {
	PACKAGE_STATE_FREE,
	PACKAGE_STATE_UNTREATED,
	PACKAGE_STATE_READY,
	PACKAGE_STATE_WAIT,
	PACKAGE_STATE_TRANSMIT
};

//----------------------------------------------------------------------------//

const char *get_enum_package_state_char(package_state_t type);

//============================================================================//

#endif
