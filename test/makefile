
# Slower ones first for optimial multi-core usage (whole suite time)
#CASES = sctest13 sctest11 sctest12 sctest08 sctest10 sctest02
#CASES += sctest01 sctest03 sctest04 sctest05 sctest06 sctest07 sctest09 sctest15

# Quicker ones first for exploratory (max feedback ASAP)
CASES =  sctest09 sctest15 sctest06 sctest07 sctest04 sctest05 sctest01 sctest03
CASES += sctest10 sctest02 sctest12 sctest08 sctest13 sctest11

# Use pointeris (very quick to test) as "canary"
SRCASES = pointeris $(CASES) 

SRMINICASES = sctest15

KINDS = execution sr rep

test/autotestd : autotestd.cpp
	g++ autotestd.cpp -o autotestd

$(CASES:%=test/results/execution/%.pass) : test/results/execution/%.pass : test/examples/%.cpp test/makefile inferno.exe resource test/test.sh
	@mkdir -p test/results/execution
	@rm -f test/results/execution/$*.
	test/test.sh $< -tq 2>&1 > test/results/execution/$*.log 
	touch test/results/execution/$*.pass
		
test/results/execution.pass : $(CASES:%=test/results/execution/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "EXECUTION TESTS PASSED"
	@spd-say -r -100 -p -100 "the tests have parssed"

$(SRCASES:%=test/results/sr/%.pass) : test/results/sr/%.pass : test/examples/%.cpp test/makefile inferno.exe test/srtest.sh
	@mkdir -p test/results/sr
	@rm -f test/results/sr/$*.
	test/srtest.sh $< test/reference/srtest -tq
	@touch test/results/sr/$*.pass
	
test/results/sr.pass : $(SRCASES:%=test/results/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "SR TESTS PASSED"
	@spd-say -r -100 -p -100 "the tests have parssed"

$(SRMINICASES:%=test/results/srmini/%.pass) : test/results/srmini/%.pass : test/examples/%.cpp test/makefile inferno.exe test/srtest.sh
	@mkdir -p test/results/srmini
	@rm -f test/results/srmini/$*.
	test/srtest.sh $< test/reference/srminitest -t -q0.3
	@touch test/results/srmini/$*.pass
	
test/results/srmini.pass : $(SRMINICASES:%=test/results/srmini/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "SR MINI TESTS PASSED"
	@spd-say -r -100 -p -100 "the tests have parssed"

$(CASES:%=test/results/rep/%.pass) : test/results/rep/%.pass : test/examples/%.cpp test/makefile inferno.exe test/reptest.sh
	@mkdir -p test/results/rep
	@rm -f test/results/rep/$*.
	test/reptest.sh $< -tq 2>&1 > test/results/rep/$*.log 
	touch test/results/rep/$*.pass
	
test/results/rep.pass : $(CASES:%=test/results/rep/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "REP TESTS PASSED"
	@spd-say -r -100 -p -100 "the tests have passed"

.PHONY: all test
