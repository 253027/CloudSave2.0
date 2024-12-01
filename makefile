.PHONY: all
all: src GatewayServer SessionServer

.PHONY: src
src:
	$(MAKE) -C src

.PHONY: SessionServer
SessionServer: src
	$(MAKE) -C SessionServer

.PHONY: GatewayServer
GatewayServer: src
	$(MAKE) -C GatewayServer

.PHONY: clean
clean:
	$(RM) -r build
	$(MAKE) -C src clean
	$(MAKE) -C GatewayServer clean
	$(MAKE) -C SessionServer clean
	$(MAKE) -C test clean