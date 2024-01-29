################################################################################
#
# Copyright (C) EM Microelectronic US Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied.
#
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier:Apache-2.0 WITH SHL-2.0
# 
################################################################################
# - Addresses coverage hole in which load and store instructions are not
#   generated for which operand rs1=ZERO.
# - THIS TEST IS SELF CHECKING but should be run against a
#   reference model to check results.
# - This test can be abandoned and corev_rand_instr_test should produce
#   this combination when https://github.com/google/riscv-dv/issues/752
#   is addressed.
################################################################################
.include "user_define.h"
.section .text.start
.globl _start
.section .text
.type _start, @function

_start:
    j _start_main

.globl _start_main
.section .text
_start_main:

/**

 * This code performs simple load and store instructions to test the functionnality of secure operations
 */
       
    li a3, 0x12341234
    sw a3, 1(zero)
    lw a3, 1(zero)



    
test_pass:

    li x18, 123456789	     	
    li x17, 0x20000000
    sw x18,0(x17)
    wfi


