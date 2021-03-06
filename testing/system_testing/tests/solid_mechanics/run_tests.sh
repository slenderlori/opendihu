#!/bin/bash

echo "running example $(pwd)"

workdir=$(pwd)
variant="debug"
#variant="release"

# change to build directory
mkdir -p build_${variant}
cd build_${variant}

# remove old output data
rm -rf out

# command arguments: <analytical-jacobian> <name>
#./mooney_rivlin_incompressible_penalty2d            ../settings_2d.py  0 mooney_rivlin_incompressible_penalty2d_numeric_jacobian_scenario_1
#./mooney_rivlin_incompressible_penalty2d            ../settings_2d.py  1 mooney_rivlin_incompressible_penalty2d_analytic_jacobian_scenario_1
#./mooney_rivlin_incompressible_penalty2d            ../settings_2d.py  0 mooney_rivlin_incompressible_penalty2d_numeric_jacobian_scenario_2
#./mooney_rivlin_incompressible_penalty2d            ../settings_2d.py  1 mooney_rivlin_incompressible_penalty2d_analytic_jacobian_scenario_2
#./mooney_rivlin_incompressible_penalty2d            ../settings_2d.py  0 mooney_rivlin_incompressible_penalty2d_numeric_jacobian_scenario_3
#./mooney_rivlin_incompressible_penalty2d            ../settings_2d.py  1 mooney_rivlin_incompressible_penalty2d_analytic_jacobian_scenario_3

# command arguments: <analytical jacobian> <name>
#  jacobian: 0 = numeric, 1 = analytic, 2 = combined
./mooney_rivlin_incompressible_mixed2d            ../settings_mixed_2d.py  0 mooney_rivlin_incompressible_mixed2d_numeric_jacobian_scenario_1
./mooney_rivlin_incompressible_mixed2d            ../settings_mixed_2d.py  2 mooney_rivlin_incompressible_mixed2d_analytic_jacobian_scenario_1

./mooney_rivlin_incompressible_mixed2d            ../settings_mixed_2d.py  0 mooney_rivlin_incompressible_mixed2d_numeric_jacobian_scenario_2
./mooney_rivlin_incompressible_mixed2d            ../settings_mixed_2d.py  2 mooney_rivlin_incompressible_mixed2d_analytic_jacobian_scenario_2

./mooney_rivlin_incompressible_mixed3d            ../settings_mixed_3d.py  2 mooney_rivlin_incompressible_mixed2d_analytic_jacobian_scenario_2

#./mooney_rivlin_incompressible_mixed_condensation ../settings.py  mooney_rivlin_incompressible_mixed_condensation  10
#./mooney_rivlin_incompressible_mixed              ../settings.py  mooney_rivlin_incompressible_mixed  10

cd $workdir
