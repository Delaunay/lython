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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_extra_math.h"
#include "bezier.h"

ImProjectResult ImProjectOnCubicBezier(const ImVec2& point, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const int subdivisions)
{
    // http://pomax.github.io/bezierinfo/#projections

    const float epsilon    = 1e-5f;
    const float fixed_step = 1.0f / static_cast<float>(subdivisions - 1);

    ImProjectResult result;
    result.Point    = point;
    result.Time     = 0.0f;
    result.Distance = FLT_MAX;

    // Step 1: Coarse check
    for (int i = 0; i < subdivisions; ++i)
    {
        auto t = i * fixed_step;
        auto p = ImCubicBezier(p0, p1, p2, p3, t);
        auto s = point - p;
        auto d = ImDot(s, s);

        if (d < result.Distance)
        {
            result.Point    = p;
            result.Time     = t;
            result.Distance = d;
        }
    }

    if (result.Time == 0.0f || ImFabs(result.Time - 1.0f) <= epsilon)
    {
        result.Distance = ImSqrt(result.Distance);
        return result;
    }

    // Step 2: Fine check
    auto left  = result.Time - fixed_step;
    auto right = result.Time + fixed_step;
    auto step  = fixed_step * 0.1f;

    for (auto t = left; t < right + step; t += step)
    {
        auto p = ImCubicBezier(p0, p1, p2, p3, t);
        auto s = point - p;
        auto d = ImDot(s, s);

        if (d < result.Distance)
        {
            result.Point    = p;
            result.Time     = t;
            result.Distance = d;
        }
    }

    result.Distance = ImSqrt(result.Distance);

    return result;
}