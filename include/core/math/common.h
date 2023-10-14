//
// Created by kyrosz7u on 2023/5/17.
//

#include <algorithm>
#include <cmath>
#include <cassert>
#include <cfloat>

#ifndef XEXAMPLE_CONSTANTSDEFINE_H
#define XEXAMPLE_CONSTANTSDEFINE_H

namespace Math
{
    static const float Math_POS_INFINITY = std::numeric_limits<float>::infinity();
    static const float Math_NEG_INFINITY = -std::numeric_limits<float>::infinity();
    static const float Math_PI           = 3.14159265358979323846264338327950288f;
    static const float Math_ONE_OVER_PI  = 1.0f / Math_PI;
    static const float Math_TWO_PI       = 2.0f * Math_PI;
    static const float Math_HALF_PI      = 0.5f * Math_PI;
    static const float Math_Deg2Rad     = Math_PI / 180.0f;
    static const float Math_Rad2Deg     = 180.0f / Math_PI;
    static const float Math_LOG2         = log(2.0f);
    static const float Math_EPSILON      = 1e-6f;

    static const float Float_EPSILON  = FLT_EPSILON;
    static const float Double_EPSILON = DBL_EPSILON;

    bool realEqual(float a, float b, float tolerance);
}

#endif //XEXAMPLE_CONSTANTSDEFINE_H
