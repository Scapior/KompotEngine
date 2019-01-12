#pragma once

#include <boost/algorithm/string.hpp>
#include <string>
#include <istream>
#include <ostream>
#include <sstream>

namespace KompotEngine
{

namespace Renderer
{

enum class GAPI {OGL, Vulkan};
std::istream& operator>> (std::istream&, GAPI &);
std::ostream& operator<< (std::ostream&, const GAPI &);


} // Renderer namespace

} // KompotEngine namespace
