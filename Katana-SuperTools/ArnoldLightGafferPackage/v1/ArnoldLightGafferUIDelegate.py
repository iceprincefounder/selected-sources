# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

from PackageSuperToolAPI import UIDelegate
from PackageSuperToolAPI import NodeUtils as NU
from PackageSuperToolAPI import Packages
from Katana import QT4FormWidgets, FormMaster, Plugins

from ArnoldLightGafferPackage import (
    ArnoldSpotLightGafferPackage, 
    ArnoldSpotLightGafferEditPackage,
    ArnoldPointLightGafferPackage,
    ArnoldPointLightGafferEditPackage,
    ArnoldQuadLightGafferPackage,
    ArnoldQuadLightGafferEditPackage,
    ArnoldDistantLightGafferPackage,
    ArnoldDistantLightGafferEditPackage,
    ArnoldDiskLightGafferPackage,
    ArnoldDiskLightGafferEditPackage,
    ArnoldCylinderLightGafferPackage,
    ArnoldCylinderLightGafferEditPackage,
    ArnoldMeshLightGafferPackage,
    ArnoldMeshLightGafferEditPackage,
    ArnoldGoboSpotLightGafferPackage,
    ArnoldGoboSpotLightGafferEditPackage,
    ArnoldFilterLightGafferPackage,
    ArnoldFilterLightGafferEditPackage,
    ArnoldSkyDomeLightGafferPackage,
    ArnoldSkyDomeLightGafferEditPackage,
    )


# Get the base classes for our UI delegate classes from the PackageSuperToolAPI
# using the base classes of our custom Sky Dome Package classes
GafferThreeAPI = Plugins.GafferThreeAPI
LightUIDelegate = UIDelegate.GetUIDelegateClassForPackageClass(GafferThreeAPI.PackageClasses.LightPackage)
LightEditUIDelegate = UIDelegate.GetUIDelegateClassForPackageClass(GafferThreeAPI.PackageClasses.LightEditPackage)


class ArnoldLightGafferUIDelegate(LightUIDelegate):
    """
    The UI delegate for the point light package.

    This class is responsible for exposing the parameters on each of the
    parameter tabs. This is done by creating parameter policies attached to the
    parameters on the package's nodes. We can also modify the appearance of the
    parameter tabs by modifying the hints dictionaries on the policies.
    """

    # The hash used to uniquely identify the action of creating a package
    # This was generated using:
    #   import hashlib
    #   print hashlib.md5('ArnoldGafferThreeLight.AddLight').hexdigest()    
    AddPackageActionHash = '0ea90ede9526a23ce0a26ba6e65ef5d0'

    # The keyboard shortcut for creating a package
    DefaultShortcut = 'Ctrl+A'

    def getTabPolicy(self, tabName):
        """
        The main method of a UIDelegate. This is responsible for returning a
        policy instance for each tab. The policy will contain other policies
        that should drive the actual package node's parameters.
        """
        if tabName == "Object":
            return self.getObjectTabPolicy()
        elif tabName == "Material":
            return self.getMaterialTabPolicy()
        elif tabName == "Linking":
            return self.getLinkingTabPolicy()
        else:
            return LightUIDelegate.getTabPolicy(self, tabName)

    def getObjectTabPolicy(self):
        """
        Returns the widget that should be displayed under the 'Object' tab.
        """
        # Get the create node in the package, which contains the transform
        # parameter.
        # return self._LightUIDelegate__getObjectTabPolicy()
        packageNode = self.getPackageNode()
        createNode = NU.GetRefNode(packageNode, "create")
        if createNode is None:
            return None


        # Create a root group policy and add some hints on it
        rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
        rootPolicy.getWidgetHints()['open'] = True
        rootPolicy.getWidgetHints()['hideTitle'] = True

        transformPolicy = QT4FormWidgets.PythonGroupPolicy('transform')
        transformPolicy.getWidgetHints()['open'] = True

        translatePolicy = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter("transform.translate"))
        rotatePolicy = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter("transform.rotate"))
        scalePolicy = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter("transform.scale"))

        transformPolicy.addChildPolicy(translatePolicy)
        transformPolicy.addChildPolicy(rotatePolicy)
        transformPolicy.addChildPolicy(scalePolicy)
        rootPolicy.addChildPolicy(transformPolicy)

        viewerObjectSettingsNode = NU.GetRefNode(packageNode, "viewerObjectSettings")
        annotationPolicy = QT4FormWidgets.PythonGroupPolicy('annotation')
        annotationPolicy.getWidgetHints()['open'] = False

        textPolicy = FormMaster.CreateParameterPolicy(
            None, viewerObjectSettingsNode.getParameter('args.viewer.default.annotation.text'))
        colorPolicy = FormMaster.CreateParameterPolicy(
            None, viewerObjectSettingsNode.getParameter('args.viewer.default.annotation.color'))
        previewColor = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter('previewColor'))
        pickablePolicy = FormMaster.CreateParameterPolicy(
            None, viewerObjectSettingsNode.getParameter('args.viewer.default.pickable'))

        annotationPolicy.addChildPolicy(textPolicy)
        annotationPolicy.addChildPolicy(colorPolicy)
        annotationPolicy.addChildPolicy(previewColor)
        annotationPolicy.addChildPolicy(pickablePolicy)

        rootPolicy.addChildPolicy(annotationPolicy)

        return rootPolicy


    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type

        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material")
        if materialNode:
            materialNode.checkDynamicParameters()
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            shaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            shaderPolicy = QT4FormWidgets.ValuePolicyProxy(shaderPolicy)
            shaderPolicy.setWidgetHints(shaderPolicy.getWidgetHints())
            shaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(shaderPolicy)

            paramsPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            paramsPolicy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(paramsPolicy)
            return materialPolicy
        return None

    def getLinkingTabPolicy(self):
        return self._LightUIDelegate__getLinkingTabPolicy()

