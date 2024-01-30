..
   Copyright (c) 2023 OpenHW Group
   
   Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
   https://solderpad.org/licenses/
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
  
   SPDX-License-Identifier: Apache-2.0 WITH SHL-2.0

.. _hwloop-specs:

CORE-V Hardware Loop feature
============================

To increase the efficiency of small loops, CV32E40P supports hardware
loops (HWLoop). They can be enabled by setting the ``COREV_PULP`` parameter.
Hardware loops make executing a piece of code
multiple times possible, without the overhead of branches penalty or updating a counter.
Hardware loops involve zero stall cycles for jumping to the first
instruction of a loop.

A hardware loop is defined by its start address (pointing to the first
instruction in the loop), its end address (pointing to the instruction
just after the last one executed by the loop) and a counter that is
decremented every time the last instruction of the loop body is executed.

CV32E40P contains two hardware loop register sets to support nested hardware loops,
each of them can store these three values in separate flip flops which are
mapped in the CSR address space.
Loop number 0 has higher priority than loop number 1 in a nested loop
configuration, meaning that loop 0 represents the inner loop and loop 1 is the outer loop.

Hardware Loop constraints
^^^^^^^^^^^^^^^^^^^^^^^^^

Following constraints must be respected by any toolchain compiler or by hand-written assembly code.
``Violation of these constraints will not generate any hardware exception`` and behaviour is undefined.

In order to catch **as early as possible** those software exceptions when executing a program either
on a verification Reference Model or on a virtual platform Instruction Set Simulator, ``those model/simulation platforms
should generate an error`` with a meaningfull message related to Hardware Loops constraints violation.
Those constraint checks could be done only for each instruction in the hardware loop body, meaning when (lpstartX <= PC <= lpendX - 4) and (lpcountX > 0).

The HWLoop constraints are:

-  HWLoop start, end and setup instructions addresses must be 32-bit aligned (short or long commands).

-  Start and End addresses of an HWLoop body must be 32-bit aligned.

-  End Address must be strictly greater than Start Address.

-  End address of an HWLoop must point to the instruction just after the last one of the HWLoop body.

-  HWLoop body must contain at least 3 instructions.

