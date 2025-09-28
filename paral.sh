#!/bin/bash

ncores=10

seedList="0.123 0.172 0.190 0.234 0.345 0.380 0.443 0.478 0.717 0.794"

parallel -j ${ncores} bash tests.sh {} ::: ${seedList}