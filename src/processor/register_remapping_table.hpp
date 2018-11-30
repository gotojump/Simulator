#ifndef REGISTER_REMAPPING_TABLE_H
#define REGISTER_REMAPPING_TABLE_H

// ========================================================================== //

class register_remapping_table_t{
	public:
		int32_t register_core;
		emc_opcode_package_t *entry;

	public:
		register_remapping_table_t();
		~register_remapping_table_t();
		void package_clean();
		void print_rrt_entry();
};

// ========================================================================== //

#endif
