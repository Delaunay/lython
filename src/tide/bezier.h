//------------------------------------------------------------------------------
// VERSION 0.1
//
// LICENSE
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
//
// CREDITS
//   Written by Michal Cichon
//------------------------------------------------------------------------------

#pragma

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// Project point on Cubic Bezier curve.
struct ImProjectResult
{
    ImVec2 Point;    // Point on curve
    float  Time;     // [0 - 1]
    float  Distance; // Distance to curve
};


template <typename T>
inline T ImCubicBezier(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
    const auto a = 1 - t;
    const auto b = a * a * a;
    const auto c = t * t * t;

    return b * p0 + 3 * t * a * a * p1 + 3 * t * t * a * p2 + c * p3;
}

ImProjectResult ImProjectOnCubicBezier(const ImVec2& point, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const int subdivisions);
