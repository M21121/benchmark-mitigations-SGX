# Makefile for SGX Mitigation Benchmarking System

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64

# Set DisableDebug value based on SGX_DEBUG
ifeq ($(SGX_DEBUG), 1)
	DISABLE_DEBUG_VALUE := 0
else
	DISABLE_DEBUG_VALUE := 1
endif

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_FLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_FLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

# Compiler settings
CXX := /usr/bin/g++
CC := /usr/bin/gcc

# Debug/Release flags
ifeq ($(SGX_DEBUG), 1)
	SGX_COMMON_FLAGS += -O0 -g -DDEBUG -UNDEBUG -UEDEBUG
else
	SGX_COMMON_FLAGS += -O2 -DNDEBUG -UEDEBUG -UDEBUG
endif

# Base compilation flags
SGX_COMMON_FLAGS += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
	-Waddress -Wsequence-point -Wformat-security -Wmissing-include-dirs \
	-Wfloat-equal -Wundef -Wshadow -Wcast-align -Wcast-qual -Wconversion \
	-Wredundant-decls

SGX_COMMON_CFLAGS := $(SGX_COMMON_FLAGS) -Wjump-misses-init -Wstrict-prototypes -std=c11
SGX_COMMON_CXXFLAGS := $(SGX_COMMON_FLAGS) -Wnon-virtual-dtor -std=c++14

# Security flags
SECURITY_FLAGS := -fstack-protector-strong -fPIE -fPIC -D_FORTIFY_SOURCE=2 \
	-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack

# Check for retpoline support
RETPOLINE_SUPPORTED := $(shell echo 'int main(){}' | $(CC) -mindirect-branch=thunk -x c - -o /dev/null 2>/dev/null && echo yes || echo no)
ifeq ($(RETPOLINE_SUPPORTED),yes)
	RETPOLINE_FLAGS := -mindirect-branch=thunk -mfunction-return=thunk
else
	RETPOLINE_FLAGS :=
endif

######## App Settings ########
App_Cpp_Files := app/app.cpp app/app_config.cpp app/benchmark_runner.cpp app/config_parser.cpp app/ocall_handlers.cpp
App_Include_Paths := -I$(SGX_SDK)/include -I. -Iapp
App_C_Flags := $(SGX_COMMON_CFLAGS) $(SECURITY_FLAGS) $(App_Include_Paths)
App_Cpp_Flags := $(SGX_COMMON_CXXFLAGS) $(SECURITY_FLAGS) $(App_Include_Paths)
App_Link_Flags := $(SGX_COMMON_FLAGS) $(SECURITY_FLAGS) -B/usr/bin/ -L$(SGX_LIBRARY_PATH) \
	-lsgx_urts -lpthread

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

######## Enclave Settings ########
Enclave_Cpp_Files := enclave/enclave.cpp app/mitigations.cpp
Enclave_Include_Paths := -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc \
	-I$(SGX_SDK)/include/libcxx -I. -Iapp -Ienclave

Enclave_C_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie \
	-fno-builtin-printf $(Enclave_Include_Paths)
Enclave_Cpp_Flags := $(SGX_COMMON_CXXFLAGS) -nostdinc++ -fvisibility=hidden -fpie \
	-fno-builtin-printf $(Enclave_Include_Paths)

# Add retpoline to enclave if supported
Enclave_C_Flags += $(RETPOLINE_FLAGS)
Enclave_Cpp_Flags += $(RETPOLINE_FLAGS)

Enclave_Link_Flags := $(SGX_COMMON_FLAGS) -Wl,--no-undefined -nostdlib \
	-nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -lsgx_trts -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -lsgx_tcrypto -lsgx_tservice -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
	-Wl,--defsym,__ImageBase=0 -Wl,--gc-sections

# File names
App_Name := sgx_benchmark
Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := enclave/enclave.config.xml

# Generated files
Generated_Files := enclave_u.c enclave_u.h enclave_t.c enclave_t.h

# Object files
App_Objects := app.o app_config.o benchmark_runner.o config_parser.o ocall_handlers.o enclave_u.o
Enclave_Objects := enclave.o mitigations.o enclave_t.o

