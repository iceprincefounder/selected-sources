// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifdef _WIN32
#include <FnPlatform/Windows.h>
#define _USE_MATH_DEFINES // for C++
#include <cmath>
#endif

#include <FnViewerModifier/plugin/FnViewerModifier.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttribute.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <string>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <vector>

struct Vector3f
{
    Vector3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}

    float x, y, z;
};

/**
 * Draws a line between two points. This relies on the calling code to have
 * called glBegin(GL_LINES) and to call glEnd().
 */
void drawLine(const Vector3f& pt1, const Vector3f& pt2)
{
    glVertex3f(pt1.x, pt1.y, pt1.z);
    glVertex3f(pt2.x, pt2.y, pt2.z);
}
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_LINE_LOOP);
    for (int ii = 0; ii < num_segments; ii++)   
        {
            float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
            float x = r * cosf(theta);//calculate the x component 
            float y = r * sinf(theta);//calculate the y component 
            glVertex2f(x + cx, y + cy);//output vertex 
        }
    glEnd();
}

/// Calculates the x and y point of a segment of a rounded rectangle.
void calculateDirectionalPoint(float &x, float &y, int segmentIndex,
                               int numSegments, float roundness)
{
    double a = M_PI_2 / numSegments * segmentIndex;
    double tan = tanf(a);
    double tmp = 1.0 / (1.0 + pow(tan, (double)roundness));
    x = pow(tmp, 1.0 / roundness);
    y = x * tan;
}

class DirectionalParams
{
public:
    static const int SEGMENTS = 10;

    float m_directionalMultiplier;
    float m_z;
    float m_cone_height;
    float m_cone_width;
    double m_roundness;

    DirectionalParams() : m_directionalMultiplier(0.0f), m_z(1.0f),
            m_cone_height(1.0f), m_cone_width(1.0f),  m_roundness(1.0) {}

    /// Calculates the directional properties of the light
    void init(float uSize, float vSize, float directionalMultiplier,
              float centerOfInterest)
    {
        m_directionalMultiplier = directionalMultiplier;
        // cone z position
        m_z = -centerOfInterest * sinf(m_directionalMultiplier * M_PI_2);

        // rectangle "roundness"
        m_roundness = powf(50.0f, powf(m_directionalMultiplier, 4.0f)) * 2.0f;

        m_cone_height = vSize + cos(m_directionalMultiplier * M_PI_2) * centerOfInterest;
        m_cone_width = uSize + cos(m_directionalMultiplier * M_PI_2) * centerOfInterest;
    }

    /// Returns true if a directional multiplier has been set to a non-zero value
    bool active() const
    {
        return m_directionalMultiplier > 1e-4f;
    }

};

/**
 * The LightViewerModifier controls how lights objects are displayed within
 * the viewer. Due to the inability to register multiple viewer modifiers for
 * the same location type ("light" locations in this case), this class must
 * be able to draw all types of light that may be encountered. The default for
 * lights that are not represented here is a standard point light.
 *
 * The type of light to draw is usually determined by the attributes that are
 * set on the input location, for example if a light has a cone angle attribute
 * it is assumed to be a spot light. It also possible to specify the light type
 * manually in the "material.lightParams.Type" or "material.viewerLightParams.Type"
 * attributes to one of the following:
 *
 *      - "omni"    (for point lights)
 *      - "spot"
 *      - "distant"
 *      - "quad"
 *      - "mesh"
 *      - "sphere"
 *      - "dome"
 */
class LightViewerModifier : public FnKat::ViewerModifier
{
public:
    enum LightType
    {
        LIGHT_POINT = 0,
        LIGHT_SPOT,
        LIGHT_QUAD,
        LIGHT_DISTANT,
        LIGHT_MESH,
        LIGHT_SPHERE,
        LIGHT_DOME,
        LIGHT_DISK,
        LIGHT_CYLINDER,       
    };
    LightType               m_lightType;
    float                   m_coa;
    float                   m_uSize, m_vSize;
    float                   m_centerOfInterest;
    float                   m_radius;
    GLUquadric*             m_quadric;
    DirectionalParams       m_directionalParams;

    mutable FnKat::StringAttribute  m_typeAttr; // Used to remember the type of light
    mutable bool                    m_testedTypeAttr;

    std::string m_metaLightType;
    std::string m_metaDefaultKey;

