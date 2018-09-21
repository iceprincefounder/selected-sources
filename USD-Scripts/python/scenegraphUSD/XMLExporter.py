import xml.etree.ElementTree as ET
import os,sys

try:
    from scenegraphUSD.Core import SGUSDExporter
    from scenegraphUSD.Producer import SGUSDMayaProducer
    from scenegraphUSD.Setting import *
    from scenegraphUSD import Logging    
    from scenegraphUSD import XMLWork
except ImportError:
    sys.path.append("/home/xukai/Git/git_repo/scenegraphUSD/python")
    from scenegraphUSD.Core import SGUSDExporter
    from scenegraphUSD.Producer import SGUSDMayaProducer
    from scenegraphUSD.Setting import *
    from scenegraphUSD import Logging
    from scenegraphUSD import XMLWork
try:
    import maya.cmds as cmds
    import pymel.core as pm
except ImportError:
    print "Can`t find maya.cmds"
    print "Can`t find pymel.core"

if __name__ == "__main__":
    mayaProducer = SGUSDMayaProducer()
    Logging.scenegraphStepper(1, 4,"Start to Open maya file")
    mayaProducer.open()
    Logging.scenegraphLogging("Done,open maya file")
    # coolapse all assembly reference
    Logging.scenegraphStepper(2, 4,"Start to collapse assembly reference")
    mayaProducer.collapseAssemblyReferece()
    Logging.scenegraphLogging("Done,collapse assembly reference")
    # expand all assembly reference
    Logging.scenegraphStepper(3, 4,"Start to expand assembly reference")
    mayaProducer.expandAssemblyReferece()
    Logging.scenegraphLogging("Done,expand assembly reference")


    Logging.scenegraphStepper(4, 4,"Start to genarate scene XML file")
    XMLWork.genarateXML()
    Logging.scenegraphLogging("Done,genarate scene USD file")
