// custom_parser.h
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string_view>

//////////////////////////////////////////////////////////////////////////////////////////////////////

class JSONTokenizer {
public:
    JSONTokenizer(std::string_view s);

    // 토큰이 남았으면 true, 더 없으면 false
    operator bool() const { return p < end; }

    std::string_view front();
    void pop();

    std::string_view next_token; // 이번 토큰
	const char *p;					// 다음 토큰을 찾기 시작할 위치
	const char *end;				// end of buffer
};
