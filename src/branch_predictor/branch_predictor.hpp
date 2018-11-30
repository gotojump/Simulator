#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

// ========================================================================== //

class branch_predictor_t{
	public:
		btb_line_t **btb;
		uint32_t btbHits;
		uint32_t btbMiss;
		uint32_t index;
		uint8_t way;

		#if TWO_BIT
		twoBit_t *branchPredictor;
		#else
		piecewise_t *branchPredictor;
		#endif

	public:
		inline uint32_t searchLRU(btb_line_t **btb);
		uint32_t installLine(opcode_package_t instruction);
		uint32_t searchLine(uint64_t pc);
		uint32_t branchTaken;
		uint32_t branchNotTaken;
		uint32_t branches;
		uint32_t branchTakenMiss;
		uint32_t branchNotTakenMiss;

		branch_predictor_t();
		~branch_predictor_t();
		void allocate();
		uint32_t solveBranch(opcode_package_t instruction, opcode_package_t nextOpcode);
		void statistics();
};

// ========================================================================== //

#endif
