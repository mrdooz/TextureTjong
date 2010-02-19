#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <D3DX10math.h>
#include "Utils.hpp"

class Texture
{
public:

  Texture();
  ~Texture();

  Texture(const int32_t width, const int32_t height);
  Texture(const Texture& t);
  void operator=(const Texture& t);

  int32_t data_size() const { return 4 * _width * _height; }
  int32_t width() const { return _width; }
  int32_t height() const { return _height; }
  uint8_t* data() const { return _data; }

  D3DXCOLOR at(const float x, const float y) const;
  void set_pixel(const int32_t x, const int32_t y, const D3DXCOLOR& col);

private:
  DWORD* ptr(const int32_t x, const int32_t y);

  void init();
  void assign(const Texture& t);

  uint8_t *_data;
  int32_t _width;
  int32_t _height;
};

inline D3DXCOLOR Texture::at(const float x, const float y) const
{
  // TOOD: We should really lerp here
  const int int_x = clamp(x, 0.f, 1.f) * (_width - 1);
  const int int_y = clamp(y, 0.f, 1.f) * (_height - 1);

  return D3DXCOLOR(*((uint32_t*)_data + int_x + int_y * _width));
}


#endif // TEXTURE_HPP
