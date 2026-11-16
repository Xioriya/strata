all:
	$(MAKE) -C step

test: all
	./tests/run.sh

install:
	$(MAKE) -C step install

clean:
	$(MAKE) -C step clean

.PHONY: all test install clean
