#include "stdafx.h"
#include "Texture.hpp"
#include "Utils.hpp"
#include "TextureLib.hpp"

Texture::Texture() 
  : _data(NULL), _width(0), _height(0) 
{
  if (texture_create_callback) {
    texture_create_callback(this);
  }
}

Texture::~Texture()
{
  if (texture_delete_callback) {
    texture_delete_callback(this);
  }

  delete [] (xchg_null(_data));
}

Texture::Texture(const int32_t width, const int32_t height)
  : _width(width)
  , _height(height)
{
  init();

  if (texture_create_callback) {
    texture_create_callback(this);
  }

}

Texture::Texture(const Texture& t) 
  : _data(NULL)
{
  *this = t;

  if (texture_create_callback) {
    texture_create_callback(this);
  }

}

void Texture::operator=(const Texture& t)
{
  if (&t == this) {
    return;
  }
  delete [] (xchg_null(_data));
  assign(t);
}

void Texture::init()
{
  _data = new uint8_t[data_size()];
  ZeroMemory(_data, data_size());
}

void Texture::assign(const Texture& t)
{
  _width = t._width;
  _height = t._height;
  init();
  memcpy(_data, t._data, data_size());
}



void Texture::set_pixel(const int32_t x, const int32_t y, const D3DXCOLOR& col)
{
  const int32_t safe_x = clamp(x, 0, _width-1);
  const int32_t safe_y = clamp(y, 0, _height-1);
  *ptr(safe_x, safe_y) = col;
}

DWORD* Texture::ptr(const int32_t x, const int32_t y)
{
  assert(x < _width);
  assert(y < _height);
  assert(_data != NULL);
  return (DWORD*)&_data[4*(x + y*_width)];
}