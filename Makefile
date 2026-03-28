BUILD_DIR ?= build
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Release
JOBS ?= 4
.PHONY: all configure core games graphicals clean fclean re test
all: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)
configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)
core: configure
	cmake --build $(BUILD_DIR) --target core --parallel $(JOBS)
games: configure
	cmake --build $(BUILD_DIR) --target games --parallel $(JOBS)
graphicals: configure
	cmake --build $(BUILD_DIR) --target graphicals --parallel $(JOBS)
clean:
	@if [ -d "$(BUILD_DIR)" ]; then cmake --build $(BUILD_DIR) --target clean; fi
fclean:
	rm -rf $(BUILD_DIR)
	rm -f arcade lib/arcade_*.so
re: fclean
	$(MAKE) all
test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure
