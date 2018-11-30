#ifndef CACHE_H
#define CACHE_H

// ========================================================================== //

class cache_t{
	private:
		uint64_t cacheHit;
		uint64_t cacheMiss;
		uint64_t cacheAccess;
		uint64_t cacheRead;
		uint64_t cacheWrite;
		uint64_t cacheWriteBack;
		uint64_t changeLine;
	public:
		uint32_t id;
		const char* level;
		uint32_t nSets;
		uint32_t nLines;
		uint32_t installLatency;
		uint32_t readLatency;
		uint64_t *cycle;
		linha_t **sets;
		uint32_t shiftData;

	public:
		cache_t();
		~cache_t();
		inline void printLine(linha_t *linha);
		inline void printCacheConfiguration();

		void statistics();
		void allocate(const char* level, uint32_t _nSets,  uint32_t _nLines, uint64_t* _cycle);
		void setLatencies( uint32_t instLat, uint32_t readLat );
		void shotdown(uint64_t address);									//shotdown line inclusive cache
		void writeBack(linha_t *line);										//makes writeback of line
		void returnLine(uint64_t address,cache_t *cache);	//return line from lower cache level

		uint32_t getCycle();															//return the global cycle
		uint32_t idxSetCalculation(uint64_t address);			//calculate index of data
		uint32_t tagSetCalculation(uint64_t address);			//makes tag from address
		uint32_t searchLru(linha_t **set);								//searh LRU to substitue
		linha_t* installLine(uint64_t address);						//install line of cache |mem_controller -> caches|
		uint32_t moveLineTo(uint64_t address,cache_t *cache,linha_t *linha);	// move line to a upper or lower cache level
		uint32_t read(uint64_t address,uint32_t &ttc);
		uint32_t write(uint64_t address);

		INSTANTIATE_GET_SET_ADD(uint64_t,cacheHit);
		INSTANTIATE_GET_SET_ADD(uint64_t,cacheMiss);
		INSTANTIATE_GET_SET_ADD(uint64_t,cacheAccess);
		INSTANTIATE_GET_SET_ADD(uint64_t,cacheRead);
		INSTANTIATE_GET_SET_ADD(uint64_t,cacheWrite);
		INSTANTIATE_GET_SET_ADD(uint64_t,cacheWriteBack);
		INSTANTIATE_GET_SET_ADD(uint64_t,changeLine);
};

// ========================================================================== //

#endif
