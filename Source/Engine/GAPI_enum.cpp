#include "GAPI_enum.hpp"

using namespace KompotEngine::Renderer;

std::istream& KompotEngine::Renderer::operator>> (std::istream& stream, GAPI &api)
{
    std::string token;
    stream >> token;

    boost::to_upper(token);

    if (token == "VULKAN" or token == "1")
    {
        api = GAPI::Vulkan;
    }
    else /*if (token == "OGL")*/
    {
        api = GAPI::OGL; // default GAPI
    }

    return stream;
}

std::ostream& KompotEngine::Renderer::operator<< (std::ostream& stream, const GAPI &api)
{
    if (api == GAPI::Vulkan)
    {
        stream << "VULKAN";
    }
    else
    {
        stream << "OGL";
    }
    return stream;
}
