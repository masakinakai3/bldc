# =============================================================================
# VESC Firmware PC Build Makefile
# Phase 1: Foundation - ChibiOS POSIX layer and STM32 stubs
# Phase 2: Utility build - utils_math, buffer, crc
# Phase 3: ChibiOS-dependent utilities
# Phase 4: HAL/Peripheral stubs and utils_sys
# =============================================================================

# Build output directory
BUILDDIR = build

# Root directory (relative to pc_build)
ROOT = ..

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -g
CFLAGS += -DNO_STM32 -DUSE_PC_BUILD

# Include paths (order matters - PC stubs take priority)
INCLUDES = -I. -Ichibios_posix -Istubs -Ihw_stub -I$(ROOT)/util -I$(ROOT) -I$(ROOT)/comm -I$(ROOT)/applications

# Libraries
LIBS = -lpthread -lm -lrt

# =============================================================================
# Source files
# =============================================================================

# ChibiOS POSIX layer
CHIBIOS_SRC = \
    chibios_posix/ch_core.c \
    chibios_posix/hal_stub.c

# STM32 stubs
STUBS_SRC = \
    stubs/stm32f4xx_pc.c \
    stubs/app_stub.c

# =============================================================================
# Phase 2: Utility sources
# =============================================================================

UTIL_SRC = \
    $(ROOT)/util/utils_math.c \
    $(ROOT)/util/buffer.c \
    $(ROOT)/util/crc.c

# =============================================================================
# Phase 3: ChibiOS-dependent utilities
# =============================================================================

PHASE3_SRC = \
    $(ROOT)/util/digital_filter.c \
    $(ROOT)/util/worker.c \
    $(ROOT)/util/mempools.c

# =============================================================================
# Phase 4: HAL/Peripheral stubs and utils_sys
# =============================================================================

PHASE4_SRC = \
    $(ROOT)/util/utils_sys.c

PHASE4_STUB_SRC = \
    hw_stub/hw_stub.c

# Test program
TEST_SRC = test_main.c

# All sources for Phase 1 test
ALL_SRC = $(CHIBIOS_SRC) $(STUBS_SRC) $(TEST_SRC)

# Object files
OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(ALL_SRC))

# Utility object files (Phase 2)
UTIL_OBJS = $(BUILDDIR)/util/utils_math.o $(BUILDDIR)/util/buffer.o $(BUILDDIR)/util/crc.o

# Phase 3 object files
PHASE3_OBJS = $(BUILDDIR)/util/digital_filter.o $(BUILDDIR)/util/worker.o $(BUILDDIR)/util/mempools.o

# Phase 4 object files
PHASE4_OBJS = $(BUILDDIR)/util/utils_sys.o
PHASE4_STUB_OBJS = $(BUILDDIR)/hw_stub/hw_stub.o

# ChibiOS/Stubs object files for library
CHIBIOS_OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(CHIBIOS_SRC))
STUBS_OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(STUBS_SRC))

# =============================================================================
# Targets
# =============================================================================

.PHONY: all clean test dirs lib test_utils test_phase3 test_phase4 help

# Default target
all: dirs $(BUILDDIR)/test_pc

# Create build directories
dirs:
	@mkdir -p $(BUILDDIR)/chibios_posix
	@mkdir -p $(BUILDDIR)/stubs
	@mkdir -p $(BUILDDIR)/util
	@mkdir -p $(BUILDDIR)/hw_stub

# =============================================================================
# Phase 1: Test executable
# =============================================================================

# Link test executable
$(BUILDDIR)/test_pc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "Build successful: $@"

# Compile C files (Phase 1)
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run Phase 1 test
test: $(BUILDDIR)/test_pc
	@echo "Running PC build test..."
	@./$(BUILDDIR)/test_pc

# =============================================================================
# Phase 2: Static library and utility tests
# =============================================================================

# Compile utility sources
$(BUILDDIR)/util/utils_math.o: $(ROOT)/util/utils_math.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/util/buffer.o: $(ROOT)/util/buffer.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/util/crc.o: $(ROOT)/util/crc.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# =============================================================================
# Phase 3: ChibiOS-dependent utility sources
# =============================================================================

$(BUILDDIR)/util/digital_filter.o: $(ROOT)/util/digital_filter.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/util/worker.o: $(ROOT)/util/worker.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -Wno-return-type -c $< -o $@

