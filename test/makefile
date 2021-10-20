
# Slower ones first for optimial multi-core usage (whole suite time)
CASES = sctest13 sctest11 sctest12 sctest08 sctest10 sctest02
CASES += sctest01 sctest03 sctest04 sctest05 sctest06 sctest07 sctest09 sctest15

# Quicker ones first for exploratory (max feedback ASAP)
#CASES =  sctest09 sctest15 sctest06 sctest07 
#CASES += sctest04 sctest05 sctest01 sctest03
#CASES += sctest10 sctest02 sctest12 sctest08 sctest13 sctest11

# Use pointeris (very quick to test) as "canary"
SRCASES = pointeris $(CASES) 

MINICASES = sctest02 sctest01 sctest07 sctest09 sctest06 sctest05 sctest04 sctest15 sctest03

test/autotestd : autotestd.cpp
	g++ autotestd.cpp -o autotestd

$(CASES:%=test/results/execution/%.pass) : test/results/execution/%.pass : test/examples/%.cpp test/makefile inferno.exe test/test.sh resource
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
	@spd-say -r -100 -p -100 "the execution tests have parssed"

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
	@spd-say -r -100 -p -100 "the search and replace tests have parssed"

test/results/mini.pass : $(MINICASES:%=test/results/execution/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "MINI EXECUTION TESTS PASSED"
	@spd-say -r -100 -p -100 "the mini execution tests have parssed"

$(CASES:%=test/results/rep/%.pass) : test/results/rep/%.pass : test/examples/%.cpp test/makefile inferno.exe test/reptest.sh resource
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
	@spd-say -r -100 -p -100 "the rep tests have passed"

test/results/graph.pass : test/examples/*.cpp test/makefile inferno.exe gen_dev_graphs.sh
	./gen_dev_graphs.sh
	diff -r test/reference/graphs/ graphs/ -x '*.svg'
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "GRAPH TESTS PASSED"
	@spd-say -r -100 -p -100 "the graph tests have passed"

.PHONY: all test test/results/graph.pass
