#pragma once

#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace GLMUtils {
    std::string to_string(const glm::vec2& v);
    std::string to_string(const glm::vec3& v);
    std::string to_string(const glm::vec4& v);
    std::string to_string(const glm::quat& q);
}