-  When both loops are nested, the End address of the outermost HWLoop (must be #1) must be at least 2
   instructions further than the End address of the innermost HWLoop (must be #0),
   i.e. HWLoop[1].endaddress >= HWLoop[0].endaddress + 8.
   Remark: To avoid to add 2 NOPs in case nothing can be put there by the compiler, lpcount setting of the the inner loop could be moved after it
   without forgetting to add the same in the preamble before the outer loop start address.

-  HWLoop must always be entered from its start location (no branch/jump to a location inside a HWLoop body).

-  No HWLoop #0 (resp. #1) CSR should be modified inside the HWLoop #0 (resp. #1) body.

-  No Compressed instructions (RVC) allowed in the HWLoop body.

-  No jump or branch instructions allowed in the HWLoop body.

-  No memory ordering instructions (fence, fence.i) allowed in the HWLoop body.

-  No privileged instructions (mret, dret, wfi) allowed in the HWLoop body, except for ebreak and ecall.

The rationale of NOT generating any hardware exception when violating any of those constraints is that it would add resources
(32-bit adders and substractors needed for the third and fourth rules) which are costly in area and power consumption.
These additional (and costly) resources would be present just to catch situations that should never happen. 
This in an architectural choice in order to keep CV32E40P area and power consumption to its lowest level.

The rationale of putting the end-of-loop label to the first instruction after the last one of the loop body
is that it greatly simplifies compiler optimization (relative to basic blocks management).

In order to use hardware loops, the compiler needs to setup the loops beforehand with cv.start/i, cv.end/i, cv.count/i or cv.setup/i instructions.
The compiler will use HWLoop automatically whenever possible without the need of assembly.

For debugging, interrupts and context switches, the hardware loop registers are mapped into the CSR custom read-only address space.
To read them csrr instructions should be used and to write them register flavour of hardware loop instructions should be used.
Using csrw instructions to write hardware loop registers will generate an illegal instruction exception.
The CSR HWLoop registers are described in the :ref:`cs-registers` section.

Below an assembly code example of a nested HWLoop that computes a matrix addition.

.. code-block:: c
   :linenos:

   asm volatile (
       "add %[i],x0, x0;"
       "add %[j],x0, x0;"
       "cv.count  1, %[N];"
       ".balign 4;"
       "cv.endi   1, endO;"
       "cv.starti 1, startO;"
       "any instructions here"
       ".balign 4;"
       "cv.endi   0, endZ;"
       "cv.starti 0, startZ;"
       "cv.count 0, %[N];"
       "any instructions here"
       ".balign 4;"
       ".option norvc;"
       "startO:;"
       "    startZ:;"
       "        addi %[i], %[i], 1;"
       "        addi %[i], %[i], 1;"
       "        addi %[i], %[i], 1;"
       "    endZ:;"
       "    cv.count 0, %[N];"
       "    addi %[j], %[j], 2;"
       "endO:;"
       : [i] "+r" (i), [j] "+r" (j)
       : [N] "r" (10)
   );


At the beginning of the HWLoop, the registers %[i] and %[j] are 0.
The innermost loop, from startZ to (endZ - 4), adds to %[i] three times 1 and
it is executed 10x10 times. Whereas the outermost loop, from startO to (endO - 4),
executes 10 times the innermost loop and adds 2 to the register %[j].
At the end of the loop, the register %[i] contains 300 and the register %[j] contains 20.

.. _hwloop-exceptions_handlers:

Hardware loops impact on application, exceptions handlers and debugger
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Application and ebreak/ecall exception handlers
-----------------------------------------------

When an ebreak or an ecall instruction is used in an application, special care should be given for those instruction handlers in case they are placed as the last instruction of an HWLoop.
Those handlers should manage MEPC and lpcountX CSRs updates because an hw loop early-exit could happen if not done.

At the end of the handlers after restoring the context/CSRs, a piece of smart code should be added with following highest to lowest order of priority:

1. if MEPC = lpend0 - 4 and lpcount0 > 1 then MPEC should be set to lpstart0 and lpcount0 should be decremented by 1,
2. else if MEPC = lpend0 - 4 and lpcount0 = 1 then MPEC should be incremented by 4 and lpcount0 should be decremented by 1,
3. else if MEPC = lpend1 - 4 and lpcount1 > 1 then MPEC should be set to lpstart1 and lpcount1 should be decremented by 1,
4. else if MEPC = lpend1 - 4 and lpcount1 = 1 then MPEC should be incremented by 4 and lpcount1 should be decremented by 1,
5. else if (lpstart0 <= MEPC < lpend0 - 4) or (lpstart1 <= MEPC < lpend1 - 4) then MPEC should be incremented by 4,
6. else if instruction at MEPC location is either ecall or ebreak then MPEC should be incremented by 4,
7. else if instruction at MEPC location location is c.ebreak then MPEC should be incremented by 2.

The 2 last cases are the standard ones when ebreak/ecall are not inside an HWLopp.

Interrupt handlers
------------------

When an interrupt is happening on the last HWLoop instruction, its execution is cancelled, its address is saved in MEPC and its execution will be resumed when returning from interrupt handler.
There is nothing special to be done in those interrupt handlers with respect to MEPC and lpcountX updates, they will be correctly managed by design when executing this last HWLoop instruction after interrupt handler execution.

Moreover since hardware loop could be used in interrupt routine, the registers have to be saved (resp. restored) at the beginning (resp. end) of the interrupt routine together with the general purpose registers.

Illegal instruction exception handler
-------------------------------------

Depending if an application is going to resume or not after Illegal instruction exception handler, same MEPC/HWLoops CSRs management than ebreak/ecall could be necessary.

Debugger
--------

If ebreak is used to enter in Debug Mode (:ref:`ebreak_scenario_2`) and put at the last instruction location of an HWLoop (not very likely to happen), same management than above should be done but on DPC rather than on MEPC.

When ebreak instruction is used as Software Breakpoint by a debugger when in debug mode and is placed at the last instruction location of an HWLoop in instruction memory, no special management is foreseen.
When executing the Software Breakpoint/ebreak instruction, control is given back to the debugger which will manage the different cases.
For instance in Single-Step case, original instruction is put back in instruction memory, a Single-Step command is executed on this last instruction (with desgin updating PC and lpcountX to correct values) and Software Breakpoint/ebreak is put back by the debugger in memory.
 
When ecall instruction is used by a debugger to execute System Calls and is placed at the last instruction location of an HWLoop in instruction memory, debugger ecall handler in debug rom should do the same than described above for application case.

