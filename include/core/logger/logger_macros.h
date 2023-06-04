//
// Created by kyrosz7u on 2023/5/16.
//

#ifndef XEXAMPLE_MACROS_H
#define XEXAMPLE_MACROS_H

#include "logger.h"

#define LOG_HELPER(LOG_LEVEL, ...) \
    Logger::Instance().logger(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_DEBUG(...) LOG_HELPER(Logger::LogLevel::debug, __VA_ARGS__);

#define LOG_INFO(...) LOG_HELPER(Logger::LogLevel::info, __VA_ARGS__);

#define LOG_WARN(...) LOG_HELPER(Logger::LogLevel::warn, __VA_ARGS__);

#define LOG_ERROR(...) LOG_HELPER(Logger::LogLevel::error, __VA_ARGS__);

#define LOG_FATAL(...) LOG_HELPER(Logger::LogLevel::fatal, __VA_ARGS__);

#ifdef _DEBUG
#define VK_CHECK_RESULT(f)                                                                                \
{                                                                                                        \
    VkResult res = (f);                                                                                    \
    if (res != VK_SUCCESS)                                                                                \
    {                                                                                                    \
        LOG_FATAL("VkResult is {}",VulkanUtil::errorString(res));                          \
        assert(res == VK_SUCCESS);                                                                        \
    }                                                                                                    \
}
#else
#define VK_CHECK_RESULT(f) {f;}
#endif

#endif //XEXAMPLE_MACROS_H
