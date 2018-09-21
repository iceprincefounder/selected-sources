#include "shader_utils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcTriplanarMtd)


enum Params 
{
    p_input,
    p_space,
    p_normal,
    p_global_zoom,
    p_blend_softness,
    p_frequency,
    p_offset,
    p_scale,
    p_rotate,
    p_rotjitter,
};

node_parameters 
{
    AiParameterRGBA("input", 1.0f, 1.0f, 1.0f, 1.0f);
    AiParameterEnum("space", 0, triplanarSpaceNames);
    AiParameterEnum("normal", 0, triplanarNormalNames);
    AiParameterFlt("global_zoom", 1.f);
    AiParameterFlt("blend_softness", 0.1f);
    AiParameterFlt("frequency", 1.0f);
    AiParameterVec("offset",0.0f,0.0f,0.0f);
    AiParameterVec("scale",1.0f,1.0f,1.0f);
    AiParameterVec("rotate",0.0f,0.0f,0.0f);
    AiParameterVec("rotjitter",1.0f,1.0f,1.0f);
}


node_initialize 
{
}

node_finish 
{
}

node_update 
{
}


shader_evaluate 
{
    // get shader parameters

    int space = AiShaderEvalParamInt(p_space);

    int normal = AiShaderEvalParamInt(p_normal);

    float global_zoom = AiShaderEvalParamFlt(p_global_zoom);

    float blend_softness = AiClamp(AiShaderEvalParamFlt(p_blend_softness), 0.f, 1.f);

    float frequency = AiShaderEvalParamFlt(p_frequency);

    AtVector scale = AiShaderEvalParamVec(p_scale);

    AtVector rotate = AiShaderEvalParamVec(p_rotate);

    AtVector offset = AiShaderEvalParamVec(p_offset);

    // AtVector rotjitter = AiShaderEvalParamVec(p_rotjitter);

    SGCache SGC;
    SGC.initCache(sg);

    // set up P and blend weights
    AtVector P;
    AtVector N;
    AtVector dPdx;
    AtVector dPdy;


    float weights[3];
    int blends[3];
    getProjectionGeometry(node, sg, space, normal, &P, &N, &dPdx, &dPdy);
    getBlendWeights(N, space, blend_softness, weights);

    P *= frequency;
    // compute texture values

    AtRGBA colorResult[3];

    // gllobal scale of texture
    scale *= global_zoom;

    // lookup X
    AtVector projP;
    
    projP.x = (P.z + 123.94 + offset.x) * scale.x;
    projP.y = (P.y + 87.22 + offset.x) * scale.x;
    projP.z = 0.;
    rotateUVs(projP, rotate.x);

    sg->u = projP.x;
    sg->v = projP.y;
    sg->dudx = dPdx.z * scale.x;
    sg->dudy = dPdy.z * scale.x;
    sg->dvdx = dPdx.y * scale.x;
    sg->dvdy = dPdy.y * scale.x;

    if (weights[0] > 0.) 
    {
        colorResult[0] = AiShaderEvalParamRGBA(p_input);
        blends[0] = 1;
    } 
    else 
    {
        colorResult[0] = AI_RGBA_ZERO;
        blends[0] = 0;
    }
    // lookup Y
    projP.x = (P.x + 74.1 + offset.y) * scale.y;
    projP.y = (P.z + 9.2 + offset.y) * scale.y;
    projP.z = 0.;
    rotateUVs(projP, rotate.y);

    sg->u = projP.x;
    sg->v = projP.y;
    sg->dudx = dPdx.x * scale.y;
    sg->dudy = dPdy.x * scale.y;
    sg->dvdx = dPdx.z * scale.y;
    sg->dvdy = dPdy.z * scale.y;

    if (weights[1] > 0.) 
    {
        colorResult[1] = AiShaderEvalParamRGBA(p_input);
        blends[1] = 1;
    } 
    else 
    {
        colorResult[1] = AI_RGBA_ZERO;
        blends[1] = 0;
    }

    // lookup Z
    projP.x = (P.x + 123.94 + offset.z) * scale.z;
    projP.y = (P.y + 87.22 + offset.z) * scale.z;
    projP.z = 0.;
    rotateUVs(projP, rotate.z);

    sg->u = projP.x;
    sg->v = projP.y;
    sg->dudx = dPdx.x * scale.z;
    sg->dudy = dPdy.x * scale.z;
    sg->dvdx = dPdx.y * scale.z;
    sg->dvdy = dPdy.y * scale.z;

    if (weights[2] > 0.) 
    {
        colorResult[2] = AiShaderEvalParamRGBA(p_input);
        blends[2] = 1;
    } 
    else 
    {
        colorResult[2] = AI_RGBA_ZERO;
        blends[2] = 0;
    }

    AtRGBA result = AI_RGBA_ZERO;
    for (unsigned int i = 0; i < 6; ++i)
    {
        result = lerp(result, colorResult[i%3]*blends[i%3], weights[i%3]);
    }

    // sg->out.RGBA() = AtRGBA(N.x, N.y, N.z, 1.0f);
    sg->out.RGBA() = result;
    SGC.restoreSG(sg);
}