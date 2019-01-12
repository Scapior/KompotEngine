#include "ProgramOptions.hpp"

using namespace KompotEngine;

template<typename T>
void ProgramOptions::Options::Option::setByPointer(const Variant &variant, PointerVariant &pointerVariant) const
{
    T &reference = *std::get<T*>(pointerVariant);
    reference = std::get<T>(variant);
}

template<typename T>
void ProgramOptions::Options::Option::setFromStream(std::stringstream &optionsStream)
{
    std::cout << "Stringstream position before setting:" << optionsStream.tellg() << std::endl;
    T valueBuffer;
    optionsStream >> valueBuffer;
    value = valueBuffer;
    std::cout << "Setted variant key '" << key << "' with value [" << valueBuffer << ']' << std::endl;
    std::cout << "Stringstream position after setting:" << optionsStream.tellg() << std::endl;
}

void ProgramOptions::Options::Option::notify() const
{   
    if (std::holds_alternative<std::string>(value))
    {
        setByPointer<std::string>(value, pointer);
    }
    else if(std::holds_alternative<bool>(value))
    {
        setByPointer<bool>(value, pointer);
    }
    else if(std::holds_alternative<Renderer::GAPI>(value))
    {
        setByPointer<Renderer::GAPI>(value, pointer);
    }
    else if(std::holds_alternative<int8_t>(value))
    {
        setByPointer<int8_t>(value, pointer);
    }
    else if(std::holds_alternative<int16_t>(value))
    {
        setByPointer<int16_t>(value, pointer);
    }
    else if(std::holds_alternative<int32_t>(value))
    {
        setByPointer<int32_t>(value, pointer);
    }
    else if(std::holds_alternative<int64_t>(value))
    {
        setByPointer<int64_t>(value, pointer);
    }
    else if(std::holds_alternative<uint8_t>(value))
    {
        setByPointer<uint8_t>(value, pointer);
    }
    else if(std::holds_alternative<uint16_t>(value))
    {
        setByPointer<uint16_t>(value, pointer);
    }
    else if(std::holds_alternative<uint32_t>(value))
    {
        setByPointer<uint32_t>(value, pointer);
    }
    else if(std::holds_alternative<uint64_t>(value))
    {
        setByPointer<uint64_t>(value, pointer);
    }
}

void ProgramOptions::Options::Option::set(std::stringstream &optionsStream)
{
    if (std::holds_alternative<std::string>(value))
    {
        setFromStream<std::string>(optionsStream);
    }
    else if(std::holds_alternative<bool>(value))
    {
        setFromStream<bool>(optionsStream);
    }
    else if(std::holds_alternative<Renderer::GAPI>(value))
    {
        setFromStream<Renderer::GAPI>(optionsStream);
    }
    else if(std::holds_alternative<int8_t>(value))
    {
        setFromStream<int8_t>(optionsStream);
    }
    else if(std::holds_alternative<int16_t>(value))
    {
        setFromStream<int16_t>(optionsStream);
    }
    else if(std::holds_alternative<int32_t>(value))
    {
        setFromStream<int32_t>(optionsStream);
    }
    else if(std::holds_alternative<int64_t>(value))
    {
        setFromStream<int64_t>(optionsStream);
    }
    else if(std::holds_alternative<uint8_t>(value))
    {
        setFromStream<uint8_t>(optionsStream);
    }
    else if(std::holds_alternative<uint16_t>(value))
    {
        setFromStream<uint16_t>(optionsStream);
    }
    else if(std::holds_alternative<uint32_t>(value))
    {
        setFromStream<uint32_t>(optionsStream);
    }
    else if(std::holds_alternative<uint64_t>(value))
    {
        setFromStream<uint64_t>(optionsStream);
    }
}

void ProgramOptions::Options::notify() const
{
    for (const auto& option : m_options)
    {
        option.notify();
    }
}

bool ProgramOptions::Options::contains(const std::string &key) const
{
    for (const auto& option : m_options)
    {
        if (boost::iequals(option.key, key))
        {
            return true;
        }
    }
    return false;
}

void  ProgramOptions::Options::setByKeyFromStream(const std::string &key, std::stringstream &optionsStream)
{
    for (auto& option : m_options)
    {
        if (boost::iequals(option.key, key))
        {
            option.set(optionsStream);
            return;
        }
    }
}

bool ProgramOptions::Options::keyIsBoolean(const std::string &key) const
{
    for (const auto& option : m_options)
    {
        if (boost::iequals(option.key, key))
        {
            return std::holds_alternative<bool>(option.value);
        }
    }
    return false;
}

void ProgramOptions::loadFromArguments(int argc, char **argv)
{
    std::stringstream argumentStream;
    for (int i = 1; i < argc; ++i)
    {
        std::string argument(argv[i]);
        if (argument.find("--") != 0)
        {
            continue;
        }
        argument = argument.substr(2u);
        if (!m_options.contains(argument))
        {
            continue;
        }
        if (m_options.keyIsBoolean(argument))
        {
            argumentStream << argument << ' ' << true << ' ';
            continue;
        }
        if (++i >= argc)
        {
            continue;
        }
        const std::string argumentValue(argv[i]);
        argumentStream << argument << ' ' << argumentValue << ' ';
    }
    SetOptionsFromStringstream(argumentStream);
}

void ProgramOptions::loadFromFile(std::ifstream& inputStream)
{
    std::stringstream optionsStream;
    while (!inputStream.eof())
    {
        std::string buffer;
        std::getline(inputStream, buffer);
        const  auto equalsSignPosition = buffer.find_first_of("=");
        if (equalsSignPosition == std::string::npos)
        {
            continue;
        }
        const std::string key = buffer.substr(0u, equalsSignPosition);
        if (!m_options.contains(key))
        {
            continue;
        }
        const std::string value = buffer.substr(equalsSignPosition + 1u);
        optionsStream << key << ' ' << value << ' ';
    }
    SetOptionsFromStringstream(optionsStream);
}

void ProgramOptions::SetOptionsFromStringstream(std::stringstream &optionsStream)
{
    std::cout << optionsStream.str() << std::endl;
    while (!optionsStream.eof())
    {
        std::string key;
        optionsStream >> key;
        if (key.length() == 0ull)
        {
            break;
        }
        m_options.setByKeyFromStream(key, optionsStream);
    }
}

void ProgramOptions::notify() const
{
    m_options.notify();
}
