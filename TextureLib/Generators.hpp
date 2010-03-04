#ifndef GENERATORS_HPP
#define GENERATORS_HPP

class Texture;
struct D3DXCOLOR;

void single_color(const D3DXCOLOR& col, Texture* t);
void split_color(const D3DXCOLOR& a, const D3DXCOLOR& b, const float ratio, Texture* t);
void radial(const float inner, const float outer, Texture *t);

void noise(float a, float b, Texture *t);

#endif
