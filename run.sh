
length=16
echo length is ${length}
cd /scratch/zekailin00/cosimulation-benchmark
time /scratch/zekailin00/riscv-isa-sim/drops/bin/spike /scratch/zekailin00/riscv-pk/drops/riscv64-unknown-elf/bin/pk sample-riscv > exe-log${length}.txt &
cd /scratch/zekailin00/chipyard/sims/vcs
timeout 360 make run-binary CONFIG=RadianceConfig BINARY=socket

length=32
echo length is ${length}
cd /scratch/zekailin00/cosimulation-benchmark
time /scratch/zekailin00/riscv-isa-sim/drops/bin/spike /scratch/zekailin00/riscv-pk/drops/riscv64-unknown-elf/bin/pk sample-riscv > exe-log${length}.txt &
cd /scratch/zekailin00/chipyard/sims/vcs
timeout 360 make run-binary CONFIG=RadianceConfig BINARY=socket