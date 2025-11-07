#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A ATR-25-7
#SBATCH -n 1 
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --comment "use:vtune=2022.2.0"
#SBATCH -t 0:10:00
#SBATCH --mail-type END
#SBATCH -J AVS-advisor
#SBATCH --exclusive             # ensure full node access (avoid interference)

cd $SLURM_SUBMIT_DIR
ml purge
ml VTune Advisor intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0

# === Compiler and runtime affinity settings ===
export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OMP_NUM_THREADS=1
export MKL_NUM_THREADS=1
export KMP_AFFINITY=compact,1,0

[ -d build_advisor_opt ] && rm -rf build_advisor_opt
[ -d build_advisor_opt ] || mkdir build_advisor_opt
cd build_advisor_opt

CC=icx CXX=icpx cmake -DUSE_O3=ON ..
make


for calc in "ref" "batch" "line"; do
    rm -rf Advisor-$calc
    mkdir Advisor-$calc

    # Basic survey
    taskset -c 0 advixe-cl -collect survey -project-dir Advisor-$calc  -- ./mandelbrot -c $calc -s 4096

    # Roof line
    taskset -c 0 advixe-cl -collect tripcounts -flop -project-dir Advisor-$calc  -- ./mandelbrot -c $calc -s 4096
done
