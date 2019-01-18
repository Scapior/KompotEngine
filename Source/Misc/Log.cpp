#include "Log.hpp"

void Log::callbackForGlfw(int code, const char* text)
{
    Log &log = Log::getInstance();
    log << "GLFW error: code [" << code << "], text: " << text << std::endl;
}
