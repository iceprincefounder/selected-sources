#pragma once

#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "constants.h"

#include "stdlib.h"
#include "SpeedtreeReader/SpeedTreeVertex.h"
#include "SpeedtreeReader/readSpeedTreeSrt.h"
#include "SpeedtreeReader/SpeedTreeVertex.h"
#include "SpeedtreeReader/utilities.h"

#include "Core/Core.h"
#include "Core/Wind.h"
#include "Core/CoordSys.h"


#define  int32 int
#define  FVector vec3
#define  FVector4 vec4

using namespace SpeedTree;
using namespace std;
using namespace glm;

// The struct contents polymesh vertex data.
struct ProcArrays
{
    AtArray *nsidesArray; //number of primitives and size of each primitive, "motion key":  motion blur time sample

    AtArray *vidxsArray; //mesh vertices, Anticlockwise, index group
    AtArray *vlistArray; //final vertext list array
    AtArray *vdefaultlistArray; //VERTEX_PROPERTY_POSITION, mesh vertices, real value,index ordered

    AtArray *uvidxsArray; //uv vertices, the same order with vidx
    AtArray *uvlistArray; //VERTEX_PROPERTY_DIFFUSE_TEXCOORDS,uv vertices, real value, index ordered

    AtArray *nidxsArray; //normal vertices, index group
    AtArray *nlistArray; //VERTEX_PROPERTY_NORMAL, normal vertices, real value, index ordered

    AtArray *tidxsArray; //tangent vertices
    AtArray *tlistArray; //VERTEX_PROPERTY_TANGENT, tangent vertices, real value


    int GeometryType;
};

// Procedural parameters
struct SpeedTreeData
{
  FSpeedTreeData speedtreeMotionData;              //a structure
  FSpeedTreeData speedtreePreMotionData;           //pre structure
  FSpeedTreeData speedtreePostMotionData;           //post structure
  AtString srtFile;
  string meshName;

  bool useGlobalMotion;
  int currentFrame;
  bool enableMotionBlur;
  AtArray *motionSamples;

  float frame;
  float fps;

  int WindType;
  int LODType;
  int LODSmoothType;

  float GlobalFrequency;
  float GustFrequency;
  float WindSpeed = 1.0f;
  Vec3 WindDirection = Vec3(1.0f, 0.0f, 0.0f);

  vector<float> SpeedKeyFrames;
  vector<float> SpeedResponseTimes;
  vector<float> SpeedKeyValues;
  vector<float> DireKeyFrames;
  vector<float> DireResponseTimes;
  vector<Vec3> DireKeyValues;

  ReadSpeedTreeSrt *readSTS = new ReadSpeedTreeSrt();
  SLod *sLod = new SLod();
  ProcArrays* procArrays[4] = {new ProcArrays(), new ProcArrays(),new ProcArrays(),new ProcArrays()};

  int id; // current polymesh node id
  int sum; // number of the polymesh nodes
};


// The function which marks the polymesh by user-data.
inline int getProceduralShaderID(const SRenderState* sRenderState)
{
  int result;

  if (sRenderState->m_bVertBillboard)
  {
    result = SPEEDTREE_BILLBOARD;
  }
  else if (sRenderState->m_bUsedAsGrass)
  {
    result = SPEEDTREE_GRASS;
  }
  else
  {
    if (sRenderState->m_bBranchesPresent)
      result = SPEEDTREE_BRANCHES;
    else if (sRenderState->m_bFrondsPresent)
      result = SPEEDTREE_FRONDANDCAPS;
    else if (sRenderState->m_bLeavesPresent)
      result = SPEEDTREE_LEAVES;
    else if (sRenderState->m_bFacingLeavesPresent)
      result = SPEEDTREE_FACINGLEAVES;
    else
      result = SPEEDTREE_RIGIDMESHES;
  }
  return result;
}

static const char* getUniqueName( char* buf, const char* basename )
{
   static unsigned int g_counter = 0;
   sprintf( buf, "%s__%X", basename, g_counter++ );
   return buf;
}

static void Vec3ToAtArray(vector<Vec3> srtPnt, AtArray* arnoldArray)
{
    for (unsigned int i = 0; i < srtPnt.size(); ++i)
    {
        AtVector tmp;
        tmp.x = float(srtPnt[i][0]);
        tmp.y = float(srtPnt[i][1]);
        tmp.z = float(srtPnt[i][2]);
        AiArraySetVec(arnoldArray, i, tmp);
    }
}


