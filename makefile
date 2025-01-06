DEBUG ?= 1

CXXFLAGS = -std=c++11

LIBS ?= -lpthread

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -D_DEBUG -fsanitize=address
else
	CXXFLAGS += -O2
endif

LDFLAGS += $(LIBS)

export CXXFLAGS
export LDFLAGS
export DEBUG

.PHONY: all
all: src GatewayServer SessionServer FileServer

.PHONY: src
src:
	$(MAKE) -C src

.PHONY: SessionServer
SessionServer: src
	$(MAKE) -C SessionServer

.PHONY: GatewayServer
GatewayServer: src
	$(MAKE) -C GatewayServer

.PHONY: FileServer
FileServer: src
	$(MAKE) -C FileServer

.PHONY: clean
clean:
	$(RM) -r build
	$(MAKE) -C src clean
	$(MAKE) -C GatewayServer clean
	$(MAKE) -C SessionServer clean
	$(MAKE) -C test clean