    LightViewerModifier(FnKat::GroupAttribute args) : FnKat::ViewerModifier(args),
            m_lightType(LIGHT_SPOT),
            m_coa(80.0f),
            m_uSize(1.0f),
            m_vSize(1.0f),
            m_centerOfInterest(10.0f),
            m_radius(1.0f),
            m_quadric(0),
            m_testedTypeAttr(false)
    {
        // Empty
    }

    static FnKat::ViewerModifier* create(FnKat::GroupAttribute args)
    {
        return (FnKat::ViewerModifier*)new LightViewerModifier(args);
    }

    static FnKat::GroupAttribute getArgumentTemplate()
    {
        FnKat::GroupBuilder gb;
        return gb.build();
    }

    /**
     * Gets the type of scene graph location that this viewer modifier runs on.
     */
    static const char* getLocationType()
    {
        return "light";
    }

    /**
     * Called per instance before each draw
     */
    void deepSetup(FnKat::ViewerModifierInput& input)
    {
        // Draw only the VMP representation in the viewer
        input.overrideHostGeometry();
    }

    /**
     * Called once per VMP instance when constructed
     */
    void setup(FnKat::ViewerModifierInput& input)
    {
        // Used for Spot, sphere and Dome lights
        m_quadric = gluNewQuadric();

        // Ensure that we pick up any changed light type attributes
        m_testedTypeAttr = false;

        // Try to get light type from material.meta attribute
        // Call through to other is*Light methods to interpret type and
        // gather type-specific args.
        lookupMetaLightType(input);

        FnKat::DoubleAttribute centerOfInterestAttr =
                input.getGlobalAttribute("geometry.centerOfInterest");
        m_centerOfInterest = centerOfInterestAttr.getValue(10.0f, false);

        // Default to point light
        m_lightType = LIGHT_POINT;
        if(isSpotLight(input))
        {
            m_lightType = LIGHT_SPOT;
        }
        else if(isDistantLight(input))
        {
            m_lightType = LIGHT_DISTANT;
        }
        else if(isPointLight(input))
        {
            m_lightType = LIGHT_POINT;
        }
        else if(isQuadLight(input))
        {
            m_lightType = LIGHT_QUAD;
        }
        else if(isMeshLight(input))
        {
            m_lightType = LIGHT_MESH;
        }
        else if(isSphereLight(input))
        {
            m_lightType = LIGHT_SPHERE;
        }
        else if(isDomeLight(input))
        {
            m_lightType = LIGHT_DOME;
        }
        else if(isDiskLight(input))
        {
            m_lightType = LIGHT_DISK;
        }
        else if(isCylinderLight(input))
        {
            m_lightType = LIGHT_CYLINDER;
        }

    }

    /**
     * Draw the light representation.
     * Note that this is also called during the selection picking pass, during
     * which point you should not adjust the assigned color.
     *
     * eg. if(input.getDrawOption("isPicking"))
     *     {
     *          // Drawing code for the selection buffer
     *     }
     */
    void draw(FnKat::ViewerModifierInput& input)
    {
        // Don't draw the light representation if we're being looked through
        if (input.isLookedThrough())
        {
            return;
        }

        glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);

        glDisable(GL_LIGHTING);

        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glLineWidth(1);

        // Set the color to use for drawing the light's representation
        float color[4] = {1, 1, 0, 1};  // yellow by default
        if(input.isSelected())
        {
            // Turn the color white
            color[2] = 1;
        }
        else
        {
            FnAttribute::FloatAttribute previewColorAttr = input.getAttribute(
                "geometry.previewColor");
            if (previewColorAttr.isValid())
            {
                FnAttribute::FloatAttribute::array_type previewColor =
                    previewColorAttr.getNearestSample(0);
                if (previewColor.size() >= 3)
                {
                    color[0] = previewColor[0];
                    color[1] = previewColor[1];
                    color[2] = previewColor[2];
                }
            }
        }
        glColor4fv(color);

        // Draw light depending on light type
        switch (m_lightType)
        {
        case LIGHT_POINT:
            drawPointLight(input);
            break;
        case LIGHT_SPOT:
            drawSpotLight(input);
            break;
        case LIGHT_QUAD:
            drawQuadLight(input);
            break;
        case LIGHT_DISTANT:
            drawDistantLight(input);
            break;
        case LIGHT_MESH:
            // Draw nothing
            break;
        case LIGHT_SPHERE:
            drawSphereLight(input);
            break;
        case LIGHT_DOME:
            drawDomeLight(input);
            break;
        case LIGHT_DISK:
            drawDiskLight(input);
            break;
        case LIGHT_CYLINDER:
            drawCylinderLight(input);
            break;
        };

