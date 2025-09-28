#!/bin/bash
seed=$1

export AWK_STACKSIZE=32768

instances_dir="datasets"
output_dir="testing_nuevo/50/base"

mkdir -p "${output_dir}"

popsize=20
ngen=500
nobj=2
pcross_bin=0.9
pmut_bin=0.01
base=0

for instance_path in "${instances_dir}"/*; do

    instance=$(basename "${instance_path}")

    output_file="${output_dir}/out_${instance}_${seed}.out"

    ./nsga2r "${seed}" "${instance_path}" "${popsize}" "${ngen}" "${nobj}" "${pcross_bin}" "${pmut_bin}" "${base}" > "${output_file}"

    echo "Generado: ${output_file}"
done
