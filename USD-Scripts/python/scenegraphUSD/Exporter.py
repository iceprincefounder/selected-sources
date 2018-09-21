import os,sys

try:
    from scenegraphUSD.Core import SGUSDExporter
    from scenegraphUSD.Setting import *
    from scenegraphUSD import Logging
    from scenegraphUSD import Utility
except ImportError:
    sys.path.append("/home/xukai/Git/git_repo/scenegraphUSD/python")
    from scenegraphUSD.Core import SGUSDExporter
    from scenegraphUSD.Setting import *
    from scenegraphUSD import Logging
    from scenegraphUSD import Utility
try:
    import maya.cmds as cmds
    import pymel.core as pm
    from pxr import Usd,Sdf,UsdGeom,Kind
except ImportError:
    print "Can`t find maya.cmds"
    print "Can`t find pymel.core"
    print "Can`t find Pxiar`s USD"

from argparse import ArgumentParser

class SGUSDMayaExporter(SGUSDExporter):
    def __init__(self): 
        super(SGUSDMayaExporter, self).__init__()
        self.__args__ = None
    def argsChaser(self):
        parser = ArgumentParser(usage='Genrate the USD file from Maya')
        parser.add_argument("-f", "--mayaFile",metavar="*.ma/*.mb", help="Maya file path use to export USD file", default="")
        parser.add_argument("-sn", "--scn", type=bool, nargs='?',
                                            const=True, default=False,metavar="|assets|scn",
                                            help="Genarate layout |assets|scn")
        parser.add_argument("-ar", "--asb", type=bool, nargs='?',
                                            const=True, default=False,metavar="|master",
                                            help="Genarate asb assets |master")
        args=None
        temp_args,unknow = parser.parse_known_args()
        args=vars(temp_args)
        self.__args__ = args
        print args
        return args
    def open(self, file_path=None):
        if not file_path:
            file_path = self.__args__["mayaFile"]
        Logging.scenegraphLogging("Open:"+file_path)
        cmds.file(file_path,o=True,f=True)
        return
    def export(self):
        Logging.scenegraphLogging("Export: \n"+str(self.__args__))
        if self.__args__["scn"]:
            self.exportStaticAssmblyRefUSD("|assets|scn")
        elif self.__args__["asb"]:
            self.exportStaticAssmblyRefUSD("|master")
        else:
            pass
        Logging.scenegraphLogging("Done: "+self.__args__["mayaFile"])

if __name__ == "__main__":
    Utility.USDPluginLoading()
    mayaExporter = SGUSDMayaExporter()
    Logging.scenegraphLogging("chasing Args")
    mayaExporter.argsChaser()
    Logging.scenegraphLogging("open file")
    mayaExporter.open()
    Logging.scenegraphLogging("export usd")
    mayaExporter.export()