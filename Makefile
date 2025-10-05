# 3D Printer Locker - Makefile for PlatformIO
# Provides convenient shortcuts for common build operations

.PHONY: all build clean upload monitor size speed debug help size-report

# Default target
all: build

# Standard build
build:
	@echo "Building with standard optimizations..."
	pio run -e nanoatmega328new

# Size-optimized build
size:
	@echo "Building with maximum size optimization..."
	pio run -e nanoatmega328new_size

# Speed-optimized build
speed:
	@echo "Building with speed optimization..."
	pio run -e nanoatmega328new_speed

# Debug build
debug:
	@echo "Building debug version..."
	pio run -e nanoatmega328new_debug

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	pio run --target clean

# Build and upload
upload: build
	@echo "Uploading to device..."
	pio run -e nanoatmega328new --target upload

# Open serial monitor
monitor:
	@echo "Opening serial monitor..."
	pio device monitor

# Show memory usage
size-report:
	@echo "Memory usage report:"
	@echo "==================="
	pio run -e nanoatmega328new --target size

# Install dependencies
install:
	@echo "Installing dependencies..."
	pio lib install

# Update dependencies
update:
	@echo "Updating dependencies..."
	pio lib update

# Check code
check:
	@echo "Running code checks..."
	pio check

# Show help
help:
	@echo "Available targets:"
	@echo "  build       - Standard optimized build (default)"
	@echo "  size        - Maximum size optimization"
	@echo "  speed       - Speed optimization"
	@echo "  debug       - Debug build with symbols"
	@echo "  clean       - Clean build directory"
	@echo "  upload      - Build and upload to device"
	@echo "  monitor     - Open serial monitor"
	@echo "  size-report - Show detailed memory usage"
	@echo "  install     - Install dependencies"
	@echo "  update      - Update dependencies"
	@echo "  check       - Run code checks"
	@echo "  help        - Show this help"
