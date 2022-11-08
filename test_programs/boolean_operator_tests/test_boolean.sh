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
test_program and_one.c 1
test_program and_zero.c 0
test_program eq_false.c 0
test_program eq_true.c 1
test_program eq_false.c 0
test_program ge_equal.c 1
test_program ge_true.c 1
test_program ge_false.c 0
test_program leq_equal.c 1
test_program leq_true.c 1
test_program leq_false.c 0
test_program neq_true.c 1
test_program neq_false.c 0
test_program or_one.c 1
test_program or_zero.c 1
test_program precedence_1.c 1
test_program precedence_2.c 0
test_program precedence_3.c 0
test_program precedence_4.c 1

echo

echo "[Info] testing invalid code"

echo -e "${RED}[TODO]: test short circuiting with variables once added${CLEAR}"
