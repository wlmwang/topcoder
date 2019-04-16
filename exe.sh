#!/bin/sh
# you should exec compile_cpp.sh first

if [ $# != 4 ]
then
	echo "Usage: "$0" inputfile outputfile timefile tempdir"
	exit 1
fi

BIN_PATH=./
cd ${BIN_PATH}

echo "status:running" >> coder_info

./a.out $1 $2 $3 $4 > exe_result 2>&1
