#ifndef MODIFIERS_HPP
#define MODIFIERS_HPP

class Texture;
void mixer(const Texture *a, const Texture *b, const float ratio, Texture *out);
void modulate(const Texture *a, const Texture *b, Texture *out);
void displace(const Texture *a, const Texture *b, const float scale, Texture *out);
void remap(const Texture *a, const D3DXCOLOR& start_color, const D3DXCOLOR& end_color, Texture *out);
void combine(const Texture *r, const Texture *g, const Texture* b, const Texture* out);
void scale(const Texture* t, const float s, const Texture* out);

#endif // MODIFIERS_HPP
