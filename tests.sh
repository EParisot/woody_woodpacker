#! /bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

make re


printf "\n${RED}* Tests Corrupted: ${NC}\n"

for file in "corrupted"/*; do
	printf "\n${YELLOW}* ./woody_woodpacker $file: ${NC}\n"
	./woody_woodpacker $file
	if [ -f "woody" ]; then
		printf "\n${BLUE}./woody [args]: ${NC}\n"
		./woody
		printf "${RED}[FAIL]${NC}\n"
		rm -f woody
	else
		printf "${GREEN}[OK]${NC}\n"
	fi
done

printf "\n${YELLOW}* Sample32 ${NC}\n"
./woody_woodpacker ressources/sample32
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${RED}* Tests PIE: ${NC}\n"

printf "\n${YELLOW}* Sample64 PIE ${NC}\n"
./woody_woodpacker ressources/sample64
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* Sample64 with ARGS PIE ${NC}\n"
./woody_woodpacker ressources/sample_args
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd="./woody test"
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* Sample64 STRIPPED ${NC}\n"
./woody_woodpacker ressources/sample_stripped
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* Sample64 with ARGS STRIPPED ${NC}\n"
./woody_woodpacker ressources/sample_args_stripped
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=""./woody test""
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${RED}* Tests NO PIE: ${NC}\n"

printf "\n${YELLOW}* Sample64 NO PIE ${NC}\n"
./woody_woodpacker ressources/sample_no_pie
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* Sample64 NO PIE with ARGS ${NC}\n"
./woody_woodpacker ressources/sample_args_no_pie
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd="./woody test"
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* Sample64 NO PIE STRIPPED ${NC}\n"
./woody_woodpacker ressources/sample_stripped_no_pie
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* Sample64 NO PIE with ARGS STRIPPED ${NC}\n"
./woody_woodpacker ressources/sample_args_stripped_no_pie
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd="./woody test"
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${RED}* Tests real world programs: ${NC}\n"

printf "\n${YELLOW}* ./woody_woodpacker /bin/date ${NC}\n"
./woody_woodpacker /bin/date
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....'; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi

printf "\n${YELLOW}* ./woody_woodpacker /bin/ls ${NC}\n"
./woody_woodpacker /bin/ls
if [ -f "woody" ]; then
	printf "\n${BLUE}./woody [args]: ${NC}\n"
	cmd=./woody
	res=`$cmd | tr -d '\0'`
	if echo "$res" | grep -q '....WOODY....' ; then
		echo "$res"
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
	rm -f woody
fi
