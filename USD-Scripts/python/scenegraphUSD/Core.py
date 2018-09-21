import os,shutil,sys

try:
    from scenegraphUSD.Utility import queue
    from scenegraphUSD import Logging
    from scenegraphUSD.Setting import *
except ImportError:
    ## Developling envrionment
    sys.path.append("/home/xukai/Git/git_repo/scenegraphUSD/python")
    from scenegraphUSD.Utility import queue
    from scenegraphUSD import Logging
    from scenegraphUSD.Setting import *
try:
    import maya.cmds as cmds
    import pymel.core as pm
    from pxr import Usd,Sdf,UsdGeom,Kind
except ImportError:
    print "Can`t find maya.cmds"
    print "Can`t find pymel.core"
    print "Can`t find Pxiar`s USD"

class SGUSD(object):
    def __init__(self):
        self.__stage__ = None
        self.__exist__ = None
        self.__destination__=None
    def __create__(self, destination=None,postfix=None, force=False):
        '''
        Create USD destination path,if destination is None,find it in projects path.
        '''
        if not destination:
            destination=self.fetchPathSceneGraphUSDSaving(LCA_USD_SEARCH_PATH,postfix)
        if os.path.isfile(destination):
            if force:
                os.remove(destination)
                self.__exist__ = False
                stage = Usd.Stage.CreateNew(destination)
                self.__stage__ = stage
            else:
                self.__exist__ = True
            # stage = Usd.Stage.Open(destination)
        else:
            self.__exist__ = False
            stage = Usd.Stage.CreateNew(destination)
            self.__stage__ = stage
        self.__destination__=destination
    def __export__(self):
        return self.__stage__
    def __save__(self):
        self.__stage__.GetRootLayer().Save()
        Logging.scenegraphLogging("LCA USD file Saving to: %s"%self.__destination__)
    def __clear__(self):
        self.__stage__ = None
    def __defineUSDReferencePrim__(self, dagnode_path , reference_path):
        '''
        Create a prim and add the reference on it.
        '''
        scenegraph_path = self.fetchNameSceneGraphPrim(dagnode_path)
        reference_gprim = self.__stage__.DefinePrim(scenegraph_path,'Xform')
        reference_master_gprim = self.__stage__.DefinePrim(os.path.join(scenegraph_path,"master"))
        reference_master_gprim.GetPrim().GetReferences().AddReference(reference_path)
    def __defineUSDXformPrim__(self, dagnode_path):
        '''
        Create the UsdPrim that type is Xform.
        '''
        scenegraph_path = self.fetchNameSceneGraphPrim(dagnode_path)
        gprim = Usd.ModelAPI(UsdGeom.Xform.Define(self.__stage__, scenegraph_path))
    def __defineUSDXformPrimByLoops__(self, dagnode_path):
        '''
        Create the UsdPrim that type as Xform,
        and have a loop to create every prim one by one.
        '''
        scenegraph_path = self.fetchNameSceneGraphPrim(dagnode_path)
        scenegraph_tree = self.fetchListUSDPrim(dagnode_path)
        for prim_node in scenegraph_tree:
            Usd.ModelAPI(UsdGeom.Xform.Define(self.__stage__, prim_node))
    def __defineUSDDefaultSetting__(self, dagnode_path, rangeTimeCode=None):
        '''
        Set the default of USD such as TimeCode, DefaultPrim and UpAxis.
        '''
        # set USD default setting
        if rangeTimeCode:
            self.__stage__.SetStartTimeCode(rangeTimeCode[0])
            self.__stage__.SetEndTimeCode(rangeTimeCode[1])
        scenegraph_path = self.fetchNameSceneGraphPrim(dagnode_path)
        root_prim = self.__stage__.GetPrimAtPath( self.fetchNameUsdRoot(scenegraph_path))
        self.__stage__.SetDefaultPrim(root_prim)
        UsdGeom.SetStageUpAxis(self.__stage__, UsdGeom.Tokens.y)
    def __defineStaticUSDPrimTransform__(self,dagnode_path):
        '''
        Try to define static prim xform information.
        '''
        scenegraph_path = self.fetchNameSceneGraphPrim(dagnode_path)
        scenegraph_data = self.fetchDataStaticMayaXform(dagnode_path)
        prim = self.__stage__.GetPrimAtPath( scenegraph_path ) 
        if scenegraph_data["visibility"]:
            UsdGeom.Imageable(prim).MakeVisible()
        else:
            UsdGeom.Imageable(prim).MakeInvisible()
        rotateXYZ = scenegraph_data["rotateXYZ"]
        UsdGeom.XformCommonAPI(prim).SetRotate(tuple(rotateXYZ),UsdGeom.XformCommonAPI.RotationOrderXYZ)
        scale = scenegraph_data["scale"]
        UsdGeom.XformCommonAPI(prim).SetScale(tuple(scale))
        translate = scenegraph_data["translate"]
        UsdGeom.XformCommonAPI(prim).SetTranslate(tuple(translate))
    def traverse(self, node, stuckAR=False):
        '''
        Traverse all DAG node under inpur parameter node.
        '''
        parent_layer = self.staticMayaRelatives(node)
        ensemble = queue(parent_layer)
        all_descendents = [] # the list content all DAG node we 
        while True:
            if not ensemble.knocked():
                break
            all_descendents.append(ensemble.front)
            if (stuckAR):
                # if current node is assemblyReference,we would never dig it ang more, 
                # we would stop from this branch and start the new one
                if ( not pm.PyNode(ensemble.front).type()=="assemblyReference"):
                    if ( self.staticMayaRelatives(ensemble.front) ):
                        ensemble.enExpand( self.staticMayaRelatives(ensemble.front) )
            else:
                # yes,we would travse for all DAG node
                if ( self.staticMayaRelatives(ensemble.front) ):
                        ensemble.enExpand( self.staticMayaRelatives(ensemble.front) )
            ensemble.deQueue()
        return all_descendents
    def useDefaultSearchPath(self, reference_path):
        if reference_path.startswith(LCA_PROJ_PATH):
            return reference_path[len(LCA_PROJ_PATH)+1:]
        if reference_path.startswith(LCA_USD_SEARCH_PATH):
            return reference_path[len(LCA_USD_SEARCH_PATH)+1:]
    def useCurrentFolderPath(self, reference_path):
        return os.path.join(".",reference_path.split("/")[-1])
    def useOriginalSceneGraphName(self,dagnode_path):
        # replace all colons into periods,but in USD world, we use double underscore
        dagnode_path = dagnode_path.replace(":","__")
        return dagnode_path
    def isUSDFileExist(self):
        destination=self.fetchPathSceneGraphUSDSaving(LCA_USD_SEARCH_PATH,postfix)
        if os.path.isfile(destination):
            return True
        else:
            return False
    def isAssembleReference(self,dagnode_path):
        '''
        Checkout if this DAG node is assenblyReference.
        '''
        node = pm.PyNode(dagnode_path)
        if node.type() == "assemblyReference":
            return True
        else:
            return False
    def isDAGNodeBeMoved(self, dagnode_path):
        '''
        Checkout if this DAG node have been animated.
        '''
        default_matrix = "[[1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 1.0]]"
        node = pm.PyNode(dagnode_path)
        visibility = node.attr('visibility').get()
        ## To DO: we should find out if attribute visibility is being animated.
        matrix = node.attr("matrix").get()
        if str(matrix) == default_matrix:
            return False
        else:
            return True
    def fetchListUSDPrim(self,dagnode_path):
        scenegraph_list = self.fetchNameSceneGraphPrim(dagnode_path,combine=False)
        result = []
        current_node = ""
        for node in scenegraph_list:
            current_node = current_node+"/"+node
            result.append( current_node )
        return result
    def fetchPathAssembleReferece(self,node):
        '''
        Input a DAG node which type is "assembleReference" and find the USD file path.
        '''
        pynode = pm.PyNode(node)
        definition = pynode.attr("definition").get()
        namespace = pynode.attr("repNamespace").get()

        usd_path = self.fetchPathSceneGraphUSDSaving2(definition)

        return usd_path
    def fetchPathSceneGraphUSDSaving(self, specific_path=None,postfix=None):
        '''
        Find out the location that USD file shoule be wrote
        from the maya file path.
        Also,add a version number to assets!
        '''
        maya_path = cmds.file(sceneName=True,query=True)
        file_name = maya_path.split("/")[-1][:-3]
        if postfix:
            file_name = maya_path.split("/")[-1][:-3] + postfix
        publish_path = maya_path[:maya_path.find("/publish/") + len("/publish")]
        prefix_proj_path = maya_path[len(LCA_PROJ_PATH)+1:-len(maya_path.split("/")[-1])-len(maya_path.split("/")[-2])-2]
        all_version = sorted(os.listdir(publish_path))
        ## there is a special situation, if <flo> folder keeps the stereo folder,we should skip it.
        for i in range(len(all_version)):
            if all_version[i].find(".stereo.") > 0:
                continue
            last_version = all_version[i]
        if specific_path:
            usd_saving_path = os.path.join(specific_path, prefix_proj_path, last_version, "usd", file_name+".usda")
        else:
            usd_saving_path = os.path.join(LCA_PROJ_PATH, prefix_proj_path, last_version, "usd", file_name+".usda")
        return usd_saving_path
    def fetchPathSceneGraphUSDSaving2(self,maya_path, specific_path=None):
        '''
        Input a DAG node which type is "assembleReference" and find the USD file path.
        '''
        file_name = maya_path.split("/")[-1][:-3]
        publish_path = maya_path[:maya_path.find("/publish/") + len("/publish")]
        all_version = sorted(os.listdir(publish_path))
        ## there is a special situation, if <flo> folder keeps the stereo folder,we should skip it.
        for i in range(len(all_version)):
            if all_version[i].find(".stereo.") > 0:
                continue
            last_version = all_version[i]
        if specific_path:
            usd_saving_path = os.path.join(specific_path, publish_path, last_version, "usd", file_name+".usda")
        else:
            usd_saving_path = os.path.join(LCA_PROJ_PATH, publish_path, last_version, "usd", file_name+".usda")
        return usd_saving_path
    def fetchNameUsdRoot(self,scenegraph_path):
        return "/"+scenegraph_path.split("/")[1]
    def fetchNameMayaRoot(self,dagnode_path):
        return "|"+dagnode_path.split("|")[1]
    def fetchNameSceneGraphPrim(self,dagnode_path,combine=True):
        '''
        Genrate the scenegraph tree just similar to XML
        '''
        result = []
        next_path = dagnode_path
        while True:
            current_path = next_path
            current_path = self.mathCurrentNode(current_path)
            ##################################
            # We use original scenegraph node name at this place,
            # but maybe we should change this to adopt our pipeline
            current_path = self.useOriginalSceneGraphName(current_path)
            # insert element at the beginning of list
            result.insert(0,current_path)
            next_path = self.mathPreviousNode(next_path)
            # if next_path is empty, break the loops
            if not next_path:
                break
        if combine:
            final_path = ""
            for element in result:
                final_path += ("/" + element)
            result = final_path
        return result
    def fetchDataStaticMayaXform(self,dagnode_path):
        node = pm.PyNode(dagnode_path)
        # the struct result = {{frame}:{{"visibility":bool,"translation":[],"rotation":[],"scale":[]}}}
        result = {}
        visibility = node.attr('visibility').get()
        matrix = node.attr("matrix").get()
        translation = node.attr("translate").get()
        rotation = node.attr("rotate").get()
        scale = node.attr("scale").get()
        result = {}
        result["visibility"] = visibility # a bool value
        result["rotateXYZ"] = rotation # a list of x y z
        result["scale"] = scale # a list of x y z
        result["translate"] = translation # a list of x y z
        result["xformOpOrder"] = ["xformOp:translate", "xformOp:rotateXYZ", "xformOp:scale"]
        return result
    def fetchDataDynamicMayaXform(self,dagnode_path,rangeTimeCode,motionSample):
        '''
        # arg "rangeTimeCode" should be a tuple like (1001,1010)
        # arg "motionSample" should be a tuple like (-0.15,0,0.15)
        '''
        node = pm.PyNode(dagnode_path)
        # the struct result = {{frame}:{{"visibility":bool,"translation":[],"rotation":[],"scale":[]}}}
        result = {}
        for current_frame in range(rangeTimeCode[0],rangeTimeCode[1]+1):
            for current_sample in motionSample:
                cmds.currentTime( current_frame+current_sample )
                current_timesamples = current_frame+current_sample
                visibility = node.attr('visibility').get()
                matrix = node.attr("matrix").get()
                translation = node.attr("translate").get()
                rotation = node.attr("rotate").get()
                scale = node.attr("scale").get()
                temporary = {}
                temporary["visibility"] = visibility # a bool value
                temporary["rotateXYZ"] = rotation # a list of x y z
                temporary["scale"] = scale # a list of x y z
                temporary["translate"] = translation # a list of x y z
                temporary["xformOpOrder"] = ["xformOp:translate", "xformOp:rotateXYZ", "xformOp:scale"]
                result[current_timesamples] = temporary
        
        cmds.currentTime( rangeTimeCode[0] )
        return result
    @staticmethod
    def mathPreviousNode(asset_name):
        '''
        The asset name should look like this:
            u'|assets|prp|brazier_tower_above:master'
        '''
        all_pieces = asset_name.split("|")
        last_one = all_pieces[-1]
        end_point = len(last_one) + 1
        return asset_name[:-end_point]
    @staticmethod
    def mathCurrentNode(asset_name):
        '''
        The asset name should look like this:
            u'|assets|prp|brazier_tower_above:master'
        '''
        all_pieces = asset_name.split("|")
        last_one = all_pieces[-1]
        return last_one
    @staticmethod
    def staticMayaRelatives(node):
        try:
            result = cmds.listRelatives(node,fullPath=True)
            if not result:
                result = []
        except TypeError:
            result = []
        return result

