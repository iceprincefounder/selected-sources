#ifndef __EXR_IO_H
#define __EXR_IO_H

#include <cstdio>
#include <cstdlib>

#include <string>
#include <vector>

#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <half.h>

#include <cassert>

using namespace Imf;
using namespace Imath;
using namespace std;

struct MapAttrbute
{
public:
    MapAttrbute(){}
    MapAttrbute(string a,string b):m_attribute(a),m_value(b){}

    string attribute()
    {
        return m_attribute;
    }
    string value()
    {
        return m_value;
    }
    void setValue(string input)
    {
        m_value = input;
    }
private:
    string m_attribute;
    string m_value;
};

struct HeaderData
{
public:
    HeaderData()
    {
        // CustomAttributeList.push_back(MapAttrbute("resolution", ""));
        CustomAttributeList.push_back(MapAttrbute("arnold/version", ""));
        
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/f834d0a/name", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/f834d0a/manifest", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/f834d0a/hash", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/f834d0a/conversion", ""));
        
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/bda530a/name", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/bda530a/manifest", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/bda530a/hash", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/bda530a/conversion", ""));

        CustomAttributeList.push_back(MapAttrbute("cryptomatte/28322e9/name", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/28322e9/manifest", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/28322e9/hash", ""));
        CustomAttributeList.push_back(MapAttrbute("cryptomatte/28322e9/conversion", ""));

    }
    ~HeaderData(){}

    // const string arnoldVersion = "arnold/version";

    Box2i dataWindow;
    Box2i displayWindow;
    
    string renderer;
    vector<MapAttrbute>* get()
    {
        return &CustomAttributeList;
    }
private:
    vector<MapAttrbute> CustomAttributeList;
};

bool ReadEXR(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha);
void WriteEXR(const char *name, float *pixels, int xRes, int yRes);
Box2i ReadDataWindow(const char *name);
Box2i ReadDisplayWindow(const char *name);
bool ReadEXR2(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha, bool &isCorpped);
void WriteEXR2(const char *name, float *pixels, int xRes, int yRes, Box2i dw);
void ReadHeaderData(const char *name, HeaderData& data);

#endif