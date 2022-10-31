int CharIsLetter(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

int CharIsDigit(char c) {
    return ('0' <= c && c <= '9');
}

int CharIsWhiteSpace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

#if 0

StringMatch() {
    
}
#endif