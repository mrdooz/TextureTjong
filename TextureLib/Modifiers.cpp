#include "stdafx.h"
#include "Modifiers.hpp"
#include "Texture.hpp"
#include "Utils.hpp"

namespace
{
  float color_to_monochrome(const D3DXCOLOR& c)
  {
    return c.r * 0.3f + c.g * 0.59f + c.b * 0.1f;
  }

}

void mixer(const Texture *a, const Texture *b, const float ratio, Texture *out)
{
  const float x_inc = 1 / (float)(out->width() - 1);
  const float y_inc = 1 / (float)(out->height() - 1);

  DWORD* dst = (DWORD*)out->data();
  float y_acc = 0;
  for (int32_t i = 0, e = out->height(); i < e; ++i) {
    float x_acc = 0;
    for (int32_t j = 0, f = out->width(); j < f; ++j) {
      *dst++ = ratio * a->at(x_acc, y_acc) + (1 - ratio) * b->at(x_acc, y_acc);
      x_acc += x_inc;
    }
    y_acc += y_inc;
  }
}

void modulate(const Texture *a, const Texture *b, Texture *out)
{
  const float x_inc = 1 / (float)(out->width() - 1);
  const float y_inc = 1 / (float)(out->height() - 1);

  DWORD* dst = (DWORD*)out->data();
  float y_acc = 0;
  for (int32_t i = 0, e = out->height(); i < e; ++i) {
    float x_acc = 0;
    for (int32_t j = 0, f = out->width(); j < f; ++j) {
      const D3DXCOLOR& ca = a->at(x_acc, y_acc);
      const D3DXCOLOR& cb = b->at(x_acc, y_acc);
      *dst++ = D3DXCOLOR(ca.r * cb.r, ca.g * cb.g, ca.b * cb.b, ca.a * cb.a);
      x_acc += x_inc;
    }
    y_acc += y_inc;
  }
}

void displace(const Texture *a, const Texture *b, const float scale, Texture *out)
{
  // displace x by r, and y by g

  const float x_inc = 1 / (float)(out->width() - 1);
  const float y_inc = 1 / (float)(out->height() - 1);

  DWORD* dst = (DWORD*)out->data();
  float y_acc = 0;
  for (int32_t i = 0, e = out->height(); i < e; ++i) {
    float x_acc = 0;
    for (int32_t j = 0, f = out->width(); j < f; ++j) {
      const D3DXCOLOR& cb = b->at(x_acc, y_acc);
      const D3DXCOLOR& ca = a->at(x_acc + scale * 2 * (cb.r - 0.5f), y_acc + scale * 2 * (cb.g - 0.5f));
      *dst++ = ca;
      x_acc += x_inc;
    }
    y_acc += y_inc;
  }
}

void remap(const Texture *a, const D3DXCOLOR& start_color, const D3DXCOLOR& end_color, Texture *out)
{
  const float x_inc = 1 / (float)(out->width() - 1);
  const float y_inc = 1 / (float)(out->height() - 1);

  DWORD* dst = (DWORD*)out->data();
  float y_acc = 0;
  for (int32_t i = 0, e = out->height(); i < e; ++i) {
    float x_acc = 0;
    for (int32_t j = 0, f = out->width(); j < f; ++j) {
      const float t = color_to_monochrome(a->at(x_acc, y_acc));
      *dst++ = linear_interpolate(start_color, end_color, t);
      x_acc += x_inc;
    }
    y_acc += y_inc;
  }
}
