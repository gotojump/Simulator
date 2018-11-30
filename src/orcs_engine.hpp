#ifndef ORCS_ENGINE_H
#define ORCS_ENGINE_H

// ========================================================================== //

class orcs_engine_t{
	public:
		char *arg_trace_file_name;								// Program input
		uint64_t global_cycle;										// Control the Global Cycle
		char *output_file_name;										// output File name
		bool simulator_alive;

		trace_reader_t *trace_reader;							// Components modeled
		processor_t *processor;
		branch_predictor_t *branchPredictor;
		cache_manager_t *cacheManager;						// Cache Manager and cache
		memory_controller_t *memory_controller;		// Memory Controller and EMC

		// Statistics related
		// Time statistics
		struct timeval stat_timer_start, stat_timer_end;

	public:
		orcs_engine_t();
		void allocate();
		uint64_t& get_global_cycle() {
			return this->global_cycle;
		};
};

// ========================================================================== //

#endif
