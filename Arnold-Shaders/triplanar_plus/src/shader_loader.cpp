#include <ai.h>
#include <cstring>
 
extern AtNodeMethods* LcTriplanarMtd;
extern AtNodeMethods* LcTriplanar3DMtd;
 
enum{
    LcTriplanar = 0,
    LcTriplanar3D
};
 
node_loader
{
    switch (i)
    {
    case LcTriplanar:
        node->methods = LcTriplanarMtd;
        node->output_type = AI_TYPE_RGBA;
        node->name = "lc_triplanar";
        node->node_type = AI_NODE_SHADER;
        break;

    case LcTriplanar3D:
        node->methods = LcTriplanar3DMtd;
        node->output_type = AI_TYPE_RGBA;
        node->name = "lc_triplanar3d";
        node->node_type = AI_NODE_SHADER;
        break;
 
    default:
        return false;
    }
 
    strcpy(node->version, AI_VERSION);
    return true;
}