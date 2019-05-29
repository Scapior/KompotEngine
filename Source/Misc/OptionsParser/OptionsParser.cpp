#include "OptionsParser.hpp"

template<typename T>
void OptionsParser::Options::Option::setByPointer(const Variant &variant, PointerVariant &pointerVariant)
{
    T &reference = *std::get<T*>(pointerVariant);
    reference = std::get<T>(variant);
}

void OptionsParser::Options::Option::notify() const
{
    std::visit(visitorPointer{}, value, pointer);
}

template<typename T>
void OptionsParser::Options::Option::setFromStream(Variant &variant, const T &value)
{
    variant = value;
}

void OptionsParser::Options::Option::set(std::stringstream &optionsStream)
{
    std::visit(visitorStream{optionsStream, value}, value);
}

void OptionsParser::Options::notify() const
{
    for (const auto& option : m_options)
    {

        option.notify();
    }
}

bool OptionsParser::Options::contains(const std::string &key) const
{
    for (const auto& option : m_options)
    {
        if (caseInsensitiveStringEquals(option.key, key))
        {
            return true;
        }
    }
    return false;
}

void  OptionsParser::Options::setByKeyFromStream(const std::string &key, std::stringstream &optionsStream)
{
    for (auto& option : m_options)
    {
        if (caseInsensitiveStringEquals(option.key, key))
        {
            option.set(optionsStream);
            return;
        }
    }
}

bool OptionsParser::Options::keyIsBoolean(const std::string &key) const
{
    for (const auto& option : m_options)
    {
        if (caseInsensitiveStringEquals(option.key, key))
        {
            return std::holds_alternative<bool>(option.value);
        }
    }
    return false;
}

void OptionsParser::loadFromArguments(int argc, char **argv)
{
    std::stringstream argumentStream;
    for (int i = 1; i < argc; ++i)
    {
        const bool isLastArgument = i+1 >= argc;
        const bool nextArgumentIsKey = isLastArgument ? false : std::string(argv[i+1]).find("--") == 0;

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
            if (isLastArgument || nextArgumentIsKey)
            {
                argumentStream << argument << ' ' << true << ' ';
            }
            else
            {
                const std::string nextArgument(argv[++i]);

                if (caseInsensitiveStringEquals(nextArgument, "1") || caseInsensitiveStringEquals(nextArgument, "true"))
                {
                    argumentStream << argument << ' ' << true << ' ';
                }
                else if (caseInsensitiveStringEquals(nextArgument, "0") || caseInsensitiveStringEquals(nextArgument, "false"))
                {
                    argumentStream << argument << ' ' << false << ' ';
                }
            }
            continue;
        }
        if (isLastArgument)
        {
            continue;
        }
        const std::string argumentValue(argv[++i]);
        argumentStream << argument << ' ' << argumentValue << ' ';
    }
    SetOptionsFromStringstream(argumentStream);
}

void OptionsParser::loadFromFile(std::ifstream& inputStream)
{
    std::stringstream optionsStream{};
    while (!inputStream.eof())
    {
        std::string buffer;
        std::getline(inputStream, buffer);
        const  auto equalsSignPosition = buffer.find_first_of("=");
        if (equalsSignPosition == std::string::npos)
        {
            continue;
        }
        const std::string key = trim(buffer.substr(0u, equalsSignPosition));
        const std::string value = trim(buffer.substr(equalsSignPosition + 1u));

        if (!m_options.contains(key))
        {
            continue;
        }
        if (m_options.keyIsBoolean(key))
        {
            if (caseInsensitiveStringEquals(value, "1") || caseInsensitiveStringEquals(value, "true"))
            {
                optionsStream << key << ' ' << true << ' ';
            }
            else if (caseInsensitiveStringEquals(value, "0") || caseInsensitiveStringEquals(value, "false"))
            {
                optionsStream << key << ' ' << false << ' ';
            }
        }
        else
        {
            optionsStream << key << ' ' << value << ' ';
        }
    }
    SetOptionsFromStringstream(optionsStream);
}

void OptionsParser::SetOptionsFromStringstream(std::stringstream &optionsStream)
{
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

void OptionsParser::notify() const
{
    m_options.notify();
}


bool OptionsParser::compareChar(char c1, char c2)
{
    if (c1 == c2)
        return true;
    else if (std::toupper(c1) == std::toupper(c2))
        return true;
    return false;
}

bool OptionsParser::caseInsensitiveStringEquals(const std::string & stringLeft, const std::string &stringRight)
{
    return ( (stringLeft.size() == stringRight.size() ) &&
             std::equal(stringLeft.begin(), stringLeft.end(), stringRight.begin(), &compareChar) );
}

std::string OptionsParser::trim(const std::string &text)
{
    const auto leftSpacePosition = text.find_first_not_of(' ');
    auto result = text.substr(leftSpacePosition);
    const auto rightSpacePosition = text.find_last_not_of(' ');
    result = text.substr(0_u64t, rightSpacePosition + 1_u64t);
    return result;
}
