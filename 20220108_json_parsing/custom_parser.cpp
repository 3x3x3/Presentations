// custom_parser.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "custom_parser.h"
#include <cctype>
#include <cassert>

JSONTokenizer1::JSONTokenizer1(std::string_view s): p(s.data()), end(s.data() + s.size()) {
    pop(); // 첫 번째 토큰을 찾는다
}

// 지금 맨 앞에 있는 토큰을 반환한다.
std::string_view JSONTokenizer1::front() {
    assert(p < end);
    return next_token;
}

// 다음 토큰을 찾아서 next_token 에 저장한다.
void JSONTokenizer1::pop() {
    // 1. 토큰 시작 위치 찾기: 경계 문자가 아닌 문자가 나올 때까지 p를 전진시킨다.
    while(p < end && (*p == '{' || *p == '[' || *p == '}' || *p == ']' ||
                                        *p == ':' || *p == ' ' || *p == '\t' || *p == ',')) {
        ++p;
    }

    // 문자열 끝에 도달했으면 더 이상 토큰이 없다.
    if (p >= end) { return; }

    char tmp = *p;

    // 2. 해당 토큰의 끝을 찾는다. 그러려면 토큰이 어떤 종류인지 알아야 한다.
    // 여기서는 문자열과 숫자만 지원한다. (true / false / null 등은 일단 무시)
    if (*p == '"') { // 따옴표로 시작하면 문자열
        // TODO escaped quote 파싱하던지 말던지 할것

        char* q = (char*)p + 1;
        // 문자열 끝을 찾는다
        while(*q != '"') ++q;
        *q = 0;
        next_token = std::string_view(p + 1, q - p - 1);
        
        // q가 문자열을 닫는 따옴표이니, 다음 칸부터 시작해서 다음 토큰을 찾는다
        p = q + 1;
    }
    else {
        
        // 문자열 아니면 실수일 것이다
        assert(std::isdigit(*p) || *p == '-');
        char* q = (char*)p + 1;
        while(std::isdigit(*q) || *q == '.' || *q == 'e' || *q == '+') ++q;
        *q = 0;
        next_token = std::string_view(p, q-p);
        p = q + 1;
    }
}
