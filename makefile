DEBUG ?= 1

CXXFLAGS = -std=c++11

LIBS ?= -lpthread

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -D_DEBUG -fsanitize=address -MMD -MP
else
	CXXFLAGS += -O2 -MMD -MP
endif

LDFLAGS += $(LIBS)

export CXXFLAGS
export LDFLAGS
export DEBUG

.PHONY: all
all: src FileServer

.PHONY: src
src:
	$(MAKE) -C src

.PHONY: FileServer
FileServer: src
	$(MAKE) -C FileServer

.PHONY: clean
clean:
	$(RM) -r build
	$(MAKE) -C src clean
	$(MAKE) -C FileServer clean