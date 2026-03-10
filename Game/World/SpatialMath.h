#pragma once

struct RectF
{
    float left;
    float top;
    float right;
    float bottom;
};

inline bool Intersects(const RectF& a, const RectF& b)
{
    if (a.right < b.left)   return false;
    if (a.left > b.right)   return false;
    if (a.bottom < b.top)   return false;
    if (a.top > b.bottom)   return false;
    return true;
}

inline bool Contains(const RectF& outer, const RectF& inner)
{
    return outer.left <= inner.left &&
        outer.right >= inner.right &&
        outer.top <= inner.top &&
        outer.bottom >= inner.bottom;
}