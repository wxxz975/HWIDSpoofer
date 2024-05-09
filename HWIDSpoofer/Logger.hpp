#include <ntifs.h>
#include <cstdarg>
#include <ntstrsafe.h>


// 在内核层面不能使用用户层API 或者标准库的函数
inline void log_(const char* file, int line, const char* fmt, ...)
{
    char buffer[256] = { 0 };
   
    NTSTATUS status = RtlStringCbPrintfA(buffer, sizeof(buffer), "[+][Log %s:%d] ", file, line);
    if (NT_SUCCESS(status))
    {
        va_list args;
        va_start(args, fmt);
        vDbgPrintExWithPrefix(buffer, 0, 0, fmt, args);
        va_end(args);
    } 
}

inline void error_(const char* file, int line, const char* fmt, ...)
{
    char buffer[256] = { 0 };

    NTSTATUS status = RtlStringCbPrintfA(buffer, sizeof(buffer), "[+][Error %s:%d] ", file, line);
    if (NT_SUCCESS(status))
    {
        va_list args;
        va_start(args, fmt);
        vDbgPrintExWithPrefix(buffer, 0, 0, fmt, args);
        va_end(args);
    }
}

#ifdef NO_OUTPUT // 控制是否打印输出
#define log(fmt, ...) 
#define err(fmt, ...)
#else
#define log(fmt, ...) log_(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define err(fmt, ...) error_(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

