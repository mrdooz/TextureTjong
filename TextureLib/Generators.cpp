#include "stdafx.h"
#include "Generators.hpp"
#include "Texture.hpp"
#include "Utils.hpp"

namespace
{
	float noise_random(const int x, const int y)
	{
		const int r1 = 15731;
		const int r2 = 789221;
		const int r3 = 1376312589;
		int n = x + y * 57;
		n = (n<<13) ^ n;
		return 1.0 - ( (n * (n * n * r1 + r2) + r3) & 0x7fffffff) / 1073741824.0;

	}

	// 0--1
	// |  |
	// 2--3
	float bilinear(const float v0, const float v1, const float v2, const float v3, const float x, const float y)
	{
		const float tx = x - (int)x;
		const float ty = y - (int)y;
		const float i0 = cosine_interpolate(v0, v1, tx);
		const float i1 = cosine_interpolate(v2, v3, tx);
		const float v = cosine_interpolate(i0, i1, ty);
		return v;
	}

	float perlin_noise(const float x, const float y)
	{
		const float n0 = noise_random((int)x+0, (int)y+0);
		const float n1 = noise_random((int)x+1, (int)y+0);
		const float n2 = noise_random((int)x+0, (int)y+1);
		const float n3 = noise_random((int)x+1, (int)y+1);
		return bilinear(n0, n1, n2, n3, x, y);
	}

	DWORD make_col(float v)
	{
		const uint32_t b = (uint32_t)(255 * clamp(v, 0.0f, 1.0f));
		return b << 24 | b << 16 | b << 8 | b;
	}
}

void noise(float a, float b, Texture *t)
{
	const int cNumOctaves = 4;

	DWORD* ptr = (DWORD*)t->data();
	for (int i = 0; i < t->height(); ++i) {
		for (int j = 0; j < t->width(); ++j) {

			float sum = 0;
			for (int n = 0; n < cNumOctaves; ++n) {
				const float cb = powf(b, n);
				const float ca = powf(a, n);
				sum += 0.1f * perlin_noise(cb * j, cb * i) / ca;
			}
			*ptr++ = make_col(sum);
		}
	}
}


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

void radial(const float inner, const float outer, Texture *t)
{
  const int32_t middle_x = t->width() / 2;
  const int32_t middle_y = t->height() / 2;
  const float s = min(middle_x, middle_y);
  uint32_t *p = (uint32_t*)t->data();

  for (int32_t y = 0, e = t->height(); y < e; ++y) {
    for (int32_t x = 0, f = t->width(); x < f; ++x) {
      const int32_t diff_x = (x - middle_x);
      const int32_t diff_y = (y - middle_y);
      const float r = sqrtf(diff_x*diff_x + diff_y*diff_y);
			float v = 0;
			if (r > outer) {
				v = 0;
			} else if (r < inner) {
				v = 1;
			} else {
				v = 1 - (r - inner) / (outer - inner);
			}
      
			//const float value = 1 - clamp( diff/ s, 0.f, 1.f);
      *p++ = make_col(v);

    }
  }
}


