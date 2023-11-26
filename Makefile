CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic -Wfatal-errors

CXXFLAGS += -I/home/zekailin00/Desktop/vortex/pocl/pocl_rt_spike_drops/include
CXXFLAGS += -I/home/zekailin00/Desktop/vortex/opencl-test/kernels

CXXFLAGS_x86 = $(CXXFLAGS) 
#-static-libstdc++ -static-libgcc
CXXFLAGS_RISCV = $(CXXFLAGS) -fPIC -static

DRIVER = /home/zekailin00/Desktop/vortex/driver/socket/vortex.o
UTILITY = /home/zekailin00/Desktop/vortex/driver/socket/vx_utils.o
SOCKET = /home/zekailin00/Desktop/vortex/driver/socket/socketlib.o
RUNTIME = /home/zekailin00/Desktop/vortex/pocl/pocl_rt_spike_drops_ilp32d_elf_old/lib/static/libOpenCL.a

# DRIVER_x86 = /home/zekailin00/Desktop/vortex/vortex-socket/driver/socketsim/libvortex.so
# # UTILITY = /home/zekailin00/Desktop/vortex/driver/socket/vx_utils.o
# SOCKET_x86 = /home/zekailin00/Desktop/vortex/vortex-socket/driver/socketsim/libsocketlib.so
# RUNTIME_x86 = /home/zekailin00/Desktop/vortex/pocl/pocl_rt_socket_drops/lib/libOpenCL.so

LDFLAGS += -L/home/zekailin00/Desktop/vortex/pocl/pocl_rt_socket_drops/lib -lOpenCL
LDFLAGS += -L/home/zekailin00/Desktop/vortex/vortex-socket/driver/socketsim -lsocketlib -lvortex

SRCS = main.cpp kernel.cpp

PROJECT_RISCV = sample-riscv
PROJECT_x86 = sample-x86

# old toolchain
CXX_RISCV = /home/zekailin00/Desktop/riscv-gnu-toolchain-old/drops-32/bin/riscv32-unknown-linux-gnu-g++
DUMP_RISCV = /home/zekailin00/Desktop/riscv-gnu-toolchain-old/drops-32/bin/riscv32-unknown-linux-gnu-objdump


all: $(PROJECT_RISCV) $(PROJECT_x86) dump.txt

$(PROJECT_RISCV): $(SRCS) $(UTILITY) $(SOCKET) $(DRIVER) $(RUNTIME)
	$(CXX_RISCV) $(CXXFLAGS_RISCV) $^  -o $@

$(PROJECT_x86): $(SRCS) $(RUNTIME_x86) $(DRIVER_x86) $(SOCKET_x86)
	$(CXX) $(CXXFLAGS_x86) $^ $(LDFLAGS) -o $@

dump.txt: $(PROJECT_RISCV)
	$(DUMP_RISCV) -d $^ > $@

clean:
	rm -f *.o *.a *.txt
	rm -f $(PROJECT_RISCV)
	rm -f $(PROJECT_x86)