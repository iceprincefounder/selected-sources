# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

"""
scenegraphXML.py

Python classes to represent structured hierarchical scenegraph data
that can be read and written using XML and Tako .hdf files.
"""

import xml.etree.ElementTree as ET
import os.path
import logging


__version__ = '0.1.0'

# Defines the logging level
SG_XML_LOG_LEVEL = logging.WARNING

# Set the Logger level
logging.getLogger().setLevel(SG_XML_LOG_LEVEL)


def floatOrNone(val):
    """
    Returns None if val is None and cast it into float otherwise.
    """
    if val is None:
        return None
    else:
        return float(val)


def intOrNone(val):
    """
    Returns None if val is None and cast it into int otherwise.
    """
    if val is None:
        return None
    else:
        return int(val)


def createScenegraphElementFromXMLData(xmlElement):
    """
    Utility function to create appropriate scenegraph element type when
    reading in XML data. Uses the tag value of the XML data to create and
    instance of the appropriate class.
    """
    elementType = xmlElement.get('type')

    if elementType == 'group':
        newGroup = Group()
        newGroup.readXMLData(xmlElement)
        return newGroup

    if elementType == 'reference':
        newReference = Reference()
        newReference.readXMLData(xmlElement)
        return newReference

    raise ValueError, 'no valid scenegraph element found when reading XML data for ScenegraphElement'


def applyXformToVector(m, v):
    """
    Utility function to calculate the effect of an Xform (list of 16 values) on a 3D vector (list of 3 values)
    """
    # matrix multiplication based on homogenous vectors of form (x, y, z, 1)
    x = m[0]*v[0] + m[4]*v[1] + m[8]*v[2] + m[12]
    y = m[1]*v[0] + m[5]*v[1] + m[9]*v[2] + m[13]
    z = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]
    w = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]

    # trap for case w == 0
    if w == 0:
        raise "ValueError, w==0 after homegenous matrix multiplication"
    else:
        return [x/w, y/w, z/w]

def applyXformToBounds(curXform, curBounds):
    """
    Utility function to calculate the effect of an Xform (list of 16 values) to a bounds (list of 6 values)
    """
    newBounds = None
    # loop through the 8 corners of the bounding box
    for ix, iy, iz in ( (0,2,4), (1,2,4), (0,3,4), (1,3,4), (0,2,5), (1,2,5), (0,3,5), (1,3,5) ):
        # calculate position of corner under the Xform
        curPos = applyXformToVector(curXform, [curBounds[ix], curBounds[iy], curBounds[iz]])
        # if this is the first corner, initialize the bounds
        if newBounds is None:
            newBounds = [curPos[0], curPos[0], curPos[1], curPos[1], curPos[2], curPos[2]]
        # otherwise see if the position of the corner extends the bounds
        else:
            if curPos[0] < newBounds[0]:
                newBounds[0] = curPos[0]
            if curPos[0] > newBounds[1]:
                newBounds[1] = curPos[0]
            if curPos[1] < newBounds[2]:
                newBounds[2] = curPos[1]
            if curPos[1] > newBounds[3]:
                newBounds[3] = curPos[1]
            if curPos[2] < newBounds[4]:
                newBounds[4] = curPos[2]
            if curPos[2] > newBounds[5]:
                newBounds[5] = curPos[2]
    # return result
    return newBounds

