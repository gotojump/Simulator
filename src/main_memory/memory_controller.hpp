#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

// ========================================================================== //

class memory_controller_t{
	private:
		uint64_t requests_made;					// Data Requests made
		uint64_t operations_executed;		// number of operations executed
		uint64_t requests_emc;					// Data Requests made to DRAM
		uint64_t requests_llc;					// Data Requests made to LLC
	public:
		emc_t *emc;

	public:
		void allocate();								// Aloca recursos do Memory Controller
		memory_controller_t();
		~memory_controller_t();

		void clock();
		void statistics();
		INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
		INSTANTIATE_GET_SET_ADD(uint64_t,operations_executed)
		INSTANTIATE_GET_SET_ADD(uint64_t,requests_emc)
		INSTANTIATE_GET_SET_ADD(uint64_t,requests_llc)
		uint64_t requestDRAM();				
};

// ========================================================================== //

#endif
