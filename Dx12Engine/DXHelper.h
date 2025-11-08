#pragma once
#include <string>

//exe있는곳에서 Resources경로 찾기
inline void GetResourcesPath(_Out_writes_(pathSize) wchar_t* path, UINT pathSize)
{
    if (path == nullptr)
    {
        throw std::exception();
    }
    DWORD size = GetModuleFileName(nullptr, path, pathSize);

    if (size == 0 || size == pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }
    
    for (int i = 0; i < 3; i++) //3번 이전 폴더로
    {
        WCHAR* lastSlash = wcsrchr(path, L'\\');
        if (lastSlash)
        {
            *(lastSlash) = L'\0';
        }
    }

    wcscat_s(path, pathSize, L"\\Resources\\");

}

template<typename T>
void SafeDeleteArray(T** ptr) 
{
    if (*ptr)
    {
        delete[] *ptr;
        *ptr = nullptr;
    }
}

inline bool fgets_trim(char* buffer, size_t size, FILE* stream)
{
    if (!fgets(buffer, size, stream))
        return false;

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    return true;
}


inline void FindStr(char* startPlace, std::string& dest)
{
    char* start = strchr(startPlace, '"');
    if (start)
    {
        start++; // 큰따옴표 다음 문자

        char* end = strchr(start, '"');
        if (end)
        {
            dest.assign(start, end - start); // 문자열 구간 복사
        }
    }
}

inline bool checkTitle(char* line, const char* str, int& currentIndent)
{
    char* start = strstr(line, str);

    if (start) {
        currentIndent = start - line;
        return true;
    }
    return false;

}

inline char* findNthNumberStart(char* line, int n)
{
    char* ptr = line;
    int count = 0;

    while (*ptr != '\0') // 문자열 끝날 때까지
    {
        // 숫자나 '-'(음수 표시)가 나오면 숫자의 시작 가능성
        if (isdigit(*ptr) || *ptr == '-' || *ptr == '.')
        {
            // 앞이 공백이거나 문자열의 시작이라면 진짜 숫자 시작
            if (ptr == line || isspace(*(ptr - 1)))
            {
                count++;
                if (count == n)
                    return ptr; // n번째 숫자의 시작 주소 반환
            }
        }

        ptr++;
    }

    return nullptr; // 못 찾은 경우
}