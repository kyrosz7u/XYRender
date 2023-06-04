//
// Created by kyrosz7u on 2023/5/16.
//

#ifndef XEXAMPLE_LOGGER_H
#define XEXAMPLE_LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "core/singleton_decorator.h"

class Logger : public SingletonDecorator<Logger> {
    friend class SingletonDecorator<Logger>;

public:
    enum class LogLevel : uint8_t {
        debug,
        info,
        warn,
        error,
        fatal
    };

    template<typename... Targs>
    void logger(LogLevel level, Targs &&... args) {
        switch (level) {
            case LogLevel::debug:
                m_logger->debug(std::forward<Targs>(args)...);
                break;
            case LogLevel::info:
                m_logger->info(std::forward<Targs>(args)...);
                break;
            case LogLevel::warn:
                m_logger->warn(std::forward<Targs>(args)...);
                break;
            case LogLevel::error:
                m_logger->error(std::forward<Targs>(args)...);
                break;
            case LogLevel::fatal:
                m_logger->critical(std::forward<Targs>(args)...);
                break;
            default:
                break;
        }
    }

protected:
    Logger() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%^%l%$] %v");

        spdlog::init_thread_pool(8192, 1);
        const spdlog::sinks_init_list sink_list = {console_sink};
        m_logger = std::make_shared<spdlog::async_logger>("async_logger",
                                                          sink_list.begin(),
                                                          sink_list.end(),
                                                          spdlog::thread_pool(),
                                                          spdlog::async_overflow_policy::block);
# ifdef _DEBUG
        m_logger->set_level(spdlog::level::trace);
# else
        m_instance->m_logger->set_level(spdlog::level::info);
# endif
        spdlog::register_logger(m_logger);
    }

    ~Logger() {
        m_logger->flush();
        spdlog::drop_all();
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

#endif //XEXAMPLE_LOGGER_H
