#pragma once

#include "global.hpp"
#include <string>
#include <variant>
#include <fstream>
#include <sstream>
#include <cctype>
#include <typeinfo>

class OptionsParser
{
private:
    class Options;
public:
    Options& addOptions()
    {
        return m_options;
    }

    void loadFromArguments(int, char**);
    void loadFromFile(std::ifstream&);

    void notify() const;

    static bool compareChar(char, char);
    static bool caseInsensitiveStringEquals(const std::string&, const std::string&);
    static std::string trim(const std::string&);

private:
    typedef std::variant<std::string, bool,
                        int8_t, int16_t, int32_t, int64_t,
                        uint8_t, uint16_t, uint32_t, uint64_t> Variant;

    typedef std::variant<std::string*, bool*,
                        int8_t*, int16_t*, int32_t*, int64_t*,
                        uint8_t*, uint16_t*, uint32_t*, uint64_t*> PointerVariant;
    class Options
    {
    public:
        template<typename T>
        Options& operator() (const std::string& key, const std::string& description, const T& value, T* pointer)
        {
            m_options.push_back(Option{key, description, Variant(value), PointerVariant(pointer)});
            return *this;
        }

        bool contains(const std::string&) const;
        bool keyIsBoolean(const std::string&) const;
        void setByKeyFromStream(const std::string&, std::stringstream&);
        void notify() const;
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
             static void setByPointer(const Variant& variant, PointerVariant& pointerVariant);

             template<typename T>
             static void setFromStream(Variant& , const T&);

             struct visitorPointer
             {
                 template< typename T >
                 void operator() ( const T &value, PointerVariant pointer) const
                 {
                     setByPointer<T>(value, pointer);
                 }
             };

             struct visitorStream
             {
                 visitorStream(std::stringstream &stream, Variant &var)
                     : optionsStream(stream), variant(var) { }

                 template<typename T>
                 void operator() (const T&) const
                 {
                     T value;
                     optionsStream >> value;
                     if (optionsStream.fail())
                     {
                         optionsStream.clear();
                         return;
                     }
                     setFromStream<T>(variant, value);
                 }
             private:
                 std::stringstream &optionsStream;
                 Variant &variant;
             };

        };
        std::vector<Option> m_options;
    };
    Options m_options;
    void setOptionsFromStringstream(std::stringstream&);
};