class ArnoldLightGafferEditUIDelegate(LightEditUIDelegate):
    """
    The UI delegate for the ArnoldPointLightEdit package.
    """
    def getTabPolicy(self, tabName):
        """
        The main method of a UIDelegate. This is responsible for returning a
        Value Policy for each tab. The Value Policy will contain other policies
        that should drive the actual package node's parameters.
        """
        if tabName == "Object":
            return self.getObjectTabPolicy()
        elif tabName == "Material":
            return self.getMaterialTabPolicy()
        elif tabName == "Linking":
            return self._LightEditUIDelegate__getLinkingTabPolicy()
        else:
            return LightEditUIDelegate.getTabPolicy(self, tabName)

    def getObjectTabPolicy(self):
        return self._LightEditUIDelegate__getObjectTabPolicy()
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type

        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material_edit")
        materialNode.checkDynamicParameters()
        if materialNode:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            shaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            shaderPolicy = QT4FormWidgets.ValuePolicyProxy(shaderPolicy)
            shaderPolicy.setWidgetHints(shaderPolicy.getWidgetHints())
            shaderPolicy.getWidgetHints()['readOnly'] = False
            materialPolicy.addChildPolicy(shaderPolicy)

            paramsPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            paramsPolicy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(paramsPolicy)
            return materialPolicy
        return None

    def getLinkingTabPolicy(self):
        return self._LightEditUIDelegate__getLinkingTabPolicy()


class ArnoldSpotLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddSpotLight'
    DefaultShortcut = 'Ctrl+1'

class ArnoldPointLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddPointLight'
    DefaultShortcut = 'Ctrl+2'

class ArnoldQuadLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddQuadLight'
    DefaultShortcut = 'Ctrl+3'

class ArnoldDistantLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddDistantLight'
    DefaultShortcut = 'Ctrl+4'

class ArnoldDiskLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddDiskLight'
    DefaultShortcut = 'Ctrl+5'

class ArnoldCylinderLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddCylinderLight'
    DefaultShortcut = 'Ctrl+6'

class ArnoldMeshLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddMeshLight'
    DefaultShortcut = 'Ctrl+7'

class ArnoldGoboSpotLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddGoboSpotLight'
    DefaultShortcut = 'Ctrl+8'
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type
        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material")
        if materialNode is not None:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            lightShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            lightShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightShaderPolicy)
            lightShaderPolicy.setWidgetHints(lightShaderPolicy.getWidgetHints())
            lightShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(lightShaderPolicy)

            lightfilterShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterShader'))
            lightfilterShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightfilterShaderPolicy)
            lightfilterShaderPolicy.setWidgetHints(lightfilterShaderPolicy.getWidgetHints())
            lightfilterShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(lightfilterShaderPolicy)

            imageShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceShader'))
            imageShaderPolicy = QT4FormWidgets.ValuePolicyProxy(imageShaderPolicy)
            imageShaderPolicy.setWidgetHints(imageShaderPolicy.getWidgetHints())
            imageShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(imageShaderPolicy)

            params1Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            params1Policy.getWidgetHints()['open'] = True
            params2Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterParams'))
            params2Policy.getWidgetHints()['open'] = True
            params3Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceParams'))
            params3Policy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(params1Policy)
            materialPolicy.addChildPolicy(params2Policy)
            materialPolicy.addChildPolicy(params3Policy)
            return materialPolicy
        return None

