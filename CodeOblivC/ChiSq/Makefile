testName=ChiSq
CONFIGFILE=../../Code/paths.config
CILPATH=$(shell cat $(CONFIGFILE) | grep "pathoblivcdownload" | awk -F "=" '{print $$2}')
REMOTE_HOST=localhost
CFLAGS += -DREMOTE_HOST=$(REMOTE_HOST) -O3
./a.out: $(testName).oc $(testName).c util.c $(CILPATH)/_build/libobliv.a
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . $(testName).oc $(testName).c util.c
clean:
	rm -f a.out

