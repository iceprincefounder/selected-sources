#include <ai.h>
 
#include "constants.h"

using namespace std;

AI_SHADER_NODE_EXPORT_METHODS(SpeedTreeShaderMethods);

enum SpeedTreeParams 
{
    p_branches = 0,
    p_leaves,
    p_frond_and_caps,
    p_facing_leaves,
    p_rigid_meshes,
    p_grass,
    p_billboard,
};

node_parameters
{
   AiParameterClosure("branches");
   AiParameterClosure("leaves");
   AiParameterClosure("frond_and_caps");
   AiParameterClosure("facing_leaves");
   AiParameterClosure("rigid_meshes");
   AiParameterClosure("grass");
   AiParameterClosure("billboard");
}

node_initialize
{
}
  
node_update
{
}

node_finish
{
}

shader_evaluate
{
    AtString attribute = AtString(SPEEDTREE_SHADER);
    int shader_value;
    AtClosureList branches = AiOrenNayarBSDF(sg, AtRGB(1.0f, 0.0f, 0.0f), sg->N);
    AtClosureList leaves = AiOrenNayarBSDF(sg, AtRGB(0.0f, 1.0f, 0.0f), sg->N);
    AtClosureList frond_and_caps = AiOrenNayarBSDF(sg, AtRGB(0.0f, 0.0f, 1.0f), sg->N);
    AtClosureList facing_leaves = AiOrenNayarBSDF(sg, AtRGB(1.0f, 1.0f, 0.0f), sg->N);
    AtClosureList rigid_meshes = AiOrenNayarBSDF(sg, AtRGB(1.0f, 0.0f, 1.0f), sg->N);
    AtClosureList grass = AiOrenNayarBSDF(sg, AtRGB(0.0f, 1.0f, 1.0f), sg->N);
    AtClosureList billboard = AiOrenNayarBSDF(sg, AtRGB(1.0f, 1.0f, 1.0f), sg->N);
    if (AiUDataGetInt(attribute, shader_value)){
        switch (shader_value)
        {
        case ST_BRANCHES:
            if(AiNodeIsLinked(node, "branches"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_branches);
            else
              sg->out.CLOSURE() = branches;
            break;
        case ST_LEAVES:
            if(AiNodeIsLinked(node, "leaves"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_leaves);
            else
              sg->out.CLOSURE() = leaves;
            break;
        case ST_FRONDANDCAPS:
            if(AiNodeIsLinked(node, "frond_and_caps"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_frond_and_caps);
            else
              sg->out.CLOSURE() = frond_and_caps;
            break;
        case ST_FACINGLEAVES:
            if(AiNodeIsLinked(node, "facing_leaves"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_facing_leaves);
            else
              sg->out.CLOSURE() = facing_leaves;
            break;
        case ST_RIGIDMESHES:
            if(AiNodeIsLinked(node, "rigid_meshes"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_rigid_meshes);
            else
              sg->out.CLOSURE() = rigid_meshes;
            break;
        case ST_GRASS:
            if(AiNodeIsLinked(node, "grass"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_grass);
            else
              sg->out.CLOSURE() = grass;
            break;
        case ST_BILLBOARD:
            if(AiNodeIsLinked(node, "billboard"))
              sg->out.CLOSURE() = AiShaderEvalParamClosure(p_billboard);
            else
              sg->out.CLOSURE() = billboard;
            break;
        default:
            sg->out.CLOSURE() = AiClosureMatte(sg,AI_RGB_WHITE);
            break;
        }
    }
    else
        sg->out.CLOSURE() = AiClosureMatte(sg,AI_RGB_WHITE);
}

node_loader
{
   if (i > 0)
      return false;
   node->methods     = SpeedTreeShaderMethods;
   node->output_type = AI_TYPE_CLOSURE;
   node->name        = "speedtree_shader";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}