#! /bin/sh
#
# Copyright (c) International Business Machines  Corp., 2001
# Author: Nageswara R Sastry <nasastry@in.ibm.com>
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

export TCID="Power_Management02"
export TST_TOTAL=1

. test.sh
. pm_include.sh

# Checking test environment
check_kervel_arch

# Check sched_smt_power_savings interface on HT machines
hyper_threaded=$(is_hyper_threaded)
if [ ! -f /sys/devices/system/cpu/sched_smt_power_savings ] ; then
	tst_brkm TCONF "Required kernel configuration for SCHED_SMT NOT set"
else
	if [ $hyper_threaded -ne 0 ]; then
		tst_brkm TCONF "sched_smt_power_saving interface in system" \
			"not hyper-threaded"
	fi
fi

if test_sched_smt.sh ; then
	tst_resm TPASS "SCHED_SMT sysfs test"
else
	tst_resm TFAIL "SCHED_SMT sysfs test"
fi

tst_exit