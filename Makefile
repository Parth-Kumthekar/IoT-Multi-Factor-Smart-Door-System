# Root Wrapper Makefile

# Default target: compile both the app and the tests
all: compile

compile:
	@echo "Creating build directory..."
	@mkdir -p build
	@echo "Running CMake..."
	@cd build && cmake .. && make -j$(nproc)
	@echo "-------------------------------------------------------"
	@echo "Build Complete!"
	@echo "Run App:   sudo ./build/smart_door_demo"
	@echo "Run Tests: ./build/run_tests"
	@echo "-------------------------------------------------------"

# Run the main smart door application
run:
	@sudo ./build/smart_door_demo

# Run the test cases
test:
	@./build/run_tests

# Clean up all build artifacts
clean:
	@rm -rf build
	@echo "Project cleaned (build directory removed)."

# Ensure scripts are executable and run setup
setup:
	@chmod +x scripts/*.sh
	@./scripts/setup_nfc.sh

.PHONY: all compile run test clean setup