        // Restore the original options
        if(input.getDrawOption("isPicking"))
        {
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Restore Draw State
        glPopAttrib();
    }

    /**
     * Called when the location is removed/refreshed.
     */
    void cleanup(FnKat::ViewerModifierInput& input)
    {
        // Empty
    }

    /**
     * Called per instance after each draw
     */
    void deepCleanup(FnKat::ViewerModifierInput& input)
    {
        // Empty
    }

    /**
     * Returns a bounding box for the current location for use with the viewer
     * scene graph.
     */
    FnKat::DoubleAttribute getLocalSpaceBoundingBox(FnKat::ViewerModifierInput& input)
    {
        if(m_lightType == LIGHT_QUAD)
        {
            if(m_directionalParams.active())
            {
                double bounds[6] = {
                    std::min(-m_uSize, -m_directionalParams.m_cone_width),
                    std::max(m_uSize, m_directionalParams.m_cone_width),
                    std::min(-m_vSize, -m_directionalParams.m_cone_height),
                    std::max(m_vSize, m_directionalParams.m_cone_height),
                    m_directionalParams.m_z, 1.0
                };
                return FnKat::DoubleAttribute(bounds, 6, 1);
            }
            else
            {
                double bounds[6] = {-m_uSize, m_uSize, -m_vSize, m_vSize, -1, 1};
                return FnKat::DoubleAttribute(bounds, 6, 1);
            }
        }
        else if(m_lightType == LIGHT_SPHERE)
        {
            double bounds[6] = {-m_radius, m_radius, -m_radius, m_radius, -m_radius, m_radius};
            return FnKat::DoubleAttribute(bounds, 6, 1);
        }
        else
        {
            double bounds[6] = {-1, 1, -1, 1, -1, 1};
            return FnKat::DoubleAttribute(bounds, 6, 1);
        }
    }

    static void flush() {}

    static void onFrameBegin() {}

    static void onFrameEnd() {}

private:
    //=======================================================================
    // Light type identification
    //=======================================================================

    void lookupMetaLightType(FnKat::ViewerModifierInput& input)
    {
        FnKat::GroupAttribute materialAttr =
                input.getGlobalAttribute("material");

        FnKat::StringAttribute defaultKeyAttr =
                materialAttr.getChildByName("meta.defaultKey");
        m_metaDefaultKey = defaultKeyAttr.getValue("", false);

        if (!m_metaDefaultKey.empty())
        {
            FnKat::StringAttribute lightTypeAttr =
                    materialAttr.getChildByName("meta.lightType." +
                            m_metaDefaultKey);
            m_metaLightType = lightTypeAttr.getValue("", false);
        }
        else
        {
            FnKat::GroupAttribute lightTypeGroupAttr = 
                    materialAttr.getChildByName("meta.lightType");

            for (int64_t i = 0, e = lightTypeGroupAttr.getNumberOfChildren();
                 i != e; ++i)
            {
                FnKat::StringAttribute lightTypeAttr =
                        lightTypeGroupAttr.getChildByIndex(i);

                if (lightTypeAttr.isValid())
                {
                    m_metaDefaultKey = lightTypeGroupAttr.getChildName(i);
                    m_metaLightType = lightTypeAttr.getValue("", false);
                    break;
                }
            }
        }
    }

