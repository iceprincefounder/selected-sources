#ifndef __EXR_IO_H
#define __EXR_IO_H

#include <cstdio>
#include <cstdlib>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfRgbaFile.h>
#include <half.h>
#include <cassert>

using namespace Imf;
using namespace Imath;

bool ReadEXR(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha);
void WriteEXR(const char *name, float *pixels, int xRes, int yRes);
Box2i ReadDataWindow(const char *name);
Box2i ReadDisplayWindow(const char *name);
bool ReadEXR2(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha, bool &isCorpped);
void WriteEXR2(const char *name, float *pixels, int xRes, int yRes, Box2i dw);
#endif // defined(__EXR_IO_H)