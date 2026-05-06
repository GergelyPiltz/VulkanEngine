#pragma once

// std
#include <iostream>
#include <limits>
#include <functional>

// libs
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/ext/scalar_constants.hpp>

// from: https://stackoverflow.com/a/57595105
// Combines multiple hashes
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hashCombine(seed, rest), ...);
};

// Normalizes a glm vector but returns a null vector if a null vector is received
template<glm::length_t L, typename T, glm::qualifier Q>
glm::vec<L, T, Q> safeNormalize(const glm::vec<L, T, Q>& v)
{
    T len2 = glm::length2(v);

    // Avoid sqrt unless necessary; also protects against division by zero
    if (len2 <= std::numeric_limits<T>::epsilon())
        return glm::zero<glm::vec<L, T, Q>>();

    return v * glm::inversesqrt(len2);
}

// Overloads the << operator to print glm vectors more easily
inline static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

// Formatter to print glm vectors more easily
template <>
struct std::formatter<glm::vec3> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const glm::vec3& v, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
    }
};