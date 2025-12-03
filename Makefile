# Convenience Makefile for quick builds
# For full control, use CMake directly

.PHONY: all build clean install test help

all: build

build:
	@echo "Building Bitrecover..."
	@mkdir -p build
	@cd build && cmake .. && make -j$$(nproc)

clean:
	@echo "Cleaning build files..."
	@rm -rf build bin lib

install: build
	@echo "Installing..."
	@cd build && make install

test: build
	@echo "Running tests..."
	@./build/bin/bitrecover --list-devices

help:
	@echo "Bitrecover Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  build    - Build the project (default)"
	@echo "  clean    - Remove build files"
	@echo "  install  - Build and install"
	@echo "  test     - List available GPUs"
	@echo "  help     - Show this message"

