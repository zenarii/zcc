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

function test_invalid() {
	~/dev/ccompiler/build/zcc --delete-asm $1
	if [ $? == 1 ]
	then
		echo -e "${GREEN}[Passed] $1 ${CLEAR}"
	else
		echo -e "${RED}[FAILED] $1 ${CLEAR}"
	fi
}


echo "[Info] testing valid code"
test_program else_if.c 4
test_program if_one.c 1
test_program is_even_assign_in_statement.c 0
test_program is_even.c 0
echo

echo "[Info] testing invalid code"
test_invalid invalid-unmatched_else.c
