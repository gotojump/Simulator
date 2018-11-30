#include "simulator.hpp"

// ========================================================================== //

cache_manager_t::cache_manager_t(){

	this->inst_cache = NULL;
	this->n_caches = 0;
};

// -------------------------------------------------------------------------- //

cache_manager_t::~cache_manager_t()=default;

// -------------------------------------------------------------------------- //

void cache_manager_t::allocate(){

	//Allocate I$
	this->inst_cache = new cache_t;
	this->configCache( this->inst_cache, Pconf.cache[INTRUCTION_CACHE] );

	this->n_caches = CACHE_LEVELS;
	this->data_cache = new cache_t[CACHE_LEVELS];
	for( uint32_t n = 0 ; n < CACHE_LEVELS ; n++ ){
		this->configCache( &this->data_cache[n], Pconf.cache[DATA_CACHE + n]);
	}

	//Read/Write counters
	this->set_readHit(0);
	this->set_readMiss(0);
	this->set_writeHit(0);
	this->set_writeMiss(0);

	//Allocate Prefetcher
	#if PREFETCHER_ACTIVE
	this->prefetcher = new prefetcher_t;
	this->prefetcher->allocate();
	#endif
};

// -------------------------------------------------------------------------- //

void cache_manager_t::configCache(cache_t *cache, caches cache_conf){

	cache->allocate(cache_conf.name, cache_conf.sets, cache_conf.associativity, &orcs_engine.get_global_cycle());
	cache->setLatencies(RAM_LATENCY, cache_conf.latency);
}

// -------------------------------------------------------------------------- //

uint32_t cache_manager_t::searchInstruction(uint64_t instructionAddress){
	uint32_t ttc = 0;
	uint32_t latency_request = 0;
	uint32_t hit = this->inst_cache->read(instructionAddress,ttc);
	uint32_t llc = this->n_caches - 1;

	//if hit, add Searched instructions. Must be equal inst cache hit
	latency_request += ttc;
	if(hit == HIT){
		this->inst_cache->add_cacheAccess();
		this->inst_cache->add_cacheHit();
		this->add_instructionSearched();
	}
	else{
		this->inst_cache->add_cacheAccess();
		this->inst_cache->add_cacheMiss();
		this->add_instructionLLCSearched();

		hit = this->data_cache[llc].read(instructionAddress,ttc);

		/*
		// update inst cache miss, update instruction llc search.
		// Inst cache miss must be equal llc search inst
		// ORCS_PRINTF("Latency L1 MISS %u\n",latency_request)
		*/

		latency_request += ttc;
		this->data_cache[llc].add_cacheAccess();

		if(hit == HIT){
			this->data_cache[llc].add_cacheHit();
			this->data_cache[llc].returnLine(instructionAddress,this->inst_cache);
			#if CACHE_MANAGER_DEBUG
			//ORCS_PRINTF("Latency LLC HIT %u\n",latency_request)
			#endif
		}
		else{
			this->data_cache[llc].add_cacheMiss();

			//llc inst miss
			//request to Memory Controller
			ttc = orcs_engine.memory_controller->requestDRAM();
			orcs_engine.memory_controller->add_requests_llc();		 // requests made by LLC
			latency_request += ttc;
			#if CACHE_MANAGER_DEBUG
			//ORCS_PRINTF("Latency LLC MISS %u\n",latency_request)
			#endif

			// Install cache lines
			linha_t *linha_inst = NULL;
			linha_t *linha_llc = NULL;
			linha_llc = this->data_cache[llc].installLine(instructionAddress);
			linha_inst = this->inst_cache->installLine(instructionAddress);
			linha_inst->linha_ptr_sup=linha_llc;
			linha_llc->linha_ptr_inf=linha_inst;
		}
	}

	return latency_request;
};

// -------------------------------------------------------------------------- //

