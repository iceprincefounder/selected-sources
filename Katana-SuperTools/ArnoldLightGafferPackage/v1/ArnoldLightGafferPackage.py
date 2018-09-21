# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI, Decorators, Plugins
import PackageSuperToolAPI.NodeUtils as NU
from PackageSuperToolAPI import Packages

import os
import textwrap
import logging
import tempfile

log = logging.getLogger("GafferThree.ArnoldLightGafferPackage")

_iconsDir = os.path.join(os.path.dirname(__file__), 'icons')

# Get base classes for our packages from the registered GafferThree packages
GafferThreeAPI = Plugins.GafferThreeAPI
LightPackage = GafferThreeAPI.PackageClasses.LightPackage
LightEditPackage = GafferThreeAPI.PackageClasses.LightEditPackage


class ArnoldLightGafferPackage(LightPackage):
    """
    Implements an Arnold spot_light as a package. We inherit LightPackage,
    and override some functions to modify behaviour.
    """

    # Class Variables ---------------------------------------------------------

    ADD_MENU_GROUP_NAME = "LCA Arnold"

    # The name of the package type as it should be shown in the UI
    DISPLAY_NAME = 'LCA Arnold Light'

    # The default name of a package when it is created. This also defines the
    # default name of the package's scene graph location
    DEFAULT_NAME = 'Lighting GOD'

    # The icon to use to represent this package type in the UI
    DISPLAY_ICON = os.path.join(_iconsDir, "aiSpotLight16.png")

    # The default size of the spot
    DEFAULT_SIZE = 1

    # Class Functions ---------------------------------------------------------

    @classmethod
    def create(cls, enclosingNode, locationPath, shaderName=''):
        """
        A factory method which returns an instance of the class.

        @type enclosingNode: C{NodegraphAPI.Node}
        @type locationPath: C{str}
        @rtype: L{LightPackage}
        @param enclosingNode: The parent node within which the new
            package's node should be created.
        @param locationPath: The path to the location to be created/managed
            by the package.
        @return: The newly-created package instance.
        """
        # Create the package node
        packageNode = NodegraphAPI.CreateNode('Group', enclosingNode)
        packageNode.addOutputPort('out')

        # Add parameter containing the package type and location path to the package node
        NU.AddPackageTypeAndPath(packageNode, cls.__name__, locationPath)

        # Create an expression to link the name of the sky dome location to the name of the package
        locExpr = '=^/%s' % NU.GetPackageLocationParameterPath()

        # Create geometry for the light - in this case a bounded plane
        createNode = NodegraphAPI.CreateNode('LightCreate', packageNode)
        createNode.getParameter('transform.scale.x').setValue(cls.DEFAULT_SIZE, 0)
        scaleExpr = "=transform.scale.x"
        createNode.getParameter('transform.scale.y').setExpression(scaleExpr)
        createNode.getParameter('transform.scale.z').setExpression(scaleExpr)
        createNode.getParameters().createChildNumber('forceAsStaticScene', 1)

        createNodeExtraAttrsParameter = createNode.getParameters().createChildGroup('extraAttrs')
        createNodeExtraAttrsParameter.createChildString('__gafferPackage', '').setExpression('@%s' % packageNode.getName())
        createNode.getParameter('name').setExpression(locExpr)
        createNode.addInputPort('masterMaterial')

        # Store the package class as a parameter on the create node
        NU.SetOrCreateDeepScalarParameter(createNode.getParameters(),
                                          'extraAttrs.info.gaffer.packageClass',
                                          cls.__name__)

        # Add the package location to the light list at /root
        lightListEditNode = NodegraphAPI.CreateNode('LightListEdit', packageNode)
        lightListEditNode.setName("LightListEdit")
        lightListEditNode.getParameter('locations.i0').setExpression(locExpr)

        # Create the Material node
        materialNode = NodegraphAPI.CreateNode('Material', packageNode)
        materialNode.getParameter('action').setValue('edit material', 0)
        materialNode.getParameter('makeInteractive').setValue('Yes', 0)
        materialNode.getParameters().createChildNumber('makeInteractiveSpecificToMaterial', 1)
        materialNode.getParameter('edit.location').setExpression(locExpr)

        # Create an OpScript node to copy the transform from the sphere to the shader
        copyXformOpScriptNode = cls.createOpScriptNode(packageNode,
                                                         'CopyXformOpScript',
                                                         cls.getCopyXformOpScript())

        # Create a DistantPort node to reference the master material
        masterMaterialDistantPortNode = NodegraphAPI.CreateNode('DistantPort', packageNode)
        masterMaterialDistantPortNode.setBypassed(True)

        # Create a ViewerObjectSettings node in case we want to customize how
        # the spot light is displayed in the viewer
        viewerObjectSettingsNode = NodegraphAPI.CreateNode('ViewerObjectSettings', packageNode)
        viewerObjectSettingsNode.getParameter('CEL').setExpression(locExpr)

        # Create the Merge node, for merging in child packages
        mergeNode = NodegraphAPI.CreateNode('Merge', packageNode)
        mergeNode.addInputPort('i0')

        # Add node references to the package node
        NU.AddNodeRef(packageNode, 'create', createNode)
        NU.AddNodeRef(packageNode, 'lightListEdit', lightListEditNode)
        NU.AddNodeRef(packageNode, 'material', materialNode)
        NU.AddNodeRef(packageNode, 'master', masterMaterialDistantPortNode)
        NU.AddNodeRef(packageNode, 'viewerObjectSettings', viewerObjectSettingsNode)
        NU.AddNodeRef(packageNode, 'merge', mergeNode)

        # Wire up and position the nodes
        NU.WireInlineNodes(packageNode, (masterMaterialDistantPortNode,
                                         createNode,
                                         lightListEditNode,
                                         materialNode,
                                         copyXformOpScriptNode,
                                         viewerObjectSettingsNode,
                                         mergeNode))

        # Create and append light linking nodes
        linkingNodes = Packages.LinkingMixin.getLinkingNodes(packageNode, create=True)
        NU.AppendNodes(packageNode, tuple(linkingNode for linkingNode in linkingNodes if linkingNode is not None))

        # Create a package node instance
        result = cls(packageNode)
        Packages.CallbackMixin.executeCreationCallback(result)

        # By default we use inline shaders, rather than forcing the user through look files, so this is old:
        # Set our material to point at a baked Look File material
        #result.setLookFileMaterial(cls.DEFAULT_BAKED_LIGHT_FILENAME, "/root/materials/%s" % cls.DEFAULT_BAKED_LIGHT_NAME)

        # Set the main shader
        result.setShader('arnoldLight', shaderName)

        return result

    @classmethod
    def createOpScriptNode(cls, parentNode, nodeName, opScript):
        """
        Creates and returns an OpScript node with the given name, under the
        given parent node, and sets the text of the OpScript to the given Lua
        code. The OpScript node will be set to run at the location of this
        package.

        @type parentNode: C{NodegraphAPI.Node}
        @type nodeName: C{str}
        @type opScript: C{str}
        @rtype: C{NodegraphAPI.Node}
        @param parentNode: The enclosing Group node under which to create the
            new OpScript node.
        @param nodeName: The name to give the new OpScript node.
        @param opScript: The text of the Lua code for the OpScript node.
        @return: The newly created OpScript node.
        """
        opScriptNode = NodegraphAPI.CreateNode('OpScript', parentNode)
        opScriptNode.setName(nodeName)
        opScriptNode.getParameter('CEL').setExpression("=^/__gaffer.location")
        opScriptNode.getParameter('script.lua').setValue(opScript, 0)
        return opScriptNode

    @classmethod
    def getCopyXformOpScript(cls):
        return textwrap.dedent(
            """
            -- Get the global xform for the current location
            local xformGrp = Interface.GetGlobalXFormGroup()
            local xformMatrix = XFormUtils.CalcTransformMatrixAtTime(xformGrp, 0.0)

            -- Copy the xform to the shader's matrix parameter
            Interface.SetAttr("material.parameters.matrix", xformMatrix)
            """).strip()

    @classmethod
    def getViewerUVFlipOpScript(cls):
        return textwrap.dedent(
            """
            -- OpScript that sets the geometry.viewerFlipU and
            -- geometry.viewerFlipV values for the representation of the
            -- skydome in the viewer, based on the values of 'sflip' and
            -- 'tflip' from the material.

            -- This is hardcoded to assume that the default values for sflip
            -- and tflip are 0. It might be better (but less efficient) if we
            -- were to get those default values from the material itself.

            -- From comparing the results from using Arnold's skydome_light
            -- shader and what is seen in the viewer, we set the viewer's FlipU
            -- to be the inverse of the shader's sflip values.

            local materialSflipAttr = Interface.GetAttr("material.parameters.sflip")
            local materialSflipValue = (materialSflipAttr ~= nil) and materialSflipAttr:getValue() or 0
            Interface.SetAttr("geometry.viewerFlipU", IntAttribute(1 - materialSflipValue))

            local materialTflipAttr = Interface.GetAttr("material.parameters.tflip")
            Interface.SetAttr("geometry.viewerFlipV", materialTflipAttr)
            """).strip()