class ArnoldGoboSpotLightGafferEditUIDelegate(ArnoldLightGafferEditUIDelegate):
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type
        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material_edit")
        if materialNode is not None:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            lightShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            lightShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightShaderPolicy)
            lightShaderPolicy.setWidgetHints(lightShaderPolicy.getWidgetHints())
            lightShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(lightShaderPolicy)

            lightfilterShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterShader'))
            lightfilterShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightfilterShaderPolicy)
            lightfilterShaderPolicy.setWidgetHints(lightfilterShaderPolicy.getWidgetHints())
            lightfilterShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(lightfilterShaderPolicy)

            imageShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceShader'))
            imageShaderPolicy = QT4FormWidgets.ValuePolicyProxy(imageShaderPolicy)
            imageShaderPolicy.setWidgetHints(imageShaderPolicy.getWidgetHints())
            imageShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(imageShaderPolicy)

            params1Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            params1Policy.getWidgetHints()['open'] = True
            params2Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterParams'))
            params2Policy.getWidgetHints()['open'] = True
            params3Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceParams'))
            params3Policy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(params1Policy)
            materialPolicy.addChildPolicy(params2Policy)
            materialPolicy.addChildPolicy(params3Policy)
            return materialPolicy
        return None


class ArnoldFilterLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddFliterLight'
    DefaultShortcut = 'Ctrl+9'
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type
        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material")
        if materialNode is not None:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            lightShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            lightShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightShaderPolicy)
            lightShaderPolicy.setWidgetHints(lightShaderPolicy.getWidgetHints())
            lightShaderPolicy.getWidgetHints()['readOnly'] = False
            materialPolicy.addChildPolicy(lightShaderPolicy)

            lightfilterShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterShader'))
            lightfilterShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightfilterShaderPolicy)
            lightfilterShaderPolicy.setWidgetHints(lightfilterShaderPolicy.getWidgetHints())
            lightfilterShaderPolicy.getWidgetHints()['readOnly'] = False
            materialPolicy.addChildPolicy(lightfilterShaderPolicy)

            params1Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            params1Policy.getWidgetHints()['open'] = True
            params2Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterParams'))
            params2Policy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(params1Policy)
            materialPolicy.addChildPolicy(params2Policy)    
            return materialPolicy
        return None

class ArnoldFilterLightGafferEditUIDelegate(ArnoldLightGafferEditUIDelegate):
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type
        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material_edit")
        if materialNode is not None:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            lightShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            lightShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightShaderPolicy)
            lightShaderPolicy.setWidgetHints(lightShaderPolicy.getWidgetHints())
            lightShaderPolicy.getWidgetHints()['readOnly'] = False
            materialPolicy.addChildPolicy(lightShaderPolicy)

            lightfilterShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterShader'))
            lightfilterShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightfilterShaderPolicy)
            lightfilterShaderPolicy.setWidgetHints(lightfilterShaderPolicy.getWidgetHints())
            lightfilterShaderPolicy.getWidgetHints()['readOnly'] = False
            materialPolicy.addChildPolicy(lightfilterShaderPolicy)

            params1Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            params1Policy.getWidgetHints()['open'] = True
            params2Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightFilterParams'))
            params2Policy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(params1Policy)
            materialPolicy.addChildPolicy(params2Policy)    
            return materialPolicy
        return None