uint32_t cache_manager_t::searchData(memory_order_buffer_line_t *mob_line){
	#if CACHE_MANAGER_DEBUG
	ORCS_PRINTF("===================================\n")
	ORCS_PRINTF("Global_Cycle %lu\n",orcs_engine.get_global_cycle())
	#endif
	uint32_t ttc = 0;
	uint32_t latency_request = 0;
	uint32_t hit = this->data_cache[0].read(mob_line->memory_address,ttc);
	uint32_t llc = this->n_caches - 1;

	this->data_cache[0].add_cacheRead();
	latency_request += ttc;

	//L1 Hit
	if(hit == HIT){
		#if CACHE_MANAGER_DEBUG
		ORCS_PRINTF("L1 TTC %u\n",ttc)
		#endif

		this->data_cache[0].add_cacheAccess();
		this->data_cache[0].add_cacheHit();
	}
	else{
		uint32_t cl = 0;
		while( (cl < llc) && (hit != HIT) ){
			// MISS
			this->data_cache[cl].add_cacheAccess();
			this->data_cache[cl].add_cacheMiss();

			cl++;
			hit = this->data_cache[cl].read(mob_line->memory_address,ttc);
			this->data_cache[cl].add_cacheRead();

			// update inst cache miss, update instruction llc search.
			// Inst cache miss must be equal llc search inst

			latency_request += ttc;
			#if CACHE_MANAGER_DEBUG
			ORCS_PRINTF("LLC TTC %u\n",ttc)
			#endif
		}
		this->data_cache[cl].add_cacheAccess();

		if(hit == HIT){
			// LLC Hit
			this->data_cache[cl].add_cacheHit();

			for(int32_t c = cl - 1 ; c >= 0 ; c--)
				this->data_cache[cl].returnLine(mob_line->memory_address,&this->data_cache[c]);

			#if PREFETCHER_ACTIVE
			this->prefetcher->prefecht(mob_line,&this->data_cache[cl]);
			#endif
		}
		else{
			// LLC MISS
			this->add_readMiss();
			this->data_cache[cl].add_cacheMiss();

			orcs_engine.processor->has_llc_miss = true; // setting llc miss

			#if PREFETCHER_ACTIVE
			this->prefetcher->prefecht(mob_line,&this->data_cache[cl]);
			#endif

			//llc inst miss
			//request to Memory Controller
			ttc = orcs_engine.memory_controller->requestDRAM();
			orcs_engine.memory_controller->add_requests_llc(); // requests made by LLC
			latency_request += ttc;
			mob_line->waiting_dram = true;

			// Install cache lines
			linha_t *linha_l1 = NULL;
			linha_t *linha_llc = NULL;
			linha_llc = this->data_cache[llc].installLine(mob_line->memory_address);

			for(int32_t c = llc - 1 ; c >= 0 ; c--){
				linha_l1 = this->data_cache[c].installLine(mob_line->memory_address);
				linha_l1->linha_ptr_sup = linha_llc;
				linha_llc->linha_ptr_inf = linha_l1;
				linha_llc = linha_l1;
			}

			//EMC
			#if EMC_ACTIVE
			linha_t *linha_emc = NULL;
			linha_emc = orcs_engine.memory_controller->emc->data_cache->installLine(mob_line->memory_address);
			linha_llc->linha_ptr_emc = linha_emc;
			linha_emc->linha_ptr_inf = linha_llc;
			#endif

			#if CACHE_MANAGER_DEBUG
			ORCS_PRINTF("DRAM TTC %u\n",ttc)
			#endif
		}
	}

	#if CACHE_MANAGER_DEBUG
	ORCS_PRINTF("Total TTC %u\n",latency_request)
	ORCS_PRINTF("===================================\n")
	sleep(1);
	#endif

	return latency_request;
};

// -------------------------------------------------------------------------- //

uint32_t cache_manager_t::writeData(memory_order_buffer_line_t *mob_line){
	uint32_t ttc = 0;
	uint32_t latency_request = 0;
	uint32_t hit = this->data_cache[0].read(mob_line->memory_address,ttc);
	uint32_t llc = this->n_caches - 1;

	latency_request += ttc;

	if(hit==HIT){		//if hit, add Searched instructions. Must be equal inst cache hit
		this->data_cache[0].add_cacheAccess();
		this->data_cache[0].add_cacheHit();

		#if CACHE_MANAGER_DEBUG
		//ORCS_PRINTF("L1 Hit TTC %u\n",ttc)
		//ORCS_PRINTF("L1 Hit LR %u\n",latency_request)
		#endif

		this->data_cache[0].write(mob_line->memory_address);
	}
	else{

		uint32_t cl = 0;
		while( (cl < llc) && (hit != HIT) ){
			this->data_cache[cl].add_cacheAccess();
			this->data_cache[cl].add_cacheMiss();

			cl++;
			ttc = 0;
			hit = this->data_cache[cl].read(mob_line->memory_address,ttc);
		}

		/*
		// update inst cache miss, update instruction llc search.
		// Inst cache miss must be equal llc search inst
		*/

		this->data_cache[cl].add_cacheAccess();
		if(hit == HIT){
			this->data_cache[cl].add_cacheHit();

			latency_request+=ttc;
			//linha_t* linha_l1 = NULL;

			//install line new on d0
			for(int32_t c = cl - 1; c >= 0 ; c--){
				this->data_cache[cl].returnLine(mob_line->memory_address, &this->data_cache[c]);
			}
			this->data_cache[0].write(mob_line->memory_address);

			#if PREFETCHER_ACTIVE
			this->prefetcher->prefecht(mob_line,&this->data_cache[cl]);
			#endif
		}
		else{
			this->add_writeMiss();
			this->data_cache[cl].add_cacheMiss();

			#if PREFETCHER_ACTIVE
			this->prefetcher->prefecht(mob_line,&this->data_cache[cl]);
			#endif

			//llc inst miss
			ttc = orcs_engine.memory_controller->requestDRAM();
			orcs_engine.memory_controller->add_requests_llc();				// requests made by LLC
			latency_request += ttc;
			mob_line->waiting_dram = true;

			// Install cache lines
			linha_t *linha_l1 = NULL;
			linha_t *linha_llc = NULL;

			linha_llc = this->data_cache[llc].installLine(mob_line->memory_address);
			for(int32_t c = llc - 1 ; c >= 0 ; c--){
				linha_l1 = this->data_cache[c].installLine(mob_line->memory_address);
				linha_l1->linha_ptr_sup = linha_llc;
				linha_llc->linha_ptr_inf = linha_l1;
				linha_llc = linha_l1;
			}

			this->data_cache[0].write(mob_line->memory_address);
		}
	}
	return latency_request;
};

