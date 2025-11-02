#pragma once


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