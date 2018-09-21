import xml.etree.ElementTree as ET
import os,sys

try:
    from scenegraphUSD.Setting import *
    from scenegraphLIB.sgXmlParser import SgXmlParser
    from scenegraphLIB.lcShotgun import lcShotgun
except ImportError:
    sys.path.append("/home/xukai/Git/git_repo/scenegraphUSD/python")
    sys.path.append("/mnt/utility/linked_tools/lcatools/lib")

    from scenegraphUSD.Setting import *
    from scenegraphLIB.sgXmlParser import SgXmlParser
    from scenegraphLIB.lcShotgun import lcShotgun

def findLastVersion(xml_path):
    publish_folder =  xml_path[:xml_path.find("/publish/")+len("/publish")]
    old_version = xml_path[xml_path.find("/publish/")+len("/publish/"):xml_path.find("/scene_graph_xml/")]
    all_version = os.listdir(publish_folder)
    last_version =  sorted(all_version)[-1]
    xml_path = xml_path.replace("/"+old_version+"/","/"+last_version+"/")
    return xml_path

def updateVersion(input_path, output_path):
    tree = ET.parse( input_path )
    root = tree.getroot()
    for tree in root.findall("instanceList/instance/instanceList/instance"):
        if not tree.get("name")=="prp":
            continue
        for node in tree.findall("instanceList/instance"):
            xml_path = node.get("refFile")
            last_version_path = findLastVersion(xml_path)
            print last_version_path
            node.set('refFile', last_version_path)
    tree = ET.ElementTree(root)
    tree.write(output_path)

def genarateXML(output_path=None):
    """
    Export a simple XML file with this function.
    """
    import pymel.core as pm
    import maya.cmds as cmds
    file_name = cmds.file(sceneName=True,q=True)
    shot_name = file_name[file_name.find("/shot/")+6+4:file_name.find("/shot/")+6+4+6]
    if output_path:
        xml_path = output_path
    else:
        xml_path = file_name.replace(LCA_PROJ_PATH, LCA_USD_SEARCH_PATH).replace(".ma",".xml")
    l_frames = [1001]
    xml = SgXmlParser()
    xml.exportXml(pm.PyNode('|assets'), xml_path,  shot_name + u"_cam", l_frames)
    shot_handle = lcShotgun("pws",shot_name)
    shot_handle.initShotgunInfo()
    shot_handle.get_lay_xml()
    shot_handle.xmlRemake_elementTree2(xml_path,xml_path)
    return xml_path
if __name__ == "__main__":
    xml_path = genarateXML()
    
    USDCacheExporter = os.path.join(SGUSD_ROOT_PATH,"..","..","bin","USDCacheExporter")
    command = USDCacheExporter + " -p pws " + " -x " + xml_path + " -o " + LCA_USD_SEARCH_PATH
    if auto:
        os.system(command)
    else:
        Logging.scenegraphLogging("@Skip to genarate assets!")
