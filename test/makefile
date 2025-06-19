.PHONY: test minitest srtest minisrtest reptest graphtest
 
# Slower ones first for optimial multi-core usage (whole suite time)
SC_CASES = sctest13 sctest03 sctest11 sctest12 sctest08 sctest10 sctest02 
SC_CASES += sctest01 sctest04 sctest05 sctest06 sctest07 sctest09 sctest15
SC_CASES += issue677 issue667

# Quicker ones first for exploratory (max feedback ASAP)
#SC_CASES =  sctest09 sctest15 sctest06 sctest07 
#SC_CASES += sctest04 sctest05 sctest01 sctest03
#SC_CASES += sctest10 sctest02 sctest12 sctest08 sctest13 sctest11

# Use pointeris (very quick to test) as "canary" even more so "small"
SR_CASES = sc10_reduce ${SC_CASES} 
MINI_SC_CASES = sctest02 sctest03 sctest01 sctest07 sctest09 sctest06 sctest05 sctest04 sctest15
MINI_SC_SR_CASES = small pointeris ${MINI_SC_CASES} 

# 791 to fix pointeris
# #788 to fix small pointeris

# The original C/C++ test cases, no SystemC
CPP_CASES = test11 test13 test16
# #757 to fix test04
# #785 to fix test02 test15
# #786 to fix test03
# #787 to fix test12
# #788 to fix test05 test06 test07 test09 test10
# #789 to fit test14
# #790 to fix test07.cpp (again)
 
# The whole lot for the benefit of auto rules
ALL_CASES = ${SC_CASES} ${CPP_CASES} ${SR_CASES}

# Arguments to inferno executable:
# Args supplied via eg I=-uc will be squished together and appended to the
# results directory. This will change the make target - i.e. make will 
# regard each different combination of arguments separately when deciding 
# what to update. However, we don't variant the reference path, i.e. there
# is one ref for each combination of make args. 
I_NO_SPACES=$(shell echo ${I} | tr -d '[:space:]')
RESULTS_PATH=test/results${I_NO_SPACES}
REFERENCE_PATH=test/reference

# Prepend default inferno args so they can be overridden. Thus, no need 
# for these to be appended to the results directory (see above)
DEFAULT_I_ARG="-tq"
IARG="${DEFAULT_I_ARG} ${I}"

# Looking for the "Keep going" thing? Do it by passing -k directly to make.

test/autotestd : autotestd.cpp
	g++ autotestd.cpp -o autotestd

$(ALL_CASES:%=${RESULTS_PATH}/execution/%.pass) : ${RESULTS_PATH}/execution/%.pass : test/examples/%.cpp test/makefile inferno.exe test/test.sh ${RESOURCE_ARTEFACTS}
	@mkdir -p ${RESULTS_PATH}/execution
	@rm -f ${RESULTS_PATH}/execution/$*.
	test/test.sh $< ${RESULTS_PATH}/execution ${IARG} 2>&1 > ${RESULTS_PATH}/execution/$*.log
	touch ${RESULTS_PATH}/execution/$*.pass
		
test : $(ALL_CASES:%=${RESULTS_PATH}/execution/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "EXECUTION TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the tests have parssed"

minitest : $(MINI_SC_CASES:%=${RESULTS_PATH}/execution/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI EXECUTION TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the tests have parssed"

cpptest : $(CPP_CASES:%=${RESULTS_PATH}/execution/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "CPP EXECUTION TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the tests have parssed"

$(SR_CASES:%=${RESULTS_PATH}/sr/%.pass) : ${RESULTS_PATH}/sr/%.pass : test/examples/%.cpp test/makefile inferno.exe test/srtest.sh
	@mkdir -p ${RESULTS_PATH}/sr
	@rm -f ${RESULTS_PATH}/sr/$*.
	test/srtest.sh $< ${REFERENCE_PATH}/sr ${RESULTS_PATH}/sr ${IARG}
	@touch ${RESULTS_PATH}/sr/$*.pass
	
srtest : $(SR_CASES:%=${RESULTS_PATH}/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "SR TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the tests have parssed"

minisrtest : $(MINI_SC_SR_CASES:%=${RESULTS_PATH}/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI SR TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the mini search and replace tests have parssed"

$(SC_CASES:%=${RESULTS_PATH}/rep/%.pass) : ${RESULTS_PATH}/rep/%.pass : test/examples/%.cpp test/makefile inferno.exe test/reptest.sh ${RESOURCE_ARTEFACTS}
	@mkdir -p ${RESULTS_PATH}/rep
	@rm -f ${RESULTS_PATH}/rep/$*.
	test/reptest.sh $< -tq 2>&1 > ${RESULTS_PATH}/rep/$*.log 
	touch ${RESULTS_PATH}/rep/$*.pass
	
reptest : $(SC_CASES:%=${RESULTS_PATH}/rep/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "REP TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the tests have passed"

graphtest : test/examples/*.cpp test/makefile inferno.exe gen_dev_graphs.sh
	./gen_dev_graphs.sh --pattern --docs --intermediate
	diff -r ${REFERENCE_PATH}/graphs/ graphs/ -x '*.svg'
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "GRAPH TESTS PASSED"
	@spd-say -i 0 -r -100 -p -100 "the tests have passed"

.PHONY: all test ${RESULTS_PATH}/graph.pass
