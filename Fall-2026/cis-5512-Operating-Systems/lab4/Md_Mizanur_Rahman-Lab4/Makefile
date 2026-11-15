# Rebuild supplied binaries for the local compiler/machine.
.PHONY: all test
all:
	$(MAKE) -B -C netlib
	$(MAKE) -B -C tsh
	$(MAKE) -B -C tools
	$(MAKE) -B -C matrix
test: all
	python3 tests/review_smoke.py
