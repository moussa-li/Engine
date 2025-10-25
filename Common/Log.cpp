#include "Common/Log.hpp"
#include "Common/Clock.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <thread>
#include <fstream>
#include <filesystem>

#include <utility>

namespace EgLab
{
    constexpr size_t LOG_MSG_MAX_LEN = 30000;
    // Define Const Color
    const std::string RESET     = "\033[0m"; 
    const std::string RED       = "\033[31m";   
    const std::string GREEN     = "\033[32m"; 
    const std::string YELLOW    = "\033[33m"; 
    const std::string BLUE      = "\033[34m";   
    const std::string MAGENTA   = "\033[35m"; 
    const std::string CYAN      = "\033[36m";    
    const std::string WHITE     = "\033[37m";   

    class LogStreamBuf : public std::streambuf
    {
    public:
        LogStreamBuf(char* buffer, size_t length)
        {
            setp(buffer, buffer + length - 2);
        }

        int_type overflow(int_type ch) { return ch; }

        size_t pcount() const { return static_cast<size_t>(pptr() - pbase()); }

        char* pbase() const { return std::streambuf::pbase(); }
    };

    class LogStream : public std::ostream
    {
    public:
        LogStream(char* buffer, size_t length, int64_t counter)
            : std::ostream(nullptr), _buffer(buffer, length), _counter(counter)
        {
            memset(buffer, 0, length);
            rdbuf(&_buffer);
        }

        LogStream(LogStream&& other)noexcept
            :std::ostream(nullptr), _buffer(std::move(other._buffer)),_counter(std::exchange(other._counter,0))
        {
            rdbuf(&_buffer);
        }

        LogStream& operator=(LogStream&& other) noexcept {
            _buffer = std::move(other._buffer);
            _counter = std::exchange(other._counter, 0);
            rdbuf(&_buffer);
            return *this;
        }

        size_t pcount() const { return _buffer.pcount(); }
        char* pbase() const { return _buffer.pbase(); }
        char* str() const { return pbase(); }
    
        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;

    private:
        LogStreamBuf _buffer;
        int64_t _counter;
    };



    class Logger::Impl
    {
    public:
        Impl(
            const char* file,
            size_t line,
            LogSeverity severity)
            :_stream(LogStream(_content, LOG_MSG_MAX_LEN, 0)),
            _fullname(file),
            _basename(baseName(file)),
            _line(line),
            _severity(severity),
            _threadId(std::this_thread::get_id())
        {
        }

        void output()
        {
            LogSeverity level = _currentLevel;
            if (level > _severity)
            {
                _activate = false;
                return;
            }
            /*
            if (Configure::getInstance()->isLogFilter(_severity))
            {
                _activate = false;
                return;
            }
                */
            switch (_severity)
            {
            case LogSeverity::LOG_INFO:
                _stream << "[  INFO  ] ";
                break;
            case LogSeverity::LOG_WARNING:
                _stream << YELLOW;
                _stream << "[ WARNING ]";
                _stream << RESET;
                break;
            case LogSeverity::LOG_ERROR:
                _stream << RED;
                _stream << "[  ERROR  ]";
                _stream << RESET;
                break;
            case LogSeverity::LOG_FATAL:
                _stream << MAGENTA;
                _stream << "[  FATAL  ]";
                _stream << RESET;
                break;
            default:
                _stream << "[LOG_ERROR_Severity]";
            }

            int hour, minute, second;
            Clock::time(hour, minute, second);

#if 0
            _stream << '['<< std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0')<< std::setw(2) << minute << ':' << std::setfill('0')<< std::setw(2) << second 
                << ' ' << _threadId << " " << _basename << ':' << _line
                << ']';
#else
            _stream << '['<< std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0')<< std::setw(2) << minute << ':' << std::setfill('0')<< std::setw(2) << second 
                << ' ' << _threadId
                << ']';
#endif
            
        }

        void flush()
        {
            if (_activate == false)
                return;
            _stream << "\r\n";
            std::cout << _content;

#ifdef ENABLE_LOG_FILE
            std::filesystem::path dirPath(LOGFILE_PATH);
            if (!std::filesystem::exists(dirPath.parent_path()))
            {
                std::filesystem::create_directories(dirPath.parent_path());
            }
            std::ofstream logFile(LOGFILE_PATH, std::ios::app);
            if (logFile.is_open())
            {
                logFile << _content;
                logFile.close();
            }
            else
            {
                std::cerr << "cannot open log file" << std::endl;
            }
#endif
        }


        const char* baseName(const char* filepath)
        {
            const char* base = strrchr(filepath, '/');
#ifdef _WIN32  // Look for either path separator in Windows
            if (!base) base = strrchr(filepath, '\\');
#endif // _WIN32
            return base ? (base + 1) : filepath;
        }

        LogStream _stream;
        LogSeverity _severity;
        const char* _basename;
        const char* _fullname;
        size_t _line;
        std::thread::id _threadId;
        char _content[LOG_MSG_MAX_LEN + 1]{};
        bool _activate{ true };
        static LogSeverity _currentLevel;
        //static LogSeverity _currentFilter; TODO: add Log Filter

    };

    LogSeverity Logger::Impl::_currentLevel = LogSeverity::LOG_INFO;

    Logger::Logger(const char* file, size_t line, LogSeverity severity)
        : _impl(std::make_unique<Impl>(file,line,severity))
    {
        _impl->output();
    }

    Logger::~Logger() noexcept
    {
        _impl->flush();
    }

    std::ostream& Logger::stream()
    {
        return _impl->_stream;
    }

}

