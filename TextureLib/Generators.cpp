#include "stdafx.h"
#include "Generators.hpp"
#include "Texture.hpp"
#include "Utils.hpp"


void single_color(const D3DXCOLOR& col, Texture* t)
{
  DWORD* p = (DWORD*)t->data();
  const DWORD c = col;
  for (int i = 0, e = t->width() * t->height(); i != e; ++i) {
    *p++ = col;
  }

  std::strstream os;
  os << "single_color " << col;
  t->name(os.str());
}

void split_color(const D3DXCOLOR& a, const D3DXCOLOR& b, const float ratio, Texture* t)
{
  DWORD* p = (DWORD*)t->data();
  const DWORD a_col = a;
  const DWORD b_col = b;
  const int32_t half = ratio * t->width() * t->height();
  for (int i = 0, e = half; i != e; ++i) {
    *p++ = a_col;
  }

  for (int i = half, e = t->width() * t->height(); i != e; ++i) {
    *p++ = b_col;
  }

  std::strstream os;
  os << "split_color(" << a << ", " << b << ", " << ratio << ")";
  t->name(os.str());
}

void radial(const float radius, Texture *t)
{
  const int32_t middle_x = t->width() / 2;
  const int32_t middle_y = t->height() / 2;
  const float r = min(middle_x, middle_y);
  uint32_t *p = (uint32_t*)t->data();
  for (int32_t y = 0, e = t->height(); y < e; ++y) {
    for (int32_t x = 0, f = t->width(); x < f; ++x) {
      const int32_t diff_x = (x - middle_x);
      const int32_t diff_y = (y - middle_y);
      const float diff = sqrtf(diff_x*diff_x + diff_y*diff_y);
      const float value = 1 - clamp( diff/ r, 0.f, 1.f);
      *p++ = D3DXCOLOR(value, value, value, 1);

    }
  }

  std::strstream os;
  os << "radial(" << radius << ")";
  t->name(os.str());

}


