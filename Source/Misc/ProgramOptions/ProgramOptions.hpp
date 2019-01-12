#pragma once

#include "global.hpp"
#include "Engine/GAPI_enum.hpp"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
#include <variant>
#include <fstream>
#include <sstream>
#include <iterator>

#include <iostream>

namespace KompotEngine
{

class ProgramOptions
{
private:
    typedef std::variant<std::string, bool, Renderer::GAPI,
                        int8_t, int16_t, int32_t, int64_t,
                        uint8_t, uint16_t, uint32_t, uint64_t> Variant;

    typedef std::variant<std::string*, bool*, Renderer::GAPI*,
                        int8_t*, int16_t*, int32_t*, int64_t*,
                        uint8_t*, uint16_t*, uint32_t*, uint64_t*> PointerVariant;
    class Options
    {
    private:
        struct Option {
             std::string   key;
             std::string   description;
             Variant       value;
             mutable PointerVariant pointer;
             void notify() const;
             void set(std::stringstream&);

        private:
             template<typename T>
             void setByPointer(const Variant&, PointerVariant&) const;

             template<typename T>
             void setFromStream(std::stringstream&);
        };
        std::vector<Option> m_options;
    public:

    template<typename T>
    Options& operator() (const std::string &key, const std::string &description, const T& value, T *pointer)
    {
        m_options.push_back(Option{key, description, Variant(value), PointerVariant(pointer)});
        return *this;
    }

    bool contains(const std::string&) const;
    bool keyIsBoolean(const std::string&) const;
    void setByKeyFromStream(const std::string&, std::stringstream&);
    void notify() const;

    };
    Options m_options;
    void SetOptionsFromStringstream(std::stringstream&);
public:

    Options& addOptions()
    {
        return m_options;
    }

    void loadFromArguments(int, char**);
    void loadFromFile(std::ifstream&);

    void notify() const;
};

} // namespace KompotEngine
