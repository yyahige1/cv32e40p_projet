
**tb_riscv**

This folder contains a Verilog testbench for the riscv processor.

**cache.sv**

This file contains the Verilog code for the cache. The cache is a direct-mapped cache implementing a 
FIFO replacement policy. The coherence policy is not yet implemented.

**cv32e40p_tb_wrapper.sv**

This file is a wrapper for the testbench. Contains the interface between the cache, memory and the core

**cv32e40p_tb_wrapper.sv.bck**

This file is a backup of cv32e40p_tb_wrapper.sv. 

**dp_ram.sv**

This file contains the Verilog code for a dual port RAM.

**mm ram.sV**

This file contains the Verilog code for a memory mapped RAM.

**tb_top.sv**

This file is the top level of the testbench.

**tb_top_verilator.cpp**

This file is a C++ file that is used to drive the Verilator testbench.

**tb_top_verilator.sv**

This file is a Verilog file that is used to interface the C++ testbench with the Verilator testbench.
