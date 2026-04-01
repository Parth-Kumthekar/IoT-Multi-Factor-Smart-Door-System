# Root Wrapper Makefile

all: compile

compile:
	@mkdir -p build
	@cd build && cmake .. && make -j$(nproc)
	@echo "-------------------------------------------------------"
	@echo "Build Complete! Run with: sudo ./build/smart_door_demo"
	@echo "-------------------------------------------------------"

# Run the main application
run:
	@sudo ./build/smart_door_demo

# Clean all build files
clean:
	@rm -rf build
	@echo "Project cleaned."

# Optional: target to run your shell scripts
setup:
	@chmod +x scripts/*.sh
	@./scripts/setup_nfc.sh

.PHONY: all compile run clean setup