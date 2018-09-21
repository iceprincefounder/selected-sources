#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* LcTextureRepetitionMethods;
extern AtNodeMethods* LcTextureBlendMethods;
extern AtNodeMethods* LcTextureEntranceMethods;

enum SHADERS
{
   LcTextureRepetition,
   LcTextureBlend,
   LcTextureEntrance
};

node_loader
{
   switch (i) 
   {     
      case LcTextureRepetition:
         node->methods     = (AtNodeMethods*) LcTextureRepetitionMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "lc_texture_repetition";
         node->node_type   = AI_NODE_SHADER;
      break;
      case LcTextureBlend:
         node->methods     = (AtNodeMethods*) LcTextureBlendMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "lc_texture_blend";
         node->node_type   = AI_NODE_SHADER;
      break;
      case LcTextureEntrance:
         node->methods     = (AtNodeMethods*) LcTextureEntranceMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "lc_texture_entrance";
         node->node_type   = AI_NODE_SHADER;
      break;
      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}