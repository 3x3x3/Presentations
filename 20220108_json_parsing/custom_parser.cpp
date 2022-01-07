// custom_parser.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "custom_parser.h"
#include <cctype>
#include <cassert>
#include <string>

JSONTokenizer::JSONTokenizer(std::string_view s): p(s.data()), end(s.data() + s.size()) {
    pop(); // 첫 번째 토큰을 찾는다
}

// 지금 맨 앞에 있는 토큰을 반환한다.
std::string_view JSONTokenizer::front() {
    assert(p < end);
    return next_token;
}

// 다음 토큰을 찾아서 next_token 에 저장한다.
void JSONTokenizer::pop() {
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

JSONParser::JSONParser(std::string_view json_str): p(json_str.size()), buf(json_str) {

}

// starts from ix, consumes a value. returns false when it meets a closing delimiter or end of string.
bool JSONParser::consume(int& ix) {
    int start = ix;
    int begin;
    while(std::isspace(buf[ix]) || buf[ix] == ':' || buf[ix] == ',') ++ix;
    assert(buf[ix] != '\0'); // buf did not prematurely end

    switch(buf[ix]) {
        case ']': // fall through
        case '}':
            ix++; // we still consume ending delimiters!
            begin = -1;
            break;
        case '"':
            begin = ix++;
            while(buf[ix] != '"') ++ix;
            ++ix; // consume the closing quote
            break;
        case '[':
            begin = ix++;
            while(consume(ix));
            // closing delimiter is already consumed
            break;
        case '{':
            begin = ix++;
            while(consume(ix)) {} // value always comes in pairs in objects
            // closing delimiter is already consumed
            break;
        case 't': // true
            begin = ix;
            ix += 4;
            break;
        case 'f': // false
            begin = ix;
            ix += 5;
            break;
        default:
            begin = ix;
            while(std::isdigit(buf[ix]) || buf[ix] == 'e' || buf[ix] == 'E' || buf[ix] == '-' || buf[ix] == '+' || buf[ix] == '.') { ++ix; }
            break;
    }

    p[start].begin = begin;
    p[start].end = ix;

    return begin >= 0;
}

StrPos& JSONParser::find_value_pos(const int& st, std::string_view key) {
    StrPos& cur = p[st];

    while (0 < cur.begin) {
        if ( buf[cur.begin] == '"' && buf[cur.begin+1] == key[0] && buf[cur.begin+2] == key[1] ) {
            cur = p[cur.end];
            break;
        }
        else {
            cur = p[cur.end];
        }
    }

    return cur;
}

StrPos& JSONParser::find_value_pos(const int& st, const int& idx) {
    StrPos& cur = p[st];

    for ( int i=0 ; i<idx ; i++ ) {
        cur = p[cur.end];
    }
    
    return cur;
}

double JSONParser::find_value(const int& st, std::string_view key) {
    StrPos& cur = find_value_pos(st, key);
    std::string tmp = std::string(&buf[cur.begin], cur.end-cur.begin);
    return atof(tmp.c_str());
}