class ArnoldLightGafferEditPackage(LightEditPackage):
    """
    The edit package that allows a GafferThree to edit an existing spot light
    package present in the input Scenegraph.

    This package uses a TransformEdit node to edit the spot light's transform.
    """

    # Class Variables ---------------------------------------------------------

    DISPLAY_ICON = os.path.join(_iconsDir, "aiSpotLight16.png")

    # Class Functions ---------------------------------------------------------

    @classmethod
    def create(cls, enclosingNode, locationPath):
        """
        Creates the contents of the EditStackNode that contains the edit nodes.
        This could be any other kind of node with at least one input and one
        output, but the createPackageEditStackNode() helper function does all
        of the configuration boilerplate code of an EditStackNode for you.
        The return value is a ArnoldSpotLightGafferEditPackage instance.

        This particular package node will contain a TransformEdit node on it,
        which will allow to edit the transform of a spot light.
        """
        # Create the package node. Since this is an edit package we want to use
        # an EditStackNode instead of a GroupNode, since it already has an
        # input and an output by default. This also adds some necessary
        # parameters to this node.
        packageNode = cls.createPackageEditStackNode(enclosingNode, locationPath)

        # Build material edit node
        materialNode = NodegraphAPI.CreateNode('Material', packageNode)
        actionParam = materialNode.getParameter('action')
        actionParam.setValue('edit material', 0)

        editLocationParam = materialNode.getParameter('edit.location')
        editLocationParam.setExpression('=^/__gaffer.location')
        editLocationParam.setExpressionFlag(True)
        NU.AddNodeRef(packageNode, 'material_edit', materialNode)

        packageNode.buildChildNode(adoptNode=materialNode)

        # Build transform edit node
        transformEditNode = NodegraphAPI.CreateNode('TransformEdit', packageNode)
        actionParam = transformEditNode.getParameter('action')
        actionParam.setValue('override interactive transform', 0)

        pathParam = transformEditNode.getParameter('path')
        pathParam.setExpression('=^/__gaffer.location')
        pathParam.setExpressionFlag(True)

        # Adds reference parameters to the transform edit node
        NU.AddNodeRef(packageNode, 'transform_edit', transformEditNode)

        # Add the transform edit node into the package node using
        # EditStackNode's buildChildNode().
        packageNode.buildChildNode(adoptNode=transformEditNode)

        # Create and append light linking nodes
        linkingNodes = Packages.LinkingMixin.getLinkingNodes(packageNode, create=True)
        NU.AppendNodes(packageNode, tuple(linkingNode for linkingNode in linkingNodes if linkingNode is not None))

        # Instantiate a package with the package node
        return cls.createPackage(packageNode)

    @classmethod
    def getAdoptableLocationTypes(cls):
        """
        Returns the set of location types adoptable by this package. In this
        case, the package can edit locations created by spot light packages, which
        are of the type light.
        """
        return set(('light',))

    # Instance Functions ------------------------------------------------------

    @Decorators.undogroup('Delete ArnoldLightEdit Package')
    def delete(self):
        LightEditPackage.delete(self)


