#! /bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

make re

printf "${YELLOW}* Sample32 ${NC}\n"
./woody_woodpacker ressources/sample32
./woody

printf "${YELLOW}* Sample64 NO PIE ${NC}\n"
./woody_woodpacker ressources/sample64_no_pie
./woody

printf "${YELLOW}* Sample64 PIE ${NC}\n"
./woody_woodpacker ressources/sample64
./woody

printf "${YELLOW}* Sample with ARGS NO PIE ${NC}\n"
./woody_woodpacker ressources/sample_args_no_pie
./woody test

printf "${YELLOW}* Sample with ARGS PIE ${NC}\n"
./woody_woodpacker ressources/sample_args
./woody test
