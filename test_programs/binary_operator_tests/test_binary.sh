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
test_program associativity.c 2
test_program associativity_2.c 1
test_program div.c 41
test_program div_neg.c 254
test_program multiply.c 12
test_program parenthesis.c 12
test_program plus.c 3
test_program precedence.c 24
test_program sub.c 1
test_program sub_2.c 255
test_program sub_neg.c 4
test_program unary_operator_1.c 1
test_program unary_operator_2.c 248
test_program unary_operator_3.c 207

echo

echo "[Info] testing invalid code"
test_invalid invalid-missing_second_operand.c
test_invalid invalid-missing_first_operand.c
test_invalid invalid-bad_parenthesis.c
test_invalid invalid-no_semicolon.c
