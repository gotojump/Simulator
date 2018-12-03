
/*
 *Arquivo definindo os parametros do modelo de processador
 */

#ifndef MODEL_H
#define MODEL_H

// ========================================================================== //

typedef struct stages{
	uint32_t fetch;
	uint32_t decode;
	uint32_t rename;
	uint32_t dispatch;
	uint32_t execute;
	uint32_t commit;
} stages;

// -------------------------------------------------------------------------- //

typedef struct FU{
	uint32_t latency;
	uint32_t wait_next;
	uint32_t _;
} FU;

// -------------------------------------------------------------------------- //

typedef struct buffs{
	uint32_t fetch;
	uint32_t decode;
	uint32_t rat;
	uint32_t rob;
	uint32_t unified_rs;
} buffs;

// -------------------------------------------------------------------------- //

typedef struct mob{
	uint32_t read;
	uint32_t write;
} mob;

// -------------------------------------------------------------------------- //

typedef struct miss_penality{
	uint32_t btb_miss_penality;
	uint32_t missprediction_penality;
} miss_penality;

// -------------------------------------------------------------------------- //

typedef struct caches{
	const char* name;
	unsigned long long size;
	uint32_t associativity;
	uint32_t latency;
	uint32_t sets;
} caches;

// -------------------------------------------------------------------------- //

typedef struct prefet{
	uint32_t stride_table_size;
	uint32_t degree;
	uint32_t distance;
} prefet;

// -------------------------------------------------------------------------- //

typedef struct mem_desamb{
	uint32_t load_hash_size;
	uint32_t store_hash_size;
	uint32_t desambiguation_block_size;
	uint32_t address_to_address;
	uint32_t register_forward;
} mem_desamb;

// -------------------------------------------------------------------------- //

typedef struct emcs{
	uint32_t dispatch;
	uint32_t integer;
	uint32_t execute;
	uint32_t commit;
} emcs;

// -------------------------------------------------------------------------- //

typedef struct emc_sizes{
	uint32_t integer_alu;
	uint32_t unified_rs;
	uint32_t uop_buffer;
	uint32_t registers;
	uint32_t lsq_size;
} emc_sizes;

// -------------------------------------------------------------------------- //

typedef struct proc_conf{
	const char* name;

	stages width, latency;

	FU int_alu, int_mul, int_div;
	FU fp_alu, fp_mul, fp_div;
	FU load, store;

	uint32_t p_loads, p_stores;
	uint32_t btb_ent, btb_way;
	buffs buff;
	mob mb;
	miss_penality mp;

	uint32_t n, m, h;		float theta;
	uint32_t line_size, cache_levels, total_levels, instruction_enabled, offset_size;

	caches *cache, ram, emc_cache;
	uint32_t max_parallel_requests;

	prefet pref;
	mem_desamb md;

	uint32_t emc_wait_next_integer;
	emcs emc_width, emc_latency;
	emc_sizes es;

	uint32_t heartbeat_clocks, clocks_to_check, wait_cycle;
} proc_conf;

// ========================================================================== //

#define FETCH_WIDTH				Pconf.width.fetch
#define DECODE_WIDTH			Pconf.width.decode
#define RENAME_WIDTH			Pconf.width.rename
#define DISPATCH_WIDTH		Pconf.width.dispatch
#define EXECUTE_WIDTH			Pconf.width.execute
#define COMMIT_WIDTH			Pconf.width.commit

// -------------------------------------------------------------------------- //

// PROCESSOR LATENCIES STAGES
#define FETCH_LATENCY			Pconf.latency.fetch
#define DECODE_LATENCY		Pconf.latency.decode
#define RENAME_LATENCY		Pconf.latency.rename
#define DISPATCH_LATENCY	Pconf.latency.dispatch
#define EXECUTE_LATENCY		Pconf.latency.execute
#define COMMIT_LATENCY		Pconf.latency.commit

// -------------------------------------------------------------------------- //

// FUNCTIONAL UNITS RELATED

// INTEGER ALU
#define LATENCY_INTEGER_ALU		Pconf.int_alu.latency
#define WAIT_NEXT_INT_ALU			Pconf.int_alu.wait_next
#define INTEGER_ALU						Pconf.int_alu._

// -------------------------------------------------------------------------- //

// INTEGER MUL
#define LATENCY_INTEGER_MUL		Pconf.int_mul.latency
#define WAIT_NEXT_INT_MUL			Pconf.int_mul.wait_next
#define INTEGER_MUL						Pconf.int_mul._

// -------------------------------------------------------------------------- //

// INTEGER DIV
#define LATENCY_INTEGER_DIV		Pconf.int_div.latency
#define WAIT_NEXT_INT_DIV			Pconf.int_div.wait_next
#define INTEGER_DIV						Pconf.int_div._

