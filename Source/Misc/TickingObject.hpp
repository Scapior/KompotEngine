#pragma once

#include <thread>
#include <chrono>
#include <atomic>

namespace KompotEngine
{

class TickingObject
{
public:
    TickingObject(const std::chrono::milliseconds&);
    ~TickingObject();
    void start();
    void stop();

private:
    std::chrono::milliseconds m_interval;
    std::atomic_bool m_isRunning;

    virtual void tick() = 0;
};


} // KompotEngine  namespace
