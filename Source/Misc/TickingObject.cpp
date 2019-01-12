#include "TickingObject.hpp"

using namespace KompotEngine;

TickingObject::TickingObject(const std::chrono::milliseconds &interval)
    : m_interval(interval)
{
    m_isRunning = false;
}
TickingObject::~TickingObject() {}

void TickingObject::start()
{
    if (m_isRunning)
    {
        return;
    }
    m_isRunning = true;
    std::thread( [this] () {
        while (m_isRunning)
        {
            const auto callTime = std::chrono::high_resolution_clock::now();
            //this->tick();
            const auto timeNow = std::chrono::high_resolution_clock::now();
            auto workDuration = timeNow - callTime;
            if (workDuration < m_interval)
            {
                auto sleepTime = m_interval - workDuration;
                std::this_thread::sleep_for(sleepTime);
            }
        }
    }).detach();
}

void TickingObject::stop()
{
    m_isRunning = false;
}