// -------------------------------------------------------------------------- //

#define QTDE_INTEGER_FU				(INTEGER_ALU + INTEGER_MUL + INTEGER_DIV)

// -------------------------------------------------------------------------- //

// FP ULAS LATENCY

// FLOATING POINT ALU
#define LATENCY_FP_ALU				Pconf.fp_alu.latency
#define WAIT_NEXT_FP_ALU			Pconf.fp_alu.wait_next
#define FP_ALU								Pconf.fp_alu._

// -------------------------------------------------------------------------- //

// FLOATING POINT MUL
#define LATENCY_FP_MUL				Pconf.fp_mul.latency
#define WAIT_NEXT_FP_MUL			Pconf.fp_mul.wait_next
#define FP_MUL								Pconf.fp_mul._

// -------------------------------------------------------------------------- //

// FLOATING POINT DIV
#define LATENCY_FP_DIV				Pconf.fp_div.latency
#define WAIT_NEXT_FP_DIV			Pconf.fp_div.wait_next
#define FP_DIV								Pconf.fp_div._

// -------------------------------------------------------------------------- //

#define QTDE_FP_FU						(FP_ALU + FP_MUL + FP_DIV)

// -------------------------------------------------------------------------- //

// MEMORY FU

// Load Units
#define LOAD_UNIT							Pconf.load._
#define WAIT_NEXT_MEM_LOAD		Pconf.load.wait_next
#define LATENCY_MEM_LOAD			Pconf.load.latency

// -------------------------------------------------------------------------- //

// Store Units
#define STORE_UNIT						Pconf.store._
#define WAIT_NEXT_MEM_STORE		Pconf.store.wait_next
#define LATENCY_MEM_STORE			Pconf.store.latency

// -------------------------------------------------------------------------- //

#define QTDE_MEMORY_FU				(LOAD_UNIT + STORE_UNIT)

// -------------------------------------------------------------------------- //

// Parallel Loads
#define PARALLEL_LOADS				Pconf.p_loads
#define PARALLEL_STORES				Pconf.p_stores

// -------------------------------------------------------------------------- //

// PROCESSOR BUFFERS SIZE
#define FETCH_BUFFER					Pconf.buff.fetch
#define DECODE_BUFFER					Pconf.buff.decode
#define RAT_SIZE							Pconf.buff.rat
#define ROB_SIZE							Pconf.buff.rob
#define UNIFIED_RS						Pconf.buff.unified_rs

// -------------------------------------------------------------------------- //

//MOB
#define MOB_READ							Pconf.mb.read
#define MOB_WRITE							Pconf.mb.write

// -------------------------------------------------------------------------- //

// BRANCH PREDICTOR
#define BTB_ENTRIES						Pconf.btb_ent
#define BTB_WAYS							Pconf.btb_way

// -------------------------------------------------------------------------- //

#define BTB_MISS_PENALITY						Pconf.mp.btb_miss_penality
#define MISSPREDICTION_PENALITY			Pconf.mp.missprediction_penality

// -------------------------------------------------------------------------- //

#define N														Pconf.n
#define M														Pconf.m
#define H														Pconf.h
#define	THETA												Pconf.theta

// -------------------------------------------------------------------------- //

// DEFINES CACHE
#define KILO 									1024
#define MEGA									(KILO*KILO)

// -------------------------------------------------------------------------- //

// CACHES

#define INTRUCTION_CACHE			0										// Cache de instrução ocupa a posição 0 no arquivo de configuração
#define DATA_CACHE						1										// Cache de dados começam na posição 1 no arquivo de configuração

// ATTR COMMON
#define LINE_SIZE							Pconf.line_size
#define CACHE_LEVELS					Pconf.cache_levels
#define INSTRUCTION_ENABLED		Pconf.instruction_enabled
#define OFFSET_SIZE						Pconf.offset_size

// -------------------------------------------------------------------------- //

// LEVEL 1
// https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf
// Valores retirados do manual de otimização

// D$
#define L1_DATA_SIZE					Pconf.cache[0].size
#define L1_DATA_ASSOCIATIVITY	Pconf.cache[0].associativity
#define L1_DATA_LATENCY				Pconf.cache[0].latency
#define L1_DATA_SETS					(L1_DATA_SIZE/LINE_SIZE)/L1_DATA_ASSOCIATIVITY

// -------------------------------------------------------------------------- //

// I$
#define L1_INST_SIZE					Pconf.cache[1].size
#define L1_INST_ASSOCIATIVITY	Pconf.cache[1].associativity
#define L1_INST_LATENCY				Pconf.cache[1].latency
#define L1_INST_SETS					(L1_INST_SIZE/LINE_SIZE)/L1_INST_ASSOCIATIVITY

// -------------------------------------------------------------------------- //