    FnKat::Attribute lookupMetaShaderParam(FnKat::ViewerModifierInput& input,
            const std::string & shaderName)
    {
        FnKat::StringAttribute nameAttr = input.getGlobalAttribute(
                "material.meta." + shaderName + "." + m_metaDefaultKey);
        if (nameAttr.isValid())
        {
            FnKat::StringAttribute::array_type values =
                    nameAttr.getNearestSample(0.0);

            for (FnKat::StringAttribute::array_type::const_iterator I =
                    values.begin(), E = values.end(); I != E; ++I)
            {
                FnKat::Attribute shaderAttr = 
                        input.getGlobalAttribute(
                                std::string("material.")+(*I));
                if (shaderAttr.isValid())
                {
                    return shaderAttr;
                }
            }
        }

        return FnKat::Attribute();
    }
    // isDiskLight
    bool isDiskLight(FnKat::ViewerModifierInput& input)
    {
        FnKat::StringAttribute aMaterialAttr = input.getGlobalAttribute("material.arnoldLightShader");
        std::string lightShader = aMaterialAttr.getValue("",false);
        if(lightShader == "disk_light")
        {
            return true;
        }
        else
        {
            return false;
        }    
    }
    // isCylinderLight
    bool isCylinderLight(FnKat::ViewerModifierInput& input)
    {
        FnKat::StringAttribute aMaterialAttr = input.getGlobalAttribute("material.arnoldLightShader");
        std::string lightShader = aMaterialAttr.getValue("",false);
        if(lightShader == "cylinder_light")
        {
            return true;
        }
        else
        {
            return false;
        }    
    }
    /**
     * Determine whether this is a spot light and store cone angle for drawing.
     */
    bool isSpotLight(FnKat::ViewerModifierInput& input)
    {
        if (m_metaLightType == "spot")
        {
            FnKat::FloatAttribute coaAttr =
                    lookupMetaShaderParam(input, "coneAngleName");

            if (coaAttr.isValid())
            {
                // Store the cone outer angle for use in drawing
                m_coa = coaAttr.getValue(80.0f, false);
                return true;
            }
        }

        static std::vector<std::string> coaAttrNames;
        if(coaAttrNames.empty())
        {
            coaAttrNames.push_back("material.prmanLightParams.Cone_Outer_Angle");
            coaAttrNames.push_back("material.prmanLightParams.OuterAngle");
            coaAttrNames.push_back("material.arnoldLightParams.wide_angle");
            coaAttrNames.push_back("material.arnoldLightParams.cone_angle");
            coaAttrNames.push_back("material.testLightParams.coneAngle");
            coaAttrNames.push_back("material.parameters.Cone_Outer_Angle");
            coaAttrNames.push_back("material.parameters.OuterAngle");
            coaAttrNames.push_back("material.parameters.wide_angle");
            coaAttrNames.push_back("material.parameters.cone_angle");
            coaAttrNames.push_back("material.viewerLightParams.Cone_Outer_Angle");
            coaAttrNames.push_back("material.parameters.coneAngle");
            coaAttrNames.push_back("material.vrayLightParams.coneAngle");
        }

        std::vector<std::string>::const_iterator it = coaAttrNames.begin();
        for(; it != coaAttrNames.end(); ++it)
        {
            FnKat::FloatAttribute coaAttr = 
                    input.getGlobalAttribute(*it);
            if(coaAttr.isValid())
            {
                // Store the cone outer angle for use in drawing
                m_coa = coaAttr.getValue(80.0f, false);
                return true;
            }
        }

        // Test if light type parameter is set - in this case the default cone
        // angle is used
        return isLightOfType(input, "spot");
    }

    /**
     * Determine whether this is a distance / directional light.
     */
    bool isDistantLight(FnKat::ViewerModifierInput& input) const
    {
        FnKat::StringAttribute aMaterialAttr = input.getGlobalAttribute("material.arnoldLightShader");
        std::string lightShader = aMaterialAttr.getValue("",false);
        if(lightShader == "distant_light")
        {
            return true;
        }
        else
        {
            return false;
        }    
    }
/*    {
        static std::vector<std::string> distantLightAttrNames;
        if(distantLightAttrNames.empty())
        {
            distantLightAttrNames.push_back("material.arnoldLightParams.direction");
            distantLightAttrNames.push_back("material.parameters.direction");
            distantLightAttrNames.push_back("material.testLightParams.physical_sun");
            distantLightAttrNames.push_back("material.vrayLightParams.beamRadius");
            distantLightAttrNames.push_back("material.parameters.beamRadius");
            distantLightAttrNames.push_back("material.vrayLightParams.ozone");
            distantLightAttrNames.push_back("material.parameters.ozone");
        }

        std::vector<std::string>::const_iterator it = distantLightAttrNames.begin();
        for(; it != distantLightAttrNames.end(); ++it)
        {
            // One of these attributes only needs to be valid
            if(input.getGlobalAttribute(*it).isValid())
            {
                return true;
            }
        }

        // Test if light type parameter is set
        return isLightOfType(input, "distant");
    }*/

    /**
     * Determine whether this is a point light.
     */
    bool isPointLight(FnKat::ViewerModifierInput& input) const
    {
        // Test if light type parameter is set
        return isLightOfType(input, "omni") || isLightOfType(input, "point");
    }

