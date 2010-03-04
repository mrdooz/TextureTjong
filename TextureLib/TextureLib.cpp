#include "stdafx.h"
#include "TextureLib.hpp"
#include "Texture.hpp"
#include "Utils.hpp"
#include "Generators.hpp"
#include "Modifiers.hpp"

pfnTexture texture_create_callback = NULL;
pfnTexture texture_delete_callback = NULL;

extern "C"
{
  __declspec(dllexport) void setTextureInit(pfnTexture pfnInit)
  {
    texture_create_callback = pfnInit;
  }

  __declspec(dllexport) void setTextureClose(pfnTexture pfnClose)
  {
    texture_delete_callback = pfnClose;
  }
}

BOOST_PYTHON_MODULE(texture_ext)
{
  using namespace boost::python;

  def("single_color", single_color);
  def("split_color", split_color);
  def("radial", radial);
	def("noise", noise);

  def("displace", displace);
	def("remap", remap);
	def("combine", combine);
	def("modulate", modulate);
	def("scale", scale);

  def("mixer", mixer);

  def("save_bitmap", save_bitmap);

  class_<D3DXCOLOR>("Color", init<float, float, float, float>())
    .add_property("r", &D3DXCOLOR::r)
    .add_property("g", &D3DXCOLOR::g)
    .add_property("b", &D3DXCOLOR::b)
    .add_property("a", &D3DXCOLOR::a)
    ;

  class_<Texture>("Texture", init<int32_t, int32_t>())
    .add_property("width", &Texture::width)
    .add_property("height", &Texture::height)
    .add_property("data_size", &Texture::data_size)
    .def("set_pixel", &Texture::set_pixel)
    ;

}
