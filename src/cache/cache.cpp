#include "simulator.hpp"

// ========================================================================== //

cache_t::cache_t(){

	this->id = 0;
	this->nSets = 0;
	this->nLines = 0;
	this->installLatency = 0;
	this->readLatency = 0;
	this->sets = NULL;
	this->shiftData = 0;
	this->cycle = NULL;

	this->level = "";
	this->cacheHit = 0;
	this->cacheMiss = 0;
	this->cacheAccess = 0;
	this->cacheRead = 0;
	this->cacheWrite = 0;
	this->cacheWriteBack = 0;
	this->changeLine = 0;
}

// -------------------------------------------------------------------------- //

cache_t::~cache_t(){

	if(this->sets!=NULL) delete[] &sets;
}

// -------------------------------------------------------------------------- //

inline void cache_t::printLine(linha_t *linha){

	ORCS_PRINTF("[TAG: %lu| DIRTY: %u| lru : %lu| PREFETCHED: %u| VALID: %u| READY AT %lu]\n",linha->tag,linha->dirty,linha->lru,linha->prefetched, linha->valid,linha->readyAt)
	#if SLEEP
	usleep(500);
	#endif
};

// -------------------------------------------------------------------------- //

inline void cache_t::printCacheConfiguration(){

	ORCS_PRINTF("[Cache Level: %s|Cache ID: %u| Cache Sets: %u| Cache Lines: %u] \n", this->level ,this->id,this->nSets,this->nLines)
	#if SLEEP
	usleep(500);
	#endif
};

// -------------------------------------------------------------------------- //

void cache_t::allocate(const char* level, uint32_t _nSets, uint32_t _nLines, uint64_t* _cycle ){

	this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
	this->cycle = _cycle;
	this->level = level;
	this->nSets = _nSets;
	this->nLines = _nLines;
	this->sets = new linha_t *[_nSets];

	for (size_t i = 0; i < _nSets; i++){
		this->sets[i] = new linha_t[_nLines];
		for ( uint j = 0; j < this->nLines; j++){
			this->sets[i][j].clean_line();
		}
		// std::memset(&this->sets[i][0],0,(L1_INST_ASSOCIATIVITY*sizeof(linha_t)));
	}

	this->set_cacheAccess(0);
	this->set_cacheHit(0);
	this->set_cacheMiss(0);
	this->set_cacheRead(0);
	this->set_cacheWrite(0);
	this->set_cacheWriteBack(0);
};

// -------------------------------------------------------------------------- //

void cache_t::setLatencies( uint32_t instLat, uint32_t readLat ){

	this->installLatency = instLat;
	this->readLatency = readLat;
}

// -------------------------------------------------------------------------- //

inline uint32_t cache_t::tagSetCalculation(uint64_t address){
	uint32_t tag = (address >> this->shiftData);

	return tag;
};

// -------------------------------------------------------------------------- //

inline uint32_t cache_t::idxSetCalculation(uint64_t address){
	uint32_t getBits = (this->nSets)-1;
	uint32_t tag = this->tagSetCalculation(address);
	uint32_t index = tag & getBits;

	return index;
};

// -------------------------------------------------------------------------- //

uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = this->tagSetCalculation(address);

	//this->add_cacheRead();
	for (size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx][i].tag == tag){

			if(this->sets[idx][i].readyAt <= this->getCycle()){	// Se ready Cycle for menor que o atual, a latencia é apenas da leitura, sendo um hit.
				#if PREFETCHER_ACTIVE
				if (this->sets[idx][i].prefetched == 1){
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					this->sets[idx][i].prefetched =0;
				}
				#endif

				this->sets[idx][i].lru = this->getCycle();
				ttc += this->readLatency;


				#if CACHE_MANAGER_DEBUG
				char* str = NULL;
				if(this->level == L1)
					str = "L1";
				if(this->level == LLC)
					str = "LLC";
				if(this->level == EMC_DATA_CACHE)
					str = "LLC"

				ORCS_PRINTF("%s Ready At %lu\n", str, this->sets[idx][i].readyAt)
				#endif

				return HIT;
			}

			// Se ready Cycle for maior que o atual, a latencia é dada pela demora a chegar
			else{
				#if PREFETCHER_ACTIVE
				if (this->sets[idx][i].prefetched == 1){
					orcs_engine.cacheManager->prefetcher->add_latePrefetches();
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate()+
					(this->sets[idx][i].readyAt - this->getCycle());
					orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
					this->sets[idx][i].prefetched =0;
				}
				#endif

				ttc += (this->sets[idx][i].readyAt - this->getCycle());
				this->sets[idx][i].lru = ttc;
				return HIT;
			}
		}
	}//end search, se nao encontrou nada, retorna latencia do miss

	ttc += this->readLatency;

	return MISS;
};

// -------------------------------------------------------------------------- //

