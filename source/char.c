int CharIsLetter(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

int CharIsDigit(char c) {
    return ('0' <= c && c <= '9');
}

int CharIsWhiteSpace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

int StringCompareN(char * a, char * b, int n) {
    int match = 1;
    for(int i = 0; i < n; ++i) {
        if(a[i] != b[i]) {
            match = 0;
            break;
        }
    }
    return match;
}