static void Vec2ToAtArray(vector<Vec3> srtPnt, AtArray* arnoldArray)
{
    for (unsigned int i = 0; i < srtPnt.size(); ++i)
    {
        AtVector2 tmp;
        tmp.x = float(srtPnt[i][0]);
        tmp.y = float(srtPnt[i][1]);
        AiArraySetVec2(arnoldArray, i, tmp);
    }
}

static void uintToAtArray(vector<int> srtInt, AtArray* arnoldArray)
{
    for (unsigned int i = 0; i < srtInt.size(); ++i)
        AiArraySetUInt(arnoldArray, i, srtInt[i]);
}


static void srt2AtArray(const SDrawCall& sDrawCall, SpeedTreeData* data)
{
    ReadSpeedTreeSrt *readSTS = data->readSTS;
    // init AtArray-s
    AiMsgInfo("[speedtree] Node : %s", data->meshName.c_str());
    AiMsgInfo("[speedtree] \t sDrawCall.m_nNumVertices: %d", sDrawCall.m_nNumVertices);
    AiMsgInfo("[speedtree] \t sDrawCall.m_nNumIndices: %d", sDrawCall.m_nNumIndices);

    int motion_key = 1;
    if (data->enableMotionBlur)
        motion_key = AiArrayGetNumElements(data->motionSamples);

    data->procArrays[data->id]->nsidesArray = AiArrayAllocate(sDrawCall.m_nNumIndices/3, 1, AI_TYPE_UINT);        //number of primitives and size of each primitive, "motion key":  motion blur time sample

    data->procArrays[data->id]->vidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);        //mesh vertices, Anticlockwise, index group
    data->procArrays[data->id]->vlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, motion_key, AI_TYPE_VECTOR);        //final vertex position
    data->procArrays[data->id]->vdefaultlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR);        //VERTEX_PROPERTY_POSITION, mesh vertices, real value,index ordered

    data->procArrays[data->id]->uvidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);        //uv vertices, the same order with vidx
    data->procArrays[data->id]->uvlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);    //VERTEX_PROPERTY_DIFFUSE_TEXCOORDS,uv vertices, real value, index ordered

    data->procArrays[data->id]->nidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices,  1, AI_TYPE_UINT);        //normal vertices, index group
    data->procArrays[data->id]->nlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, motion_key, AI_TYPE_VECTOR);        //VERTEX_PROPERTY_NORMAL, normal vertices, real value, index ordered

    data->procArrays[data->id]->tidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices,  1, AI_TYPE_UINT);        //tangent vertices
    data->procArrays[data->id]->tlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR);        //VERTEX_PROPERTY_TANGENT, tangent vertices, real value

    // assign values
    uintToAtArray(readSTS->nsides, data->procArrays[data->id]->nsidesArray);
    uintToAtArray(readSTS->vidxs, data->procArrays[data->id]->vidxsArray);
    Vec3ToAtArray(readSTS->vertexlist, data->procArrays[data->id]->vdefaultlistArray);

    uintToAtArray(readSTS->uvidxsArray, data->procArrays[data->id]->uvidxsArray);
    Vec2ToAtArray(readSTS->uvlistArray, data->procArrays[data->id]->uvlistArray);

    uintToAtArray(readSTS->nidxs, data->procArrays[data->id]->nidxsArray);
    Vec3ToAtArray(readSTS->nlist, data->procArrays[data->id]->nlistArray);

    uintToAtArray(readSTS->tidxs, data->procArrays[data->id]->tidxsArray);
    Vec3ToAtArray(readSTS->tlist, data->procArrays[data->id]->tlistArray);

    data->procArrays[data->id]->GeometryType = readSTS->GeometryType;

    // calculate value
    FVertexParameters vtxParam;
    for (unsigned int i = 0; i < sDrawCall.m_nNumVertices; ++i)
    {
        AtVector vertex = AiArrayGetVec(data->procArrays[data->id]->vdefaultlistArray, i);
        vtxParam.WorldPosition = Vec3(vertex.x, vertex.y, vertex.z);    // P: shading point in world-space. Vec3

        vtxParam.InstanceLocalToWorld = glm::float4x4(1.0f);                              // sg->
        vtxParam.InstanceWorldToLocal = glm::float4x4(1.0f);                              // sg->Minv
        vtxParam.InstanceLocalPosition = Vec3(vertex.x, vertex.y, vertex.z);              // sg->Po

        AtVector normal = AiArrayGetVec(data->procArrays[data->id]->nlistArray, i);
        vtxParam.Normal = Vec3(normal.x, normal.y, normal.z);               // N: shading normal, Vec3

        AtVector tangent = AiArrayGetVec(data->procArrays[data->id]->tlistArray, i);
        vtxParam.Tangent = Vec3(tangent.x, tangent.y, tangent.z);           // Tangent: shading normal, Vec3


        if (data->enableMotionBlur)
        {

            // pre motion key
            vec3 offset_pre_motion = GetSpeedTreeVertexOffset(vtxParam, 
                                                              data->procArrays[data->id]->GeometryType, 
                                                              data->WindType, 
                                                              data->LODSmoothType,
                                                              0.1, true, 
                                                              data->speedtreePreMotionData);
            AtVector tmp_pre_m;
            tmp_pre_m.x = vertex.x + float(offset_pre_motion.x);
            tmp_pre_m.y = vertex.y + float(offset_pre_motion.y);
            tmp_pre_m.z = vertex.z + float(offset_pre_motion.z);
            AiArraySetVec(data->procArrays[data->id]->vlistArray, i, tmp_pre_m);
            AiArraySetVec(data->procArrays[data->id]->nlistArray, i, normal);

            // sub motion key
            vec3 offset_post_motion = GetSpeedTreeVertexOffset(vtxParam, 
                                                              data->procArrays[data->id]->GeometryType, 
                                                              data->WindType, 
                                                              data->LODSmoothType,
                                                              0.1, true, 
                                                              data->speedtreePostMotionData);
            AtVector tmp_post_m;
            tmp_post_m.x = vertex.x + float(offset_post_motion.x);
            tmp_post_m.y = vertex.y + float(offset_post_motion.y);
            tmp_post_m.z = vertex.z + float(offset_post_motion.z);
            AiArraySetVec(data->procArrays[data->id]->vlistArray, (i + sDrawCall.m_nNumVertices), tmp_post_m);
            AiArraySetVec(data->procArrays[data->id]->nlistArray, (i + sDrawCall.m_nNumVertices), normal);


        }
        else
        {

          vec3 offset = GetSpeedTreeVertexOffset(vtxParam, data->procArrays[data->id]->GeometryType,  data->WindType, data->LODSmoothType,
                                                  0.1, true, data->speedtreeMotionData);
          
          // reset pos
          AtVector tmp;
          tmp.x = vertex.x + float(offset.x);
          tmp.y = vertex.y + float(offset.y);
          tmp.z = vertex.z + float(offset.z);

          AiArraySetVec(data->procArrays[data->id]->vlistArray, i, tmp);
        }
    }
}


