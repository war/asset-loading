#include "GLMUtils.h"

#include <sstream>
#include <iomanip>

namespace GLMUtils {
    template<typename T>
    std::string vec_to_string(const T& v, int components) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4) << "(";
        for (int i = 0; i < components; ++i) {
            oss << v[i];
            if (i < components - 1) oss << ", ";
        }
        oss << ")";
        return oss.str();
    }

    std::string to_string(const glm::vec2& v) {
        return vec_to_string(v, 2);
    }

    std::string to_string(const glm::vec3& v) {
        return vec_to_string(v, 3);
    }

    std::string to_string(const glm::vec4& v) {
        return vec_to_string(v, 4);
    }

    std::string to_string(const glm::quat& q) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4)
            << "(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
        return oss.str();
    }
}