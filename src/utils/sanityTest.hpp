#ifndef SANITY_TEST_H
#define SANITY_TEST_H

// ========================================================================== //

class sanity_test_t{
	private:
		uint64_t stallFetch;
		uint64_t stallBTB;
		uint64_t stallBP;
		uint64_t stallCache;
			
	public:
		sanity_test_t();
		~sanity_test_t();
		INSTANTIATE_GET_SET(uint64_t,stallFetch);
		INSTANTIATE_GET_SET(uint64_t,stallBTB);
		INSTANTIATE_GET_SET(uint64_t,stallBP);
		INSTANTIATE_GET_SET(uint64_t,stallCache);
		void statistics();
		void allocate();

		void calculateStallBranchPredictor();					// Method calculate stall Branch Predictor
		void check();
};

// ========================================================================== //

#endif