    /**
     * Determine whether this is a quad light and store its dimensions.
     */
    bool isQuadLight(FnKat::ViewerModifierInput& input)
    {
        static std::vector<std::string> quadLightAttrNames;
        if(quadLightAttrNames.empty())
        {
            quadLightAttrNames.push_back("material.arnoldLightParams.vertices");
            quadLightAttrNames.push_back("material.parameters.vertices");
            quadLightAttrNames.push_back("material.testLightParams.disc");
        }

        std::vector<std::string>::const_iterator it = quadLightAttrNames.begin();
        for(; it != quadLightAttrNames.end(); ++it)
        {
            // These attributes only need to be valid
            if(input.getGlobalAttribute(*it).isValid())
            {
                return true;
            }
        }

        // V-Ray Rectangle Light
        FnKat::FloatAttribute uSizeAttr =
                input.getGlobalAttribute("material.vrayLightParams.u_size");
        FnKat::FloatAttribute vSizeAttr =
                input.getGlobalAttribute("material.vrayLightParams.v_size");
        if(uSizeAttr.isValid() && vSizeAttr.isValid())
        {
            m_uSize = uSizeAttr.getValue(0.0f, false);
            m_vSize = vSizeAttr.getValue(0.0f, false);
            FnKat::FloatAttribute directionalAttr =
                    input.getGlobalAttribute("material.vrayLightParams.directional");
            m_directionalParams.init(m_uSize,
                                     m_vSize,
                                     directionalAttr.getValue(0.0f, false),
                                     m_centerOfInterest);
            return true;
        }

        uSizeAttr = input.getGlobalAttribute("material.parameters.u_size");
        vSizeAttr = input.getGlobalAttribute("material.parameters.v_size");
        if(uSizeAttr.isValid() && vSizeAttr.isValid())
        {
            m_uSize = uSizeAttr.getValue(0.0f, false);
            m_vSize = vSizeAttr.getValue(0.0f, false);
            FnKat::FloatAttribute directionalAttr =
                    input.getGlobalAttribute("material.parameters.directional");
            m_directionalParams.init(m_uSize,
                                     m_vSize,
                                     directionalAttr.getValue(0.0f, false),
                                     m_centerOfInterest);
            return true;
        }

        // Test if light type parameter is set - in this case the default
        // dimensions are used
        return isLightOfType(input, "quad");
    }

    /**
     * Determine whether this is a Mesh light.
     */
    bool isMeshLight(FnKat::ViewerModifierInput& input) const
    {
        if(input.getGlobalAttribute("material.arnoldLightParams.mesh").isValid())
            return true;

        // Test if light type parameter is set
        return isLightOfType(input, "mesh");
    }

    /**
     * Determine whether this is a Sphere light and store the sphere radius.
     */
    bool isSphereLight(FnKat::ViewerModifierInput& input)
    {
        static std::vector<std::string> sphereLightAttrNames;
        if(sphereLightAttrNames.empty())
        {
            sphereLightAttrNames.push_back("material.vrayLightParams.radius");
            sphereLightAttrNames.push_back("material.parameters.radius");
        }

        FnKat::FloatAttribute radiusAttr;
        std::vector<std::string>::const_iterator it = sphereLightAttrNames.begin();
        for(; it != sphereLightAttrNames.end(); ++it)
        {
            radiusAttr = input.getGlobalAttribute(*it);
            if(radiusAttr.isValid())
            {
                m_radius = radiusAttr.getValue(0.0f, false);
                return true;
            }
        }

        // Test if light type parameter is set - in this case the default
        // radius is used
        return isLightOfType(input, "sphere");
    }

    /**
     * Determine whether this is a dome light.
     */
    bool isDomeLight(FnKat::ViewerModifierInput& input) const
    {
        static std::vector<std::string> domeLightAttrNames;
        if(domeLightAttrNames.empty())
        {
            domeLightAttrNames.push_back("material.vrayLightParams.dome_targetRadius");
            domeLightAttrNames.push_back("material.parameters.dome_targetRadius");
        }

        std::vector<std::string>::const_iterator it = domeLightAttrNames.begin();
        for(; it != domeLightAttrNames.end(); ++it)
        {
           if(input.getGlobalAttribute(*it).isValid())
           {
               return true;
           }
        }

        // Test if light type parameter is set
        return isLightOfType(input, "dome");
    }