// -------------------------------------------------------------------------- //

uint32_t cache_manager_t::search_EMC_Data(memory_order_buffer_line_t *mob_line){
	uint32_t ttc = 0;
	uint32_t latency_request = 0;
	uint32_t hit = orcs_engine.memory_controller->emc->data_cache->read(mob_line->memory_address,ttc);
	uint32_t llc = this->n_caches - 1;

	orcs_engine.memory_controller->emc->data_cache->add_cacheRead();
	latency_request += ttc;

	if(hit == HIT){				// EMC data cache Hit
		orcs_engine.memory_controller->emc->data_cache->add_cacheAccess();
		orcs_engine.memory_controller->emc->data_cache->add_cacheHit();
	}
	else{
		// EMC CACHE MISS
		orcs_engine.memory_controller->emc->data_cache->add_cacheAccess();
		orcs_engine.memory_controller->emc->data_cache->add_cacheMiss();

		hit = this->data_cache[llc].read(mob_line->memory_address,ttc);
		// this->data_cache[1].add_cacheRead();

		/*
		// update inst cache miss, update inistruction llc search.
		// Inst cache miss must be equal llc search inst
		*/

		#if CACHE_MANAGER_DEBUG
		//ORCS_PRINTF("L1 MISS TTC %u\n",ttc)
		//ORCS_PRINTF("L1 MISS LR %u\n",latency_request)
		#endif

		if(hit == HIT){
			latency_request += ttc;

			// marcando access llc emc
			orcs_engine.memory_controller->emc->add_access_LLC();
			orcs_engine.memory_controller->emc->add_access_LLC_Hit();
		}
		else{
			orcs_engine.memory_controller->emc->add_access_LLC();
			orcs_engine.memory_controller->emc->add_access_LLC_Miss();

			linha_t *linha_llc = this->data_cache[llc].installLine(mob_line->memory_address);
			linha_t *linha_emc = orcs_engine.memory_controller->emc->data_cache->installLine(mob_line->memory_address);

			// linking emc and llc
			linha_llc->linha_ptr_emc = linha_emc;
			linha_emc->linha_ptr_inf = linha_llc;
			orcs_engine.memory_controller->add_requests_emc();				//number of requests made by emc
			orcs_engine.memory_controller->add_requests_made();				//add requests made by emc to total
			latency_request = RAM_LATENCY;
		}
	}

	return latency_request;
};

// -------------------------------------------------------------------------- //

void cache_manager_t::statistics(){
	FILE *output = stdout;

	if(orcs_engine.output_file_name != NULL)
		output = fopen(orcs_engine.output_file_name,"a+");

	if(output != NULL){
		utils_t::largestSeparator(output);
		fprintf(output,"##############	Cache Memories ##################\n");
		utils_t::largestSeparator(output);
	}

	if( output != NULL && output != stdout )
		fclose(output);

	//ORCS_PRINTF("############## Instruction Cache ##################\n")
	this->inst_cache->statistics();

	for(uint32_t c = 0 ; c < this->n_caches ; c++)
		this->data_cache[c].statistics();

	#if PREFETCHER_ACTIVE
	this->prefetcher->statistics();
	#endif
};

// ========================================================================== //
