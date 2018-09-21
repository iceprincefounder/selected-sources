#!/usr/bin/python

import os,shutil,sys,json,subprocess

try:
    from scenegraphUSD.Core import SGUSDExporter
    from scenegraphUSD.Setting import *
    from scenegraphUSD import Utility
    from scenegraphUSD import Logging    
    from scenegraphUSD import XMLWork
except ImportError:
    sys.path.append("/home/xukai/Git/git_repo/scenegraphUSD/python")
    from scenegraphUSD.Core import SGUSDExporter
    from scenegraphUSD.Setting import *
    from scenegraphUSD import Utility
    from scenegraphUSD import Logging
    from scenegraphUSD import XMLWork
try:
    import maya.cmds as cmds
    import pymel.core as pm
    from pxr import Usd,Sdf,UsdGeom,Kind
except ImportError:
    print "Can`t find maya.cmds"
    print "Can`t find pymel.core"
    print "Can`t find Pxiar`s USD"

from argparse import ArgumentParser


class SGUSDMayaProducer(SGUSDExporter):
    def __init__(self):
        super(SGUSDMayaProducer, self).__init__()
        self.__assembly_file__ = []
        self.__reference_file__ = []
        self.__args__ = {}
        self.__ignore__ = []
    def argsChaser(self):
        parser = ArgumentParser(usage='Genrate the USD file from Maya')
        parser.add_argument("-f", "--mayaFile",metavar="*.ma|*.mb",help="Maya file path use to genarate USD file", default="")
        parser.add_argument("-i", "--ignoreNode",metavar="*.json",help="A list which keep object to ignore,\n Example: ['|assets|scn|node1','|assets|scn|node2']", default="")
        args=None
        temp_args,unknow = parser.parse_known_args()
        args=vars(temp_args)
        if args["ignoreNode"]:
            try:
                with open(args["ignoreNode"]) as json_file:  
                    self.__ignore__ = json.load(json_file)
            except:
                self.__ignore__ = []
        self.__args__ = args
        return args
    def open(self, file_path=None):
        self.argsChaser()
        Logging.scenegraphLogging("$ "+self.__args__["mayaFile"].split("/")[-1])
        if not file_path:
            file_path = self.__args__["mayaFile"]
        cmds.file(file_path,o=True,f=True)
        return
    def getMayaWorkFilePath(self,file_path):
        '''
        For instance,remove assembly_definition folder
        '''
        result = "/"
        for path in file_path.split("/"):
            if not path:
                continue
            if path == "assembly_definition":
                continue
            result = os.path.join(result,path)
        return result
    def collapseAssemblyReferece(self,location_path="|assets|scn"):
        assemblyRefereceNodes = []
        # find out current assemblyReference node
        for node_name in cmds.listRelatives(location_path,allDescendents=True,fullPath=True):
            node = pm.PyNode(node_name)
            if node.type() == "assemblyReference":
                assemblyRefereceNodes.append(node)
        for ar_node in assemblyRefereceNodes:    
            definition = ar_node.attr("definition").get()
            namespace = ar_node.attr("repNamespace").get()
            full_path = ar_node.fullPath()
            # loop to find active file list
            for rep in ar_node.getListRepresentations():
                if rep.endswith(".ma") or rep.endswith(".mb"):
                    maya_file = rep
                if rep.endswith(".locator"):
                    locator_file = rep
                if rep.endswith(".proxy.abc") or rep.endswith(".abc"):
                    gpucache_file = rep
            ar_node.setActive(locator_file)
            self.collapseAssemblyReferece(full_path)

    def expandAssemblyReferece(self,location_path="|assets|scn", ignore_list=None):
        if not ignore_list:
            ignore_list = self.__ignore__
        assemblyRefereceNodes = []
        # find out current assemblyReference node
        for node_name in cmds.listRelatives(location_path,allDescendents=True,fullPath=True):
            node = pm.PyNode(node_name)
            if node.type() == "assemblyReference":
                assemblyRefereceNodes.append(node)
        for ar_node in assemblyRefereceNodes:    
            definition = ar_node.attr("definition").get()
            namespace = ar_node.attr("repNamespace").get()
            full_path = ar_node.fullPath()
            for rep in ar_node.getListRepresentations():
                if rep.endswith(".ma") or rep.endswith(".mb"):
                    maya_file = rep
                if rep.endswith(".locator"):
                    locator_file = rep
                if rep.endswith(".proxy.abc") or rep.endswith(".abc"):
                    gpucache_file = rep

            if not ar_node.name() in ignore_list and not ar_node.fullPath() in ignore_list:
                Logging.scenegraphLogging("Expand:"+ar_node.name())
                if int(definition.find("/scn/")) < 0 and int(definition.find("/asb/")) < 0:
                    # if assembly file references a prop file,we record it dag node full path
                    self.__reference_file__.append(full_path)
                # if assembly file references a asb file,we would load the maya file it referenced
                else:
                    self.__assembly_file__.append(definition)
                    ar_node.setActive(maya_file)
            else:
                Logging.scenegraphLogging("@Ignore:"+ar_node.name())

            self.expandAssemblyReferece(ar_node.fullPath() , ignore_list)

    def genarate(self):
        # open maya file
        Logging.scenegraphStepper(1, 4,"Start to Open maya file")
        self.open()
        Logging.scenegraphLogging("Done,open maya file")
        # coolapse all assembly reference
        Logging.scenegraphStepper(2, 4,"Start to collapse assembly reference")
        self.collapseAssemblyReferece()
        Logging.scenegraphLogging("Done,collapse assembly reference")
        # expand all assembly reference
        Logging.scenegraphStepper(3, 4,"Start to expand assembly reference")
        self.expandAssemblyReferece()
        Logging.scenegraphLogging("Done,expand assembly reference")


        Logging.scenegraphStepper(4, 4,"Start to genarate scene USD file")
        chr_usd = self.exportStaticMayaRefUSD("|assets|chr",postfix=".chr", force=True)
        prp_usd = self.exportStaticMayaRefUSD("|assets|prp",postfix=".prp", force=True)
        scn_usd = self.exportStaticAssmblyRefUSD("|assets|scn",postfix=".scn", force=True)
        self.setDressing(postfix=".set", force=True)

        SGUSDExporter = os.path.join(SGUSD_ROOT_PATH,"..","..","bin","SGUSDExporter")
        for node in self.__assembly_file__:
            if node.find("/scn/") >= 0 or node.find("/asb/") >= 0:
                ## remove /assembly_definition from maya_path
                true_node = self.getMayaWorkFilePath(node)
                command = SGUSDExporter + " -f " + true_node + " --asb"
                os.system(command)
        Logging.scenegraphLogging("Done,genarate scene USD file")


if __name__ == "__main__":
    Utility.USDPluginLoading()
    mayaProducer = SGUSDMayaProducer()
    mayaProducer.genarate()
    XMLWork.genarateXML()