    /**
     * Checks certain attributes to determine whether the type of light has been
     * manually specified.
     */
    bool isLightOfType(FnKat::ViewerModifierInput& input, std::string lightType) const
    {
        if (m_metaLightType == lightType) return true;

        if(m_testedTypeAttr)
        {
            if(m_typeAttr.getValue("", false) == lightType)
            {
                return true;
            }
            return false;
        }

        // Optimization to ensure that we only try to get the type once per draw
        m_testedTypeAttr = true;

        // Loop through the list of attributes where the type of light can be
        // specified directly
        static std::vector<std::string> typeAttrNames;
        if(typeAttrNames.empty())
        {
            typeAttrNames.push_back("material.lightParams.Type");
            typeAttrNames.push_back("material.viewerLightParams.Type");
        }

        FnKat::StringAttribute typeAttr;
        std::vector<std::string>::const_iterator it = typeAttrNames.begin();
        for(; it != typeAttrNames.end(); ++it)
        {
            typeAttr = input.getGlobalAttribute(*it);
            if(typeAttr.isValid())
            {
                // Store the valid attribute to use in successive queries
                m_typeAttr = typeAttr;
                return typeAttr.getValue("", false) == lightType;
            }
        }

        return false;
    }

    //=======================================================================
    // Drawing helpers
    //=======================================================================
    void drawDiskLight(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();
        glScalef(2.0, 2.0, 2.0);
        drawCircle(0,0,0.5,8);

        glPushMatrix();
        glRotatef(45,0,0,1);
        glBegin(GL_LINES);
        glVertex3f( -0.1,  0,  0);
        glVertex3f( 0.1,  0,  0);
        glVertex3f( 0,  -0.1,  0);
        glVertex3f( 0,  0.1,  0);
        glVertex3f( 0,  0,  0);
        glVertex3f( 0,  0,  -1);
        glVertex3f( 0,  0,  -1);
        glVertex3f( 0,  0.25,  -0.75);
        glEnd();        
        glPopMatrix();

        glPopMatrix();

    }
    void drawCylinderLight(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();

        glPushMatrix();
        glRotatef(90,1,0,0);
        glTranslatef(0,0,1);
        drawCircle(0,0,1,8);
        glPopMatrix();

        glPushMatrix();
        glRotatef(90,1,0,0);
        glTranslatef(0,0,-1);
        drawCircle(0,0,1,8);
        glPopMatrix();

        glPushMatrix();
        glBegin(GL_LINES);
        glVertex3f( 1,   1,  0);
        glVertex3f( 1,  -1,  0);
        glVertex3f(-1,   1,  0);
        glVertex3f(-1,  -1,  0);
        glVertex3f( 0,   1,  1);
        glVertex3f( 0,  -1,  1);
        glVertex3f( 0,   1, -1);
        glVertex3f( 0,  -1, -1);
        
        float v = sqrt(2)/2;
        glVertex3f( v,   1, v);
        glVertex3f( v,  -1, v);
        glVertex3f( -v,   1, v);
        glVertex3f( -v,  -1, v);
        glVertex3f( v,   1, -v);
        glVertex3f( v,  -1, -v);
        glVertex3f( -v,   1, -v);
        glVertex3f( -v,  -1, -v);

        glEnd();
        glPopMatrix();

        glPopMatrix();


    }


    /**
     * Draw the point light representation.
     */
    void drawPointLight(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();
        glScalef(0.2, 0.2, 0.2);

        glPushMatrix();
        glRotatef(0,1,0,0);
        drawCircle(0,0,1,8);
        glPopMatrix();
        glPushMatrix();
        glRotatef(90,1,0,0);
        drawCircle(0,0,1,8);
        glPopMatrix();
        glPushMatrix();
        glRotatef(0,0,0,1);
        glRotatef(90,0,1,0);
        drawCircle(0,0,1,8);
        glPopMatrix();


        glPopMatrix();

        glPushMatrix();

        glBegin(GL_LINES);

        glVertex3f( -1,  0,  0);             glVertex3f(  1,  0,  0);
        glVertex3f(  0, -1,  0);             glVertex3f(  0,  1,  0);
        glVertex3f(  0,  0, -1);             glVertex3f(  0,  0,  1);

        glVertex3f( -0.707, -0.707,  0.000); glVertex3f( 0.707,  0.707,  0.000);
        glVertex3f(  0.000, -0.707, -0.707); glVertex3f( 0.000,  0.707,  0.707);
        glVertex3f( -0.707,  0.000, -0.707); glVertex3f( 0.707,  0.000,  0.707);
        glVertex3f( -0.707,  0.707,  0.000); glVertex3f( 0.707, -0.707,  0.000);
        glVertex3f(  0.000, -0.707,  0.707); glVertex3f( 0.000,  0.707,  -0.707);
        glVertex3f( -0.707,  0.000,  0.707); glVertex3f( 0.707,  0.000,  -0.707);

        glEnd();
        glPopMatrix();

        
    }

