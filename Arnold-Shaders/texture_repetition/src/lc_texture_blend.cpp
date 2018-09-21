#include "shader_utils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcTextureBlendMethods);

enum TextureRepetitionParams { 
   p_color = 0,
   p_repeat,
   p_rotate,
   p_offset,
   p_noise,
};

node_parameters
{
   AiParameterRGBA("color", 0.7f, 0.7f, 0.7f, 1.0f);
   AiParameterVec2("repeatUV", 1.0f, 1.0f);
   AiParameterFlt("rotateUV", 0.0f);
   AiParameterVec2("offsetUV", 0.0f, 0.0f);
   AiParameterVec2("noiseUV", 0.00f, 0.00f);
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
	AtVector2 repeat = AiShaderEvalParamVec2(p_repeat);
	float rotate = AiShaderEvalParamFlt(p_rotate);
	AtVector2 offset = AiShaderEvalParamVec2(p_offset);
	AtVector2 noise = AiShaderEvalParamVec2(p_noise);


	AiStateSetMsgVec2(AtString("ShaderMeg_TexRepe_repeat"), repeat);
	AiStateSetMsgFlt(AtString("ShaderMeg_TexRepe_rotate"), rotate);
	AiStateSetMsgVec2(AtString("ShaderMeg_TexRepe_offset"), offset);
	AiStateSetMsgVec2(AtString("ShaderMeg_TexRepe_noise"), noise);

	SGCache SGC,SGB;
	SGC.initCache(sg);

	float inBlendU = sg->u;
	float inBlendV = sg->v;
	float inBlendDuDx = sg->dudx;
	float inBlendDuDy = sg->dudy;
	float inBlendDvDx = sg->dvdx;
	float inBlendDvDy = sg->dvdy;

	float outBlendU = inBlendU;
	float outBlendV = inBlendV;
	float outBlendDuDx = inBlendDuDx;
	float outBlendDuDy = inBlendDuDy;
	float outBlendDvDx = inBlendDvDx;
	float outBlendDvDy = inBlendDvDy;


	AtRGBA image,image_u,image_v;
	AtVector2 move = lerpOffset(repeat);
	float center;

	// image 
	image = AiShaderEvalParamRGBA(p_color);

	// image_v
	SGB.initCache(sg);
	//center = floor(inBlendU) + 0.5f;
	//inBlendU = center - (inBlendU - center);
	center = floor(inBlendV) + 0.5f;
	inBlendV = center - (inBlendV - center);
	outBlendU = inBlendU;
	outBlendV = inBlendV + move.y;
	sg->u = outBlendU;
	sg->v = outBlendV;
	image_v = AiShaderEvalParamRGBA(p_color);
	SGB.restoreSG(sg);

	// blend
	AtVector2 iuv = AtVector2(sg->u,sg->v);
	AtVector2 blend;
	AtVector2 len = AtVector2(1/repeat.x,1/repeat.y);
	AtVector2 fuv;
	fuv.x = (iuv.x-floor(iuv.x/len.x)*len.x)/len.x;
	fuv.y = (iuv.y-floor(iuv.y/len.y)*len.y)/len.y;
	float p = 1/SAMPLE;
	// blend u
	if(fuv.x > (1 - p) && fuv.x <= 1)
	{
	  //sg->out.RGB = red;
	  blend.x = 1 - ((fuv.x - (1 - p))*SAMPLE);
	}
	else if(fuv.x > 0 && fuv.x <= p)
	{
	  blend.x = fuv.x*SAMPLE;
	}
	else
	{
	  blend.x = 1;
	}
	// blend v
	if(fuv.y > (1 - p) && fuv.y <= 1)
	{
	  blend.y = 1 - ((fuv.y - (1 - p))*SAMPLE);
	}
	else if(fuv.y > 0 && fuv.y <= p)
	{
	  blend.y = fuv.y*SAMPLE;
	}
	else
	{
	  blend.y = 1;
	}

	//sg->out.RGBA = image_v;
	sg->out.RGBA() = lerp(image_v, image, blend.y);
	// recover sg uv value
	SGC.restoreSG(sg);
}

/*node_loader
{
   if (i > 0)
      return false;

   node->methods     = TextureBlendMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "TextureBlend";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}*/   