.PHONY: test minitest srtest minisrtest reptest graphtest
 
# Slower ones first for optimial multi-core usage (whole suite time)
SC_CASES = sctest13.cpp sctest03.cpp sctest11.cpp sctest12.cpp sctest08.cpp sctest10.cpp sctest02.cpp 
SC_CASES += sctest01.cpp sctest04.cpp sctest05.cpp sctest06.cpp sctest07.cpp sctest09.cpp sctest15.cpp
SC_CASES += scissue677.cpp scissue667.cpp sc10_reduce.cpp

# Quicker ones first for exploratory (max feedback ASAP)
#SC_CASES =  sctest09.cpp  sctest15.cpp  sctest06.cpp  sctest07.cpp  
#SC_CASES += sctest04.cpp  sctest05.cpp  sctest01.cpp  sctest03.cpp 
#SC_CASES += sctest10.cpp  sctest02.cpp  sctest12.cpp  sctest08.cpp  sctest13.cpp  sctest11.cpp 

# Use pointeris (very quick to test) as "canary" even more so "small"
SR_CASES = ${SC_CASES} 
MINI_SC_CASES = sctest02.cpp sctest03.cpp sctest01.cpp sctest07.cpp sctest09.cpp sctest06.cpp sctest05.cpp sctest04.cpp sctest15.cpp 
MINI_SC_SR_CASES = small.c pointeris.c ${MINI_SC_CASES} 
MINI_SC_SR_CASES = small.c pointeris.c ${MINI_SC_CASES} 

# The original C/C++ test cases, no SystemC
CPP_CASES = test04.cpp test07.cpp test12.cpp test14.cpp methodcall.cpp ut_in_func.cpp
C_CASES = declchain.c buriedcall.c test02.c test03.c test05.c test06.c test09.c test10.c test11.c test13.c test15.c test16.c small.c compound.c ifelse.c
  
# Skip due #834: gotovar.c  
# Skip due #836: duffs_device.c  
# Skip because can't use it with -qN due ({}) outside function: pointeris.c
  
# The whole lot for the benefit of auto rules
EXEC_CASES = ${SC_CASES} ${C_CASES} ${CPP_CASES}

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

$(EXEC_CASES:%=${RESULTS_PATH}/exec/%.pass) : ${RESULTS_PATH}/exec/%.pass : test/examples/% test/makefile inferno.exe test/exec_test.sh .clang-format ${RESOURCE_DEPS}
	@mkdir -p ${RESULTS_PATH}/exec
	@rm -f ${RESULTS_PATH}/exec/$*.
	test/exec_test.sh $< ${RESULTS_PATH}/exec ${IARG} 2>&1 > ${RESULTS_PATH}/exec/$*.log
	touch ${RESULTS_PATH}/exec/$*.pass
		
test : $(EXEC_CASES:%=${RESULTS_PATH}/exec/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "EXECUTION TESTS PASSED"
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the tests have parssed"

minitest : $(MINI_SC_CASES:%=${RESULTS_PATH}/exec/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI EXECUTION TESTS PASSED"
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the tests have parssed"

cpptest : $(CPP_CASES:%=${RESULTS_PATH}/exec/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "CPP EXECUTION TESTS PASSED"
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the tests have parssed"

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
	@echo "SEARCH/REPLACE TESTS PASSED"
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the tests have parssed"

minisrtest : $(MINI_SC_SR_CASES:%=${RESULTS_PATH}/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI SEARCH/REPLACE TESTS PASSED"
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the mini search and replace tests have parssed"

$(SC_CASES:%=${RESULTS_PATH}/rep/%.pass) : ${RESULTS_PATH}/rep/%.pass : test/examples/%.cpp test/makefile inferno.exe test/reptest.sh ${RESOURCE_DEPS}
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
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the tests have passed"

graphtest : test/examples/*.cpp test/makefile inferno.exe gen_dev_graphs.sh
	./gen_dev_graphs.sh --pattern --docs --intermediate
	diff -r ${REFERENCE_PATH}/graphs/ graphs/ -x '*.svg'
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "GRAPH TESTS PASSED"
	@LANG='en_GB.UTF-8' spd-say -i 0 -r -100 -p -100 "the tests have passed"

.PHONY: all test ${RESULTS_PATH}/graph.pass