$(BUILDDIR)/util/mempools.o: $(ROOT)/util/mempools.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# =============================================================================
# Phase 4: HAL/Peripheral stubs and utils_sys
# =============================================================================

$(BUILDDIR)/util/utils_sys.o: $(ROOT)/util/utils_sys.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -include hw_stub.h -include app_stub.h -c $< -o $@

$(BUILDDIR)/hw_stub/hw_stub.o: hw_stub/hw_stub.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/stubs/app_stub.o: stubs/app_stub.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Static library (Phase 2 + Phase 3 + Phase 4)
$(BUILDDIR)/libvesc_pc.a: $(UTIL_OBJS) $(PHASE3_OBJS) $(PHASE4_OBJS) $(PHASE4_STUB_OBJS) $(CHIBIOS_OBJS) $(STUBS_OBJS)
	$(AR) rcs $@ $^
	@echo "Library built: $@"

# Library target
lib: dirs $(BUILDDIR)/libvesc_pc.a

# Utility test executable
$(BUILDDIR)/test_utils: test_utils.c $(BUILDDIR)/libvesc_pc.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L$(BUILDDIR) -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

# Run utility tests
test_utils: $(BUILDDIR)/test_utils
	@echo "Running utility tests..."
	@./$(BUILDDIR)/test_utils

# =============================================================================
# Phase 3: Integration test
# =============================================================================

# Phase 3 test executable
$(BUILDDIR)/test_phase3: test_phase3.c $(BUILDDIR)/libvesc_pc.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L$(BUILDDIR) -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

# Run Phase 3 tests
test_phase3: $(BUILDDIR)/test_phase3
	@echo "Running Phase 3 tests..."
	@./$(BUILDDIR)/test_phase3

# =============================================================================
# Phase 4: Integration test
# =============================================================================

# Phase 4 test executable
$(BUILDDIR)/test_phase4: test_phase4.c $(BUILDDIR)/libvesc_pc.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L$(BUILDDIR) -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

# Run Phase 4 tests
test_phase4: $(BUILDDIR)/test_phase4
	@echo "Running Phase 4 tests..."
	@./$(BUILDDIR)/test_phase4

# =============================================================================
# Phase 5: Motor Control / FOC Simulation
# =============================================================================

# Phase 5 include paths
PHASE5_INCLUDES = -Imotor_sim -I$(ROOT)/motor

# Motor simulation sources
MOTOR_SIM_SRC = \
    motor_sim/mcconf_stub.c \
    motor_sim/virtual_motor_pc.c \
    motor_sim/foc_control_core.c \
    motor_sim/simulation_driver.c \
    motor_sim/simulation_data.c

# FOC math from original source (hardware-independent)
FOC_MATH_SRC = $(ROOT)/motor/foc_math.c

# Motor sim object files
MOTOR_SIM_OBJS = \
    $(BUILDDIR)/motor_sim/mcconf_stub.o \
    $(BUILDDIR)/motor_sim/virtual_motor_pc.o \
    $(BUILDDIR)/motor_sim/foc_control_core.o \
    $(BUILDDIR)/motor_sim/simulation_driver.o \
    $(BUILDDIR)/motor_sim/simulation_data.o

FOC_MATH_OBJS = $(BUILDDIR)/motor/foc_math.o

# Compile motor_sim sources
$(BUILDDIR)/motor_sim/%.o: motor_sim/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_INCLUDES) -c $< -o $@

# Compile foc_math.c from motor/
$(BUILDDIR)/motor/foc_math.o: $(ROOT)/motor/foc_math.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_INCLUDES) -c $< -o $@

# Phase 5 library (motor simulation components)
$(BUILDDIR)/libmotor_sim.a: $(MOTOR_SIM_OBJS) $(FOC_MATH_OBJS)
	$(AR) rcs $@ $^
	@echo "Motor simulation library built: $@"

lib_motor_sim: dirs $(BUILDDIR)/libmotor_sim.a

# Test executables
$(BUILDDIR)/test_foc_math: tests/test_foc_math.c $(BUILDDIR)/libmotor_sim.a $(BUILDDIR)/libvesc_pc.a
	@mkdir -p $(dir $@)
	@mkdir -p results
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_INCLUDES) -o $@ $< -L$(BUILDDIR) -lmotor_sim -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