// LEVEL 2
#define L2_SIZE								Pconf.cache[2].size
#define L2_ASSOCIATIVITY			Pconf.cache[2].associativity
#define L2_LATENCY						Pconf.cache[2].latency
#define L2_SETS								(L2_SIZE/LINE_SIZE)/L2_ASSOCIATIVITY

// -------------------------------------------------------------------------- //

// LLC
#define LLC_SIZE							Pconf.cache[3].size
#define LLC_ASSOCIATIVITY			Pconf.cache[3].associativity
#define LLC_LATENCY						Pconf.cache[3].latency
#define LLC_SETS							(LLC_SIZE/LINE_SIZE)/LLC_ASSOCIATIVITY

// -------------------------------------------------------------------------- //

// RAM
#define RAM_LATENCY						Pconf.ram.latency
#define RAM_SIZE							Pconf.ram.size
#define MAX_PARALLEL_REQUESTS	Pconf.max_parallel_requests

// -------------------------------------------------------------------------- //

// PREFETCHER
#define STRIDE_TABLE_SIZE			Pconf.pref.stride_table_size
#define DEGREE								Pconf.pref.degree
#define DISTANCE							Pconf.pref.distance

// -------------------------------------------------------------------------- //

// MEMORY DESAMBIGUATION
#define LOAD_HASH_SIZE							Pconf.md.load_hash_size
#define STORE_HASH_SIZE							Pconf.md.store_hash_size
#define DESAMBIGUATION_BLOCK_SIZE		Pconf.md.desambiguation_block_size

// -------------------------------------------------------------------------- //

#define ADDRESS_TO_ADDRESS					Pconf.md.address_to_address
#define REGISTER_FORWARD						Pconf.md.register_forward

// -------------------------------------------------------------------------- //

// EMC

// WIDHTs
#define EMC_DISPATCH_WIDTH					Pconf.emc_width.dispatch
#define EMC_EXECUTE_WIDTH						Pconf.emc_width.execute
#define EMC_COMMIT_WIDTH						Pconf.emc_width.commit

// -------------------------------------------------------------------------- //

// Latencies
#define EMC_DISPATCH_LATENCY				Pconf.emc_latency.dispatch
#define EMC_INTEGER_LATENCY					Pconf.emc_latency.integer
#define EMC_COMMIT_LATENCY					Pconf.emc_latency.commit

// -------------------------------------------------------------------------- //

// pipelines
#define EMC_WAIT_NEXT_INTEGER				Pconf.emc_wait_next_integer

// -------------------------------------------------------------------------- //

// Sizes
#define EMC_INTEGER_ALU							Pconf.es.integer_alu
#define EMC_UNIFIED_RS							Pconf.es.unified_rs
#define EMC_UOP_BUFFER							Pconf.es.uop_buffer
#define EMC_REGISTERS								Pconf.es.registers
#define EMC_LSQ_SIZE								Pconf.es.lsq_size

// -------------------------------------------------------------------------- //

// EMC CACHE
#define EMC_CACHE_SIZE							Pconf.emc_cache.size
#define EMC_CACHE_ASSOCIATIVITY			Pconf.emc_cache.associativity
#define EMC_CACHE_LATENCY						Pconf.emc_cache.latency
#define EMC_CACHE_SETS							(EMC_CACHE_SIZE/LINE_SIZE)/EMC_CACHE_ASSOCIATIVITY

// -------------------------------------------------------------------------- //

// CHECKS
#define HEARTBEAT_CLOCKS			Pconf.heartbeat_clocks

#define CLOCKS_TO_CHECK				Pconf.clocks_to_check
#define WAIT_CYCLE						Pconf.wait_cycle

// -------------------------------------------------------------------------- //

#define STRIDE									1
#define HEARTBEAT								1
#define SANITY_CHECK						0
#define PERIODIC_CHECK					0
#define PARALLEL_REQUESTS				1
#define TWO_BIT									0
#define PIECEWISE								1

#define DESAMBIGUATION_ENABLED	1
#define EMC_ACTIVE							0
#define PREFETCHER_ACTIVE				1
#define PARALLEL_LIM_ACTIVE			0

// -------------------------------------------------------------------------- //

// DEBUGS
#define DEBUG									0

#if DEBUG
#define FETCH_DEBUG						0
#define DECODE_DEBUG					0
#define RENAME_DEBUG					0
#define DISPATCH_DEBUG				0
#define EXECUTE_DEBUG					0
#define MOB_DEBUG							0
#define COMMIT_DEBUG					1
#define CACHE_MANAGER_DEBUG		0

#define EMC_DISPATCH_DEBUG		0
#define EMC_EXECUTE_DEBUG			0
#define EMC_COMMIT_DEBUG			0

#endif

// ========================================================================== //

#endif
