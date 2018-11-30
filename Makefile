CC = gcc
CPP = g++
LD = g++
CFLAGS = $(FLAGS)
CPPFLAGS = $(FLAGS)
BIN_NAME = orcs
RM = rm -f

FLAGS = -ggdb3 -Wall -Wextra -Werror -std=c++11 -lefence -O3
LDFLAGS = -ggdb3

LIBRARY = -lz -lconfig++

# ============================================================================ #

# FOLDERS
OBJ = obj
SRC = src
INCLUDE = -I$(SRC) -I.

FD_PACKAGE =					$(SRC)/package
FD_PROCESSOR =				$(SRC)/processor
FD_BRANCH_PREDICTOR =	$(SRC)/branch_predictor
FD_OTHER =						$(SRC)/utils
FD_ENUMS =						$(SRC)/enums
FD_CACHE =						$(SRC)/cache
FD_PREFETCHER =				$(SRC)/prefetcher
FD_MEMORY =						$(SRC)/main_memory
FD_EMC =							emc

# ============================================================================ #

all: orcs

# ============================================================================ #

package: $(OBJ)/opcode_package.o $(OBJ)/uop_package.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/opcode_package.o: $(FD_PACKAGE)/opcode_package.cpp $(FD_PACKAGE)/opcode_package.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/uop_package.o: $(FD_PACKAGE)/uop_package.cpp $(FD_PACKAGE)/uop_package.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

trace_reader: $(OBJ)/trace_reader.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/trace_reader.o: $(SRC)/trace_reader.cpp $(SRC)/trace_reader.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

processor: $(OBJ)/processor.o $(OBJ)/reorder_buffer_line.o\
 						$(OBJ)/memory_order_buffer_line.o $(OBJ)/register_remapping_table.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/processor.o: $(FD_PROCESSOR)/processor.cpp $(FD_PROCESSOR)/processor.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/reorder_buffer_line.o: $(FD_PROCESSOR)/reorder_buffer_line.cpp $(FD_PROCESSOR)/reorder_buffer_line.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/memory_order_buffer_line.o: $(FD_PROCESSOR)/memory_order_buffer_line.cpp $(FD_PROCESSOR)/memory_order_buffer_line.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/register_remapping_table.o: $(FD_PROCESSOR)/register_remapping_table.cpp $(FD_PROCESSOR)/register_remapping_table.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

branch_predictor: $(OBJ)/branch_predictor.o $(OBJ)/piecewise.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/branch_predictor.o: $(FD_BRANCH_PREDICTOR)/branch_predictor.cpp $(FD_BRANCH_PREDICTOR)/branch_predictor.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/piecewise.o: $(FD_BRANCH_PREDICTOR)/piecewise.cpp $(FD_BRANCH_PREDICTOR)/piecewise.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

cache: $(OBJ)/cache.o $(OBJ)/cache_manager.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/cache.o: $(FD_CACHE)/cache.cpp $(FD_CACHE)/cache.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/cache_manager.o: $(FD_CACHE)/cache_manager.cpp $(FD_CACHE)/cache_manager.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

enums: $(OBJ)/instruction_operation.o $(OBJ)/memory_operation.o\
 				$(OBJ)/package_state.o $(OBJ)/processor_stage.o $(OBJ)/status_stride_prefetcher.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/instruction_operation.o: $(FD_ENUMS)/instruction_operation.cpp $(FD_ENUMS)/instruction_operation.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/memory_operation.o: $(FD_ENUMS)/memory_operation.cpp $(FD_ENUMS)/memory_operation.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/package_state.o: $(FD_ENUMS)/package_state.cpp $(FD_ENUMS)/package_state.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/processor_stage.o: $(FD_ENUMS)/processor_stage.cpp $(FD_ENUMS)/processor_stage.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/status_stride_prefetcher.o: $(FD_ENUMS)/status_stride_prefetcher.cpp $(FD_ENUMS)/status_stride_prefetcher.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

other: $(OBJ)/utils.o $(OBJ)/sanityTest.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/utils.o: $(FD_OTHER)/utils.cpp $(FD_OTHER)/utils.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/sanityTest.o: $(FD_OTHER)/sanityTest.cpp $(FD_OTHER)/sanityTest.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

prefetcher:  $(OBJ)/prefetcher.o $(OBJ)/stride_prefetcher.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/prefetcher.o: $(FD_PREFETCHER)/prefetcher.cpp $(FD_PREFETCHER)/prefetcher.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/stride_prefetcher.o: $(FD_PREFETCHER)/stride_prefetcher.cpp $(FD_PREFETCHER)/stride_prefetcher.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

memory: $(OBJ)/memory_controller.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/memory_controller.o: $(FD_MEMORY)/memory_controller.cpp $(FD_MEMORY)/memory_controller.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

emc: $(OBJ)/emc.o  $(OBJ)/emc_opcode_package.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/emc.o: $(FD_MEMORY)/$(FD_EMC)/emc.cpp $(FD_MEMORY)/$(FD_EMC)/emc.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/emc_opcode_package.o: $(FD_MEMORY)/$(FD_EMC)/emc_opcode_package.cpp $(FD_MEMORY)/$(FD_EMC)/emc_opcode_package.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

sim: $(OBJ)/simulator.o $(OBJ)/orcs_engine.o

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  #

$(OBJ)/simulator.o: simulator.cpp simulator.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

$(OBJ)/orcs_engine.o: $(SRC)/orcs_engine.cpp $(SRC)/orcs_engine.hpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) $< -o $@

# ---------------------------------------------------------------------------- #

objs: sim trace_reader package processor other branch_predictor\
			enums cache prefetcher memory emc

# ============================================================================ #

exec:
	time ./orcs -t src/execs/traces/spec_cpu2006/calculix.CFP.PP200M/calculix.CFP.PP200M

diff_exec:
	time ./orcs -t src/execs/traces/spec_cpu2006/calculix.CFP.PP200M/calculix.CFP.PP200M > exec.out
	diff exec.out result.out

orcs: objs
	$(LD) $(LDFLAGS) -o $(BIN_NAME) $(INCLUDE) $(OBJ)/* $(LIBRARY)

clean:
	rm -rf $(OBJ)/*.o $(BIN_NAME)
	@echo OrCS cleaned!
	@echo

# ============================================================================ #