    /**
     * Draw the spot light representation.
     */
    void drawSpotLight(FnKat::ViewerModifierInput& input)
    {

        float height=1;
        float scale=1;

        float coa = std::min(std::max(m_coa,0.0f), 180.0f);
        float tangent = tan(coa/2 * 3.14159f/180.0f);
        float radius = tangent*height;

        //normalize length to 1
        if(height != 0 || radius != 0)
        {
            float length = sqrt(height*height + radius*radius);
            height /= length;
            radius /= length;
        }

        height*=scale;
        radius*=scale;

        glPushMatrix();

        glScalef(2, 2, -2);
        gluCylinder(m_quadric, 0, radius, height, 8, 1);

        glPopMatrix();
    }

    /**
     * Draw the quad light representation.
     */
    void drawQuadLight(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();

        // To simplify selecting the light, when we are "picking" set
        // PolygonMode to FILL and increase the line width.
        if(input.getDrawOption("isPicking"))
        {
            glLineWidth(10);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glBegin(GL_POLYGON);
        glVertex3f(-m_uSize, -m_vSize, 0);
        glVertex3f(m_uSize, -m_vSize, 0);
        glVertex3f(m_uSize, m_vSize, 0);
        glVertex3f(-m_uSize, m_vSize, 0);
        glEnd();

        float uvAverage = (m_uSize + m_vSize) * 0.5;
        glPushMatrix();
        glRotatef(45,0,0,1);
        glBegin(GL_LINES);
        glVertex3f( -0.25,  0,  0);
        glVertex3f( 0.25,  0,  0);
        glVertex3f( 0,  -0.25,  0);
        glVertex3f( 0,  0.25,  0);
        glEnd();
        glPopMatrix();
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, -uvAverage);
        glVertex3f(0, 0, -uvAverage);
        glVertex3f(0, uvAverage * 0.25f,-uvAverage * 0.75f);
        // Draw directionality cone if one exists
        if(m_directionalParams.active())
        {
            // segment coordinates for the four quadrants
            Vector3f pt1[2], pt2[2], pt3[2], pt4[2];

            // starting points of the cone curves
            pt1[0] = Vector3f(m_directionalParams.m_cone_width, 0.0f, m_directionalParams.m_z);
            pt2[0] = Vector3f(0.0f, m_directionalParams.m_cone_height, m_directionalParams.m_z);
            pt3[0] = Vector3f(-m_directionalParams.m_cone_width, 0.0f, m_directionalParams.m_z);
            pt4[0] = Vector3f(0.0f, -m_directionalParams.m_cone_height, m_directionalParams.m_z);

            // For each segment of the far cone draw a line between the current
            // point and the previous point.
            int pointId = 1;
            for (int i = 0; i < m_directionalParams.SEGMENTS; i++)
            {
                float x, y;
                calculateDirectionalPoint(x, y, i, m_directionalParams.SEGMENTS, m_directionalParams.m_roundness);
                float x2 = y * m_directionalParams.m_cone_width;
                float y2 = x * m_directionalParams.m_cone_height;
                x *= m_directionalParams.m_cone_width;
                y *= m_directionalParams.m_cone_height;

                pt1[pointId] = Vector3f(x, y, m_directionalParams.m_z);
                drawLine(pt1[0], pt1[1]);

                pt2[pointId] = Vector3f(-x2, y2, m_directionalParams.m_z);
                drawLine(pt2[0], pt2[1]);

                pt3[pointId] = Vector3f(-x, -y, m_directionalParams.m_z);
                drawLine(pt3[0], pt3[1]);

                pt4[pointId] = Vector3f(x2, -y2, m_directionalParams.m_z);
                drawLine(pt4[0], pt4[1]);

                pointId = 1 - pointId;

                if (i == m_directionalParams.SEGMENTS / 2) {
                    // Draw the cone lines that connect the near and far planes
                    // of the cone.
                    pt1[pointId] = Vector3f(m_uSize, m_vSize, 0.0f);
                    drawLine(pt1[0], pt1[1]);

                    pt2[pointId] = Vector3f(-m_uSize, m_vSize, 0.0f);
                    drawLine(pt2[0], pt2[1]);

                    pt3[pointId] = Vector3f(-m_uSize, -m_vSize, 0.0f);
                    drawLine(pt3[0], pt3[1]);

                    pt4[pointId] = Vector3f(m_uSize, -m_vSize, 0.0f);
                    drawLine(pt4[0], pt4[1]);
                }
            }

            // close the cone curves
            pt1[pointId] = Vector3f(0.0f, m_directionalParams.m_cone_height, m_directionalParams.m_z);
            drawLine(pt1[0], pt1[1]);

            pt2[pointId] = Vector3f(-m_directionalParams.m_cone_width, 0.0f, m_directionalParams.m_z);
            drawLine(pt2[0], pt2[1]);

            pt3[pointId] = Vector3f(0.0f, -m_directionalParams.m_cone_height, m_directionalParams.m_z);
            drawLine(pt3[0], pt3[1]);

            pt4[pointId] = Vector3f(m_directionalParams.m_cone_width, 0.0f, m_directionalParams.m_z);
            drawLine(pt4[0], pt4[1]);
        }
        glEnd();


        glPopMatrix();
    }

