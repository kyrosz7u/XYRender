//
// Created by kyrosz7u on 2023/5/16.
//

#ifndef XEXAMPLE_LOGGER_H
#define XEXAMPLE_LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Logger
{
public:
    enum class LogLevel : uint8_t
    {
        debug,
        info,
        warn,
        error,
        fatal
    };
    static Logger* Instance()
    {
        if (!m_instance)
        {
            m_instance = new Logger();
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%^%l%$] %v");

            spdlog::init_thread_pool(8192, 1);
            const spdlog::sinks_init_list sink_list = {console_sink};
            m_instance->m_logger= std::make_shared<spdlog::async_logger>("async_logger",
                                                                         sink_list.begin(),
                                                                         sink_list.end(),
                                                                         spdlog::thread_pool(),
                                                                         spdlog::async_overflow_policy::block);
            m_instance->m_logger->set_level(spdlog::level::trace);
            spdlog::register_logger(m_instance->m_logger);
        }

        return m_instance;
    }

    template<typename... Targs>
    void logger(LogLevel level, Targs&&... args)
    {
        switch (level)
        {
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

    ~Logger()
    {
        if(m_instance)
        {
            m_instance->m_logger->flush();
            spdlog::drop_all();
//            delete m_instance;
        }
    }

private:
    Logger()
    {}
    static Logger *m_instance;
    std::shared_ptr<spdlog::logger> m_logger;
};

#endif //XEXAMPLE_LOGGER_H
