#pragma once
/*!
 * @file Log.h
 * @brief  »’÷æø‚
 * @details 
 * @mainpage
 * @author Moussa_Li
 * @version 1.0
 * @date 2025/09/27
 * @license GPLV2
 *
*/

#include "Common/CommonAPI.hpp"
#include <memory>

#define LOG(severity) COMPACT_IS_LOG_##severity.stream()

#define COMPACT_IS_LOG_INFO EgLab::Logger(__FILE__, __LINE__, EgLab::LogSeverity::LOG_INFO)
#define COMPACT_IS_LOG_WARNING EgLab::Logger(__FILE__, __LINE__, EgLab::LogSeverity::LOG_WARNING)
#define COMPACT_IS_LOG_ERROR EgLab::Logger(__FILE__, __LINE__, EgLab::LogSeverity::LOG_ERROR)
#define COMPACT_IS_LOG_FATAL EgLab::Logger(__FILE__, __LINE__, EgLab::LogSeverity::LOG_FATAL)
#define COMPACT_IS_LOG_EDEBUG EgLab::Logger(__FILE__, __LINE__, EgLab::LogSeverity::LOG_ENGINE_DEBUG)

#define ENABLE_LOG_FILE
#ifdef ENABLE_LOG_FILE
    #ifndef LOGFILE_PATH
        #define LOGFILE_PATH "D:/islog.txt"
    #endif
#endif // ENABLE_LOG_FILE

namespace EgLab
{

    enum class LogSeverity {
        LOG_INFO = 0,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL
    };


    /*!
     * \class Logger
     *
     * \brief log processer
     *
     * \author Moussa_Li
     * \date 2025/02/08
     */

    class CommonAPI Logger
    {
    public:
        Logger(const char* file, size_t line, LogSeverity severity);

        ~Logger() noexcept;

        std::ostream& stream();

    private:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

};