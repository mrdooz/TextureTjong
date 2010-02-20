#include "stdafx.h"
#include "Utils.hpp"
#include "Texture.hpp"
#include <tchar.h>

#define BITMAP_SIGNATURE 'MB'

#pragma pack(push, 1)
struct BitmapFileheader
{
  uint16_t Signature;
  uint32_t Size;
  uint32_t Reserved;
  uint32_t BitsOffset;
};

struct BitmapHeader
{
  uint32_t HeaderSize;
  int32_t Width;
  int32_t Height;
  uint16_t Planes;
  uint16_t BitCount;
  uint32_t Compression;
  uint32_t SizeImage;
  int32_t PelsPerMeterX;
  int32_t PelsPerMeterY;
  uint32_t ClrUsed;
  uint32_t ClrImportant;
};
#pragma pack(pop)

void save_bitmap(const char* filename, const Texture& t)
{
  FILE* f = fopen(filename, "wb");
  if (f == NULL) {
    return;
  }

  // create a bmp in memory
  const uint32_t header_size = sizeof(BitmapFileheader) + sizeof(BitmapHeader);
  const uint32_t total_size = header_size + t.data_size();

  uint8_t* bmp = new uint8_t[total_size];
  ZeroMemory(bmp, total_size);
  BitmapFileheader* file_header = (BitmapFileheader*)&bmp[0];
  BitmapHeader* bitmap_header = (BitmapHeader*)&bmp[sizeof(BitmapFileheader)];

  // copy the rows upside down
  uint8_t* src = t.data() + 4 * t.width() * (t.height()-1);
  uint8_t* dst = bmp + header_size;
  const int row = 4 * t.width();
  for (int32_t i = 0, e = t.height(); i < e; ++i) {
    memcpy(dst, src, row);
    dst += row;
    src -= row;
  }

  file_header->Signature = BITMAP_SIGNATURE;
  file_header->Size = total_size;
  file_header->BitsOffset = header_size;

  bitmap_header->HeaderSize = sizeof(BitmapHeader);
  bitmap_header->Width = t.width();
  bitmap_header->Height = t.height();
  bitmap_header->Planes = 1;
  bitmap_header->BitCount = 32;
  bitmap_header->SizeImage = t.data_size();

  fwrite(bmp, total_size, 1, f);
  fclose(f);

  delete[](xchg_null(bmp));

}

std::ostream& operator<<(std::ostream& os, const D3DXCOLOR& col)
{
  TCHAR buf[256];
  _stprintf(buf, _T("[0x%.2x%.2x%.2x%.2x]"), (uint32_t)(255 * col.r), (uint32_t)(255 * col.g), (uint32_t)(255 * col.b), (uint32_t)(255 * col.a));
  os << buf;
  return os;
}