uint32_t cache_t::write(uint64_t address){
	uint32_t tag = this->tagSetCalculation(address);
	uint32_t idx = this->idxSetCalculation(address);
	int32_t line = POSITION_FAIL;

	this->add_cacheWrite();
	//this->add_cacheAccess();

	for (size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx][i].tag == tag){
			//this->add_cacheHit();
			line = i;
			break;
		}
	}

	//acertar lru.
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, Linha nao encontrada para escrita")
	if(this->sets[idx][line].readyAt <= this->getCycle()){
		this->sets[idx][line].dirty = 1;
		this->sets[idx][line].lru = this->getCycle();
	}
	else{
		this->sets[idx][line].dirty = 1;
		this->sets[idx][line].lru = this->sets[idx][line].readyAt + L1_DATA_LATENCY;
	}

	return OK;
};

// -------------------------------------------------------------------------- //

linha_t* cache_t::installLine(uint64_t address){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = this->tagSetCalculation(address);

	for (size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx][i].valid == 0){
			this->sets[idx][i].tag = tag;
			this->sets[idx][i].lru = this->getCycle() + this->installLatency;
			this->sets[idx][i].valid = 1;
			this->sets[idx][i].dirty = 0;
			this->sets[idx][i].prefetched = 0;
			this->sets[idx][i].readyAt = this->getCycle() + this->installLatency;

			//ORCS_PRINTF("address %lu ready at %lu\n",address,this->sets[idx][i].readyAt)

			return &this->sets[idx][i];
			ORCS_PRINTF("passando do ponto\n")
		}
	//ORCS_PRINTF("not valid line\n")
	}

	uint32_t line = this->searchLru(&this->sets[idx]);
	this->add_changeLine();
	//ORCS_PRINTF("line after lru search %u\n",line)

	if(this->sets[idx][line].dirty == 1){
		this->writeBack(&this->sets[idx][line]);
		this->add_cacheWriteBack();
	}

	#if EMC_ACTIVE
		if(this->sets[idx][line].linha_ptr_emc != NULL){
			//invalida linha emc, mantendo a coerencia
			this->sets[idx][line].linha_ptr_emc->clean_line();
			this->sets[idx][line].linha_ptr_emc = NULL;
		}
	#endif

	this->sets[idx][line].tag = tag;
	this->sets[idx][line].lru = this->getCycle() + this->installLatency;
	this->sets[idx][line].valid = 1;
	this->sets[idx][line].dirty = 0;
	this->sets[idx][line].prefetched = 0;
	this->sets[idx][line].readyAt = this->getCycle() + this->installLatency;
	// ORCS_PRINTF("address %lu ready at %lu\n",address,this->sets[idx][line].readyAt)

	return &this->sets[idx][line];
};

// -------------------------------------------------------------------------- //

inline uint32_t cache_t::searchLru(linha_t **set){
	uint32_t index=0;
	uint32_t i=0;

	for (i = 1; i < this->nLines; i++){
		index = (set[0][index].lru <= set[0][i].lru)? index : i ;
	}

	return index;
};

// -------------------------------------------------------------------------- //

inline void cache_t::writeBack(linha_t *linha){

	if( !strcmp(this->level, "L1_data")){
		ERROR_ASSERT_PRINTF(linha->linha_ptr_sup!=NULL,"Erro, Linha sem referencia a nivel mais alto ")

		//Access pointer to copy status.
		linha->linha_ptr_sup->dirty = linha->dirty;										//DIRTY
		linha->linha_ptr_sup->lru = this->getCycle();									//LRU
		linha->linha_ptr_sup->readyAt = linha->readyAt;								//READY_AT

		// Nulling Pointers
		linha->linha_ptr_sup->linha_ptr_inf = NULL;										//Pointer to Lower Level

		// invalidando a linha recem feita WB.
		linha->clean_line();
	}
	else{
		if(linha->linha_ptr_inf !=NULL){
			linha->linha_ptr_inf->clean_line();													//invalidando linha Lower level
		}
		#if EMC_ACTIVE
		if(linha->linha_ptr_emc !=NULL){
			linha->linha_ptr_emc->clean_line();													//limpando linha de cache do emc, mantem coerencia
		}
		#endif
		linha->clean_line();
	}
};

// -------------------------------------------------------------------------- //

uint32_t cache_t::getCycle(){

	return( this->cycle ? *(this->cycle) : 0 );
}

// -------------------------------------------------------------------------- //

/*====================//
	move line to
	 @1 address - endereco do dado
	 @2 nivel de cache alvo da mudanca
	 @3 *retorno
//====================*/

