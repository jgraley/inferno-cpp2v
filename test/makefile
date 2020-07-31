
# Slower ones first
CASES = sctest13 sctest11 sctest12 sctest08 sctest10 sctest02
CASES += sctest01 sctest03 sctest04 sctest05 sctest06 sctest07 sctest09 sctest15

KINDS = default sr rep

test/autotestd : autotestd.cpp
	g++ autotestd.cpp -o autotestd

$(CASES:%=test/results/default/%.pass) : test/results/default/%.pass : test/examples/%.cpp test/makefile inferno.exe resource
	@mkdir -p test/results/default
	@rm -f test/results/default/$*.
	test/test.sh $< -tq 2>&1 > test/results/default/$*.log 
	touch test/results/default/$*.pass
		
test/results/default.pass : $(CASES:%=test/results/default/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "ALL TESTS PASSED"

$(CASES:%=test/results/sr/%.pass) : test/results/sr/%.pass : test/examples/%.cpp test/makefile inferno.exe
	@mkdir -p test/results/sr
	@rm -f test/results/sr/$*.
	test/srtest.sh $< -tq
	@touch test/results/sr/$*.pass
	
test/results/sr.pass : $(CASES:%=test/results/sr/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "ALL TESTS PASSED"

$(CASES:%=test/results/rep/%.pass) : test/results/rep/%.pass : test/examples/%.cpp test/makefile inferno.exe
	@mkdir -p test/results/rep
	@rm -f test/results/rep/$*.
	test/reptest.sh $< -tq 2>&1 > test/results/rep/$*.log 
	touch test/results/rep/$*.pass
	
test/results/rep.pass : $(CASES:%=test/results/rep/%.pass)
	@touch $@
	@echo ------------------------------------------
	@echo -n "Tests Run: "
	@date
	@echo "ALL TESTS PASSED"

.PHONY: all test
