#pragma once

struct SkipRange
{
    double start = 0.0;
    double end = 0.0;
};

inline bool operator==(const SkipRange &lhs, const SkipRange &rhs)
{
    return lhs.start == rhs.start && lhs.end == rhs.end;
}
