ARCH := $(shell getconf LONG_BIT)
CC = gcc
CCFLAGS = -Wall
#LDFLAGS = -lpthread -lrt
LDFLAGS = -lrt
PROGNAME = fan-monitor$(ARCH)

MODULES = fan-monitor.o

all: release

##########
# Builds #
##########

release: CCFLAGS += -O2 -DNDEBUG
release: BUILD = Release
release: $(MODULES)
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $(PROGNAME) $(MODULES)

debug: CCFLAGS += -g -D_DEBUG
debug: BUILD = Debug
debug: $(MODULES)
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $(PROGNAME) $(MODULES)

###################
# Process modules #
###################

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROGNAME)

install:
	sudo chmod +x ./install_service.sh
	install -d /usr/local/bin/
	install ./fan-monitor64 /usr/local/bin/
	./install_service.sh
