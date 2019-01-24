#include <fstream>
#include <FreeImage.h>
#include <cstring>

namespace ImageUtils
{
  inline BYTE* loadImage(const char* filepath, uint* width, uint* height, uint* bpp)
  {
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

    FIBITMAP *dib = nullptr;

    fif = FreeImage_GetFileType(filepath, 0);

    if (fif == FIF_UNKNOWN)
      fif = FreeImage_GetFIFFromFilename(filepath);

    if (fif == FIF_UNKNOWN)
    {
      throw std::runtime_error("FreeImage file format is not supported or file not exist");
    }

    if (FreeImage_FIFSupportsReading(fif))
      dib = FreeImage_Load(fif, filepath);
    if (!dib)
    {
      throw std::runtime_error("FreeImage file Cannot be read: ");
    }


    BYTE* bits = FreeImage_GetBits(dib);

    *width = FreeImage_GetWidth(dib);
    *height = FreeImage_GetHeight(dib);
    *bpp = FreeImage_GetBPP(dib);
    if (*bpp != 24 && *bpp != 32)
    {
      throw std::runtime_error("Bits per pixel is not valid (24 or 32): ");
      delete[] bits;
    }

    //printImage(bits,*width, *height, *bpp);
    //return ImageFactory::getBadBPPImage(width, height, bpp);

    int size = (*width) * (*height) * (*bpp >> 3);
    BYTE* result = new BYTE[size];
    memcpy(result,bits,size);
    FreeImage_Unload(dib);
    //delete[] bits;
    return result;
  }
}
