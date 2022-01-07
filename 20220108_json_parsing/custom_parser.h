// custom_parser.h
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string_view>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////

struct StrPos {
    int begin;
    int end;
};

class JSONTokenizer {
public:
    JSONTokenizer(std::string_view s);

    // 토큰이 남았으면 true, 더 없으면 false
    operator bool() const { return p < end; }

    std::string_view front();
    void pop();

protected:
    std::string_view next_token; // 이번 토큰
	const char *p;					// 다음 토큰을 찾기 시작할 위치
	const char *end;				// end of buffer
};

// Grossly unsafe and nonstandard, but fast JSON parser
class JSONParser {
public:
    JSONParser(std::string_view json_str);

    // starts from ix, consumes a value. returns false when it meets a closing delimiter or end of string.
    bool consume(int& ix);

    StrPos& find_value_pos(const int& st, std::string_view key);
    StrPos& find_value_pos(const int& st, const int& idx);
    double find_value(const int& st, std::string_view key);

    std::string_view buf;

    // result of initial linear scan: p[ix] tell you the result of consume() function call when starting with buf[ix]
    std::vector<StrPos> p;
};
