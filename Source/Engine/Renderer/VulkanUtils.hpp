#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <algorithm>


namespace KompotEngine
{

namespace Renderer
{

std::vector<std::string> getLayers();
std::vector<std::string> getExtensions();

} // namespace Renderer

} //namespace KompotEngine