    /**
     * Draw the distant light representation.
     */
    void drawDistantLight(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();
        glScalef(1.5,1.5,1.5);

        glPushMatrix();
        glTranslatef(0,-0.25,0);
        glBegin(GL_LINES);
        glVertex3f(0,0,1);
        glVertex3f(0,0,-1);
        glVertex3f(0,0,-0.9);
        glVertex3f(0,0.2,-0.7);
        glVertex3f(0.14,0,-1);
        glVertex3f(-0.14,0,-1);
        glVertex3f(0.14,0,-1);
        glVertex3f(0,0,-1.4);
        glVertex3f(-0.14,0,-1);
        glVertex3f(0,0,-1.4);
        glEnd();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.25,0.25,0);
        glRotatef(-45,0,0,1);
        glBegin(GL_LINES);
        glVertex3f(0,0,1);
        glVertex3f(0,0,-1);
        glVertex3f(0.14,0,-1);
        glVertex3f(-0.14,0,-1);
        glVertex3f(0.14,0,-1);
        glVertex3f(0,0,-1.4);
        glVertex3f(-0.14,0,-1);
        glVertex3f(0,0,-1.4);
        glEnd();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-0.25,0.25,0);
        glRotatef(45,0,0,1);
        glBegin(GL_LINES);
        glVertex3f(0,0,1);
        glVertex3f(0,0,-1);
        glVertex3f(0.14,0,-1);
        glVertex3f(-0.14,0,-1);
        glVertex3f(0.14,0,-1);
        glVertex3f(0,0,-1.4);
        glVertex3f(-0.14,0,-1);
        glVertex3f(0,0,-1.4);
        glEnd();
        glPopMatrix();

        glPopMatrix();

    }

    /**
     * Draw the sphere light representation.
     */
    void drawSphereLight(FnKat::ViewerModifierInput& input)
    {
        // To simplify selecting the light, when we are "picking" set
        // PolygonMode increase the line width.
        if(input.getDrawOption("isPicking"))
        {
            glLineWidth(10);
        }

        glPushMatrix();
        gluDisk(m_quadric, m_radius, m_radius, 64, 1);

        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(m_quadric, m_radius, m_radius, 64, 1);

        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        gluDisk(m_quadric, m_radius, m_radius, 64, 1);


        glPopMatrix();
    }

    /**
     * Draw the dome light representation.
     */
    void drawDomeLight(FnKat::ViewerModifierInput& input)
    {
        // To simplify selecting the light, when we are "picking" set
        // PolygonMode increase the line width.
        if(input.getDrawOption("isPicking"))
        {
            glLineWidth(10);
        }

        glPushMatrix();
        drawHalfDisk(64, 1.0f);

        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawHalfDisk(64, 1.0f);

        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(m_quadric, 1.0f, 1.0f, 64, 1);

        glPopMatrix();
    }

    /**
     * Draw a semi-circle.
     */
    static void drawHalfDisk(int segments, float radius)
    {
        glBegin(GL_LINE_STRIP);
        for (int ii = 0; ii < segments + 1; ++ii)
        {
            float angle = (M_PI / segments) * ii;
            glVertex3f(radius*cosf(angle), radius*sinf(angle), 0.0f);
        }
        glEnd();
    }
};

DEFINE_VMP_PLUGIN(LightViewerModifier)

void registerPlugins()
{
    std::cout << "[LCA PLUGIN]: Register LightViewerModifier v2.5" << std::endl;
    REGISTER_PLUGIN(LightViewerModifier, "LightViewerModifier", 0, 2);
}
