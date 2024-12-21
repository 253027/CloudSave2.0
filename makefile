.PHONY: all
DEBUG ?= 1
all: src GatewayServer SessionServer

.PHONY: src
src:
	$(MAKE) -C src DEBUG=$(DEBUG)

.PHONY: SessionServer
SessionServer: src
	$(MAKE) -C SessionServer DEBUG=$(DEBUG)

.PHONY: GatewayServer
GatewayServer: src
	$(MAKE) -C GatewayServer DEBUG=$(DEBUG)

.PHONY: clean
clean:
	$(RM) -r build
	$(MAKE) -C src clean
	$(MAKE) -C GatewayServer clean
	$(MAKE) -C SessionServer clean
	$(MAKE) -C test clean