# Borrow the indent code from http://norwied.wordpress.com/
def indent( elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i
    return

class ArbitraryAttribute:
    """
    Class to hold animated static arbitrary attribute values. Values are typed, and
    can be single values or lists of the same value type. It also writes and 
    reads the in-memory XML representation of these values using ElementTree. 
    """

    def __init__(self, name=None, dataType=None, value=None, numValues=None, channelIndex=None):
        """
            Type of the generic value is defined by the combination of dataType and listLength.
            valid values of dataType: "float", "floatList" or "string"
        """
        self.name = name
        self.dataType = dataType
        self.numValues = numValues
        self.value = value
        self.channelIndex = channelIndex
        

    def writeXMLData(self, xmlArbitraryAttribute):
        """
        Writes the Arbitrary Attribute values into the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling ArbitraryAttribute.writeXMLData')
        if self.name is None:
            raise ValueError, 'name not set for ArbitraryAttribute'
        xmlArbitraryAttribute.attrib['name'] = self.name

        if self.dataType is None:
            raise ValueError, 'dataType not specified for ArbitraryAttribute'
        xmlArbitraryAttribute.attrib['type'] = self.dataType

        if self.numValues is not None:
            xmlArbitraryAttribute.attrib['numValues'] = str(self.numValues)
        
        if self.value is None and self.channelIndex is None:
            raise ValueError, 'neither value nor channelIndex set for ArbitraryAttribute'

        if self.value is not None:
            if self.dataType == 'float':
                xmlArbitraryAttribute.attrib['value'] = str(float(self.value))
            elif self.dataType == 'floatList':
                xmlArbitraryAttribute.attrib['value'] = ' '.join(str(float(x)) for x in self.value)
            elif self.dataType == 'string':
                xmlArbitraryAttribute.attrib['value'] = self.value
            else:
                raise ValueError, 'invalid dataType for ArbitraryAttribute'

                     
        if self.channelIndex is not None:  
            xmlArbitraryAttribute.attrib['channelIndex'] = str(self.channelIndex)
       

    def readXMLData(self, xmlArbitraryAttribute):
        """
        Reads the Arbitrary Attribute values from the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling ArbitraryAttribute.readXMLData')

        self.name = xmlArbitraryAttribute.get('name')

        self.dataType = xmlArbitraryAttribute.get('type')

        self.value = xmlArbitraryAttribute.get('value')
        if self.value is not None:
            if self.dataType == 'float':
                self.value = float(self.value)
            elif self.dataType == 'floatList':
                self.value = [float(x) for x in self.value.split(' ')]
            elif self.dataType == 'string':
                pass
            else:
                raise ValueError, 'invalid dataType for ArbitraryAttribute'
                
        self.numValues = intOrNone(xmlArbitraryAttribute.get('numValues'))

        self.channelIndex = intOrNone(xmlArbitraryAttribute.get('channelIndex'))


class Bounds:
    """
    Class to hold Bounding Box  values. It also writes and reads the in-memory 
    XML representation of these values using ElementTree. 
    """

    def __init__(self, minx=None, maxx=None, miny=None, maxy=None, minz=None, maxz=None, value=None, channelIndex=None):
        if minx is not None:
            self.value = [minx, maxx, miny, maxy, minz, maxz]
        else:
            self.value = value
        self.channelIndex = channelIndex
            

    def setValue(self, value, channelData):
        if self.channelIndex is not None:
            channelData.setValue(self.channelIndex, value[0])
            channelData.setValue(self.channelIndex+1, value[1])
            channelData.setValue(self.channelIndex+2, value[2])
            channelData.setValue(self.channelIndex+3, value[3])
            channelData.setValue(self.channelIndex+4, value[4])
            channelData.setValue(self.channelIndex+5, value[5])
        else:
            self.value = value

    def getValue(self, channelData):
        if self.channelIndex is not None:
            return channelData.getValues(self.channelIndex, 6)
        else:
            return self.value

    def writeXMLData(self, xmlBounds):
        """
        Writes the Bounding Box values into the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling Bounds.writeXMLData')
        if self.value is None and self.channelIndex is None:
            raise ValueError, 'bounds not set using static values or channelIndex'

        if self.value is not None and len(self.value) > 0:
            xmlBounds.attrib['minx'] = str(float(self.value[0]))
            xmlBounds.attrib['maxx'] = str(float(self.value[1]))
            xmlBounds.attrib['miny'] = str(float(self.value[2]))
            xmlBounds.attrib['maxy'] = str(float(self.value[3]))
            xmlBounds.attrib['minz'] = str(float(self.value[4]))
            xmlBounds.attrib['maxz'] = str(float(self.value[5]))

        if self.channelIndex is not None:
            xmlBounds.attrib['channelIndex'] = str(self.channelIndex)

    def readXMLData(self, xmlBounds):
        """
        Reads the Bounding Box values from the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling Bounds.readXMLData')
        minx = floatOrNone(xmlBounds.get('minx'))
        maxx = floatOrNone(xmlBounds.get('maxx'))
        miny = floatOrNone(xmlBounds.get('miny'))
        maxy = floatOrNone(xmlBounds.get('maxy'))
        minz = floatOrNone(xmlBounds.get('minz'))
        maxz = floatOrNone(xmlBounds.get('maxz'))
        if minx is not None:
            self.value = [minx, maxx, miny, maxy, minz, maxz]
        else:
            self.value = None
        self.channelIndex = intOrNone(xmlBounds.get('channelIndex'))


class Xform:
    """
    Class to hold Transform values. It also writes and reads the in-memory 
    XML representation of these values using ElementTree. 
    """

    def __init__(self, value=None, channelIndex=None):
        self.value = value
        self.channelIndex = channelIndex

    def getValue(self, channelData):
        if self.channelIndex is not None:
            return channelData.getValues(self.channelIndex, 16)
        else:
            return self.value

    def writeXMLData(self, xmlXform):
        """
        Writes the Transformation values into the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling Xfrom.writeXMLData')
        if self.value is None and self.channelIndex is None:
            raise ValueError, 'neither value nor channelIndex set for Xform'

        if self.value is not None:
            xmlXform.attrib['value'] = ' '.join(str(float(x)) for x in self.value)

        if self.channelIndex is not None:
            xmlXform.attrib['channelIndex'] = str(self.channelIndex)

    def readXMLData(self, xmlXform):
        """
        Reads the Transformation values from the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling Xform.readXMLData')
        valueAsString = xmlXform.get('value')
        if valueAsString is not None:
            self.value = [float(x) for x in valueAsString.split(' ')]

        self.channelIndex = intOrNone(xmlXform.get('channelIndex'))


class Proxy:
    """
    Class to hold a geometry Proxy. It also writes and reads the in-memory 
    XML representation of these values using ElementTree. 
    """

    def __init__(self, name=None, ref=None):
        self.name = name
        self.ref = ref

    def writeXMLData(self, xmlProxy):
        """
        Writes the Proxy  values into the in-memory XML representation using 
        ElementTree.
        """
        logging.debug('    calling Proxy.writeXMLData')
        if self.name is None:
            raise ValueError, 'name is not set when writing out Proxy data to XML'
        xmlProxy.attrib['name'] = self.name

        if self.ref is None:
            raise ValueError, 'ref is not set when writing out Proxy data to XML'
        xmlProxy.attrib['ref'] = self.ref

    def readXMLData(self, xmlProxy):
        """
        Reads the Proxy  values from the in-memory XML representation using 
        ElementTree.
        """
        logging.debug('    calling Proxy.readXMLData')
        self.name = xmlProxy.get('name')
        if self.name is None:
            raise ValueError, 'cannot find XML attribute "name" when reading XML data for Proxy'

        self.ref = xmlProxy.get('ref')
        if self.ref is None:
            raise ValueError, 'cannot find XML attribute "ref" when reading XML data for Proxy'


class LodData:
    """
    Class to hold a geometry LOD data. It also writes and reads the in-memory 
    XML representation of these values using ElementTree. 
    """

    def __init__(self, tag=None, weight=None, channelIndex=None):
        self.tag = tag
        self.weight = weight
        self.channelIndex = channelIndex

    def writeXMLData(self, xmlLodData):
        """
        Writes the LOD Data values into the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling LodData.writeXMLData')
        if self.tag is None and self.weight is None:
            raise ValueError, 'neither tag nor weight set when writing out LodData to XML'

        if self.tag is not None:
            xmlLodData.attrib['tag'] = self.tag

        if self.weight is not None:
            xmlLodData.attrib['weight'] = str(float(self.weight))

        if self.channelIndex is not None:
            xmlLodData.attrib['channelIndex'] = str(self.channelIndex)


    def readXMLData(self, xmlLodData):
        """
        Reads the LOD Data values from the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling LodData.readXMLData')
        self.tag = xmlLodData.get('tag')
        self.weight = floatOrNone(xmlLodData.get('weight'))
        self.channelIndex = intOrNone(xmlLodData.get('channelIndex'))


class LookFile:

    def __init__(self, ref=None, channelIndex=None):
        self.ref = ref        
        self.channelIndex = channelIndex

    def writeXMLData(self, xmlLookFile):
        logging.debug('    calling LookFile.writeXMLData')
        if self.ref is not None:
            xmlLookFile.attrib['ref'] = self.ref
        if self.channelIndex is not None:
            xmlLookFile.attrib['channelIndex'] = str(self.channelIndex)

    def readXMLData(self, xmlLookFile):
        logging.debug('    calling LookFile.readXMLData')
        self.ref = xmlLookFile.get('ref')
        self.channelIndex = intOrNone(xmlLookFile.get('channelIndex'))


class AttributeFile:

    def __init__(self, ref=None, groupName=None, customParser=None, channelIndex=None):
        self.ref = ref
        self.groupName = groupName
        self.customParser = customParser
        self.channelIndex = channelIndex

    def writeXMLData(self, xmlAttributeFile):
        logging.debug('    calling AttributeFile.writeXMLData')
        if self.ref is not None:
            xmlAttributeFile.attrib['ref'] = self.ref
        if self.groupName is not None and self.groupName != '':
            xmlAttributeFile.attrib['groupName'] = self.groupName
        if self.customParser is not None and self.customParser != '':
            xmlAttributeFile.attrib['customParser'] = self.customParser
        if self.channelIndex is not None:
            xmlAttributeFile.attrib['channelIndex'] = str(self.channelIndex)

    def readXMLData(self, xmlAttributeFile):
        logging.debug('    calling AttributeFile.readXMLData')
        self.ref = xmlAttributeFile.get('ref')
        self.groupName = xmlAttributeFile.get('groupName')
        self.customParser = xmlAttributeFile.get('customParser')
        self.channelIndex = intOrNone(xmlAttributeFile.get('channelIndex'))


class ChannelData:
    """
    Class to hold a animation Channel data. It also writes and reads the 
    in-memory XML representation of these values using ElementTree. 
    """
    
    def __init__(self, startFrame=None, endFrame=None, ref=None):
        self.startFrame = startFrame
        self.endFrame = endFrame
        self.ref = ref
        self.values = []
        self.relativeMode = True

    def isStatic(self):
        return self.startFrame == self.endFrame

    def setValue(self, index, value):
        """
        Adds a value in a given channel index. If the current number of indexes
        is smaller than the one specified, 0.0 will be added to all missing
        indexes between the currently last index and the one to be added. 
        """
        # pad value range if need be
        if index >= len(self.values):
            self.values = self.values + [0.0]*(index - len(self.values) + 1)
        self.values[index] = value

    def getValues(self, index, numElements):
        """
        Returns a single value from the channelData base
        """
        return self.values[index:index+numElements]

    def getNoValues(self):
        return len(self.values)

    def writeXMLData(self, xmlChannelData):
        """
        Writes the Channel Data values into the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    XXXXXXXX calling ChannelData.writeXMLData')
        if self.startFrame is None:
            raise ValueError, 'startFrame not set when writing ChannelData to XML'
        xmlChannelData.attrib['startFrame'] = str(self.startFrame)

        if self.endFrame is None:
            raise ValueError, 'endFrame not set when writing ChannelData to XML'
        xmlChannelData.attrib['endFrame'] = str(self.endFrame)
        
        if self.ref is not None:
            if self.relativeMode:
                relPath = self.ref[self.ref.rfind('/')+1:]
                xmlChannelData.attrib['ref'] = relPath
            else:
                xmlChannelData.attrib['ref'] = self.ref

    def readXMLData(self, xmlChannelData):
        """
        Read the Channel Data values from the in-memory XML representation  
        using ElementTree.
        """
        logging.debug('    calling ChannelData.readXMLData')
        self.startFrame = int(float(xmlChannelData.get('startFrame')))
        self.endFrame = int(float(xmlChannelData.get('endFrame')))
        self.ref = xmlChannelData.get('ref')

    def writeXMLChannelFile(self, frameNumber, verbose=True):
        """
        Writes the in-memory XML representation using ElemenTree into a channel
        xml file. frameNumber will be used in the filename before the .xml 
        extention using 4 zero padding.
        """
        filepath = self.ref + ".chan.%04d.xml" % frameNumber
        logging.debug('\nwriting XML channel data to file %s' % filepath)
        dir = os.path.dirname(filepath)
        if not os.path.isdir(dir):
            os.makedirs(dir)
        xmlRoot = ET.Element('channels')

        for curValue in self.values:
            xmlCurValue = ET.SubElement(xmlRoot, 'c')
            xmlCurValue.attrib['v'] = str(float(curValue))

        if verbose:
            print "writing file %s" % filepath

        xmlTree = ET.ElementTree(xmlRoot)
        xmlTree.write(filepath)

    def readXMLChannelFile(self, frameNumber):
        """
        Reads the contents of a channel xml file into the in-memory XML 
        representation using ElemenTree. frameNumber will be used in the 
        construction of the filename before the .xml extention using 4 zero 
        padding.
        """
        filepath = self.ref + ".chan.%04d.xml" % frameNumber
        logging.debug('\nreading XML channel data file %s' % filepath)
        if not os.path.isfile(filepath):
            print 'filePath: %s' % filepath
            raise ValueError, 'Warning: file does not exist'       
        xmlTree = ET.parse(filepath)
        xmlRoot = xmlTree.getroot()

        for index, xmlCurValue in enumerate(xmlRoot):
            value = float(xmlCurValue.get('v'))
            self.setValue(index, value)        


class ScenegraphElement:
    """
    Base class used by Group, ScenegraphRoot and Reference classes
    """

    def __init__(self, name=None, elemType=None, xform=None, bounds=None, proxyList=None, arbitraryList=None, lodData=None, lookFile=None, attributeFile=None):
        self.name = name
        self.elemType = elemType
        self.xform = xform
        self.bounds = bounds
        self.proxyList = proxyList
        self.arbitraryList = arbitraryList
        self.lodData = lodData
        self.lookFile = lookFile
        self.attributeFile = attributeFile

    def setLookFile(self, ref=None, channelIndex=None):
        self.lookFile = LookFile(ref, channelIndex)

    def setAttributeFile(self, ref=None, groupName=None, customParser=None, channelIndex=None):
        self.attributeFile = AttributeFile(ref, groupName, customParser, channelIndex)

    def setBounds(self, minx=None, maxx=None, miny=None, maxy=None, minz=None, maxz=None, value=None, channelIndex=None):
        """
        Sets a Bounding Box's values. 
        """
        self.bounds = Bounds(minx, maxx, miny, maxy, minz, maxz, value, channelIndex)

    def getBounds(self, channelData=None):
        # Returns the bounds for a instance node. This method is over-ridden
        # for Group nodes to allow auto-calculation of bounds based on children
        # Note: 'None' is returned if there aren't any valid bounds
        if self.bounds is None:
            return None
        else:
            return self.bounds.getValue(channelData)

    def setXform(self, value=None, channelIndex=None):
        """
        Sets a Transform's values. 
        """
        self.xform = Xform(value, channelIndex)

    def getXform(self, channelData):
        if self.xform is None:
            return None
        else:
            return self.xform.getValue(channelData)

    def setLodData(self, tag=None, weight=None, channelIndex=None):
        """
        Sets LOD values. 
        """
        self.lodData = LodData(tag, weight, channelIndex)

    def addProxy(self, name, ref):
        """
        Adds a proxy geometry (tagged by the provided name) 
        """
        newProxy = Proxy(name, ref)
        if self.proxyList is None:
            self.proxyList = [newProxy]
        else:
            self.proxyList.append(newProxy)

    def addArbitraryAttribute(self, name, dataType, value=None, numValues=None, channelIndex=None):
        """
        Adds an arbitrary attribute.
        """
        newArbitraryAttribute = ArbitraryAttribute(name, dataType, value, numValues, channelIndex)
        if self.arbitraryList is None:
            self.arbitraryList = [newArbitraryAttribute]
        else:
            self.arbitraryList.append(newArbitraryAttribute)


    def writeXMLCommonData(self, xmlElement):
        """
        Writes the full scene's in-memory XML representation using ElementTree 
        """
        logging.debug('  calling ScenegraphElement.writeXMLCommonData for %s' % self.name)
        if self.name is not None:
            xmlElement.attrib['name'] = self.name

        if self.elemType is not None:
            xmlElement.attrib['type'] = self.elemType

        if self.bounds is not None:
            xmlBounds = ET.SubElement(xmlElement, 'bounds')
            self.bounds.writeXMLData(xmlBounds)

        if self.xform is not None:
            xmlXform = ET.SubElement(xmlElement, 'xform')
            self.xform.writeXMLData(xmlXform)

        if self.lodData is not None:
            xmlLodData = ET.SubElement(xmlElement, 'lodData')
            self.lodData.writeXMLData(xmlLodData)

        if self.proxyList is not None:
            xmlProxyList = ET.SubElement(xmlElement, 'proxyList')
            for curProxy in self.proxyList:
                xmlCurProxy = ET.SubElement(xmlProxyList, 'proxy')
                curProxy.writeXMLData(xmlCurProxy)

        if self.arbitraryList is not None:
            xmlArbitraryList = ET.SubElement(xmlElement, 'arbitraryList')
            for curAttribute in self.arbitraryList:
                xmlCurAttribute = ET.SubElement(xmlArbitraryList, 'attribute')
                curAttribute.writeXMLData(xmlCurAttribute)

        if self.lookFile is not None:
            xmlLookFile = ET.SubElement(xmlElement, 'lookFile')
            self.lookFile.writeXMLData(xmlLookFile)

        if self.attributeFile is not None:
            xmlAttributeFile = ET.SubElement(xmlElement, 'attributeFile')
            self.attributeFile.writeXMLData(xmlAttributeFile)


    def readXMLCommonData(self, xmlElement):
        """
        Reads a full scene's in-memory XML representation using ElementTree 
        """
        self.name = xmlElement.get('name')
        logging.debug('  calling ScenegraphElement.readXMLCommonData for %s' % self.name)

        xmlBounds = xmlElement.find('bounds')
        if xmlBounds is not None:
            self.bounds = Bounds()
            self.bounds.readXMLData(xmlBounds)
        else:
            self.bounds = None

        xmlXform = xmlElement.find('xform')
        if xmlXform is not None:
            self.xform = Xform()
            self.xform.readXMLData(xmlXform)
        else:
            self.xfrom = None

        xmlLodData = xmlElement.find('lodData')
        if xmlLodData is not None:
            self.lodData = LodData()
            self.lodData.readXMLData(xmlLodData)
        else:
            self.lodData = None

        xmlProxyList = xmlElement.find('proxyList')
        if xmlProxyList is not None:
            self.proxyList = []
            for xmlCurProxy in xmlProxyList:
                curProxy = Proxy()
                curProxy.readXMLData(xmlCurProxy)
                self.proxyList.append(curProxy)
        else:
            self.proxyList = None

        xmlArbitraryList = xmlElement.find('arbitraryList')
        if xmlArbitraryList is not None:
            self.arbitraryList = []
            for xmlCurAttribute in xmlArbitraryList:
                curAttribute = ArbitraryAttribute()
                curAttribute.readXMLData(xmlCurAttribute)
                self.arbitraryList.append(curAttribute)
        else:
            self.arbitraryList = None

        xmlLookFile = xmlElement.find('lookFile')
        if xmlLookFile is not None:
            self.lookFile = LookFile()
            self.lookFile.readXMLData(xmlLookFile)
        else:
            self.lookFile = None
        
        xmlAttributeFile = xmlElement.find('attributeFile')
        if xmlAttributeFile is not None:
            self.attributeFile = AttributeFile()
            self.attributeFile.readXMLData(xmlAttributeFile)
        else:
            self.attributeFile = None


class Group(ScenegraphElement):
    """
    Represents a Group node. A group node contains children that an be of any
    node type. 
    """

    def __init__(self, name=None, instanceList=None, groupType=None, xform=None, bounds=None, proxyList=None, arbitraryList=None, lodData=None):
        ScenegraphElement.__init__(self, name=name, xform=xform, bounds=bounds, proxyList=proxyList, arbitraryList=arbitraryList, lodData=lodData)
        self.instanceList = instanceList
        self.elemType = 'group'
        self.groupType = groupType
        self.boundsAutoCalc = True

    def setInstanceList(self, instanceList):
        self.instanceList = instanceList

    def addInstance(self, newInstance):
        if self.instanceList is None:
            self.instanceList = []
        self.instanceList.append(newInstance)

    def addInstances(self, newInstances):
        for curInstance in newInstances:
            self.addInstance(curInstance)

    def getBounds(self, channelData=None):
        if self.instanceList is None:
            return None

        # returns the bounds for a group node, calculating it from the instances in the
        # instanceList if we're using BoundsAutoCalc on this node. Note: 'None' is
        # returned if there aren't any valid bounds                
        if self.boundsAutoCalc:            
            calculatedBounds = None
            for curInstance in self.instanceList:
                # get current values for Xform and Bounds
                curInstanceBounds = curInstance.getBounds(channelData)

                if curInstanceBounds is not None:
                    curInstanceXform = curInstance.getXform(channelData)
                    # if wen have an Xform, apply Xform to Bounds
                    if curInstanceXform is not None:
                        curInstanceBounds = applyXformToBounds(curInstanceXform, curInstanceBounds)
                    # if this is the first valid bounds value, we set curBounds to it
                    if calculatedBounds is None:
                        calculatedBounds = curInstanceBounds[:]
                    # otherwise we compare it with the existing value to see if it extends the bounds
                    else:
                        if curInstanceBounds[0] < calculatedBounds[0]:
                            calculatedBounds[0] = curInstanceBounds[0]
                        if curInstanceBounds[1] > calculatedBounds[1]:
                            calculatedBounds[1] = curInstanceBounds[1]
                        if curInstanceBounds[2] < calculatedBounds[2]:
                            calculatedBounds[2] = curInstanceBounds[2]
                        if curInstanceBounds[3] > calculatedBounds[3]:
                            calculatedBounds[3] = curInstanceBounds[3]
                        if curInstanceBounds[4] < calculatedBounds[4]:
                            calculatedBounds[4] = curInstanceBounds[4]
                        if curInstanceBounds[5] > calculatedBounds[5]:
                            calculatedBounds[5] = curInstanceBounds[5]
            # check if this node has bounds for export, in which case store the calculated curBounds 
            if self.bounds is not None and calculatedBounds is not None:
                self.bounds.setValue(calculatedBounds, channelData)
            # return the calculated bounds value
            return calculatedBounds
        else:
            # we're not auto calculating the bounds on this node, so we need to return value directly
            if self.bounds is None:
                return None
            else:
                return self.bounds.getValue(channelData)
        
    def writeXMLData(self, sgElement, xmlElement, channelMapping=None, channelData=None):
        if channelMapping is not None:
            if sgElement in channelMapping.keys():
                channelNo = channelMapping[sgElement]
                channelValues = channelData.getValues(channelNo, 6)
                sgElement.setBounds(value=channelValues)
            else:
                logging.debug('Missing [%s]' % sgElement.name)

        logging.debug('calling Group.writeXMLData for %s' % self.name)
        self.writeXMLCommonData(xmlElement)

        if self.groupType is not None:
            xmlElement.attrib['groupType'] = self.groupType

        xmlInstanceList = ET.SubElement(xmlElement, 'instanceList')
        if self.instanceList is not None:
            for curInstance in self.instanceList:
                xmlCurInstance = ET.SubElement(xmlInstanceList, 'instance')
                curInstance.writeXMLData(curInstance, xmlCurInstance, channelMapping, channelData)

    def readXMLData(self, xmlElement):
        logging.debug('calling Group.readXMLData')
        self.readXMLCommonData(xmlElement)

        self.groupType = xmlElement.get('groupType')

        xmlInstanceList = xmlElement.find('instanceList')
        if xmlInstanceList is None:
            raise ValueError, 'cannot find XML element "instanceList" when reading XML data for Group'
        self.instanceList = []
        for xmlCurInstance in xmlInstanceList:
            curInstance = createScenegraphElementFromXMLData(xmlCurInstance)
            self.instanceList.append(curInstance)


class Reference(ScenegraphElement):
    """
    Represents a Reference node. A Reference points to another xml file containing
    some sub-SceneGraph.  
    """

    def __init__(self, name=None, refFile=None, refType='xml', xform=None, bounds=None, proxyList=None, 
                 arbitraryList=None, lodData=None, lookFile=None, attributeFile=None, groupType=None):
        ScenegraphElement.__init__(self, name=name, xform=xform, bounds=bounds, proxyList=proxyList, arbitraryList=arbitraryList, lodData=lodData, lookFile=lookFile, attributeFile=attributeFile)
        self.elemType = 'reference'
        self.refFile = refFile
        self.refType = refType
        self.groupType = groupType

    def setReference(self, refFile, refType='xml'):
        self.refFile = refFile
        self.refType = refType
 
    def setRefFile(self, refFile):
        self.refFile = refFile
        
    def setRefType(self, refType):
        self.refType = refType
        
    def writeXMLData(self, sgElement, xmlElement, channelMapping=None, channelData=None):      
        if channelMapping is not None:
            if sgElement in channelMapping.keys():
                channelNo = channelMapping[sgElement]
                channelValues = channelData.getValues(channelNo, 6)
                sgElement.setBounds(value=channelValues)
            else:
                logging.debug('Missing [%s]' % sgElement.name)

        logging.debug('calling Reference.writeXMLData for %s' % self.name)
        self.writeXMLCommonData(xmlElement)

        if self.refType is not None:
            xmlElement.attrib['refType'] = self.refType

        if self.refFile is not None:
            xmlElement.attrib['refFile'] = self.refFile
        else:
            raise ValueError, 'refFile not set when writing out XML for Reference'

        if self.groupType is not None:
            xmlElement.attrib['groupType'] = self.groupType

    def readXMLData(self, xmlElement):
        logging.debug('calling Reference.readXMLData')
        self.readXMLCommonData(xmlElement)

        self.refType = xmlElement.get('refType')

        self.refFile = xmlElement.get('refFile')
        if self.refFile is None:
            raise ValueError, 'cannot find XML attribute "refFile" when reading XMl data for Reference'


class ScenegraphRoot(Group):
    """
    Represents a root node of a full Scene.
    """
    
    def __init__(self, name=None, instanceList=None, channelData=None):
        Group.__init__(self, name=name, instanceList=instanceList)
        self.channelData = channelData
        self.channelMapping = {}

    def setChannelDataValue(self, index, value):        
        self.channelData.setValue(index, value)

    def calcBounds(self):
        return self.getBounds(self.channelData)

    def writeXMLFile(self, filepath, verbose=True):
        logging.debug('\nwriting XML to file %s' % filepath)
        dir = os.path.dirname(filepath)
        if not os.path.isdir(dir):
            os.makedirs(dir)   
        xmlRoot = ET.Element('scenegraphXML')
        xmlRoot.attrib['version'] = __version__
        if self.channelData is not None and not self.channelData.isStatic():
            xmlChannelData = ET.SubElement(xmlRoot, 'channelData')
            self.channelData.writeXMLData(xmlChannelData)

        if self.instanceList is None:
            raise ValueError, 'instanceList not set when writing out XMl for ScenegraphRoot'
        xmlInstanceList = ET.SubElement(xmlRoot, 'instanceList')
        
        # Avoid if statements within a for loop
        channelMappingPackage = None
        if self.channelData is not None and self.channelData.isStatic():
            channelMappingPackage = self.channelMapping
        
        for curInstance in self.instanceList:
            xmlCurInstance = ET.SubElement(xmlInstanceList, 'instance')
            curInstance.writeXMLData(curInstance, xmlCurInstance, channelMappingPackage, self.channelData)

        if verbose:
            print "writing file %s" % filepath

        indent(xmlRoot)
        xmlTree = ET.ElementTree(xmlRoot)
        xmlTree.write(filepath)

    def readXMLFile(self, filepath):
        logging.debug('\nreading XML file %s' % filepath)
        if not os.path.isfile(filepath):
            print 'filePath: %s' % filepath
            raise ValueError, 'Warning: file does not exist'       
        xmlTree = ET.parse(filepath)
        xmlRoot = xmlTree.getroot()
        fileVersion = xmlRoot.get('version')
        if fileVersion != __version__:
            print 'Warning: xml file version does not match'

        xmlChannelData = xmlRoot.find('channelData')
        if xmlChannelData is not None:
            self.channelData = ChannelData()
            self.channelData.readXMLData(xmlChannelData)
        else:
            self.ChannelData = None

        xmlInstanceList = xmlRoot.find('instanceList')
        if xmlInstanceList is None:
            raise ValueError, 'cannot find XML element "instanceList" when reading XML data for ScenegraphRoot'
        self.instanceList = []
        for xmlCurInstance in xmlInstanceList:
            curInstance = createScenegraphElementFromXMLData(xmlCurInstance)
            self.instanceList.append(curInstance)

    def addChannelMapping(self, channelNo, element):
        self.channelMapping[element] = channelNo

    def writeXMLChannelFile(self, frameNumber):
        self.channelData.writeXMLChannelFile(frameNumber)

    def readXMLChannelFile(self, frameNumber):
        self.channelData.readXMLChannelFile(frameNumber)


# A test / example script:
if __name__ == '__main__':

    #geometry1 = Reference('gnomeGeo', refType='tako', refFile='/mnt/netpics/Katana/TestScenes/Iteration-10.2/Importomatic/gnome.hdf')
    geometry1 = Reference('marketGeo', refType='tako', refFile='/mnt/nethome/users/orn/Thor/Marketplace/Market.hdf')
    geometry1.setBounds(100, 400, -150, 150, 245, 525)
    geometry1.setLodData(tag="hi", weight=1.0)

    #rootGroup = Group('GnomeAsset')
    rootGroup = Group('Market')
    rootGroup.addInstances([geometry1])
    #rootGroup.setLookFile('/mnt/netpics/Katana/TestScenes/Iteration-10.2/Importomatic/10818_GnomeMaterial.klf')

    #channelData = ChannelData(startFrame=1, endFrame=10, ref='/mnt/netpics/Katana/TestScenes/Iteration-10.2/Importomatic/ChannelDataLook')
    #channelData = ChannelData(startFrame=1, endFrame=1, ref='/mnt/nethome/users/orn/ScenegraphXml/Marketplace/ChannelData')
    root1 = ScenegraphRoot()
    #root1 = ScenegraphRoot(channelData=channelData)
    root1.addInstance(rootGroup)
    
    #root1.writeXMLFile('/mnt/netpics/Katana/TestScenes/Iteration-10.2/Importomatic/SG_Look.xml')
    root1.writeXMLFile('/mnt/nethome/users/orn/ScenegraphXML/Marketplace/Market.xml')

    #root1.setChannelDataValue(0,0.1)
    #root1.setChannelDataValue(1,0.2)
    #root1.setChannelDataValue(2,0.3)
    #root1.setChannelDataValue(3,0.4)
    #root1.writeXMLChannelFile(10)
