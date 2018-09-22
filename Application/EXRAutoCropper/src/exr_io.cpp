#include <cstdio>
#include <cstdlib>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfRgbaFile.h>
#include <half.h>
#include <cassert>

#include "exr_io.h"
 
using namespace Imf;
using namespace Imath;
Box2i ReadDisplayWindow(const char *name)
{
    Box2i result(V2i(0,0), V2i(1000, 1000));
    try {
    InputFile file(name);
    Box2i dw = file.header().displayWindow();
    result = dw;
    } catch (const std::exception &e) {
        fprintf(stderr, "Unable to read image file \"%s\": %s", name, e.what());
        return result;
    }

    return result;
}
Box2i ReadDataWindow(const char *name)
{
    Box2i result(V2i(0,0), V2i(1000, 1000));
    try {
    InputFile file(name);
    Box2i dw = file.header().dataWindow();
    result = dw;
    } catch (const std::exception &e) {
        fprintf(stderr, "Unable to read image file \"%s\": %s", name, e.what());
        return result;
    }

    return result;
}

bool ReadEXR(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha)
{
    try {
        InputFile file(name);
        Box2i dw = file.header().dataWindow();
        xRes = dw.max.x - dw.min.x + 1;
        yRes = dw.max.y - dw.min.y + 1;

        half *hrgba = new half[4 * xRes * yRes];

        // for now...
        hasAlpha = true;
        int nChannels = 4;

        hrgba -= 4 * (dw.min.x + dw.min.y * xRes);
        FrameBuffer frameBuffer;
        frameBuffer.insert("R", Slice(HALF, (char *)hrgba,
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
        frameBuffer.insert("G", Slice(HALF, (char *)hrgba+sizeof(half),
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
        frameBuffer.insert("B", Slice(HALF, (char *)hrgba+2*sizeof(half),
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
        frameBuffer.insert("A", Slice(HALF, (char *)hrgba+3*sizeof(half),
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 1.0));

        file.setFrameBuffer(frameBuffer);
        file.readPixels(dw.min.y, dw.max.y);

        hrgba += 4 * (dw.min.x + dw.min.y * xRes);
        rgba = new float[nChannels * xRes * yRes];
        for (int i = 0; i < nChannels * xRes * yRes; ++i)
            rgba[i] = hrgba[i];
        delete[] hrgba;
    }
    catch (const std::exception &e) 
    {
        fprintf(stderr, "Unable to read image file \"%s\": %s", name, e.what());
        return NULL;
    }

    return rgba;
}

void WriteEXR(const char *name, float *pixels, int xRes, int yRes) {
    Rgba *hrgba = new Rgba[xRes * yRes];
    for (int i = 0; i < xRes * yRes; ++i)
        hrgba[i] = Rgba(pixels[4*i], pixels[4*i+1], pixels[4*i+2], pixels[4*i+3]);

    Box2i displayWindow(V2i(0,0), V2i(xRes-1, yRes-1));
    Box2i dataWindow = displayWindow;

    RgbaOutputFile file(name, displayWindow, dataWindow, WRITE_RGBA);
    file.setFrameBuffer(hrgba, 1, xRes);
    try 
    {
        // file.writePixels(yRes);
        file.writePixels(dataWindow.max.y - dataWindow.min.y + 1);
    }
    catch (const std::exception &e) {
        fprintf(stderr, "Unable to write image file \"%s\": %s", name,
                e.what());
    }

    delete[] hrgba;
}

bool ReadEXR2(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha, bool &isCorpped)
{
    try {
        InputFile file(name);
        Box2i dw = file.header().dataWindow();
        Box2i dsw = file.header().displayWindow();
        if (dw == dsw)
            isCorpped = false;
        else
            isCorpped = true;
        xRes = dw.max.x - dw.min.x + 1;
        yRes = dw.max.y - dw.min.y + 1;

        half *hrgba = new half[4 * xRes * yRes];

        // for now...
        hasAlpha = true;
        int nChannels = 4;

        hrgba -= 4 * (dw.min.x + dw.min.y * xRes);
        FrameBuffer frameBuffer;
        frameBuffer.insert("R", Slice(HALF, (char *)hrgba,
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
        frameBuffer.insert("G", Slice(HALF, (char *)hrgba+sizeof(half),
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
        frameBuffer.insert("B", Slice(HALF, (char *)hrgba+2*sizeof(half),
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
        frameBuffer.insert("A", Slice(HALF, (char *)hrgba+3*sizeof(half),
                      4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 1.0));

        file.setFrameBuffer(frameBuffer);
        file.readPixels(dw.min.y, dw.max.y);

        hrgba += 4 * (dw.min.x + dw.min.y * xRes);
        rgba = new float[nChannels * xRes * yRes];
        for (int i = 0; i < nChannels * xRes * yRes; ++i)
            rgba[i] = hrgba[i];
        delete[] hrgba;
    }
    catch (const std::exception &e) 
    {
        fprintf(stderr, "Unable to read image file \"%s\": %s", name, e.what());
        return NULL;
    }

    return rgba;
}


void WriteEXR2(const char *name, float *pixels, int xRes, int yRes, Box2i dw)
{
    Rgba *hrgba = new Rgba[xRes * yRes];
    for (int i = 0; i < xRes * yRes; ++i)
        hrgba[i] = Rgba(pixels[4*i], pixels[4*i+1], pixels[4*i+2], pixels[4*i+3]);

    Box2i displayWindow(V2i(0,0), V2i(xRes-1, yRes-1));
    Box2i dataWindow = dw;

    RgbaOutputFile file(name, displayWindow, dataWindow, WRITE_RGBA);
    file.setFrameBuffer(hrgba, 1, xRes);
    try 
    {
        // file.writePixels(yRes);
        file.writePixels(dataWindow.max.y - dataWindow.min.y + 1);
    }
    catch (const std::exception &e) {
        fprintf(stderr, "Unable to write image file \"%s\": %s", name,
                e.what());
    }

    delete[] hrgba;
}