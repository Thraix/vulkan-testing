#include <fstream>
#include <FreeImage.h>
#include <cstring>

namespace ImageUtils
{
  inline BYTE* loadImage(const char* filepath, uint32_t* width, uint32_t* height)
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
    uint32_t bpp = FreeImage_GetBPP(dib);
    // FreeImage uses bits per pixel, We want bytes per pixel
    bpp >>= 3;
    if (bpp != 3 && bpp != 4)
    {
      delete[] bits;
      throw std::runtime_error("Bits per pixel is not valid (24 or 32): ");
    }

    // Force 4 byte per pixel
    int size = (*width) * (*height) * 4;
    BYTE* result = new BYTE[size];
    uint32_t resultI = 0;
    uint32_t bitsI = 0;
    for(uint32_t y = 0; y < *height; y++)
    {
      for(uint32_t x = 0; x < *width; x++)
      {
        // Incase the order of FreeImage is not RGB (its probably BGRA)
        result[resultI++] = bits[bitsI + FI_RGBA_RED];
        result[resultI++] = bits[bitsI + FI_RGBA_GREEN];
        result[resultI++] = bits[bitsI + FI_RGBA_BLUE];
        if(bpp == 4)
          result[resultI++] = bits[bitsI + FI_RGBA_ALPHA];
        else
          result[resultI++] = 0xff; 
        bitsI += bpp;
      }
    }
    FreeImage_Unload(dib);
    return result;
  }
}