$(BUILDDIR)/test_virtual_motor: tests/test_virtual_motor.c $(BUILDDIR)/libmotor_sim.a $(BUILDDIR)/libvesc_pc.a
	@mkdir -p $(dir $@)
	@mkdir -p results
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_INCLUDES) -o $@ $< -L$(BUILDDIR) -lmotor_sim -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

$(BUILDDIR)/test_foc_simulation: tests/test_foc_simulation.c $(BUILDDIR)/libmotor_sim.a $(BUILDDIR)/libvesc_pc.a
	@mkdir -p $(dir $@)
	@mkdir -p results
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_INCLUDES) -o $@ $< -L$(BUILDDIR) -lmotor_sim -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

$(BUILDDIR)/test_regression: tests/test_regression.c $(BUILDDIR)/libmotor_sim.a $(BUILDDIR)/libvesc_pc.a
	@mkdir -p $(dir $@)
	@mkdir -p results
	$(CC) $(CFLAGS) $(INCLUDES) $(PHASE5_INCLUDES) -o $@ $< -L$(BUILDDIR) -lmotor_sim -lvesc_pc $(LIBS)
	@echo "Build successful: $@"

# Run Phase 5 tests
test_foc_math: $(BUILDDIR)/test_foc_math
	@echo "Running FOC math unit tests..."
	@./$(BUILDDIR)/test_foc_math

test_virtual_motor: $(BUILDDIR)/test_virtual_motor
	@echo "Running virtual motor tests..."
	@./$(BUILDDIR)/test_virtual_motor

test_foc_simulation: $(BUILDDIR)/test_foc_simulation
	@echo "Running FOC simulation tests..."
	@./$(BUILDDIR)/test_foc_simulation

test_regression: $(BUILDDIR)/test_regression
	@echo "Running regression tests..."
	@./$(BUILDDIR)/test_regression

# Run all Phase 5 tests
test_phase5: test_foc_math test_virtual_motor test_foc_simulation test_regression
	@echo "All Phase 5 tests completed"

# Build all Phase 5 targets
phase5: lib lib_motor_sim $(BUILDDIR)/test_foc_math $(BUILDDIR)/test_virtual_motor $(BUILDDIR)/test_foc_simulation $(BUILDDIR)/test_regression
	@echo "Phase 5 build complete"

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)
	rm -rf results

# =============================================================================
# Help
# =============================================================================

help:
	@echo "VESC Firmware PC Build"
	@echo ""
	@echo "Phase 1 Targets:"
	@echo "  all        - Build Phase 1 test executable (default)"
	@echo "  test       - Build and run Phase 1 test"
	@echo ""
	@echo "Phase 2 Targets:"
	@echo "  lib        - Build static library (libvesc_pc.a)"
	@echo "  test_utils - Build and run utility tests"
	@echo ""
	@echo "Phase 3 Targets:"
	@echo "  test_phase3 - Build and run Phase 3 tests"
	@echo ""
	@echo "Phase 4 Targets:"
	@echo "  test_phase4 - Build and run Phase 4 tests"
	@echo ""
	@echo "Phase 5 Targets:"
	@echo "  lib_motor_sim      - Build motor simulation library"
	@echo "  test_foc_math      - Run FOC math unit tests"
	@echo "  test_virtual_motor - Run virtual motor tests"
	@echo "  test_foc_simulation - Run FOC simulation tests"
	@echo "  test_regression    - Run regression tests"
	@echo "  test_phase5        - Run all Phase 5 tests"
	@echo "  phase5             - Build all Phase 5 components"
	@echo ""
	@echo "Common Targets:"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Build outputs:"
	@echo "  $(BUILDDIR)/test_pc         - Phase 1 test"
	@echo "  $(BUILDDIR)/libvesc_pc.a    - Static library"
	@echo "  $(BUILDDIR)/test_utils      - Phase 2 utility test"
	@echo "  $(BUILDDIR)/test_phase3     - Phase 3 test"
	@echo "  $(BUILDDIR)/test_phase4     - Phase 4 test"
	@echo "  $(BUILDDIR)/libmotor_sim.a  - Motor simulation library"
	@echo "  $(BUILDDIR)/test_foc_math   - FOC math tests"
	@echo "  $(BUILDDIR)/test_virtual_motor - Virtual motor tests"
	@echo "  $(BUILDDIR)/test_foc_simulation - FOC simulation tests"
	@echo "  $(BUILDDIR)/test_regression - Regression tests"
