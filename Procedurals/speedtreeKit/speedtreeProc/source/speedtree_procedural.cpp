#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "constants.h"
#include "geometry.h"

using namespace std;

AI_PROCEDURAL_NODE_EXPORT_METHODS(SpeedTreeMethods);


bool SortFloatArray(AtArray* array, vector<float> & cvector){
   unsigned int nElements = (array) ? AiArrayGetNumElements(array) : 0;
   if (nElements > 0)
   {
      for (unsigned int i; i < nElements; ++i)
      {
         cvector.push_back(AiArrayGetFlt(array, i));      
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool SortVec3Array(AtArray* array, vector<Vec3> & cvector){
   unsigned int nElements = (array) ? AiArrayGetNumElements(array) : 0;
   if (nElements > 0)
   {
      for (unsigned int i; i < nElements; ++i)
      {
            AtVector tempVec = AiArrayGetVec(array, i);
            cvector.push_back(Vec3(tempVec.x, tempVec.y, tempVec.z));
      }
   }
   else
   {
      return false;  
   }
}


node_parameters
{
   AiParameterStr("srtFile", "");

   AiParameterInt("useGlobalMotion", 1);
   AiParameterFlt("currentFrame", 1001.0f);
   AiParameterInt("enableMotionBlur", 0);
   AiParameterArray("motionSamples", AiArray(-0.25f, 0.25f, AI_TYPE_FLOAT));

   AiParameterFlt("fps", 24.0);
   AiParameterFlt("globalFrequency", 0.0);
   AiParameterFlt("gustFrequency", 0.0);

   AiParameterFlt("windSpeed", 0.0);
   AiParameterFlt("windDirection_x", 1.0);
   AiParameterFlt("windDirection_y", 0.0);
   AiParameterFlt("windDirection_z", 0.0);

   AiParameterInt("windType", 6);
   AiParameterInt("LODType", 0);
   AiParameterInt("LODSmoothType", 0);

   AiParameterArray("speedKeyFrame", AiArray(0, 1, AI_TYPE_FLOAT));
   AiParameterArray("speedResponseTime", AiArray(0, 1, AI_TYPE_FLOAT));
   AiParameterArray("speedKeyValue", AiArray(0, 1, AI_TYPE_FLOAT));
   AiParameterArray("direKeyFrame", AiArray(0, 1, AI_TYPE_FLOAT));
   AiParameterArray("direResponseTime", AiArray(0, 1, AI_TYPE_FLOAT));
   AiParameterArray("direKeyValue", AiArray(0, 1, AI_TYPE_VECTOR));
}

procedural_init
{
   SpeedTreeData *proc_data = new SpeedTreeData();
   *user_ptr = proc_data;
   proc_data -> srtFile = AiNodeGetStr(node, "srtFile");

   proc_data->useGlobalMotion = (bool)AiNodeGetInt(node, "useGlobalMotion");
   proc_data -> currentFrame = AiNodeGetFlt(node, "currentFrame");
   proc_data->enableMotionBlur = (bool)AiNodeGetInt(node, "enableMotionBlur");
   proc_data -> motionSamples = AiNodeGetArray(node, "motionSamples");
   proc_data->GlobalFrequency = AiNodeGetFlt(node, "globalFrequency");
   proc_data->GustFrequency = AiNodeGetFlt(node, "gustFrequency");

   proc_data->WindSpeed = AiNodeGetFlt(node, "windSpeed");

   proc_data->WindDirection = Vec3(
      AiNodeGetFlt(node, "windDirection_x"),
      AiNodeGetFlt(node, "windDirection_y"),
      AiNodeGetFlt(node, "windDirection_z"));

   proc_data->WindType = AiNodeGetInt(node, "windType");
   proc_data->LODType = AiNodeGetInt(node, "LODType");
   proc_data->LODSmoothType = AiNodeGetInt(node, "LODSmoothType");

   AtArray* SpeedKeyFrame = AiNodeGetArray(node, "speedKeyFrame");
   SortFloatArray(SpeedKeyFrame, proc_data->SpeedKeyFrames);

   AtArray* SpeedResponseTimes = AiNodeGetArray(node, "speedResponseTime");
   SortFloatArray(SpeedResponseTimes, proc_data->SpeedResponseTimes);

   AtArray* SpeedKeyValues = AiNodeGetArray(node, "speedKeyValue");
   SortFloatArray(SpeedKeyValues, proc_data->SpeedKeyValues);

   AtArray* DireKeyFrames = AiNodeGetArray(node, "direKeyFrame");
   SortFloatArray(DireKeyFrames, proc_data->DireKeyFrames);

   AtArray* DireResponseTimes = AiNodeGetArray(node, "direResponseTime");
   SortFloatArray(DireResponseTimes, proc_data->DireResponseTimes);

   AtArray* DireKeyValues = AiNodeGetArray(node, "direKeyValue");
   SortVec3Array(DireKeyValues, proc_data->DireKeyValues);

   int sKF = AiArrayGetNumElements(SpeedKeyFrame);
   int sRT = AiArrayGetNumElements(SpeedResponseTimes);
   int sKV = AiArrayGetNumElements(SpeedKeyValues);
   int dKF = AiArrayGetNumElements(DireKeyFrames);
   int dKT = AiArrayGetNumElements(DireResponseTimes);
   int dKV = AiArrayGetNumElements(DireKeyValues);

   proc_data -> id = 0;

   AiMsgInfo("[speedtree] Reading srt file : %s", proc_data -> srtFile.c_str());

   // if all the arrays of speedtree control handle are empty,
   // we would use global windSpeed and windDirection.
   if (sKF+sRT+sKV+dKF+dKT+dKV == 0)
   {
      AiMsgInfo("[speedtree] \t enable global wind Speed and direction!");
      proc_data->SpeedKeyFrames.push_back(1.0f);
      proc_data->SpeedResponseTimes.push_back(1.0f);
      proc_data->SpeedKeyValues.push_back(proc_data->WindSpeed);
      proc_data->DireKeyFrames.push_back(1.0f);
      proc_data->DireResponseTimes.push_back(1.0f);
      proc_data->DireKeyValues.push_back(proc_data->WindDirection);
   }


   // get current frame & fps, the global render settings are stored in the options node,(Maya)
   AtNode* options = AiUniverseGetOptions();
   if (proc_data->useGlobalMotion)
   {
      proc_data->frame = AiNodeGetFlt(options, "frame");
      AiMsgInfo("[speedtree] \t enable global motion frame!");
   }
   else
   {
      proc_data->frame = AiNodeGetFlt(node, "currentFrame");
      AiMsgInfo("[speedtree] \t using currentFrame as AiOption frame!");
   }
   if (proc_data->enableMotionBlur)
      AiMsgInfo("[speedtree] \t enable motion blur!");
   else
      AiMsgInfo("[speedtree] \t disable motion blur!");
   
   proc_data->fps = AiNodeGetFlt(node, "fps");

   AiMsgInfo("[speedtree] \t frame: %f", proc_data->frame);
   AiMsgInfo("[speedtree] \t fps: %f", proc_data->fps);
   
   *(proc_data->sLod) = proc_data->readSTS->readSRT( proc_data->srtFile.c_str(), 
                                                       proc_data->WindSpeed, 
                                                       proc_data->GustFrequency,
                                                       proc_data->WindDirection,
                                                       true,
                                                       proc_data->LODType);

   proc_data->readSTS->setKeyframes( proc_data->SpeedKeyFrames, 
                                       proc_data->SpeedResponseTimes, 
                                       proc_data->SpeedKeyValues,
                                       proc_data->DireKeyFrames, 
                                       proc_data->DireResponseTimes, 
                                       proc_data->DireKeyValues);
   bool state = proc_data->readSTS->returnState();
   if (state)
      AiMsgInfo("[speedtree] Successed to read SRT file!");
   else
      AiMsgError("[speedtree] Failed to read SRT file!");
   proc_data -> sum = (int)proc_data->readSTS->meshCount;

   return true;
}

procedural_cleanup
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   delete proc_data; 
   return true;
}

procedural_num_nodes
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   return proc_data->sum;
}

procedural_get_node
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;

   if (proc_data->enableMotionBlur)
   {
     // float* motionWindShaderValues = proc_data->readSTS->readMotionWindData(proc_data->frame, motion_end, proc_data->fps);
     // proc_data->speedtreeMotionData = proc_data->readSTS->assignWindData(motionWindShaderValues, proc_data->frame + motion_end);
     proc_data->readSTS->readWindData((proc_data->frame - 1), 1.0, proc_data->fps);
     float motion_start = -0.15f;
     float motion_end = 0.15f;
     if (AiArrayGetNumElements(proc_data -> motionSamples) == 2)
     {
        motion_start = AiArrayGetFlt(proc_data -> motionSamples, 0);
        motion_end = AiArrayGetFlt(proc_data -> motionSamples, 1);
     }

     // pre motion key
     float* preMotionWindShaderValues = proc_data->readSTS->readMotionWindData(proc_data->frame,
                                                                                motion_start,
                                                                               proc_data->fps);
     proc_data->speedtreePreMotionData = proc_data->readSTS->assignWindData(preMotionWindShaderValues, 
                                                                            (proc_data->frame + motion_start));
     // frame
     float* motionWindShaderValues = proc_data->readSTS->readMotionWindData(proc_data->frame, 
                                                                      0.0, 
                                                                      proc_data->fps);
     proc_data->speedtreeMotionData = proc_data->readSTS->assignWindData(motionWindShaderValues, 
                                                                         proc_data->frame);
     //post motion key
     float* postMotionWindShaderValues = proc_data->readSTS->readMotionWindData(proc_data->frame, 
                                                                            motion_end,
                                                                            proc_data->fps);
     proc_data->speedtreePostMotionData = proc_data->readSTS->assignWindData(postMotionWindShaderValues, 
                                                                            (proc_data->frame + motion_end));
   }
   else
   {
     float* motionWindShaderValues = proc_data->readSTS->readWindData(proc_data->frame, 1.0, proc_data->fps);
     proc_data->speedtreeMotionData = proc_data->readSTS->assignWindData(motionWindShaderValues, proc_data->frame);
   }
   AtNode* polymesh = createNode(node, proc_data);
   return polymesh;
}

node_loader
{
   if (i>0)
      return false;

   node->methods      = SpeedTreeMethods;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "speedtree_procedural";
   node->node_type    = AI_NODE_SHAPE_PROCEDURAL;
   strcpy(node->version, AI_VERSION);

   return true;
}