##########################################################
# Arnold Spot Light
class ArnoldSpotLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Spot Light'

    DEFAULT_NAME = 'arnoldSpotLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiSpotLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldSpotLightGafferPackage, cls).create(enclosingNode, locationPath, 'spot_light')

class ArnoldSpotLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass

##########################################################
# Arnold Point Light
class ArnoldPointLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Point Light'

    DEFAULT_NAME = 'arnoldPointLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiPointLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldPointLightGafferPackage, cls).create(enclosingNode, locationPath, 'point_light')

class ArnoldPointLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass


##########################################################
# Arnold Quad Light
class ArnoldQuadLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Quad Light'

    DEFAULT_NAME = 'arnoldQuadLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiAreaLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldQuadLightGafferPackage, cls).create(enclosingNode, locationPath, 'quad_light')

class ArnoldQuadLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass

##########################################################
# Arnold Distant Light
class ArnoldDistantLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Distant Light'

    DEFAULT_NAME = 'arnoldDistantLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiDistantLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldDistantLightGafferPackage, cls).create(enclosingNode, locationPath, 'distant_light')

class ArnoldDistantLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass

##########################################################
# Arnold Disk Light
class ArnoldDiskLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Disk Light'

    DEFAULT_NAME = 'arnoldDiskLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiDiskLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldDiskLightGafferPackage, cls).create(enclosingNode, locationPath, 'disk_light')

class ArnoldDiskLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass

##########################################################
# Arnold Cylinder Light
class ArnoldCylinderLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Cylinder Light'

    DEFAULT_NAME = 'arnoldCylinderLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiCylinderLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldCylinderLightGafferPackage, cls).create(enclosingNode, locationPath, 'cylinder_light')

class ArnoldCylinderLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass

##########################################################
# Arnold Mesh Light
class ArnoldMeshLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Mesh Light'

    DEFAULT_NAME = 'arnoldMeshLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiMeshLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        return super(ArnoldMeshLightGafferPackage, cls).create(enclosingNode, locationPath, 'mesh_light')

class ArnoldMeshLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass

##########################################################
# Arnold GoboSpot Light
class ArnoldGoboSpotLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold GoboSpot Light'

    DEFAULT_NAME = 'arnoldGoboSpotLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiSpotLight16.png')
    
    @classmethod
    def create(cls, enclosingNode, locationPath):
        """
        A factory method which returns an instance of the class.

        @type enclosingNode: C{NodegraphAPI.Node}
        @type locationPath: C{str}
        @rtype: L{LightPackage}
        @param enclosingNode: The parent node within which the new
            package's node should be created.
        @param locationPath: The path to the location to be created/managed
            by the package.
        @return: The newly-created package instance.
        """
        # Create the package node
        packageNode = NodegraphAPI.CreateNode('Group', enclosingNode)
        packageNode.addOutputPort('out')

        # Add parameter containing the package type and location path to the package node
        NU.AddPackageTypeAndPath(packageNode, cls.__name__, locationPath)

        # Create an expression to link the name of the sky dome location to the name of the package
        locExpr = '=^/%s' % NU.GetPackageLocationParameterPath()

        # Create geometry for the light - in this case a bounded plane
        createNode = NodegraphAPI.CreateNode('LightCreate', packageNode)
        createNode.getParameter('transform.scale.x').setValue(cls.DEFAULT_SIZE, 0)
        scaleExpr = "=transform.scale.x"
        createNode.getParameter('transform.scale.y').setExpression(scaleExpr)
        createNode.getParameter('transform.scale.z').setExpression(scaleExpr)
        createNode.getParameters().createChildNumber('forceAsStaticScene', 1)

        createNodeExtraAttrsParameter = createNode.getParameters().createChildGroup('extraAttrs')
        createNodeExtraAttrsParameter.createChildString('__gafferPackage', '').setExpression('@%s' % packageNode.getName())
        createNode.getParameter('name').setExpression(locExpr)
        createNode.addInputPort('masterMaterial')

        # Store the package class as a parameter on the create node
        NU.SetOrCreateDeepScalarParameter(createNode.getParameters(),
                                          'extraAttrs.info.gaffer.packageClass',
                                          cls.__name__)


        # Add the package location to the light list at /root
        lightListEditNode = NodegraphAPI.CreateNode('LightListEdit', packageNode)
        lightListEditNode.setName("LightListEdit")
        lightListEditNode.getParameter('locations.i0').setExpression(locExpr)

        # Create the Material node
        materialNode = NodegraphAPI.CreateNode('Material', packageNode)
        materialNode.getParameter('action').setValue('edit material', 0)
        materialNode.getParameter('makeInteractive').setValue('Yes', 0)
        materialNode.getParameters().createChildNumber('makeInteractiveSpecificToMaterial', 1)
        materialNode.getParameter('edit.location').setExpression(locExpr)

        # Create an OpScript node to copy the preview texture from the shader to the sphere
        copyPreviewTextureOpScriptNode = cls.createOpScriptNode(packageNode,
                                                                  'CopyPreviewTextureOpScript',
                                                                  cls.__getPreviewTextureOpScript())

        # Create an OpScript node to copy the transform from the sphere to the shader
        copyXformOpScriptNode = cls.createOpScriptNode(packageNode,
                                                         'CopyXformOpScript',
                                                         cls.getCopyXformOpScript())

        # Create an OpScript node to flip the UVs in the viewer
        viewerUVFlipOpScriptNode = cls.createOpScriptNode(packageNode,
                                                            'ViewerUVFlipOpScript',
                                                            cls.getViewerUVFlipOpScript())

        # Create a DistantPort node to reference the master material
        masterMaterialDistantPortNode = NodegraphAPI.CreateNode('DistantPort', packageNode)
        masterMaterialDistantPortNode.setBypassed(True)

        # Create a ViewerObjectSettings node in case we want to customize how
        # the spot light is displayed in the viewer
        viewerObjectSettingsNode = NodegraphAPI.CreateNode('ViewerObjectSettings', packageNode)
        viewerObjectSettingsNode.getParameter('CEL').setExpression(locExpr)

        # Create the Merge node, for merging in child packages
        mergeNode = NodegraphAPI.CreateNode('Merge', packageNode)
        mergeNode.addInputPort('i0')

        # Add node references to the package node
        NU.AddNodeRef(packageNode, 'create', createNode)
        NU.AddNodeRef(packageNode, 'lightListEdit', lightListEditNode)
        NU.AddNodeRef(packageNode, 'material', materialNode)
        NU.AddNodeRef(packageNode, 'master', masterMaterialDistantPortNode)
        NU.AddNodeRef(packageNode, 'viewerObjectSettings', viewerObjectSettingsNode)
        NU.AddNodeRef(packageNode, 'merge', mergeNode)

        # Wire up and position the nodes
        NU.WireInlineNodes(packageNode, (masterMaterialDistantPortNode,
                                         createNode,
                                         lightListEditNode,
                                         materialNode,
                                         copyPreviewTextureOpScriptNode,
                                         copyXformOpScriptNode,
                                         viewerUVFlipOpScriptNode,
                                         viewerObjectSettingsNode,
                                         mergeNode))

        # Create and append light linking nodes
        linkingNodes = Packages.LinkingMixin.getLinkingNodes(packageNode, create=True)
        NU.AppendNodes(packageNode, tuple(linkingNode for linkingNode in linkingNodes if linkingNode is not None))

        # Create a package node instance
        result = cls(packageNode)
        Packages.CallbackMixin.executeCreationCallback(result)

        # By default we use inline shaders, rather than forcing the user through look files, so this is old:
        # Set our material to point at a baked Look File material
        #result.setLookFileMaterial(cls.DEFAULT_BAKED_LIGHT_FILENAME, "/root/materials/%s" % cls.DEFAULT_BAKED_LIGHT_NAME)

        # Use a couple of inline shaders; the 'surface' shader gets picked up automatically
        # and wired into the color parameter of the gobo light filter
        result.setShader('arnoldLight', 'spot_light')
        result.setShader('arnoldLightFilter', 'gobo')
        result.setShader('arnoldSurface', 'image')

        return result

    @classmethod
    def __getPreviewTextureOpScript(cls):
        return textwrap.dedent(
            """
            -- Get the path to the HDRI map from the material attributes
            local previewTextureAttr = Interface.GetAttr("material.parameters.HDRI_map")
            local previewTextureValue = (previewTextureAttr ~= nil) and previewTextureAttr:getValue() or ""

            -- Copy the path to the preview material attribute if it isn't blank
            if (previewTextureValue ~= "")
            then
                Interface.SetAttr("previewMaterial.texture", previewTextureAttr)
            end
            """).strip()

class ArnoldGoboSpotLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass


##########################################################
# Arnold Light with LightFilter
class ArnoldFilterLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold Single Filter Light'

    DEFAULT_NAME = 'arnoldFilterLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiPhotometricLight16.png')
    @classmethod
    def create(cls, enclosingNode, locationPath):
        result = super(ArnoldFilterLightGafferPackage, cls).create(enclosingNode, locationPath, 'spot_light')
        result.setShader('arnoldLightFilter', 'barndoor')
        return result

class ArnoldFilterLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass


##########################################################
# Arnold SkyDome Light
class ArnoldSkyDomeLightGafferPackage(ArnoldLightGafferPackage):
    DISPLAY_NAME = 'LCA Arnold SkyDome Light'

    DEFAULT_NAME = 'arnoldSkyDomeLight'

    DISPLAY_ICON = os.path.join(_iconsDir, 'aiSky16.png')

    DEFAULT_SIZE = 1000
    @classmethod
    def create(cls, enclosingNode, locationPath):
        """
        A factory method which returns an instance of the class.

        @type enclosingNode: C{NodegraphAPI.Node}
        @type locationPath: C{str}
        @rtype: L{LightPackage}
        @param enclosingNode: The parent node within which the new
            package's node should be created.
        @param locationPath: The path to the location to be created/managed
            by the package.
        @return: The newly-created package instance.
        """
        # Create the package node
        packageNode = NodegraphAPI.CreateNode('Group', enclosingNode)
        packageNode.addOutputPort('out')

        # Add parameter containing the package type and location path to the package node
        NU.AddPackageTypeAndPath(packageNode, cls.__name__, locationPath)

        # Create an expression to link the name of the sky dome location to the name of the package
        locExpr = '=^/%s' % NU.GetPackageLocationParameterPath()

        # Create geometry for the light - in this case a sphere
        createNode = NodegraphAPI.CreateNode('PrimitiveCreate', packageNode)
        createNode.getParameter('type').setValue('coordinate system sphere', 0)
        createNode.getParameter('transform.scale.x').setValue(cls.DEFAULT_SIZE, 0)
        scaleExpr = "=transform.scale.x"
        createNode.getParameter('transform.scale.y').setExpression(scaleExpr)
        createNode.getParameter('transform.scale.z').setExpression(scaleExpr)
        createNode.getParameters().createChildNumber('forceAsStaticScene', 1)

        createNodeExtraAttrsParameter = createNode.getParameters().createChildGroup('extraAttrs')
        createNodeExtraAttrsParameter.createChildString('__gafferPackage', '').setExpression('@%s' % packageNode.getName())
        createNode.getParameter('name').setExpression(locExpr)
        createNode.addInputPort('masterMaterial')

        # Store the package class as a parameter on the create node
        NU.SetOrCreateDeepScalarParameter(createNode.getParameters(),
                                          'extraAttrs.info.gaffer.packageClass',
                                          cls.__name__)

        # Set the type of the package location to "light", so that the Gaffer recognizes it as such
        typeAttrSetNode = NodegraphAPI.CreateNode('AttributeSet', packageNode)
        typeAttrSetNode.setName("SetTypeAttributeSet")
        typeAttrSetNode.getParameter('paths.i0').setExpression(locExpr)
        typeAttrSetNode.getParameter('attributeName').setValue('type', 0)
        typeAttrSetNode.getParameter('attributeType').setValue('string', 0)
        typeAttrSetNode.getParameter('stringValue.i0').setValue('light', 0)

        # Set the "viewer.locationType" attribute on the package location to
        # "nurbspatch", so that the viewer knows to display it as geometry
        viewerTypeAttrSetNode = NodegraphAPI.CreateNode('AttributeSet', packageNode)
        viewerTypeAttrSetNode.setName("SetViewerTypeAttributeSet")
        viewerTypeAttrSetNode.getParameter('paths.i0').setExpression(locExpr)
        viewerTypeAttrSetNode.getParameter('attributeName').setValue('viewer.locationType', 0)
        viewerTypeAttrSetNode.getParameter('attributeType').setValue('string', 0)
        viewerTypeAttrSetNode.getParameter('stringValue.i0').setValue('nurbspatch', 0)

        # Add the package location to the light list at /root
        lightListEditNode = NodegraphAPI.CreateNode('LightListEdit', packageNode)
        lightListEditNode.setName("LightListEdit")
        lightListEditNode.getParameter('locations.i0').setExpression(locExpr)

        # Create the Material node
        materialNode = NodegraphAPI.CreateNode('Material', packageNode)
        materialNode.getParameter('action').setValue('edit material', 0)
        materialNode.getParameter('makeInteractive').setValue('Yes', 0)
        materialNode.getParameters().createChildNumber('makeInteractiveSpecificToMaterial', 1)
        materialNode.getParameter('edit.location').setExpression(locExpr)

        # Create an OpScript node to copy the preview texture from the shader to the sphere
        copyPreviewTextureOpScriptNode = cls.createOpScriptNode(packageNode,
                                                                  'CopyPreviewTextureOpScript',
                                                                  cls.__getPreviewTextureOpScript())

        # Create an OpScript node to copy the transform from the sphere to the shader
        copyXformOpScriptNode = cls.createOpScriptNode(packageNode,
                                                         'CopyXformOpScript',
                                                         cls.getCopyXformOpScript())

        # Create an OpScript node to flip the UVs in the viewer
        viewerUVFlipOpScriptNode = cls.createOpScriptNode(packageNode,
                                                            'ViewerUVFlipOpScript',
                                                            cls.getViewerUVFlipOpScript())

        # Create a DistantPort node to reference the master material
        masterMaterialDistantPortNode = NodegraphAPI.CreateNode('DistantPort', packageNode)
        masterMaterialDistantPortNode.setBypassed(True)

        # Create a ViewerObjectSettings node in case we want to customize how
        # the skydome is displayed in the viewer
        viewerObjectSettingsNode = NodegraphAPI.CreateNode('ViewerObjectSettings', packageNode)
        viewerObjectSettingsNode.getParameter('CEL').setExpression(locExpr)

        # Create the Merge node, for merging in child packages
        mergeNode = NodegraphAPI.CreateNode('Merge', packageNode)
        mergeNode.addInputPort('i0')

        # Add node references to the package node
        NU.AddNodeRef(packageNode, 'create', createNode)
        NU.AddNodeRef(packageNode, 'lightListEdit', lightListEditNode)
        NU.AddNodeRef(packageNode, 'material', materialNode)
        NU.AddNodeRef(packageNode, 'master', masterMaterialDistantPortNode)
        NU.AddNodeRef(packageNode, 'viewerObjectSettings', viewerObjectSettingsNode)
        NU.AddNodeRef(packageNode, 'merge', mergeNode)

        # Wire up and position the nodes
        NU.WireInlineNodes(packageNode, (masterMaterialDistantPortNode,
                                         createNode,
                                         typeAttrSetNode,
                                         viewerTypeAttrSetNode,
                                         lightListEditNode,
                                         materialNode,
                                         copyPreviewTextureOpScriptNode,
                                         copyXformOpScriptNode,
                                         viewerUVFlipOpScriptNode,
                                         viewerObjectSettingsNode,
                                         mergeNode))

        # Create and append light linking nodes
        linkingNodes = Packages.LinkingMixin.getLinkingNodes(packageNode, create=True)
        NU.AppendNodes(packageNode, tuple(linkingNode for linkingNode in linkingNodes if linkingNode is not None))

        # Create a package node instance
        result = cls(packageNode)
        Packages.CallbackMixin.executeCreationCallback(result)

        # By default we use inline shaders, rather than forcing the user through look files, so this is old:
        # Set our material to point at a baked Look File material
        #result.setLookFileMaterial(cls.DEFAULT_BAKED_LIGHT_FILENAME, "/root/materials/%s" % cls.DEFAULT_BAKED_LIGHT_NAME)

        # Use a couple of inline shaders; the 'surface' shader gets picked up automatically
        # and wired into the color parameter of the skydome light
        result.setShader('arnoldLight', 'skydome_light')
        result.setShader('arnoldSurface', 'image')

        return result

    @classmethod
    def __getPreviewTextureOpScript(cls):
        return textwrap.dedent(
            """
            -- Get the path to the HDRI map from the material attributes
            local previewTextureFile = Interface.GetAttr("material.arnoldSurfaceParams.filename")
            local previewTextureValue = (previewTextureFile ~= nil) and previewTextureFile:getValue() or ""
            local suffix = ".tx"

            -- Copy the path to the preview material attribute if it isn't blank
            if (previewTextureFile ~= "") then
                -- Replace suffix .tx with .hdr
                if ( previewTextureValue:sub(-string.len(suffix)) == suffix ) then
                    previewTextureValue = previewTextureValue:gsub(suffix, ".hdr")
                    previewTextureFile = StringAttribute( previewTextureValue )
                end
                Interface.SetAttr("previewMaterial.texture", previewTextureFile)
            end
            """).strip()

