#pragma once
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <vector>

namespace boost
{
    namespace asio
    {
        class io_context;
    }
}

class ThreadPoolManager
{
private:
    ThreadPoolManager();
    ~ThreadPoolManager();
public:
    static ThreadPoolManager* GetInstance();
    static void Release();

    bool Start(boost::uint32_t threadsCount = 1);
    bool Stop();

    boost::asio::io_context& Get();

private:
    static ThreadPoolManager* m_threadPoolManager;
    boost::shared_ptr<boost::asio::io_context> m_ioContext;
    void* m_ioServiceWork;
    std::vector<boost::shared_ptr<boost::thread> > m_threads;
    std::size_t m_threadsPoolSize;
};