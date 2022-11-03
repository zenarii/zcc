The grammer for the c compiler so far is:

```
<program>         ::= <function>
<function>        ::= "int" <id> "(" ")" "{" <statement> "}"
<statement>       ::= "return" <expression> ";"
<expression>      ::= <term> { ("+" | "-") <term> }
<term>            ::= <factor> { ("*" | "/") <term> }
<factor>          ::= "(" <expression> ")" | <unary_operator> <factor> | <int>

<binary_operator> ::= "+" | "-" | "*" | "/" | "%"
<unary_operator>  ::= "!" | "~" | "-"
```
factors are operands of unary operators
terms are the operands of binary operators
