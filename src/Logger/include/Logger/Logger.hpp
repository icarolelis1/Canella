#ifndef CANELLA_LOGGER
#define CANELLA_LOGGER
#include <stdio.h>
#include <mutex>
#ifdef WIN32
#include <windows.h>
// Define extra colours
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_CYAN (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_BLACK 0

#define FOREGROUND_INTENSE_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_BLUE (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_WHITE (FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_YELLOW (FOREGROUND_YELLOW | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_CYAN (FOREGROUND_CYAN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_MAGENTA (FOREGROUND_MAGENTA | FOREGROUND_INTENSITY)

#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN)
#define BACKGROUND_YELLOW (BACKGROUND_RED | BACKGROUND_GREEN)
#define BACKGROUND_CYAN (BACKGROUND_BLUE | BACKGROUND_GREEN)
#define BACKGROUND_MAGENTA (BACKGROUND_RED | BACKGROUND_BLUE)
#define BACKGROUND_BLACK 0

#define BACKGROUND_INTENSE_RED (BACKGROUND_RED | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_GREEN (BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_BLUE (BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_WHITE (BACKGROUND_WHITE | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_YELLOW (BACKGROUND_YELLOW | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_CYAN (BACKGROUND_CYAN | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_MAGENTA (BACKGROUND_MAGENTA | BACKGROUND_INTENSITY)
#endif

#ifdef LINUX
// the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */
#endif

namespace Engine
{
    class Logger
    {
        enum Priority
        {
            Info_LOG,
            Warn_LOG,
            Trace_LOG,
            Debug_LOG,
            Error_LOG,
            Critial_LOG
        };

    public:
        static std::mutex logger_mutex;
        static Priority log_priority;
#ifdef WIN32
        template <typename... Args>
        static void Trace(const char *message, Args... args)
        {
            if (log_priority >= Priority::Trace_LOG)
            {
                std::lock_guard<std::mutex> lock(logger_mutex);
                HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(h, FOREGROUND_CYAN);

                printf("[Trace]\t");
                printf(message, args...);
                printf("\n");
            }
        };

        template <typename... Args>
        static void Info(const char *message, Args... args)
        {
            if (log_priority >= Priority::Info_LOG)
            {
                std::lock_guard<std::mutex> lock(logger_mutex);
                HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(h, FOREGROUND_INTENSE_BLUE);

                printf("[Info]\t");
                printf(message, args...);
                printf("\n");
            }
        };

        template <typename... Args>
        static void Error(const char *message, Args... args)
        {
            if (log_priority >= Priority::Error_LOG)
            {
                std::lock_guard<std::mutex> lock(logger_mutex);
                HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(h, FOREGROUND_INTENSE_RED);

                printf("[Error]\t");
                printf(message, args...);
                printf("\n");
            }
        };

        template <typename... Args>
        static void Debug(const char *message, Args... args)
        {
            if (log_priority >= Priority::Debug_LOG)
            {
                std::lock_guard<std::mutex> lock(logger_mutex);
                HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(h, FOREGROUND_INTENSE_GREEN);

                printf("[Debug]\t");
                printf(message, args...);
                printf("\n");
            }
        };

        template <typename... Args>
        static void Warn(const char *message, Args... args)
        {
            if (log_priority >= Priority::Warn_LOG)
            {
                std::lock_guard<std::mutex> lock(logger_mutex);
                HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(h, FOREGROUND_YELLOW);

                printf("[Warn]\t");
                printf(message, args...);
                printf("\n");
            }
        };

#else
        template <typename... Args>
        static void Trace(const char *message, Args... args){
            // if (log_priority <= Priority::Trace_LOG)
            // {
            //     std::lock_guard<std::mutex> lock(logger_mutex);
            //     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            //     SetConsoleTextAttribute(h, 1);
            //     printf("[not defined]", args...);
            //     printf("\n");
            //     SetConsoleTextAttribute(h, 1);
            // }
        };
             template <typename... Args>
        static void Info(const char *message, Args... args){
            // if (log_priority <= Priority::Trace_LOG)
            // {
            //     std::lock_guard<std::mutex> lock(logger_mutex);
            //     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            //     SetConsoleTextAttribute(h, 1);
            //     printf("[not defined]", args...);
            //     printf("\n");
            //     SetConsoleTextAttribute(h, 1);
            // }
        };
             template <typename... Args>
        static void Error(const char *message, Args... args){
            // if (log_priority <= Priority::Trace_LOG)
            // {
            //     std::lock_guard<std::mutex> lock(logger_mutex);
            //     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            //     SetConsoleTextAttribute(h, 1);
            //     printf("[not defined]", args...);
            //     printf("\n");
            //     SetConsoleTextAttribute(h, 1);
            // }
        };
             template <typename... Args>
        static void Debug(const char *message, Args... args){
            // if (log_priority <= Priority::Trace_LOG)
            // {
            //     std::lock_guard<std::mutex> lock(logger_mutex);
            //     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            //     SetConsoleTextAttribute(h, 1);
            //     printf("[not defined]", args...);
            //     printf("\n");
            //     SetConsoleTextAttribute(h, 1);
            // }
        };
             template <typename... Args>
        static void Warn(const char *message, Args... args){
            // if (log_priority <= Priority::Trace_LOG)
            // {
            //     std::lock_guard<std::mutex> lock(logger_mutex);
            //     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            //     SetConsoleTextAttribute(h, 1);
            //     printf("[not defined]", args...);
            //     printf("\n");
            //     SetConsoleTextAttribute(h, 1);
            // }
        };

#endif
    };



}
#endif