void cache_t::returnLine(uint64_t address,cache_t *cache){
	uint32_t idx = this->idxSetCalculation(address);
	int32_t line = POSITION_FAIL;
	uint32_t tag = this->tagSetCalculation(address);

	// pega a linha desta cache
	for (size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx][i].tag == tag){
			this->sets[idx][i].lru = this->getCycle();;
			line = i;
			break;
		}
	}

	ERROR_ASSERT_PRINTF(line!=POSITION_FAIL,"Error, linha LLC não encontrada para retorno")
	linha_t *linha_l1 = NULL;
	linha_l1 = cache->installLine(address);
	this->sets[idx][line].linha_ptr_inf = linha_l1;
	linha_l1->linha_ptr_sup = &this->sets[idx][line];

	//copia dados da linha superior
	linha_l1->dirty = linha_l1->linha_ptr_sup->dirty;
	linha_l1->lru = linha_l1->linha_ptr_sup->lru;
	linha_l1->prefetched = linha_l1->linha_ptr_sup->prefetched;
	linha_l1->readyAt = this->getCycle();
};

// -------------------------------------------------------------------------- //

/*====================//
	move line to
	 @1 address - endereco do dado
	 @2 nivel de cache alvo da mudanca
	 @3 linha a ser movida
	 @return index da linha movida
//====================*/

uint32_t cache_t::moveLineTo(uint64_t address,cache_t *cache, linha_t *linha){
	//calcula endereco na nivel acima
	//cache representa nivel acima
	uint32_t idx = cache->idxSetCalculation(address);			//cache idx level up
	uint32_t tag = cache->tagSetCalculation(address);
	uint32_t line=0;

	for (size_t i = 0; i < cache->nLines; i++){						// busca se ja existe linha naquele nivel.

		if(cache->sets[idx][i].tag == tag){					// existindo linha, so copia do outro nivel de cache
			std::memcpy(&cache->sets[idx][i],linha,sizeof(linha_t));
			cache->sets[idx][i].lru = this->getCycle();
			return i;
		}
	}

	for (size_t i = 0; i < cache->nLines; i++){						//Busca se há alguma linha invalida

		if(cache->sets[idx][i].valid == 0){					// existindo linha, so copia do outro nivel de cache
			std::memcpy(&cache->sets[idx][i],linha,sizeof(linha_t));
			cache->sets[idx][i].lru = this->getCycle();
			return i;
		}
	}

	// não há invalido, e não tem livre, buscando lru
	line = cache->searchLru(&cache->sets[idx]);

	if(cache->sets[idx][line].dirty == 1){					// se sujo, faz WB
		cache->writeBack(&cache->sets[idx][line]);
		cache->add_cacheWriteBack();
	}
	std::memcpy(&cache->sets[idx][line],linha,sizeof(linha_t));
	cache->sets[idx][line].tag = tag;
	cache->sets[idx][line].lru = this->getCycle();

	return line;
	//this->printLine(&cache->sets[idx][line]);
};

// -------------------------------------------------------------------------- //

// ==========================================
// @address endereco para realizar o shotdown da linha de cache no level 1 quando coerente
// ==========================================

void cache_t::shotdown(uint64_t address){
	uint32_t tag = this->tagSetCalculation(address);
	uint32_t idx = this->idxSetCalculation(address);

	for(size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx][i].tag == tag){
			#if DEBUG
			printf("shoting down Line %u -> %lu\n ",idx,i);
			this->printLine(&this->sets[idx][i]);
			#endif
			this->sets[idx][i].valid = 0;
			break;
		}
	}
};

// -------------------------------------------------------------------------- //

void cache_t::statistics(){
	FILE *output = stdout;

	if(orcs_engine.output_file_name != NULL)
		output = fopen(orcs_engine.output_file_name,"a+");

	if(output != NULL){
		utils_t::largeSeparator(output);
		fprintf(output,"Cache_Level: %s\n", this->level);
		fprintf(output,"%s_Cache_Access: %lu\n",this->level,this->get_cacheAccess());
		fprintf(output,"%s_Cache_Hits: %lu %.4f\n", this->level,this->get_cacheHit(),float((this->get_cacheHit()*100.0)/this->get_cacheAccess()));
		fprintf(output,"%s_Cache_Miss: %lu %.4f\n", this->level,this->get_cacheMiss(),float((this->get_cacheMiss()*100.0)/this->get_cacheAccess()));
		fprintf(output,"%s_Cache_Read: %lu %.4f\n", this->level,this->get_cacheRead(),float((this->get_cacheRead()*100.0)/this->get_cacheAccess()));
		fprintf(output,"%s_Cache_Write: %lu %.4f\n", this->level,this->get_cacheWrite(),float((this->get_cacheWrite()*100.0)/this->get_cacheAccess()));
		if(this->get_cacheWriteBack() != 0){
			fprintf(output,"%s_Cache_WriteBack: %lu %.4f\n", this->level,this->get_cacheWriteBack(),float((this->get_cacheWriteBack()*100.0)/this->get_changeLine()));
		}
		utils_t::largeSeparator(output);
	}

	if( output != NULL && output != stdout )
		fclose(output);
}

// ========================================================================== //
