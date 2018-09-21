#include <ai.h>

#include "lc_parallax_map.h"

AI_SHADER_NODE_EXPORT_METHODS(LcParallaxMapMethods);

enum Params 
{
	p_standard_surface = 0,
	p_parallax_map,
	p_parallax_value,
};

node_parameters
{
	AiParameterClosure("standard_surface");
	AiParameterRGB("parallax_map",1.0f,1.0f,1.0f);
	AiParameterFlt("parallax_value",0.0f);
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
	float parallax_value = AiShaderEvalParamFlt(p_parallax_value);
	AtRGB parallax_map = AiShaderEvalParamRGB(p_parallax_map);
	// get tangent space
	AtVector aTangent = sg->dPdu;
	AtVector aBitangent = sg->dPdv;
	AtVector aNormal = sg->N;
	aTangent = AiV3Normalize(aTangent);
	aBitangent = AiV3Normalize(aBitangent);
	aNormal = AiV3Normalize(aNormal);
	AtMatrix TBN = AiM4Identity();
	TBN[0][0] = aTangent[0];
	TBN[0][1] = aTangent[1];
	TBN[0][2] = aTangent[2];
	TBN[0][3] = 0;
	TBN[1][0] = aBitangent[0];
	TBN[1][1] = aBitangent[1];
	TBN[1][2] = aBitangent[2];
	TBN[1][3] = 0;
	TBN[2][0] = aNormal[0];
	TBN[2][1] = aNormal[1];
	TBN[2][2] = aNormal[2];
	TBN[2][3] = 0;
	TBN[3][0] = 0.0;
	TBN[3][1] = 0.0;
	TBN[3][2] = 0.0;
	TBN[3][3] = 1.0;
	TBN = AiM4Transpose(TBN);
	AtVector viewPos = sg-> Ro;
	AtVector sgPos = sg-> P;
	// Right-multiply a vector
	//AtVector TangentViewPos = AiM4VectorByMatrixTMult(TBN,viewPos);
	//AtVector TangentFragPos = AiM4VectorByMatrixTMult(TBN,sgPos);
	AtVector TangentViewPos = AiM4PointByMatrixMult(TBN,viewPos);
	AtVector TangentFragPos = AiM4PointByMatrixMult(TBN,sgPos);

	AtVector viewDir  = AiV3Normalize(TangentFragPos - TangentViewPos);

	AtVector2 texcoords = AtVector2(sg->u,sg->v);
	AtVector2 p = AtVector2(viewDir.x,viewDir.y)/viewDir.z * (parallax_map.r * parallax_value);
	AtVector2 texCoords = texcoords - p;

	// number of depth layers
	const float minLayers = 32.0;
	const float maxLayers = 64.0;
	float numLayers = lerp(maxLayers, minLayers, std::abs(AiV3Dot(AtVector(0.0, 0.0, 1.0), viewDir))); 

	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	AtVector2 P = AtVector2(viewDir.x,viewDir.y) * parallax_value; 
	AtVector2 deltaTexCoords = P / numLayers;
	// get initial values
	AtVector2 currentTexCoords = AtVector2(sg->u,sg->v);
	float currentDepthMapValue = parallax_map.r;
	SGCache SGC;
	SGC.initCache(sg);
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		sg->u = currentTexCoords.x;
		sg->v = currentTexCoords.y;
		currentDepthMapValue = AiShaderEvalParamRGB(p_parallax_map).r;
		SGC.restoreSG(sg);
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}

	// get texture coordinates before collision (reverse operations)
	AtVector2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	sg->u = prevTexCoords.x;
	sg->v = prevTexCoords.y;
	float beforeDepth = AiShaderEvalParamRGB(p_parallax_map).r - currentLayerDepth + layerDepth;
	SGC.restoreSG(sg);
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	AtVector2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);


	//if(finalTexCoords.x > 1.0 || finalTexCoords.y > 1.0 || finalTexCoords.x < 0.0 || finalTexCoords.y < 0.0)
	//{
	//	finalTexCoords.x = sg->u;
	//	finalTexCoords.y = sg->v;	
	//}

	sg->u = finalTexCoords.x;
	sg->v = finalTexCoords.y;

	sg->out.CLOSURE() = AiShaderEvalParamClosure(p_standard_surface);

}

node_loader
{
	if (i > 0)
	  return false;
	node->methods     = LcParallaxMapMethods;
	node->output_type = AI_TYPE_CLOSURE;
	node->name        = "lc_parallax_map";
	node->node_type   = AI_NODE_SHADER;
	strcpy(node->version, AI_VERSION);
	return true;
}