class ArnoldSkyDomeLightGafferEditPackage(ArnoldLightGafferEditPackage):
    pass


# Register the package classes, and associate the edit package class with the
# create package class
GafferThreeAPI.RegisterPackageClass(ArnoldSpotLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldSpotLightGafferEditPackage)
ArnoldSpotLightGafferPackage.setEditPackageClass(ArnoldSpotLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldPointLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldPointLightGafferEditPackage)
ArnoldPointLightGafferPackage.setEditPackageClass(ArnoldPointLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldQuadLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldQuadLightGafferEditPackage)
ArnoldQuadLightGafferPackage.setEditPackageClass(ArnoldQuadLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldDistantLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldDistantLightGafferEditPackage)
ArnoldDistantLightGafferPackage.setEditPackageClass(ArnoldDistantLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldDiskLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldDiskLightGafferEditPackage)
ArnoldDiskLightGafferPackage.setEditPackageClass(ArnoldDiskLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldCylinderLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldCylinderLightGafferEditPackage)
ArnoldCylinderLightGafferPackage.setEditPackageClass(ArnoldCylinderLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldMeshLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldMeshLightGafferEditPackage)
ArnoldMeshLightGafferPackage.setEditPackageClass(ArnoldMeshLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldSkyDomeLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldSkyDomeLightGafferEditPackage)
ArnoldSkyDomeLightGafferPackage.setEditPackageClass(ArnoldSkyDomeLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldGoboSpotLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldGoboSpotLightGafferEditPackage)
ArnoldGoboSpotLightGafferPackage.setEditPackageClass(ArnoldGoboSpotLightGafferEditPackage)

GafferThreeAPI.RegisterPackageClass(ArnoldFilterLightGafferPackage)
GafferThreeAPI.RegisterPackageClass(ArnoldFilterLightGafferEditPackage)
ArnoldFilterLightGafferPackage.setEditPackageClass(ArnoldFilterLightGafferEditPackage)