.PHONY: test minitest srtest minisrtest reptest graphtest
 
# Slower ones first for optimial multi-core usage (whole suite time)
CASES = sctest13 sctest11 sctest12 sctest08 sctest10 sctest02
CASES += sctest01 sctest03 sctest04 sctest05 sctest06 sctest07 sctest09 sctest15

# Quicker ones first for exploratory (max feedback ASAP)
#CASES =  sctest09 sctest15 sctest06 sctest07 
#CASES += sctest04 sctest05 sctest01 sctest03
#CASES += sctest10 sctest02 sctest12 sctest08 sctest13 sctest11

# Use pointeris (very quick to test) as "canary"
SRCASES = pointeris ${CASES} 
MINICASES = sctest02 sctest01 sctest07 sctest09 sctest06 sctest05 sctest04 sctest15 sctest03
MINISRCASES = pointeris ${MINICASES} 

# Arguments to inferno executable:
# Args supplied via eg I=-uc will be squished together and appended to the
# results directory. This will change the make target - i.e. make will 
# regard each different combination of arguments separately when deciding 
# what to update. However, we don't variant the reference path, i.e. there
# is one ref for each combination of amke args. 
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

$(CASES:%=${RESULTS_PATH}/execution/%.pass) : ${RESULTS_PATH}/execution/%.pass : test/examples/%.cpp test/makefile inferno.exe test/test.sh ${RESOURCE_ARTEFACTS}
	@mkdir -p ${RESULTS_PATH}/execution
	@rm -f ${RESULTS_PATH}/execution/$*.
	test/test.sh $< ${RESULTS_PATH}/execution ${IARG} 2>&1 > ${RESULTS_PATH}/execution/$*.log
	touch ${RESULTS_PATH}/execution/$*.pass
		
test : $(CASES:%=${RESULTS_PATH}/execution/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "EXECUTION TESTS PASSED"
	@spd-say -r -100 -p -100 "the execution tests have parssed"

minitest : $(MINICASES:%=${RESULTS_PATH}/execution/%.pass)
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI EXECUTION TESTS PASSED"
	@spd-say -r -100 -p -100 "the mini execution tests have parssed"

$(SRCASES:%=${RESULTS_PATH}/sr/%.pass) : ${RESULTS_PATH}/sr/%.pass : test/examples/%.cpp test/makefile inferno.exe test/srtest.sh
	@mkdir -p ${RESULTS_PATH}/sr
	@rm -f ${RESULTS_PATH}/sr/$*.
	test/srtest.sh $< ${REFERENCE_PATH}/sr ${RESULTS_PATH}/sr ${IARG}
	@touch ${RESULTS_PATH}/sr/$*.pass
	
srtest : $(SRCASES:%=${RESULTS_PATH}/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "SR TESTS PASSED"
	@spd-say -r -100 -p -100 "the search and replace tests have parssed"

minisrtest : $(MINISRCASES:%=${RESULTS_PATH}/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI SR TESTS PASSED"
	@spd-say -r -100 -p -100 "the mini search and replace tests have parssed"

$(CASES:%=${RESULTS_PATH}/rep/%.pass) : ${RESULTS_PATH}/rep/%.pass : test/examples/%.cpp test/makefile inferno.exe test/reptest.sh ${RESOURCE_ARTEFACTS}
	@mkdir -p ${RESULTS_PATH}/rep
	@rm -f ${RESULTS_PATH}/rep/$*.
	test/reptest.sh $< -tq 2>&1 > ${RESULTS_PATH}/rep/$*.log 
	touch ${RESULTS_PATH}/rep/$*.pass
	
reptest : $(CASES:%=${RESULTS_PATH}/rep/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "REP TESTS PASSED"
	@spd-say -r -100 -p -100 "the rep tests have passed"

graphtest : test/examples/*.cpp test/makefile inferno.exe gen_dev_graphs.sh
	./gen_dev_graphs.sh --pattern --docs --intermediate
	diff -r ${REFERENCE_PATH}/graphs/ graphs/ -x '*.svg'
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "GRAPH TESTS PASSED"
	@spd-say -r -100 -p -100 "the graph tests have passed"

.PHONY: all test ${RESULTS_PATH}/graph.pass
