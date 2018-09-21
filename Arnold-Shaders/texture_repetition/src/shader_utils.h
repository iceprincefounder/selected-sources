#pragma once

#include <ai.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <math.h>

#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <climits>
#include <cfloat>

struct ShaderData
{
    AtTextureHandle* texturehandle;
    AtTextureParams  textureparams;
    bool useCustomUVSet;
    AtString uvSetName;
};

struct SGCache{
   
   void initCache(const AtShaderGlobals *sg){
      u = sg->u;
      v = sg->v;
      dudx = sg->dudx;
      dudy = sg->dudy;
      dvdx = sg->dvdx;
      dvdy = sg->dvdy;
   }

   void restoreSG(AtShaderGlobals *sg){
      sg->u = u;
      sg->v = v;
      sg->dudx = dudx;
      sg->dudy = dudy;
      sg->dvdx = dvdx;
      sg->dvdy = dvdy;
   }

   float u;
   float v;
   float dudx;
   float dudy;
   float dvdx;
   float dvdy;
};


const int RANDOM[10] = {0,1,1,0,1,1,0,1,1,0};

inline AtVector2 hashBlock(AtVector2 inUV,AtVector2 inBlock)
{
   AtVector2 result = AtVector2(1, 1);
   float unitX = 1/inBlock.x;
   float unitY = 1/inBlock.y;
   AtVector2 fUV;
   fUV.x = inUV.x - floor(inUV.x);
   fUV.y = inUV.y - floor(inUV.y);

   result.x = ceil(fUV.x / unitX);
   result.y = ceil(fUV.y / unitY);
   return result;
}

inline AtVector2 hashCenterUV(AtVector2 inUV,AtVector2 inBlock)
{
   AtVector2 block = hashBlock(inUV,inBlock);
   float unitX = 1/block.x;
   float unitY = 1/block.y;
   AtVector2 result;
   result.x = unitX*(block.x - 0.5);
   result.y = unitY*(block.y - 0.5);
   return result;
}

#define SAMPLE 10.0f

inline float lerp(const float a, const float b, const float t)
{
   return (1-t)*a + t*b;
}

inline AtRGB lerp(const AtRGB& a, const AtRGB& b, const float t)
{
   AtRGB r;
   r.r = lerp( a.r, b.r, t );
   r.g = lerp( a.g, b.g, t );
   r.b = lerp( a.b, b.b, t );
   return r;
}

inline AtVector lerp(const AtVector& a, const AtVector& b, const float t)
{
   AtVector r;
   r.x = lerp( a.x, b.x, t );
   r.y = lerp( a.y, b.y, t );
   r.z = lerp( a.z, b.z, t );
   return r;
}

inline AtRGBA lerp(const AtRGBA& a, const AtRGBA& b, const float t)
{
   AtRGBA r;
   r.r = lerp( a.r, b.r, t );
   r.g = lerp( a.g, b.g, t );
   r.b = lerp( a.b, b.b, t );
   r.a = lerp( a.a, b.a, t );
   return r;
}

inline AtVector2 lerpOffset(AtVector2 block)
{
   AtVector2 result;
   result.x = (1/block.x)/SAMPLE + 1/block.x;
   result.y = (1/block.y)/SAMPLE + 1/block.y;
   return result;
}

inline void TextureFileOperation(AtVector2 inUV,AtVector2 inDu,AtVector2 inDv,AtVector2 &outUV,AtVector2 &outDu,AtVector2 &outDv,AtVector2 noise,AtVector2 offset,float rotate,AtVector2 repeat)
{

   float outU = inUV.x;
   float outV = inUV.y;
   float outDuDx = inDu.x;
   float outDuDy = inDu.y;
   float outDvDx = inDv.x;
   float outDvDy = inDv.y;


   AtVector2 thisBlock = hashBlock(inUV, repeat);

   // noise uv
   if (noise.x > 0.0f)
   {
      AtVector2 uv = AtVector2(outU * 16, outV * 16);
      outU += noise.x * AiPerlin2(uv);
   }

   if (noise.y > 0.0f)
   {
      AtVector2 uv = AtVector2((1 - outU) * 16, (1 - outV) * 16);
      outV += noise.y * AiPerlin2(uv);
   }

   

   // for UVs, translate first, then rotate
   if(offset.x > 0 || offset.y > 0)
   {
      float offsetX = sin(AI_PI*(thisBlock.x+offset.x));
      float offsetY = cos(AI_PI*(thisBlock.y+offset.y));
      outU += offsetX;
      outV += offsetY;
      //outU += offset.x;
      //outV += offset.y;
   }

      

   int mirrorE = (int)thisBlock.x % 2;
   int mirrorO = (int)thisBlock.y % 2;
   // do mirror, stagger before rotation
   if (mirrorE == 0)
   {
      float center = floor(outV) + 0.5f;
      outV = center - (outV - center);

      outDuDy = -outDuDy;
      outDvDy = -outDvDy;
   }
   
   if (mirrorO == 0)
   {
      float center = floor(outU) + 0.5f;
      outU = center - (outU - center);

      outDuDx = -outDuDx;
      outDvDx = -outDvDx;
   }


   // finally rotate UV
   if (rotate <= -AI_EPSILON || rotate >= AI_EPSILON)
   {
      float x, y;
      float ca = cos(rotate);
      float sa = sin(rotate);

      x = outU - 0.5f;
      y = outV - 0.5f;
      outU = 0.5f + ca * x - sa * y;
      outV = 0.5f + ca * y + sa * x;

      x = outDuDx;
      y = outDuDy;
      outDuDx = ca * x - sa * y;
      outDuDy = ca * y + sa * x;

      x = outDvDx;
      y = outDvDy;
      outDvDx = ca * x - sa * y;
      outDvDy = ca * y + sa * x;
   }
   
   // apply repetition factor
   outU *= repeat.x;
   outV *= repeat.y;

   // replace shader globals
   outUV.x = outU;
   outUV.y = outV;
   outDu.x = outDuDx;
   outDu.y = outDuDy;
   outDv.x = outDvDx;
   outDv.y = outDvDy;
}
