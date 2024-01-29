#!/bin/bash

###############################################################################
#
# Copyright 2020 OpenHW Group
#
# Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://solderpad.org/licenses/
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.0
#
###############################################################################
# diff_signatures: script adopted from the riscv-compliance testsuite to
#                  compare (diff) signature dumps.  Will either compare the
#                  reference and signature from a single test-program, or all
#                  refs and sigs found in a regression.
#
# Usage:
#       diff_signatures ref sig  : compare reference and signature
#       diff_signatures          : (no arguments) iterates over all references
#                                  for a specific ISA (as defined by RISCV_ISA)
#                                  and compares to signatures.
#
# ENV: this script needs the following shell environment variables -
#       SUITEDIR     : path to the compliance test-suite
#       SIG_ROOT     : path to the signature files
#       REF          : path to reference file (part of the compliance test-suite)
#       SIG          : path to signature file (generated by running a sim)
#       COMPL_PROG   : name of an individual compliance program, e.g. I-ADD-01
#       RISCV_TARGET : OpenHW
#       RISCV_DEVICE : cv32e40p
#       RISCV_ISA    : one of rv32i|rv32im|rv32imc|rv32Zicsr|rv32Zifencei
###############################################################################

IGNORE=0
ERROR=0
MISCOMPARE=0
RUN=0
OK=0

declare -i status=0

diff_files() {
    # Ensure both files exist
    if [ ! -f $1 ]; then
        ref_base=$(basename $1)
        ref_stub=${sig_base//".reference_output"/}
        echo "Reference file for ${ref_base} does not exist ... ERROR!"
        ERROR=$((${ERROR} + 1))
    else
        if [ ! -f $2 ]; then
            sig_base=$(basename $2)
            sig_stub=${sig_base//".reference_output"/}
            echo "Signature file for ${sig_stub} does not exist ... IGNORE"
            IGNORE=$((${IGNORE} + 1))
        else
            sig_base=$(basename $2)
            sig_stub=${sig_base//".reference_output"/}
            echo -n "Check $(printf %24s ${sig_stub})"
            RUN=$((${RUN} + 1))

            diff --ignore-case --strip-trailing-cr $1 $2 #&> /dev/null

            if [ $? == 0 ]; then
                echo " ... OK"
                OK=$((${OK} + 1))
            else
                echo " ... MISCOMPARE!"
                MISCOMPARE=$((${MISCOMPARE} + 1))
            fi
        fi
    fi
}

print_summary() {
    if [ ${MISCOMPARE} == 0 ] && [ ${ERROR} == 0 ] && [ ${IGNORE} == 0 ]; then
        echo "--------------------------------"
        echo -n "OK: ${RUN}/${RUN} "
        status=0
    else
        echo "--------------------------------"
        echo "Summary for RISCV_TARGET=${RISCV_TARGET}; RISCV_DEVICE=${RISCV_DEVICE}; RISCV_ISA=${RISCV_ISA}"
        echo "  MISCOMPARE: ${MISCOMPARE}/${RUN} "
        echo "  IGNORE:     ${IGNORE}/${RUN} "
        echo "  ERROR:      ${ERROR}/${RUN} "
        echo "  OK:         ${OK}/${RUN} "

        if [ ${MISCOMPARE} != 0 ] || [ ${ERROR} != 0 ]; then
            status=1
        fi
    fi
    echo
}

# Script starts here
if [[ $1 = "" ]]; then
    diff_files ${REF} ${SIG}

    print_summary
else
    for ref in ${SUITEDIR}/references/*.reference_output;
    do
        #echo "ref = ${ref}"
        base=$(basename ${ref})
        stub=${base//".reference_output"/}
        sig=${SIG_ROOT}/${stub}/${RUN_INDEX}/${stub}.signature_output

        diff_files ${ref} ${sig}
    done

    print_summary
fi

if [[ $status == "0" ]]; then
    echo "All signatures passed"
fi
exit ${status}