class SGUSDExporter(SGUSD):
    def __init__(self): 
        super(SGUSDExporter, self).__init__()
        self.sublayers = []
    def __useDebugSearchPath__(self, input_path):
        output_path = input_path.replace(LCA_PROJ_PATH,LCA_USD_SEARCH_PATH)
        return output_path
    def __getPathAssemblyRefUSDCache__(self,dag_node):
        pass
    def __getPathMayaRefUSDCache__(self,dag_node):
        namespace = cmds.referenceQuery(dag_node,ns=1)[1:]
        # filename = cmds.referenceQuery(dag_node,filename=1).split("/")[-1].replace(".ma",".usda").replace(".mb",".usda")
        filename = cmds.referenceQuery(dag_node,filename=1).split("/")[-1].replace(".ma",".xml").replace(".mb",".xml")
        file_path = cmds.file(sceneName=True,q=True)
        # file_path = self.__useDebugSearchPath__(file_path)
        current_path = file_path[:-len(file_path.split("/")[-1])-1]

        shot_path = file_path[:file_path.find("/publish/")][:-4]
        cfx_path = os.path.join(shot_path,"cfx","publish")
        ani_path = os.path.join(shot_path,"ani","publish")
        flo_path = os.path.join(shot_path,"flo","publish")

        publish_list = [{"path":cfx_path,"key":".cloth."},
                        {"path":ani_path,"key":".animation."},
                        {"path":flo_path,"key":".final_layout."}]
        final_path = None
        for publish_node in publish_list:
            path = publish_node["path"]
            key = publish_node["key"]
            if not os.path.isdir(path):
                continue
            all_version = sorted(os.listdir(path))
            last_version = None
            for version in all_version:
                if not version.find(key) > 0:
                    continue
                if last_version:
                    if int(version[-3:]) > int(last_version[-3:]):
                        last_version = version
                else:
                    last_version = version
            if last_version:
                # final_path = os.path.join(path,last_version, "cache", namespace, "usd", filename)
                final_path = os.path.join(path,last_version, "cache", namespace, "geo", filename)
                if os.path.isfile(final_path):
                    break
                else:
                    continue
        final_path = self.__useDebugSearchPath__(final_path)
        return final_path
    def __getPathMayaRefUSDCache2__(self,dag_node):
        namespace = cmds.referenceQuery(dag_node,ns=1)[1:]
        file_path = cmds.file(sceneName=True,q=True)
        shot_path = file_path[:file_path.find("/publish/")][:-4].split("/")[-1]

        import production.pipeline.lcProdProj as clpp
        cp=clpp.lcProdProj()
        cp.setProj('pws')
        # print cp.getAniAsset('f40140', 'brazier_tower_above')
        cache_path = cp.getAniAsset(shot_path, namespace)
        cache_path = cache_path.replace("/geo/","/usd/").replace(".xml",".usda")
        cache_path = self.__useDebugSearchPath__(cache_path)
        return cache_path
    def __getDataFrameRange__(self):
        from production.pipeline.ShotGunProj import ShotGunProj
        file_path = cmds.file(sceneName=True,q=True)
        shot_path = file_path[:file_path.find("/publish/")][:-4].split("/")[-1]

        ShotGunProj_instance = ShotGunProj("pws")

        frame_range = ShotGunProj_instance.get_shot_framerange(shot_path)
        return frame_range
    def __getStaticModPath__(self, node):
        '''
        Input a DAG node which type is "assembleReference" and find the USD file path.
        '''
        pynode = pm.PyNode(node)
        definition = pynode.attr("definition").get()
        namespace = pynode.attr("repNamespace").get()
        final_mod_path = definition
        if definition.find("/rig/") > 0:
            assets_path = definition[:definition.find("/publish/")-4]
            filename = assets_path.split("/")[-1]
            mods_path = os.path.join(assets_path,"mod","publish")
            all_version = sorted(os.listdir(mods_path))
            last_version = all_version[-1]
            last_version_path = os.path.join(mods_path, last_version)
            final_mod_path = os.path.join(last_version_path,filename+".ma")
        
        usd_path = self.fetchPathSceneGraphUSDSaving2(final_mod_path)
        return usd_path
    def relocateWorldCenter(self,in_dagnode_path="|assets", out_dagnode_path="|assets|scn"):
        '''
        Try to define static prim xform information.
        '''
        if not cmds.objExists(in_dagnode_path) or not cmds.objExists(out_dagnode_path):
            return
        if not self.isDAGNodeBeMoved(in_dagnode_path):
            return
        scenegraph_path = self.fetchNameSceneGraphPrim(out_dagnode_path)
        scenegraph_data = self.fetchDataStaticMayaXform(in_dagnode_path)
        prim = self.__stage__.GetPrimAtPath( scenegraph_path ) 
        if scenegraph_data["visibility"]:
            UsdGeom.Imageable(prim).MakeVisible()
        else:
            UsdGeom.Imageable(prim).MakeInvisible()
        rotateXYZ = scenegraph_data["rotateXYZ"]
        UsdGeom.XformCommonAPI(prim).SetRotate(tuple(rotateXYZ),UsdGeom.XformCommonAPI.RotationOrderXYZ)
        scale = scenegraph_data["scale"]
        UsdGeom.XformCommonAPI(prim).SetScale(tuple(scale))
        translate = scenegraph_data["translate"]
        UsdGeom.XformCommonAPI(prim).SetTranslate(tuple(translate))

    def exportStaticAssmblyRefUSD(self,location,destination=None,postfix = None, force=False, ignore=[]):
        '''
        export static(one frame) USD file from the location which build with Assembly Reference
        '''
        # create a stage
        self.__create__(destination, postfix, force)
        if self.__exist__:
            Logging.scenegraphSeparater("USD file already exist!")
            return
        # create default location as USD Xform
        self.__defineUSDXformPrimByLoops__(location)
        self.__defineUSDDefaultSetting__(location)
        self.relocateWorldCenter()
        # list all children of the parent DAG node and create USD prim
        for dag_node in self.traverse(location, stuckAR=True):
            if dag_node in ignore:
                continue
            if self.isAssembleReference(dag_node):
                reference_path = self.__getStaticModPath__(dag_node)
                reference_path = self.useDefaultSearchPath(reference_path)
                self.__defineUSDReferencePrim__(dag_node, reference_path)
            else:
                self.__defineUSDXformPrim__(dag_node)
            # checkout if this prim has animation,if true, record it.
            if self.isDAGNodeBeMoved(dag_node):
                self.__defineStaticUSDPrimTransform__(dag_node)

        self.__save__()
        self.__clear__()
        layer = self.useCurrentFolderPath(self.__destination__)
        self.sublayers.append(layer)
        return layer
    def exportStaticMayaRefUSD(self,location,destination=None, postfix = None, force=False, ignore=[]):
        '''
        export static(one frame) USD file from the location which build with Maya Reference
        '''
        self.__create__(destination, postfix, force)
        if self.__exist__:
            Logging.scenegraphSeparater("USD file already exist!")
            return
        # create default location as USD Xform
        self.__defineUSDXformPrimByLoops__(location)
        self.__defineUSDDefaultSetting__(location)

        node_list = self.staticMayaRelatives(location)
        for dag_node in node_list:
            maya_path = cmds.referenceQuery(dag_node,filename=1)
            reference_path = self.__getPathMayaRefUSDCache2__(dag_node)
            reference_path = self.useDefaultSearchPath(reference_path)
            self.__defineUSDReferencePrim__(dag_node, reference_path)
        self.__save__()
        self.__clear__()
        layer = self.useCurrentFolderPath(self.__destination__)
        self.sublayers.append(layer)
        return layer

    def exportDynamicAssmblyRefUSD(self, location, destination, rangeTimeCode, motionSample):
        scenegraph_path = self.fetchNameSceneGraphPrim(dagnode_path)
        scenegraph_tree = self.fetchListUSDPrim(dagnode_path)
        scenegraph_data = self.fetchDataDynamicMayaXform(dagnode_path,rangeTimeCode,motionSample)
        # create a UsdStage
        if os.path.isfile(destination):
            stage = Usd.Stage.Open(destination)
        else:
            stage = Usd.Stage.CreateNew(destination)
        for prim_node in scenegraph_tree:
            gprim = Usd.ModelAPI(UsdGeom.Xform.Define(stage, prim_node))
        prim = stage.GetPrimAtPath( scenegraph_path )
        root_prim = stage.GetPrimAtPath( self.fetchNameUsdRoot(scenegraph_path))
        # set USD default setting
        stage.SetStartTimeCode(rangeTimeCode[0])
        stage.SetEndTimeCode(rangeTimeCode[1])
        stage.SetDefaultPrim(root_prim)
        UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
        # set visibility rotateXYZ scale translate sampleTime data into specific prim
        for frameData in sorted(scenegraph_data.keys()):
            if scenegraph_data[frameData]["visibility"]:
                UsdGeom.Imageable(prim).MakeVisible(frameData)
            else:
                UsdGeom.Imageable(prim).MakeInvisible(frameData)
            rotateXYZ = scenegraph_data[frameData]["rotateXYZ"]
            UsdGeom.XformCommonAPI(prim).SetRotate(tuple(rotateXYZ),UsdGeom.XformCommonAPI.RotationOrderXYZ,frameData)
            scale = scenegraph_data[frameData]["scale"]
            UsdGeom.XformCommonAPI(prim).SetScale(tuple(scale),frameData)
            translate = scenegraph_data[frameData]["translate"]
            UsdGeom.XformCommonAPI(prim).SetTranslate(tuple(translate), frameData)
        # save UsdStage
        stage.GetRootLayer().Save()

    def setDressing(self, destination=None, postfix=None, force=False):
        # We use the Sdf API here to quickly create layers.  Also, we're using it
        # as a way to author the subLayerPaths as there is no way to do that
        # directly in the Usd API.
        if not self.sublayers:
            return
        self.__create__(destination, postfix, force)
        from pxr import Sdf
        rootLayer = self.__stage__.GetRootLayer()
        for subLayerPath in self.sublayers:
            Sdf.Layer.CreateNew(subLayerPath)
            rootLayer.subLayerPaths.append(subLayerPath)
        rangeTimeCode = self.__getDataFrameRange__()
        self.__stage__.SetStartTimeCode(rangeTimeCode[0])
        self.__stage__.SetEndTimeCode(rangeTimeCode[1])
        UsdGeom.SetStageUpAxis(self.__stage__, UsdGeom.Tokens.y)
        self.__save__()
        self.__clear__()

if __name__ == "__main__":
    # example = SGUSDExporter()
    # chr_usd = example.exportStaticMayaRefUSD("|assets|chr",postfix=".chr", force=True)
    # prp_usd = example.exportStaticMayaRefUSD("|assets|prp",postfix=".prp", force=True)
    # scn_usd = example.exportStaticAssmblyRefUSD("|assets|scn",postfix=".scn", force=True)
    # set_usd = example.setDressing(postfix=".set", force=True)
    example = SGUSDExporter()
    example.exportStaticAssmblyRefUSD("|master",force=True)