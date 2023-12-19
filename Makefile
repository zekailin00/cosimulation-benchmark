CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic -Wfatal-errors

CXXFLAGS += -I/scratch/zekailin00/pocl/pocl_rt_socket_drops/include
CXXFLAGS += -I/scratch/zekailin00/cosimulation-benchmark

CXXFLAGS_x86 = $(CXXFLAGS) 
CXXFLAGS_RISCV = $(CXXFLAGS) -fPIC -static

DRIVER = /scratch/zekailin00/vortex-socket/driver/socketsim/vortex.o
UTILITY = /scratch/zekailin00/vortex-socket/driver/socketsim/vx_utils.o
SOCKET = /scratch/zekailin00/vortex-socket/driver/socketsim/socketlib.o
RUNTIME = /scratch/zekailin00/pocl/build_rt_spike_64gc/lib/CL/libOpenCL.a

LDFLAGS += -L/scratch/zekailin00/pocl/pocl_rt_socket_drops/lib -lOpenCL
LDFLAGS += -L/scratch/zekailin00/vortex-socket/driver/socketsim -lsocketlib -lvortex

# chipyard riscv toolchain
CXX_RISCV = riscv64-unknown-linux-gnu-g++
DUMP_RISCV = riscv64-unknown-linux-gnu-objdump

SRCS = main.cpp kernel.cpp

PROJECT_RISCV = sample-riscv
PROJECT_x86 = sample-x86


all: $(PROJECT_RISCV) $(PROJECT_x86) dump.txt

$(PROJECT_RISCV): $(SRCS) $(UTILITY) $(SOCKET) $(DRIVER) $(RUNTIME)
	$(CXX_RISCV) -D RISCV_COMPILATION $(CXXFLAGS_RISCV) $^  -o $@

$(PROJECT_x86): $(SRCS) $(RUNTIME_x86) $(DRIVER_x86) $(SOCKET_x86)
	$(CXX) $(CXXFLAGS_x86) $^ $(LDFLAGS) -o $@

dump.txt: $(PROJECT_RISCV)
	$(DUMP_RISCV) -d $^ > $@

clean:
	rm -f *.bin
	rm -f *.o *.a *.txt
	rm -f $(PROJECT_RISCV)
	rm -f $(PROJECT_x86)