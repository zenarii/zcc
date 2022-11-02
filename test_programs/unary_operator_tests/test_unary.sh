RED="\033[0;31m"
GREEN="\033[;32m"
CLEAR="\033[0m"

# test_program source_file.c expected_value
function test_program () {
	~/dev/ccompiler/build/zcc --delete-asm $1
	./out > /dev/null
	returned_value=$?
	if [ $returned_value == $2 ]
	then
		echo -e "${GREEN}[PASSED] $1 ${CLEAR}"
	else
		echo -e "${RED}[FAILED] $1 expected $2, got $returned_value ${CLEAR}"
	fi
	rm out
}

test_program not_zero.c 1
test_program not_seven.c 0
# Note(abiab): as bash values mapped 0 to 255, expect the value to be 256 - 20;
test_program neg.c 236
test_program bitwise.c 240
test_program bitwise_zero.c 255
test_program nested_one.c 255
test_program nested_two.c 2
