#include "cuckoo_reversible_move_table.hpp"

Cuckoo_RM_Table_Storage<CUCKOO_RM_TABLE_SIZE> Cuckoo_RM_Table::m_storage =
    initialize_cuckoo_rm_storage<CUCKOO_RM_TABLE_SIZE>();
