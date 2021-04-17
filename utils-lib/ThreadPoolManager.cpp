#include "ThreadPoolManager.h"
#include <cmath>
#include <boost/asio/io_context.hpp>

ThreadPoolManager::ThreadPoolManager()
    : m_ioServiceWork(nullptr)
    , m_threadsPoolSize(0)
{
    m_ioContext.reset(new boost::asio::io_context);
}

ThreadPoolManager::~ThreadPoolManager()
{

}

ThreadPoolManager* ThreadPoolManager::GetInstance()
{
    if (!m_threadPoolManager)
    {
        m_threadPoolManager = new ThreadPoolManager;
    }
    return m_threadPoolManager;
}

void ThreadPoolManager::Release()
{
    if (m_threadPoolManager)
    {
        delete m_threadPoolManager;
        m_threadPoolManager = nullptr;
    }
}

bool ThreadPoolManager::Start( boost::uint32_t threadsCount)
{
    if (m_ioServiceWork) {
        return true; // already started
    }

    if (threadsCount == 0)
    {
        m_threadsPoolSize = 1;
    } else {
        m_threadsPoolSize = threadsCount;
    }
    m_ioServiceWork = new boost::asio::io_context::work(*m_ioContext);

    for (std::size_t i = 0; i < m_threadsPoolSize; i++)
    {
        boost::shared_ptr<boost::thread> thread(new boost::thread(
            boost::bind(&boost::asio::io_context::run,m_ioContext)));

        m_threads.push_back(thread);
    }

    return true;
}

bool ThreadPoolManager::Stop()
{
    m_ioContext->stop();

    if (m_ioServiceWork)
    {
        delete (boost::asio::io_context::work *) m_ioServiceWork;
        m_ioServiceWork = nullptr;
    }

    for (std::size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i]->join();
    }

    m_threads.clear();

    return true;
}

boost::asio::io_context& ThreadPoolManager::Get()
{
    return *m_ioContext;
}

ThreadPoolManager* ThreadPoolManager::m_threadPoolManager = nullptr;
