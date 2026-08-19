/**
 * @file Thread.hpp
 * @author Moussa-Li
 * @brief wrapper the std::thread.
 * @date 2026-01-16
 */

#include <thread>

namespace EgLab::Common
{
    class Thread
    {
    public:
        template <class Function, class... Args>
        explicit Thread(Function &&func, Args &&...args)
            : _thread(std::forward<Function>(func), std::forward<Args>(args)...)
        {
        }

        ~Thread()
        {
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        void join()
        {
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

    private:
        std::thread _thread;
    };

} // namespace EgLab::Common