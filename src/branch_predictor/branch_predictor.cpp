#include "simulator.hpp"
#include <string.h>

// ========================================================================== //

branch_predictor_t::branch_predictor_t(){

	this->btb = NULL;
	this->branchPredictor = NULL;
};

// -------------------------------------------------------------------------- //

branch_predictor_t::~branch_predictor_t(){

	if(this->branchPredictor != NULL){
		delete this->branchPredictor;
	}

	delete[] this->btb;

	//Setting pointers to null
	this->btb = NULL;
	this->branchPredictor = NULL;
};

// -------------------------------------------------------------------------- //

void branch_predictor_t::allocate(){
	uint32_t size	= BTB_ENTRIES/BTB_WAYS;

	this->btb = new btb_line_t*[size];
	this->index = 0;
	this->way = 0;
	for (size_t i = 0; i < size; i++){
		this->btb[i] = new btb_line_t[BTB_WAYS];
		std::memset(&this->btb[i][0],0,(BTB_WAYS*sizeof(btb_line_t)));
	}

	//allocate branch predictor
	#if TWO_BIT
	this->branchPredictor = new twoBit_t();
	this->branchPredictor->allocate()
	#else
	this->branchPredictor = new piecewise_t();
	this->branchPredictor->allocate();
	#endif
};

// -------------------------------------------------------------------------- //

uint32_t branch_predictor_t::searchLine(uint64_t pc){
	uint32_t getBits = ( BTB_ENTRIES / BTB_WAYS );
	uint32_t tag = (pc >> 2);
	uint32_t index = tag&(getBits-1);

	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (uint32_t i = 0; i < BTB_WAYS; i++){
		//std::cout<< "%u\n",this->btb[index][i].tag);
		if(this->btb[index][i].tag == pc){
			//std::cout<< "BTB_Hit");
			this->btb[index][i].lru=orcs_engine.get_global_cycle();
			//save locate from line
			this->index = index;
			this->way = i;
			return HIT;
		}
	}
	//std::cout<< "BTB_Miss");
	return MISS;
}

// -------------------------------------------------------------------------- //

uint32_t branch_predictor_t::installLine(opcode_package_t instruction){
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (instruction.opcode_address >> 2);
	uint32_t index = tag&(getBits-1);

	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (uint32_t i = 0; i < BTB_WAYS; i++){
		// instala no primeiro invalido
		if(this->btb[index][i].validade == 0){
			this->btb[index][i].tag = instruction.opcode_address;
			this->btb[index][i].lru = orcs_engine.get_global_cycle();
			this->btb[index][i].targetAddress = instruction.opcode_address+instruction.opcode_size;
			this->btb[index][i].validade = 1;
			this->btb[index][i].typeBranch = instruction.branch_type;
			this->btb[index][i].bht = 0;
			this->index = index;
			this->way = i;
			return OK;
		}
	}

	uint32_t way = this->searchLRU(&this->btb[index]);
	this->btb[index][way].tag = instruction.opcode_address;
	this->btb[index][way].lru = orcs_engine.get_global_cycle();
	this->btb[index][way].targetAddress = instruction.opcode_address+instruction.opcode_size;
	this->btb[index][way].validade = 1;
	this->btb[index][way].typeBranch = instruction.branch_type;
	this->btb[index][way].bht = 0;
	//indexes
	this->index = index;
	this->way = way;

	return OK;
};

// -------------------------------------------------------------------------- //

inline uint32_t branch_predictor_t::searchLRU(btb_line_t **btb){
	uint32_t index=0;

	for (uint32_t i = 1; i < BTB_WAYS; i++){
		index = (btb[0][index].lru <= btb[0][i].lru)? index : i ;
	}

	return index;
};

// -------------------------------------------------------------------------- //

void branch_predictor_t::statistics(){
	FILE *output = stdout;

	if(orcs_engine.output_file_name != NULL)
		output = fopen(orcs_engine.output_file_name,"a+");

	if(output != NULL){
		utils_t::largestSeparator(output);
		fprintf(output,"BTB Hits: %u -> %.2f\n",this->btbHits,(this->btbHits*100.0)/(float)(this->btbHits+this->btbMiss));
		fprintf(output,"BTB Miss: %u -> %.2f\n\n",this->btbMiss,(this->btbMiss*100.0)/(float)(this->btbHits+this->btbMiss));
		fprintf(output,"Total Branchs: %u\n",this->branches);
		fprintf(output,"Total Branchs Taken: %u -> %.2f \n",this->branchTaken,((this->branchTaken*100.0)/this->branches));
		fprintf(output,"Total Branchs Not Taken: %u -> %.2f\n",this->branchNotTaken,((this->branchNotTaken*100.0)/this->branches));
		fprintf(output,"Correct Branchs Taken: %u -> %.2f\n",(this->branchTaken-this->branchTakenMiss),((this->branchTaken-this->branchTakenMiss)*100.0)/this->branchTaken);
		fprintf(output,"Incorrect Branchs Taken: %u -> %.2f\n",this->branchTakenMiss,((this->branchTakenMiss*100.0)/this->branchTaken));
		fprintf(output,"Correct Branchs Not Taken: %u -> %.2f\n",(this->branchNotTaken-this->branchNotTakenMiss),((this->branchNotTaken-this->branchNotTakenMiss)*100.0)/this->branchNotTaken);
		fprintf(output,"Incorrect Branchs Not Taken: %u -> %.2f\n",this->branchNotTakenMiss,((this->branchNotTakenMiss*100.0)/this->branchNotTaken));
		utils_t::largestSeparator(output);
	}

	if( output != NULL && output != stdout )
		fclose(output);
};

// -------------------------------------------------------------------------- //

uint32_t branch_predictor_t::solveBranch(opcode_package_t branchInstrucion, opcode_package_t nextInstruction){
	uint64_t stallCyles=0;
	uint32_t btbStatus = this->searchLine(branchInstrucion.opcode_address);
	if(btbStatus == HIT){
		this->btbHits++;
	}
	else{
		this->btbMiss++;
		this->installLine(branchInstrucion);
		stallCyles+=BTB_MISS_PENALITY;
	}

	taken_t branchStatus = this->branchPredictor->predict(branchInstrucion.opcode_address);
	//printf("this->index %u, this->way %hhu\n",this->index,this->way);
	//sleep(1);
	if((nextInstruction.opcode_address != this->btb[this->index][this->way].targetAddress)&&
		(this->btb[this->index][this->way].typeBranch == BRANCH_COND)){

		this->branchTaken++;
		if(branchStatus == TAKEN){
			this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,TAKEN);
		}
		else{
			this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,TAKEN);
			this->branchTakenMiss++;
			stallCyles+=MISSPREDICTION_PENALITY;
		}
	}
	else{
		this->branchNotTaken++;
		if(branchStatus == NOT_TAKEN){
			this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,NOT_TAKEN);
		}
		else{
			this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,NOT_TAKEN);
			this->branchNotTakenMiss++;
			stallCyles+=MISSPREDICTION_PENALITY;
		}
	}

	return stallCyles;
};

// ========================================================================== //
