#ifndef TEXTURE_LIB_HPP
#define TEXTURE_LIB_HPP

class Texture;
typedef void(*pfnTexture)(const Texture*);

typedef void(*pfnSetTextureInit)(pfnTexture);
typedef void(*pfnSetTextureClose)(pfnTexture);

extern pfnTexture texture_create_callback;
extern pfnTexture texture_delete_callback;

#endif