class ArnoldSkyDomeLightGafferUIDelegate(ArnoldLightGafferUIDelegate):
    AddPackageActionHash = 'GafferThree-AddSkyDomeLight'
    DefaultShortcut = 'Ctrl+0'
    def getObjectTabPolicy(self):
        """
        Returns the widget that should be displayed under the 'Object' tab.
        """
        # Get the create node in the package, which contains the transform parameter
        packageNode = self.getPackageNode()
        createNode = NU.GetRefNode(packageNode, "create")
        if createNode is None:
            return None

        # Create a root group policy and add some hints on it
        rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
        rootPolicy.getWidgetHints()['open'] = True
        rootPolicy.getWidgetHints()['hideTitle'] = True

        transformPolicy = QT4FormWidgets.PythonGroupPolicy('transform')
        transformPolicy.getWidgetHints()['open'] = True

        translatePolicy = FormMaster.CreateParameterPolicy(None, createNode.getParameter("transform.translate"))
        rotatePolicy = FormMaster.CreateParameterPolicy(None, createNode.getParameter("transform.rotate"))
        scalePolicy = FormMaster.CreateParameterPolicy(None, createNode.getParameter("transform.scale"))

        transformPolicy.addChildPolicy(translatePolicy)
        transformPolicy.addChildPolicy(rotatePolicy)
        transformPolicy.addChildPolicy(scalePolicy)

        rootPolicy.addChildPolicy(transformPolicy)

        return rootPolicy
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type
        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material")
        if materialNode is not None:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            lightShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            lightShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightShaderPolicy)
            lightShaderPolicy.setWidgetHints(lightShaderPolicy.getWidgetHints())
            lightShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(lightShaderPolicy)

            imageShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceShader'))
            imageShaderPolicy = QT4FormWidgets.ValuePolicyProxy(imageShaderPolicy)
            imageShaderPolicy.setWidgetHints(imageShaderPolicy.getWidgetHints())
            imageShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(imageShaderPolicy)

            params1Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            params1Policy.getWidgetHints()['open'] = True
            params2Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceParams'))
            params2Policy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(params1Policy)
            materialPolicy.addChildPolicy(params2Policy)
            return materialPolicy
        return None
    def getLinkingTabPolicy(self):
        return LightUIDelegate.GetLightLinkingTabPolicy(
            self.getReferencedNode("node_lightLink_illumination"),
            self.getReferencedNode("node_lightLink_shadow"),
            self.getReferencedNode("node_lightListEdit"))

class ArnoldSkyDomeLightGafferEditUIDelegate(ArnoldLightGafferEditUIDelegate):
    def getMaterialTabPolicy(self):
        # Create a new material policy that just has the prmanLightParams for
        # each light type
        packageNode = self.getPackageNode()
        materialNode = NU.GetRefNode(packageNode, "material_edit")
        if materialNode is not None:
            materialPolicy = QT4FormWidgets.PythonGroupPolicy('material')
            materialPolicy.getWidgetHints()['hideTitle'] = True

            lightShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightShader'))
            lightShaderPolicy = QT4FormWidgets.ValuePolicyProxy(lightShaderPolicy)
            lightShaderPolicy.setWidgetHints(lightShaderPolicy.getWidgetHints())
            lightShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(lightShaderPolicy)

            imageShaderPolicy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceShader'))
            imageShaderPolicy = QT4FormWidgets.ValuePolicyProxy(imageShaderPolicy)
            imageShaderPolicy.setWidgetHints(imageShaderPolicy.getWidgetHints())
            imageShaderPolicy.getWidgetHints()['readOnly'] = True
            materialPolicy.addChildPolicy(imageShaderPolicy)

            params1Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldLightParams'))
            params1Policy.getWidgetHints()['open'] = True
            params2Policy = FormMaster.CreateParameterPolicy(materialPolicy,
                materialNode.getParameter('shaders.arnoldSurfaceParams'))
            params2Policy.getWidgetHints()['open'] = True
            materialPolicy.addChildPolicy(params1Policy)
            materialPolicy.addChildPolicy(params2Policy)
            return materialPolicy
        return None

# Register the UI delegates

UIDelegate.RegisterUIDelegateClass(ArnoldSpotLightGafferPackage,     ArnoldSpotLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldSpotLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldPointLightGafferPackage,     ArnoldPointLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldPointLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldQuadLightGafferPackage,     ArnoldQuadLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldQuadLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldDistantLightGafferPackage,     ArnoldDistantLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldDistantLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldDiskLightGafferPackage,     ArnoldDiskLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldDiskLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldCylinderLightGafferPackage,     ArnoldCylinderLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldCylinderLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldMeshLightGafferPackage,     ArnoldMeshLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldMeshLightGafferEditPackage, ArnoldLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldGoboSpotLightGafferPackage,     ArnoldGoboSpotLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldGoboSpotLightGafferEditPackage, ArnoldGoboSpotLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldFilterLightGafferPackage,     ArnoldFilterLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldFilterLightGafferEditPackage, ArnoldFilterLightGafferEditUIDelegate)

UIDelegate.RegisterUIDelegateClass(ArnoldSkyDomeLightGafferPackage,     ArnoldSkyDomeLightGafferUIDelegate)
UIDelegate.RegisterUIDelegateClass(ArnoldSkyDomeLightGafferEditPackage, ArnoldSkyDomeLightGafferEditUIDelegate)

