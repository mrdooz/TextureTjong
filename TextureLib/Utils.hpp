#ifndef UTILS_HPP
#define UTILS_HPP

template<typename T>
T xchg_null(T& t) { T tmp = t; t = NULL; return tmp; }

template<typename T>
T clamp(T value, T min_value, T max_value)
{
  return min(max_value, max(min_value, value));
}

template<typename T>
T linear_interpolate(const T& a, const T&b, const float t)
{
  return (1-t) * a + t * b;
}

template<typename T>
T cosine_interpolate(const T& a, const T& b, const float t)
{
  const float f = (1 - cos(t * 3.1415926f)) * 0.5f;
  return linear_interpolate(a, b, f);
}

class Texture;
void save_bitmap(const char* filename, const Texture& t);

#endif
