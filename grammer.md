The grammer for the c compiler so far is:

```
<program>        ::= <function>
<function>       ::= "int" <id> "(" ")" "{" <statement> "}"
<statement>      ::= "return" <expression> ";"
<expression>     ::= <unary_operator> <expression> | <int>
<unary_operator> ::= "!" | "~" | "-"
```