# Intermediate files for cleanup
Intermediate_Files := $(Generated_Files) $(App_Objects) $(Enclave_Objects) $(Enclave_Name)

.PHONY: all clean clean-all run-tests help install-deps check-sgx

# Default target
all: $(App_Name) $(Signed_Enclave_Name)

######## EDL Generation ########
$(Generated_Files): enclave/enclave.edl
	@echo "Generating edge routines..."
	@$(SGX_EDGER8R) --untrusted enclave/enclave.edl --search-path $(SGX_SDK)/include --search-path app
	@$(SGX_EDGER8R) --trusted enclave/enclave.edl --search-path $(SGX_SDK)/include --search-path app
	@echo "GEN  =>  $(Generated_Files)"

######## App Objects ########
app_config.o: app/app_config.cpp app/mitigation_config.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

app.o: app/app.cpp enclave_u.h app/mitigation_config.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

enclave_u.o: enclave_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

benchmark_runner.o: app/benchmark_runner.cpp app/benchmark_runner.h app/cycle_counter.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

config_parser.o: app/config_parser.cpp app/config_parser.h app/mitigation_config.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

ocall_handlers.o: app/ocall_handlers.cpp enclave_u.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

######## App Binary ########
$(App_Name): $(App_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

######## Enclave Objects ########
enclave_t.o: enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

mitigations.o: app/mitigations.cpp app/mitigations.h app/mitigation_config.h enclave_t.h
	@$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

enclave.o: enclave/enclave.cpp enclave_t.h app/mitigations.h app/mitigation_config.h
	@$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

######## Enclave Binary ########
$(Enclave_Name): $(Enclave_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"

######## Signed Enclave ########
$(Signed_Enclave_Name): $(Enclave_Name) $(Enclave_Config_File)
	@$(SGX_ENCLAVE_SIGNER) sign -key enclave/enclave_private.pem -enclave $(Enclave_Name) \
		-out $@ -config $(Enclave_Config_File)
	@echo "SIGN =>  $@"

######## Test Targets ########
test-files:
	@echo "Creating test files..."
	@dd if=/dev/urandom of=test.txt bs=1024 count=100 status=none 2>/dev/null
	@dd if=/dev/urandom of=large_test.txt bs=1024 count=1024 status=none 2>/dev/null
	@echo "Created test.txt (100KB) and large_test.txt (1MB)"

test-basic: $(App_Name) $(Signed_Enclave_Name) test-files
	@echo "Running basic functionality tests..."
	@./$(App_Name) -t ecall -i 10 -m none
	@./$(App_Name) -t pure_ocall -i 10 -m none
	@./$(App_Name) -t pingpong -i 5 -m none
	@./$(App_Name) -t untrusted_file -i 5 -m none -f test.txt
	@echo "Basic tests completed successfully"

benchmark: $(App_Name) $(Signed_Enclave_Name) test-files
	@echo "Running comprehensive benchmark..."
	@chmod +x benchmark_script.sh
	@./benchmark_script.sh
	@echo "Benchmark completed. Results in benchmark_results.csv"

test-mitigations: $(App_Name) $(Signed_Enclave_Name) test-files
	@echo "Testing individual mitigations..."
	@echo "Testing lfence barriers..."
	@./$(App_Name) -t ecall -i 100 -m lfence
	@echo "Testing mfence barriers..."
	@./$(App_Name) -t ecall -i 100 -m mfence
	@echo "Testing cache flushing..."
	@./$(App_Name) -t ecall -i 100 -m cache
	@echo "Testing constant time operations..."
	@./$(App_Name) -t ecall -i 100 -m constant
	@echo "Testing memory barriers..."
	@./$(App_Name) -t ecall -i 100 -m memory
	@echo "Testing all mitigations..."
	@./$(App_Name) -t ecall -i 100 -m all
	@echo "Mitigation tests completed"

run-tests: test-basic test-mitigations
	@echo "All tests completed successfully"

######## Utility Targets ########
check-sgx:
	@echo "Checking SGX environment..."
	@if [ -d "$(SGX_SDK)" ]; then \
		echo "✓ SGX SDK found at $(SGX_SDK)"; \
	else \
		echo "✗ SGX SDK not found at $(SGX_SDK)"; \
		echo "  Set SGX_SDK environment variable or install Intel SGX SDK"; \
		exit 1; \
	fi
	@if [ -c /dev/isgx ] || [ -c /dev/sgx_enclave ]; then \
		echo "✓ SGX device found"; \
	else \
		echo "⚠ SGX device not found - running in simulation mode"; \
	fi
	@echo "SGX Mode: $(SGX_MODE)"
	@echo "SGX Architecture: $(SGX_ARCH)"
	@echo "Debug Mode: $(SGX_DEBUG)"

install-deps:
	@echo "Installing build dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y build-essential ocaml ocamlbuild automake autoconf \
		libtool wget python libssl-dev git cmake perl libssl-dev libcurl4-openssl-dev \
		protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
	@echo "Dependencies installed. Please install Intel SGX SDK separately."

enclave/enclave_private.pem:
	@echo "Generating enclave signing key..."
	@mkdir -p enclave
	@openssl genrsa -out $@ -3 3072
	@echo "Generated $@"

$(Enclave_Config_File):
	@echo "Creating enclave configuration..."
	@mkdir -p enclave
	@echo '<EnclaveConfiguration>' > $@
	@echo '  <ProdID>0</ProdID>' >> $@
	@echo '  <ISVSVN>0</ISVSVN>' >> $@
	@echo '  <StackMaxSize>0x400000</StackMaxSize>' >> $@
	@echo '  <HeapMaxSize>0x100000000</HeapMaxSize' >> $@
	@echo '  <TCSNum>1</TCSNum>' >> $@
	@echo '  <TCSPolicy>1</TCSPolicy>' >> $@
	@echo '  <DisableDebug>$(DISABLE_DEBUG_VALUE)</DisableDebug>' >> $@
	@echo '  <MiscSelect>0</MiscSelect>' >> $@
	@echo '  <MiscMask>0xFFFFFFFF</MiscMask>' >> $@
	@echo '</EnclaveConfiguration>' >> $@
	@echo "Generated $@"

######## Cleanup Targets ########
clean-intermediate:
	@rm -f $(Intermediate_Files)
	@echo "Cleaned intermediate files"

clean:
	@rm -f $(App_Name) $(Signed_Enclave_Name) $(Intermediate_Files) \
		test.txt large_test.txt *.sealed
	@echo "Cleaned all build artifacts and test files"

clean-all: clean
	@rm -f enclave/enclave_private.pem $(Enclave_Config_File)
	@echo "Cleaned everything including generated keys and configs"

######## Help Target ########
help:
	@echo "SGX Mitigation Benchmarking System"
	@echo "=================================="
	@echo ""
	@echo "Build Targets:"
	@echo "  all              - Build application and signed enclave (default)"
	@echo "  $(App_Name)      - Build application only"
	@echo "  $(Signed_Enclave_Name) - Build and sign enclave"
	@echo ""
	@echo "Test Targets:"
	@echo "  test-basic       - Run basic functionality tests"
	@echo "  test-mitigations - Test individual mitigations"
	@echo "  benchmark        - Run comprehensive performance benchmark"
	@echo "  run-tests        - Run all tests"
	@echo ""
	@echo "Utility Targets:"
	@echo "  check-sgx        - Check SGX environment and configuration"
	@echo "  install-deps     - Install build dependencies (Ubuntu/Debian)"
	@echo "  test-files       - Create test files for benchmarking"
	@echo ""
	@echo "Cleanup Targets:"
	@echo "  clean            - Remove build artifacts and test files"
	@echo "  clean-all        - Remove everything including keys and configs"
	@echo "  clean-intermediate - Remove only intermediate build files"
	@echo ""
	@echo "Configuration:"
	@echo "  SGX_SDK=$(SGX_SDK)"
	@echo "  SGX_MODE=$(SGX_MODE)  (HW or SIM)"
	@echo "  SGX_DEBUG=$(SGX_DEBUG) (1 for debug, 0 for release)"

# Ensure required files exist
$(App_Name) $(Signed_Enclave_Name): | enclave/enclave_private.pem $(Enclave_Config_File)

# Dependencies
$(App_Objects): $(Generated_Files)
$(Enclave_Objects): $(Generated_Files)
