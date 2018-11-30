#include "simulator.hpp"

// ========================================================================== //

orcs_engine_t orcs_engine;
libconfig::Config conf;
proc_conf Pconf;

// -------------------------------------------------------------------------- //

static void display_use() {

	ORCS_PRINTF("**** OrCS - Ordinary Computer Simulator ****\n\n");
	ORCS_PRINTF("Please provide -t <trace_file_basename> -f <output filename>\n");
};

// -------------------------------------------------------------------------- //

static void process_argv(int argc, char **argv) {
	static struct option long_options[] = {					// Name, {no_argument, required_argument and optional_argument}, flag, value
		{"help",				no_argument, 0, 'h'},
		{"trace",			 required_argument, 0, 't'},
		{"output_filename",			 optional_argument, 0, 'f'},
		{NULL,					0, NULL, 0}
	};

	// Count number of traces
	int opt;
	int option_index = 0;
	while ((opt = getopt_long_only(argc, argv, "h:t:f:", long_options, &option_index)) != -1) {
		switch (opt) {
		case 0:
			printf ("Option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

		case 'h':
			display_use();
			break;

		case 't':
			orcs_engine.arg_trace_file_name = optarg;
			break;
		case 'f':
			orcs_engine.output_file_name = optarg;
			break;
		case '?':
			break;

		default:
			ORCS_PRINTF(">> getopt returned character code 0%o ??\n", opt);
		}
	}

	if (optind < argc) {
		ORCS_PRINTF("Non-option ARGV-elements: ");
		while (optind < argc)
			ORCS_PRINTF("%s ", argv[optind++]);
		ORCS_PRINTF("\n");
	}


	if (orcs_engine.arg_trace_file_name == NULL) {
		ORCS_PRINTF("Trace file not defined.\n");
		display_use();
	}
};

// -------------------------------------------------------------------------- //

std::string get_status_execution(){
	std::string final_report;
	char report[1000];

	// Benchmark name
	snprintf(report,sizeof(report),"%s","==========================================================================\n");
	final_report += report;
	snprintf(report,sizeof(report),"Benchmark %s\n",orcs_engine.arg_trace_file_name);
	final_report += report;

	// get actual cicle
	snprintf(report,sizeof(report),"Actual Cycle %lu\n",orcs_engine.get_global_cycle());
	final_report += report;

	// Get	status opcodes total, executed -> calculate percent
	uint64_t total_opcodes = orcs_engine.trace_reader->get_trace_opcode_max();
	uint64_t fetched_opcodes = orcs_engine.trace_reader->get_fetch_instructions();
	snprintf(report,sizeof(report),"opcodes Processed: %lu of %lu\n",fetched_opcodes,total_opcodes);
	final_report += report;

	double percentage_complete = 100.0 * (static_cast<double>(fetched_opcodes) / static_cast<double>(total_opcodes));

	snprintf(report,sizeof(report),"Total Progress %8.4lf%%\n",percentage_complete);
	final_report += report;

	// IPC parcial
	snprintf(report,sizeof(report)," IPC(%5.3lf)\n", static_cast<double>(fetched_opcodes) / static_cast<double>(orcs_engine.get_global_cycle()));
	final_report+=report;

	// get time of execution
	gettimeofday(&orcs_engine.stat_timer_end, NULL);
	double seconds_spent = orcs_engine.stat_timer_end.tv_sec - orcs_engine.stat_timer_start.tv_sec;

	double seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
	snprintf(report,sizeof(report), "ETC(%02.0f:%02.0f:%02.0f)\n",
						floor(seconds_remaining / 3600.0),
						floor(fmod(seconds_remaining, 3600.0) / 60.0),
						fmod(seconds_remaining, 60.0));

	final_report += report;
	snprintf(report,sizeof(report),"%s","==========================================================================\n");
	final_report += report;

	return final_report;
}

// -------------------------------------------------------------------------- //

int check_config( libconfig::Config &conf ){
	bool res;

	res = true;
	res &= conf.lookupValue( "processor.stages_widht.fetch", Pconf.width.fetch );
	res &= conf.lookupValue( "processor.stages_widht.decode", Pconf.width.decode );
	res &= conf.lookupValue( "processor.stages_widht.rename", Pconf.width.rename );
	res &= conf.lookupValue( "processor.stages_widht.dispatch", Pconf.width.dispatch );
	res &= conf.lookupValue( "processor.stages_widht.execute", Pconf.width.execute );
	res &= conf.lookupValue( "processor.stages_widht.commit", Pconf.width.commit );

	if( !res ){
		ORCS_PRINTF("Fail to read stages width configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.latencies.fetch", Pconf.latency.fetch );
	res &= conf.lookupValue( "processor.latencies.decode", Pconf.latency.decode );
	res &= conf.lookupValue( "processor.latencies.rename", Pconf.latency.rename );
	res &= conf.lookupValue( "processor.latencies.dispatch", Pconf.latency.dispatch );
	res &= conf.lookupValue( "processor.latencies.execute", Pconf.latency.execute );
	res &= conf.lookupValue( "processor.latencies.commit", Pconf.latency.commit );

	if( !res ){
		ORCS_PRINTF("Fail to read latencies configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.functional_units.integer.alu.latency", Pconf.int_alu.latency );
	res &= conf.lookupValue( "processor.functional_units.integer.alu.wait_next", Pconf.int_alu.wait_next );
	res &= conf.lookupValue( "processor.functional_units.integer.alu.integer", Pconf.int_alu._ );

	res &= conf.lookupValue( "processor.functional_units.integer.mul.latency", Pconf.int_mul.latency );
	res &= conf.lookupValue( "processor.functional_units.integer.mul.wait_next", Pconf.int_mul.wait_next );
	res &= conf.lookupValue( "processor.functional_units.integer.mul.integer", Pconf.int_mul._ );

	res &= conf.lookupValue( "processor.functional_units.integer.div.latency", Pconf.int_div.latency );
	res &= conf.lookupValue( "processor.functional_units.integer.div.wait_next", Pconf.int_div.wait_next );
	res &= conf.lookupValue( "processor.functional_units.integer.div.integer", Pconf.int_div._ );

	if( !res ){
		ORCS_PRINTF("Fail to read integer configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.functional_units.floating_points.alu.latency", Pconf.fp_alu.latency );
	res &= conf.lookupValue( "processor.functional_units.floating_points.alu.wait_next", Pconf.fp_alu.wait_next );
	res &= conf.lookupValue( "processor.functional_units.floating_points.alu.FP", Pconf.fp_alu._ );

	res &= conf.lookupValue( "processor.functional_units.floating_points.mul.latency", Pconf.fp_mul.latency );
	res &= conf.lookupValue( "processor.functional_units.floating_points.mul.wait_next", Pconf.fp_mul.wait_next );
	res &= conf.lookupValue( "processor.functional_units.floating_points.mul.FP", Pconf.fp_mul._ );

	res &= conf.lookupValue( "processor.functional_units.floating_points.div.latency", Pconf.fp_div.latency );
	res &= conf.lookupValue( "processor.functional_units.floating_points.div.wait_next", Pconf.fp_div.wait_next );
	res &= conf.lookupValue( "processor.functional_units.floating_points.div.FP", Pconf.fp_div._ );

	if( !res ){
		ORCS_PRINTF("Fail to read floating point configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.functional_units.memory.load.latency", Pconf.load.latency );
	res &= conf.lookupValue( "processor.functional_units.memory.load.wait_next", Pconf.load.wait_next );
	res &= conf.lookupValue( "processor.functional_units.memory.load.unit", Pconf.load._ );

	res &= conf.lookupValue( "processor.functional_units.memory.store.latency", Pconf.store.latency );
	res &= conf.lookupValue( "processor.functional_units.memory.store.wait_next", Pconf.store.wait_next );
	res &= conf.lookupValue( "processor.functional_units.memory.store.unit", Pconf.store._ );

	if( !res ){
		ORCS_PRINTF("Fail to read memory configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.functional_units.parallel_loads", Pconf.p_loads );
	res &= conf.lookupValue( "processor.functional_units.parallel_stores", Pconf.p_stores );

	if( !res ){
		ORCS_PRINTF("Fail to read functional units configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.buffers_size.fetch_buffer", Pconf.buff.fetch );
	res &= conf.lookupValue( "processor.buffers_size.decode_buffer", Pconf.buff.decode );
	res &= conf.lookupValue( "processor.buffers_size.RAT_size", Pconf.buff.rat );
	res &= conf.lookupValue( "processor.buffers_size.ROB_size", Pconf.buff.rob );
	res &= conf.lookupValue( "processor.buffers_size.unified_RS", Pconf.buff.unified_rs );

	if( !res ){
		ORCS_PRINTF("Fail to read buffers size configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.MOB.read", Pconf.mb.read );
	res &= conf.lookupValue( "processor.MOB.write", Pconf.mb.write );

	if( !res ){
		ORCS_PRINTF("Fail to read MOB configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.branch_predictor.entries", Pconf.btb_ent );
	res &= conf.lookupValue( "processor.branch_predictor.ways", Pconf.btb_way );

	if( !res ){
		ORCS_PRINTF("Fail to read branch predictor configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.btb_miss_penality", Pconf.mp.btb_miss_penality );
	res &= conf.lookupValue( "processor.missprediction_penality", Pconf.mp.missprediction_penality );

	if( !res ){
		ORCS_PRINTF("Fail to read penalitys configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.N", Pconf.n );
	res &= conf.lookupValue( "processor.M", Pconf.m );
	res &= conf.lookupValue( "processor.H", Pconf.h );
	res &= conf.lookupValue( "processor.theta", Pconf.theta );

	if( !res ){
		ORCS_PRINTF("Fail to read constants configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.attr_common.line_size", Pconf.line_size );
	res &= conf.lookupValue( "processor.caches.attr_common.cache_levels", Pconf.cache_levels );
	res &= conf.lookupValue( "processor.caches.attr_common.instruction_enabled", Pconf.instruction_enabled );
	res &= conf.lookupValue( "processor.caches.attr_common.offset_size", Pconf.offset_size );
	res &= conf.lookupValue( "processor.caches.attr_common.total_levels", Pconf.total_levels );

	if( !res ){
		ORCS_PRINTF("Fail to read attr common configuration!");
		return 0;
	}

	Pconf.cache = new caches[ Pconf.total_levels ];
	char buffer[128];

	for( uint32_t n = 0 ; n < Pconf.total_levels ; n++ ){
		sprintf( buffer, "processor.caches.levels.[%d].name", n );						res &= conf.lookupValue( buffer, Pconf.cache[n].name );						//printf("%s ", Pconf.cache[n].name );
		sprintf( buffer, "processor.caches.levels.[%d].size", n );						res &= conf.lookupValue( buffer, Pconf.cache[n].size );						//printf("%llu ", Pconf.cache[n].size );
		sprintf( buffer, "processor.caches.levels.[%d].associativity", n );		res &= conf.lookupValue( buffer, Pconf.cache[n].associativity );	//printf("%d ", Pconf.cache[n].associativity );
		sprintf( buffer, "processor.caches.levels.[%d].latency", n );					res &= conf.lookupValue( buffer, Pconf.cache[n].latency );				//printf("%d", Pconf.cache[n].latency );
		Pconf.cache[n].sets = (Pconf.cache[n].size / Pconf.line_size) / Pconf.cache[n].associativity;																						//printf("%d\n", Pconf.cache[n].sets );

		if( !res ){
			ORCS_PRINTF("Fail to read cache configuration!");
			return 0;
		}
	}


	res &= conf.lookupValue( "processor.caches.ram.latency", Pconf.ram.latency );
	res &= conf.lookupValue( "processor.caches.ram.size", Pconf.ram.size );

	if( !res ){
		ORCS_PRINTF("Fail to read ram cache configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.prefetcher.stride_table_size", Pconf.pref.stride_table_size );
	res &= conf.lookupValue( "processor.caches.prefetcher.degree", Pconf.pref.degree );
	res &= conf.lookupValue( "processor.caches.prefetcher.distance", Pconf.pref.distance );

	if( !res ){
		ORCS_PRINTF("Fail to read prefetcher configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.memory_desambiguation.load_hash_size", Pconf.md.load_hash_size );
	res &= conf.lookupValue( "processor.caches.memory_desambiguation.store_hash_size", Pconf.md.store_hash_size );
	res &= conf.lookupValue( "processor.caches.memory_desambiguation.block_size", Pconf.md.desambiguation_block_size );

	if( !res ){
		ORCS_PRINTF("Fail to read memory desambiguation configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.address_to_address", Pconf.md.address_to_address );
	res &= conf.lookupValue( "processor.caches.register_forward", Pconf.md.register_forward );

	if( !res ){
		ORCS_PRINTF("Fail to read caches configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.emc.width.dispatch", Pconf.emc_width.dispatch );
	res &= conf.lookupValue( "processor.caches.emc.width.execute", Pconf.emc_width.execute );
	res &= conf.lookupValue( "processor.caches.emc.width.commit", Pconf.emc_width.commit );

	if( !res ){
		ORCS_PRINTF("Fail to read emc width configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.emc.latencies.dispatch", Pconf.emc_latency.dispatch );
	res &= conf.lookupValue( "processor.caches.emc.latencies.integer", Pconf.emc_latency.integer );
	res &= conf.lookupValue( "processor.caches.emc.latencies.commit", Pconf.emc_latency.commit );

	if( !res ){
		ORCS_PRINTF("Fail to read emc latencies configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.emc.wait_next_integer", Pconf.emc_wait_next_integer );

	if( !res ){
		ORCS_PRINTF("Fail to read emc configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.emc.sizes.integer_alu", Pconf.es.integer_alu );
	res &= conf.lookupValue( "processor.caches.emc.sizes.unified_rs", Pconf.es.unified_rs );
	res &= conf.lookupValue( "processor.caches.emc.sizes.uop_buffer", Pconf.es.uop_buffer );
	res &= conf.lookupValue( "processor.caches.emc.sizes.registers", Pconf.es.registers );
	res &= conf.lookupValue( "processor.caches.emc.sizes.lsq_size", Pconf.es.lsq_size );

	if( !res ){
		ORCS_PRINTF("Fail to read emc sizes configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.caches.emc.cache.size", Pconf.emc_cache.size );
	res &= conf.lookupValue( "processor.caches.emc.cache.associativity", Pconf.emc_cache.associativity );
	res &= conf.lookupValue( "processor.caches.emc.cache.latency", Pconf.emc_cache.latency );

	if( !res ){
		ORCS_PRINTF("Fail to read emc cache configuration!");
		return 0;
	}

	res &= conf.lookupValue( "processor.heartbeat_clocks", Pconf.heartbeat_clocks );
	res &= conf.lookupValue( "processor.clocks_to_check", Pconf.clocks_to_check );
	res &= conf.lookupValue( "processor.wait_cycle", Pconf.wait_cycle );

	if( !res ){
		ORCS_PRINTF("Fail to read simulator values configuration!");
		return 0;
	}

	return 1;
}

// -------------------------------------------------------------------------- //

int main(int argc, char **argv) {
	process_argv(argc, argv);

	try{	conf.readFile( MODEL_NAME );		}
	catch(FileIOException){
		puts( "Não foi possível ler o arquivo de dados do modelo do processador!!" );
		exit( 0 );
	}
	catch(ParseException){
		puts( "Houve um erro ao ler o arquivo de configurações!!" );
		exit( 0 );
	}

	if( !check_config( conf ) )
		return 0;
	else
		ORCS_PRINTF("Configurações lidas corretamente!!\n")

	// Call all the allocate's
	orcs_engine.allocate();
	orcs_engine.trace_reader->allocate(orcs_engine.arg_trace_file_name);

	orcs_engine.processor->allocate();										// Processor
	orcs_engine.branchPredictor->allocate();							// Branch Predictor
	orcs_engine.cacheManager->allocate();									// Cache Manager
	orcs_engine.memory_controller->allocate();						// Memory Controller
	orcs_engine.simulator_alive = true;										// initializate simulator

	while (orcs_engine.simulator_alive) {									// Start CLOCK for all the components
		#if HEARTBEAT
		if(orcs_engine.get_global_cycle()%HEARTBEAT_CLOCKS == 0){
			ORCS_PRINTF("%s\n",get_status_execution().c_str())
		}
		#endif
		orcs_engine.memory_controller->clock();
		orcs_engine.processor->clock();

		orcs_engine.global_cycle++;
	}

	// to be prinf
	ORCS_PRINTF("End of Simulation\n")
	orcs_engine.trace_reader->statistics();
	orcs_engine.processor->statistics();
	orcs_engine.branchPredictor->statistics();
	orcs_engine.cacheManager->statistics();
	orcs_engine.memory_controller->statistics();

	ORCS_PRINTF("Deleting Processor\n")
	delete orcs_engine.processor;
	ORCS_PRINTF("Deleting Branch predictor\n")
	delete orcs_engine.branchPredictor;
	ORCS_PRINTF("Deleting Cache manager\n")
	delete orcs_engine.cacheManager;
	ORCS_PRINTF("Deleting Memory Controller\n")
	delete orcs_engine.memory_controller;

	return(EXIT_SUCCESS);
};

// ========================================================================== //