// The function create arnold polymesh node and returned by procedural_get_node
AtNode* createNode(const AtNode* parent, SpeedTreeData *data)
{
    char buf[512];

    const SLod& sLod = *(data->sLod);
    const SDrawCall& sDrawCall = data->readSTS->readSDrawCall(data->id, sLod);

    const SRenderState* renderState = data->readSTS->readRenderState(sDrawCall);
    string meshName = data->readSTS->setMeshName(renderState);
    const char* uni_meshName = getUniqueName(buf, meshName.c_str());
    data -> meshName = uni_meshName;
    data->readSTS->readSrtData(data->WindType, data->LODType, sDrawCall);
    // Translate speedtree data to arnold origin data.
    srt2AtArray(sDrawCall, data);

    int shader_id = getProceduralShaderID(renderState);


    AtNode *polymesh = AiNode( "polymesh", uni_meshName, parent);
    AtMatrix mat = AiM4Identity();
    AiNodeSetMatrix ( polymesh, "matrix", mat );


    AiNodeSetArray(polymesh, "nsides", data->procArrays[data->id]->nsidesArray);
    AiNodeSetArray(polymesh, "vidxs", data->procArrays[data->id]->vidxsArray);
    AiNodeSetArray(polymesh, "vlist", data->procArrays[data->id]->vlistArray);
    AiNodeSetArray(polymesh, "uvidxs", data->procArrays[data->id]->uvidxsArray);
    AiNodeSetArray(polymesh, "uvlist", data->procArrays[data->id]->uvlistArray);
    AiNodeSetArray(polymesh, "nidxs", data->procArrays[data->id]->nidxsArray);
    AiNodeSetArray(polymesh, "nlist", data->procArrays[data->id]->nlistArray);

    AiNodeSetBool( polymesh, "opaque", true );
    AiNodeSetBool( polymesh, "smoothing", false );
    
    // Set user-data which would read by speedtree_shader so that we could assgin
    // different shaders to different polymeshes on one procedural node.
    AiNodeDeclare( polymesh, AtString(SPEEDTREE_SHADER), "constant INT");
    AiNodeSetInt( polymesh, AtString(SPEEDTREE_SHADER),  shader_id);
    data->id ++;
    if (data->id == data->sum)
      AiMsgInfo("[speedtree] Successed to genarate Speedtree geomtry data!");
    return polymesh;
}