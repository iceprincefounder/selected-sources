#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolib/util/Path.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

namespace { //anonymous


inline bool doesExists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

class SpeedTreeInOp : public Foundry::Katana::GeolibOp
{
public:

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        if (interface.atRoot())
        {
            interface.stopChildTraversal();
        }

        // Look for a 'c' Op argument, representing an element in the
        // hierarchy leading to the base scene graph location that will
        // contain the spheres
        FnAttribute::GroupAttribute cGrpAttr = interface.getOpArg("c");
        if (cGrpAttr.isValid())
        {
            const int64_t numChildren = cGrpAttr.getNumberOfChildren();
            if (numChildren != 1)
            {
                // We expected exactly one child attribute in 'c', if it's not
                // the case we notify the user with an error
                Foundry::Katana::ReportError(interface,
                    "Unsupported attributes convention.");
                interface.stopChildTraversal();
                return;
            }

            const std::string childName =
                FnAttribute::DelimiterDecode(cGrpAttr.getChildName(0));
            FnAttribute::GroupAttribute childArgs = cGrpAttr.getChildByIndex(0);
            // Create a child location using the attribute name and forwarding
            // the hierarchy information
            interface.createChild(childName, "", childArgs);

            // Ignore other arguments as we've already found the 'c' group
            return;
        }

        // Look for a 'leaf' Op argument
        FnAttribute::GroupAttribute leaf = interface.getOpArg("leaf");
        if (!leaf.isValid())
        {
            FnAttribute::StringAttribute srtFileAttr = interface.getOpArg("srtFile");
            std::string srtFilePath = srtFileAttr.getValue();
            std::string abcProxyPath = srtFilePath.substr(0,srtFilePath.find_last_of('.'))+".abc";
            if (doesExists(abcProxyPath))
            {
                interface.setAttr("proxies.viewer.Proxy_Op0.opType", FnAttribute::StringAttribute("AlembicIn"));
                interface.setAttr("proxies.viewer.Proxy_Op0.opArgs.fileName", FnAttribute::StringAttribute(abcProxyPath));
            }
            FnAttribute::IntAttribute useGlobalMotionAttr = interface.getOpArg("useGlobalMotion");
            FnAttribute::FloatAttribute currentFrameAttr = interface.getOpArg("currentFrame");
            FnAttribute::IntAttribute enableMotionBlurAttr = interface.getOpArg("enableMotionBlur");
            FnAttribute::Attribute motionSamplesAttr = interface.getOpArg("motionSamples");

            FnAttribute::FloatAttribute fpsAttr = interface.getOpArg("fps");
            FnAttribute::FloatAttribute globalFrequencyAttr = interface.getOpArg("globalFrequency");
            FnAttribute::FloatAttribute gustFrequencyAttr = interface.getOpArg("gustFrequency");
            FnAttribute::FloatAttribute windSpeedAttr = interface.getOpArg("windSpeed");

            FnAttribute::FloatAttribute windDirectionxAttr = interface.getOpArg("windDirection.x");
            FnAttribute::FloatAttribute windDirectionyAttr = interface.getOpArg("windDirection.y");
            FnAttribute::FloatAttribute windDirectionzAttr = interface.getOpArg("windDirection.z");

            FnAttribute::IntAttribute windTypeAttr = interface.getOpArg("windType");
            FnAttribute::IntAttribute LODTypeAttr = interface.getOpArg("LODType");
            FnAttribute::IntAttribute LODSmoothTypeAttr = interface.getOpArg("LODSmoothType");

            FnAttribute::Attribute speedKeyFrameAttr = interface.getOpArg("speedKeyFrame");
            FnAttribute::Attribute speedResponseTimeAttr = interface.getOpArg("speedResponseTime");
            FnAttribute::Attribute speedKeyValueAttr = interface.getOpArg("speedKeyValue");
            FnAttribute::Attribute direKeyFrameFrameAttr = interface.getOpArg("direKeyFrame");
            FnAttribute::Attribute direResponseTimeAttr = interface.getOpArg("direResponseTime");
            FnAttribute::Attribute direKeyValueAttr = interface.getOpArg("direKeyValue");

            interface.setAttr("rendererProcedural.args.srtFile", srtFileAttr);

            interface.setAttr("rendererProcedural.args.useGlobalMotion", useGlobalMotionAttr);
            interface.setAttr("rendererProcedural.args.currentFrame", currentFrameAttr);
            interface.setAttr("rendererProcedural.args.enableMotionBlur", enableMotionBlurAttr);
            interface.setAttr("rendererProcedural.args.motionSamples", motionSamplesAttr);

            interface.setAttr("rendererProcedural.args.fps", fpsAttr);
            interface.setAttr("rendererProcedural.args.globalFrequency", globalFrequencyAttr);
            interface.setAttr("rendererProcedural.args.gustFrequency", gustFrequencyAttr);
            interface.setAttr("rendererProcedural.args.windSpeed", windSpeedAttr);
            
            interface.setAttr("rendererProcedural.args.windDirection_x", windDirectionxAttr);
            interface.setAttr("rendererProcedural.args.windDirection_y", windDirectionyAttr);
            interface.setAttr("rendererProcedural.args.windDirection_z", windDirectionzAttr);
            
            interface.setAttr("rendererProcedural.args.windType", windTypeAttr);
            interface.setAttr("rendererProcedural.args.LODType", LODTypeAttr);
            interface.setAttr("rendererProcedural.args.LODSmoothType", LODSmoothTypeAttr);

            interface.setAttr("rendererProcedural.args.speedKeyFrame", speedKeyFrameAttr);
            interface.setAttr("rendererProcedural.args.speedResponseTime", speedResponseTimeAttr);
            interface.setAttr("rendererProcedural.args.speedKeyValue", speedKeyValueAttr);
            interface.setAttr("rendererProcedural.args.direKeyFrame", direKeyFrameFrameAttr);
            interface.setAttr("rendererProcedural.args.direResponseTime", direResponseTimeAttr);
            interface.setAttr("rendererProcedural.args.direKeyValue", direKeyValueAttr);

            interface.setAttr("type", FnAttribute::StringAttribute("renderer procedural"));
            interface.setAttr("rendererProcedural.procedural", FnAttribute::StringAttribute("speedtree_procedural"));
            interface.setAttr("rendererProcedural.node", FnAttribute::StringAttribute("speedtree_procedural"));
            // Suppress Katana outputting automatic args for the camera
            interface.setAttr("rendererProcedural.includeCameraInfo",  FnAttribute::StringAttribute("None"));
            // classic, scenegraphAttr, typedArguments; default to typedArguments
            interface.setAttr("rendererProcedural.args.__outputStyle", FnAttribute::StringAttribute("typedArguments"));
            // Skip builtin parameters from Katana; the plugin doesn't use them
            interface.setAttr("rendererProcedural.args.__skipBuiltins", FnAttribute::IntAttribute(1));
            // Don't worry about bounds, let the procedural take care of that for its children
            interface.setAttr("rendererProcedural.useInfiniteBounds",  FnAttribute::IntAttribute(1));

            interface.stopChildTraversal();
        }

    }

protected:

};

DEFINE_GEOLIBOP_PLUGIN(SpeedTreeInOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(SpeedTreeInOp, "SpeedTreeIn", 0, 1);
}
