# -*- coding:utf-8 -*-

__author__ = 'lvyuedong'

import os
import sys
import shutil
import traceback
import time
import math
from xml.etree import ElementTree

try:
    import pymel.core as pm
    import maya.OpenMaya as OpenMaya
    import maya.app.general.editUtils as editUtils

    from sceneGraphXML import maya2scenegraphXML
    import tagsInFrustum as tifr
    reload(maya2scenegraphXML)
    reload(tifr)

    MAYA_VERSION = pm.about(version=True)[:4]

    if MAYA_VERSION == '2017':
        from maya.maya_to_py_itr import PyEditItr

except ImportError:
    print "Can not find pymel.core"
    print "Can not find maya.OpenMaya"
    print "Can not find maya.app.general.editUtils"
    print "Can not find sceneGraphXML"


class SgXmlParser():
    def __init__(self):
        pm.loadPlugin('sceneAssembly', quiet=True)
        self.d_taged_nodes = {'ignored':{}, 'ref':{}, 'no_bound':{}}
        return

    def osPathConvert(self, path):
        path = path.replace('\\', '/')
        if os.name == 'nt':
            if path.startswith('/mnt/proj/'):
                return path.replace('/mnt/proj/', 'Z:/')
            elif path.startswith('/mnt/work/'):
                return path.replace('/mnt/work/', 'W:/')
            else:
                return path
        else:
            if path.startswith('Z:/'):
                return path.replace('Z:/', '/mnt/proj/')
            elif path.startswith('W:/'):
                return path.replace('W:/', '/mnt/work/')
            else:
                return path
        return path


    def elapseTime(self, previous_start_point, message):
        elapse_time = format(time.time()-previous_start_point[0], '.2f')
        previous_start_point[0] = time.time()
        print 'it takes '+elapse_time+' seconds to '+message
        pm.refresh()


    def getGlobalCtrl(self, node):
        '''
        return global_ctrl node if any by given master transform node
        '''
        if node.type() != 'transform':
            return None

        ns = node.namespace()
        if ns:
            if pm.objExists(ns+'global_ctrl'):
                g_ctrl = pm.PyNode(ns+'global_ctrl')
                if node.isParentOf(g_ctrl):
                    return g_ctrl
        return None


    def getTailMaster(self, topNode):
        master = []
        tmp = [t for t in pm.ls('*:master', r=True, rn=True, long=True) if t.isChildOf(topNode)]
        for m in tmp:
            if [p for p in m.getChildren() if ':poly' in str(p)]:
                master.append(m)
        return filter(None, set(master))


    def getAssemblyReference(self, node):
        l_ar = pm.listRelatives(node, ad=True, type='assemblyReference')
        l_asb_ar_name = []
        l_asset_ar = []
        l_asb_ar = []
        l_unload_ar = []

        # Get ASB assets name by ns
        for ar in l_ar:
            ar_name = ar.name()
            if ':' in ar_name:
                ar_ns = ':'.join(ar_name.split(':')[:-1])
                l_asb_ar_name.append(ar_ns + '_AR')

        l_asb_ar_name = list(set(l_asb_ar_name))

        for ar in l_ar:
            if ar.name() in l_asb_ar_name:
                l_asb_ar.append(ar)
            else:
                if len(pm.listRelatives(ar, c=True, type='gpuCache')) == 0:
                    l_unload_ar.append(ar)
                else:
                    l_asset_ar.append(ar)
        return l_asset_ar, l_unload_ar, l_asb_ar


    def checkLeveL(self,node):
        if not pm.objExists(node.name()):return False

        ar = pm.container(query=True, findContainer=node)
        if not ar: return False
        ar = pm.container(query=True, findContainer=ar)
        if not ar: return False
        return True


    def getReferencePath(self, node):
        filepath = ''
        if isinstance(node, type('')):
            try:
                node = pm.PyNode(node)
            except:
                pm.warning('Can not figure out what '+str(node)+' is, ignored!')
                return filepath

        if pm.referenceQuery(node, isNodeReferenced=True):
            filepath = pm.referenceQuery(node,  filename=True, wcn=True).replace('\\', '/')
        elif node.nodeType() == 'assemblyReference':
            l_rep_names = pm.assembly(node, query = True, listRepresentations= True)
            if not l_rep_names:
                return ''
            rep_name = pm.assembly(node, query = True, active = True)
            if rep_name in l_rep_names:
                if not rep_name.endswith('.locator'):
                    i = l_rep_names.index(rep_name)
                    filepath = pm.getAttr(node.name() + '.rep['+str(i)+'].rda')
                else:
                    # Choose locator
                    filepath = node.getAttr('definition')
            else:
                # Choose None
                if self.checkLeveL(node):
                    filepath = node.getAttr('definition')
                
        return filepath


    def getReferenceXform(self, node):
        xform = None
        wform = None
        if isinstance(node, type('')):
            try:
                node = pm.PyNode(node)
            except:
                pm.warning('Can not figure out what '+str(node)+' is, ignored!')
                return xfrom

        if pm.referenceQuery(node, isNodeReferenced=True):
            g_ctrl = self.getGlobalCtrl(node)
            if g_ctrl:
                xform = pm.xform( g_ctrl, query=True, matrix=True, objectSpace=True)
                wform = pm.xform( g_ctrl, query=True, matrix=True, worldSpace=True)
        elif node.nodeType() == 'assemblyReference':
            xform = pm.xform( node, query=True, matrix=True, objectSpace=True)
            wform = pm.xform( node, query=True, matrix=True, worldSpace=True)

        return xform, wform


    def getReferencePass(self, node):
        look_pass = ''
        if pm.referenceQuery(node, isNodeReferenced=True):
            g_ctrl = self.getGlobalCtrl(node)
            if g_ctrl and g_ctrl.hasAttr('lookPass'):
                look_pass = g_ctrl.getAttr('lookPass', asString=True)

        return look_pass


    def getBoundingBoxFromModelXML(self, node, scene_xml_path):
        '''
        getBoundingBoxFromModelXML(self, node, scene_xml_path)
        get bounding box from model xml file by given node, like 'hi' or 'master'
        '''
        tree = ElementTree.parse(scene_xml_path)
        root = tree.getroot()
        minx = miny = minz = maxx = maxy = maxz = 999999.999
        l_instances = root.getiterator('instance')
        for i in l_instances:
            if i.attrib['name'] == str(node):
                bb = i.getiterator('bounds')
                if bb:
                    maxx = float(bb[0].attrib['maxx'])
                    maxy = float(bb[0].attrib['maxy'])
                    maxz = float(bb[0].attrib['maxz'])
                    minx = float(bb[0].attrib['minx'])
                    miny = float(bb[0].attrib['miny'])
                    minz = float(bb[0].attrib['minz'])
                break
        return pm.dt.BoundingBox([minx, miny, minz],[maxx, maxy, maxz])


    def modv_from_xml(self, asset_xml):
        if not os.path.isfile(asset_xml):
            return ''

        tree = ElementTree.parse(asset_xml)
        root = tree.getroot()

        l_instances = root.getiterator("instance")
        for i in l_instances:
            if i.attrib['name'] == 'master':
                for attr in i.getiterator('attribute'):
                    if attr.attrib['name'] == "modVersion":
                        return attr.attrib['value']
        return ''


    def get_asset_v(self, node):
        d_v = {'modv':'', 'rigv':''}
        if isinstance(node, type('')):
            try:
                node = pm.PyNode(node)
            except:
                pm.warning('Can not figure out what '+str(node)+' is, ignored!')
                return d_v

        if pm.referenceQuery(node, isNodeReferenced=True):
            #maya2scenegraphXML.setArbAttr([n_name], 'rigVersion', str(m.attr('rigVersion').get()) if m.hasAttr('rigVersion') else '', 'string')
            if node.hasAttr('modVersion'):
                d_v['modv'] = str(node.getAttr('modVersion'))
            if node.hasAttr('rigVersion'):
                d_v['rigv'] = str(node.getAttr('rigVersion'))
        elif node.nodeType() == 'assemblyReference':
            l_children = pm.listRelatives(node, c=True)
            if len(l_children) == 0:
                return d_v

            n = l_children[0]
            if n.type() == 'transform':
                if n.hasAttr('modv'):
                    d_v['modv'] = n.getAttr('modv')
                if n.hasAttr('rigv'):
                    d_v['rigv'] = n.getAttr('rigv')
            elif n.type() == 'gpuCache':
                gpu_file = n.getAttr('cacheFileName')
                tokens = gpu_file.split('/')
                if not 'publish' in tokens:
                    return d_v

                i = tokens.index('publish')
                if not len(tokens) > i+1:
                    return d_v

                v_tokens = tokens[i+1].split('.')
                if not (len(v_tokens) == 4 and v_tokens[1] in ['mod', 'rig'] and v_tokens[-1][1:].isdigit()):
                    return d_v
                d_v[v_tokens[1] + 'v'] = v_tokens[-1][1:]

                if v_tokens[1] == 'rig':
                    asset_xml = '/'.join(tokens[:i+2]) + '/scene_graph_xml/' + v_tokens[0] + '.xml'
                    d_v['modv'] = self.modv_from_xml(asset_xml)

        return d_v


    def isReferenceEdited(self, node):
        ref_path = self.getReferencePath(node)
        if '/chr/' in ref_path or '/crd/' in ref_path:
            return True

        '''if pm.referenceQuery(node, isNodeReferenced=True):
            g_ctrl = self.getGlobalCtrl(node)
            if g_ctrl:
                raw_edits = pm.referenceQuery(g_ctrl, topReference=True, editStrings=True, showDagPath=False)
                for e in raw_edits:
                    if g_ctrl.name()+'.translate' in e or g_ctrl.name()+'.rotate' in e or g_ctrl.name()+'.scale' in e:
                        return True'''

        return False


    def isViewableMaster(self, master):
        if isinstance(master, type('')) and pm.objExists(master):
            master = pm.PyNode(master)

        if pm.attributeQuery('lca_viewable', node=master, exists=True):
            return True
        return False

    def find_city_path(self,fullpath):
        fullpath = fullpath.replace('\\', '/')
        tokens = fullpath.split('/')
        if len(tokens) > 2:
            new = '/'.join(tokens[:-1]) +'/'+tokens[-2] +'.xml'
            return new  
        return ''

    def find_version_dir(self, fullpath):
        fullpath = fullpath.replace('\\', '/')
        tokens = fullpath.split('/')
        if 'publish' in tokens:
            i = tokens.index('publish')
            if len(tokens) > i+2:
                return '/'.join(tokens[:i+2])

        return ''


    def findAssetName(self, fullpath):
        fullpath = fullpath.replace('\\', '/')
        tokens = fullpath.split('/')
        if 'publish' in tokens:
            i = tokens.index('publish')
            return tokens[i-2]

        return ''


    def check_ref_visible(self, topNode):
        hi_name = topNode.name().replace(':master', ':hi')
        md_name = topNode.name().replace(':master', ':md')
        lo_name = topNode.name().replace(':master', ':lo')
        if pm.objExists(hi_name):
            res_node = pm.PyNode(hi_name)
            return res_node.isVisible()
        elif pm.objExists(md_name):
            res_node = pm.PyNode(md_name)
            return res_node.isVisible()
        elif pm.objExists(lo_name):
            res_node = pm.PyNode(lo_name)
            return res_node.isVisible()
        else:
            return False


    def AR_visible(self, node):
        vis = node.getAttr('visibility')
        if not vis:
            return False
        for parent in pm.listRelatives(node, parent=True):
            if not self.AR_visible(parent):
                return False
        return True


    def check_AR_visible(self, topNode):
        for trans in pm.listRelatives(topNode, ad=True, type='transform'):
            if trans.name().endswith(':hi') or trans.name().endswith(':md') or trans.name().endswith(':lo'):
                return self.AR_visible(trans)

        for gpu in pm.listRelatives(topNode, ad=True, type='gpuCache'):
            if self.AR_visible(gpu):
                return True        
        return False


    def check_asset_visible(self, asset):
        if asset.nodeType() == 'assemblyReference':
            return self.check_AR_visible(asset)
        else:
            return self.check_ref_visible(asset)


    def find_ignore(self, topNode):
        fullpath = topNode.fullPath()
        maya2scenegraphXML.setArbAttr([fullpath], 'fullPath', fullpath, 'string')

        if self.d_taged_nodes['ref'].has_key(fullpath):
            return

        if self.d_taged_nodes['ignored'].has_key(fullpath):
            return

        find_ref=False
        for ref_dag in self.d_taged_nodes['ref'].keys():
            if ref_dag.startswith(fullpath):
                find_ref=True
                break

        if not find_ref:
            self.d_taged_nodes['ignored'][fullpath] = topNode

        else:
            self.d_taged_nodes['no_bound'][fullpath] = topNode
            for node in pm.listRelatives(topNode, c=True):
                self.find_ignore(node)

        return

    def get_cam_bbox_dist(self, bbox, trans_cam):
        center = ((bbox[0]+bbox[3])/2.0, (bbox[1]+bbox[4])/2.0, (bbox[2]+bbox[5])/2.0)
        ray = (trans_cam[0] - center[0], trans_cam[1] - center[1], trans_cam[2] - center[2])
        l = math.sqrt(ray[0]*ray[0]+ ray[1]*ray[1]+ ray[2]*ray[2])
        
        v0 = abs(ray[0]) *2 /max(0.000001, abs(bbox[3]-bbox[0]))
        v1 = abs(ray[1]) *2 /max(0.000001, abs(bbox[4]-bbox[1]))
        v2 = abs(ray[2]) *2 /max(0.000001, abs(bbox[5]-bbox[2]))

        d = l / max(v0, v1, v2)
        return max(0.0, l-d)


    def cosVector(self, x,y):
        if(len(x)!=len(y)):
            print('error input,x and y is not in the same space')
            return;
            
        result1=0.0;
        result2=0.0;
        result3=0.0;
        for i in range(len(x)):
            result1+=x[i]*y[i]   #sum(X*Y)
            result2+=x[i]**2     #sum(X*X)
            result3+=y[i]**2     #sum(Y*Y)
        #print("result is "+str(result1/((result2*result3)**0.5))) #结果显示
        return result1/((result2*result3)**0.5)


    def get_angle_on_cam(self, n, cam_name):
        cam = pm.PyNode(cam_name)
        n = pm.PyNode(n)

        if n.nodeType() == 'assemblyReference' and n.getActiveLabel() == '':
            bbox = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        else:
            bbox = pm.exactWorldBoundingBox(n, ignoreInvisible=True)
        trans_cam = pm.xform(cam, q=True, translation=True, ws=True)

        v0 = [trans_cam[0] - bbox[0], trans_cam[1] - bbox[1], trans_cam[2] - bbox[2]]
        v1 = [trans_cam[0] - bbox[3], trans_cam[1] - bbox[4], trans_cam[2] - bbox[5]]
        cos = min(1.0, self.cosVector(v0, v1)) # To prevent the precision problem
        try:
            angle = math.acos(cos)/3.14 * 180
        except:         # when cos = -1.0, math domain error happens
            angle = 0.0
        distance = self.get_cam_bbox_dist(bbox, trans_cam)
        return angle, distance


    def get_angle(self, cam, l_frames):
        self.d_angles = {}
        for m in self.l_masters + self.l_unload_ar + self.l_asset_ar:
            self.d_angles[m.fullPath()] = {'angle':[], 'distance':[]}

        for frame in l_frames:
            pm.currentTime(frame)
            for master in self.l_masters:
                if pm.objExists(master.name()[:-7] + ':hi'):
                    a, d = self.get_angle_on_cam(master.name()[:-7] + ':hi', cam)
                else:
                    a, d = self.get_angle_on_cam(master, cam)

                self.d_angles[master.fullPath()]['angle'].append(a)
                self.d_angles[master.fullPath()]['distance'].append(d)

            for ar in self.l_asset_ar + self.l_unload_ar:
                n = ar
                active = pm.assembly(ar, query = True, active = True)
                if active.endswith('.ma'):
                    l_ar_nodes = pm.listRelatives(ar, c=True)
                    for c in l_ar_nodes:
                        if c.name().endswith(':master') and pm.objExists(c.name()[:-7] + ':hi'):
                            n = c.name()[:-7] + ':hi'
                a, d = self.get_angle_on_cam(n, cam)
                self.d_angles[ar.fullPath()]['angle'].append(a)
                self.d_angles[ar.fullPath()]['distance'].append(d)

        return


    def get_cam_angle(self):
        PI = 3.1415926536
        c = pm.PyNode(self.cam)
        a = c.getAttr('horizontalFilmAperture') * 25.4 / 2
        b = c.getAttr('focalLength')
        tan = a/b
        cam_angle = math.atan(tan) / PI * 180 * 2
        return cam_angle

    def isOverrideAR(self, m):
        if not pm.referenceQuery(m,isNodeReferenced=1):return False
        ref = m.referenceFile()
        if not ref:return False
        refNode = ref.refNode
        if not refNode:return False
        if pm.hasAttr(refNode,'AROrign'):
            print pm.getAttr(refNode.name()+'.AROrign')
            return True
        return False

    def getOverrideAR(self,m):
        if not pm.referenceQuery(m,isNodeReferenced=1):return ''
        ref = m.referenceFile()
        if not ref:return ''
        refNode = ref.refNode
        if not refNode:return ''
        if pm.hasAttr(refNode,'AROrign'):
            name= pm.getAttr(refNode.name()+'.AROrign')
            if name and pm.objExists(name):
                node = pm.PyNode(name)
                #print 'fullpath ',node.fullPath()
                return node.fullPath()
        return ''

    def getModifiedName(self,node_name):
        node_name =maya2scenegraphXML.getMayaNodeName(node_name)
        if node_name.endswith(':master'):
            node_name = node_name[:-7]
        elif node_name.endswith('_AR'):
            node_name = node_name[:-3]
        return  node_name.replace(':','.')


    def addTag(self, topNode):
        self.tagObj = tifr.TagsInFrustum()
        masters = self.tagObj.getObjectInFrustum()
        if masters:
            self.tagObj.tagMasters(masters)

        if pm.objExists('|assets|lay'):
            pm.lockNode('|assets|lay', l=False)
            self.d_taged_nodes['ignored']['|assets|lay'] = pm.PyNode('|assets|lay')

        self.l_masters = self.getTailMaster(topNode)
        self.l_asset_ar, self.l_unload_ar, self.l_asb_ar = self.getAssemblyReference(topNode)
        l_unload_ar_path = [n.fullPath() for n in self.l_unload_ar]

        if self.cam:
            self.get_angle(self.cam, self.l_frames)

        if self.skip_unload:
            l_all_assets = self.l_masters + self.l_asset_ar
        else:
            l_all_assets = self.l_masters + self.l_asset_ar + self.l_unload_ar

        for m in l_all_assets:
            ref_path = self.getReferencePath(m)
            asset_name = self.findAssetName( ref_path )
            version_dir = self.find_version_dir( ref_path )
            xml_file = self.osPathConvert( version_dir +'/scene_graph_xml/'+ asset_name + '.xml' )
            if '/cty/' in version_dir:
                city_xml_path = self.find_city_path(ref_path)
                if city_xml_path: 
                    xml_file = city_xml_path

            ire = self.isReferenceEdited(m)
            if not os.path.isfile(xml_file):
                self.d_taged_nodes['ignored'][m.fullPath()] = m
                print '### find ignored', m.fullPath()
                print 'invalid xml', xml_file
                continue

            self.d_taged_nodes['ref'][m.fullPath()] = {'xml':xml_file, 'xml_reform':xml_file.replace('Z:/', '/mnt/proj/'), 'bb':None, 'node':m, 'xform':None, 'wform':None, 'isReferenceEdited':ire, 'loaded':True}
            if m.fullPath() in l_unload_ar_path:
                self.d_taged_nodes['ref'][m.fullPath()]['loaded'] = False

        self.find_ignore(topNode)

        for n_name in sorted(self.d_taged_nodes['ignored'].keys()):
            maya2scenegraphXML.setIgnore([n_name])
            #print 'ignore:', n_name

        for n_name in sorted(self.d_taged_nodes['no_bound'].keys()):
            maya2scenegraphXML.setBoundsWriteMode([n_name], value=False)
            #print 'no bound:', n_name

        for n_name in sorted(self.d_taged_nodes['ref'].keys()):
            m = self.d_taged_nodes['ref'][n_name]['node']

            self.d_taged_nodes['ref'][n_name]['bb'] = self.getBoundingBoxFromModelXML('hi', self.d_taged_nodes['ref'][n_name]['xml'])
            self.d_taged_nodes['ref'][n_name]['xform'], self.d_taged_nodes['ref'][n_name]['wform'] = self.getReferenceXform(m)
            maya2scenegraphXML.setReference([n_name], self.d_taged_nodes['ref'][n_name]['xml_reform'] )

            if self.d_taged_nodes['ref'][n_name]['loaded']:
                d_v = self.get_asset_v(m)
                maya2scenegraphXML.setArbAttr([n_name], 'modVersion', d_v['modv'], 'string')
                maya2scenegraphXML.setArbAttr([n_name], 'rigVersion', d_v['rigv'], 'string')
                maya2scenegraphXML.setArbAttr([n_name], 'lookPass', self.getReferencePass(m), 'string')

            if self.cam and self.d_angles.has_key(n_name):
                maya2scenegraphXML.setArbAttr([n_name], 'angle', str(max(self.d_angles[n_name]['angle'])), 'string')
                maya2scenegraphXML.setArbAttr([n_name], 'distance', str(min(self.d_angles[n_name]['distance'])), 'string')

            if self.isViewableMaster(m):
                maya2scenegraphXML.setArbAttr([n_name], 'viewable', 'yes', 'string')
            elif not self.check_asset_visible(m):
                maya2scenegraphXML.setArbAttr([n_name], 'hidden', 'yes', 'string')

            if self.isOverrideAR(m):
                print '[override]', m
                linked_name = self.getOverrideAR(m)
                if linked_name:
                    maya2scenegraphXML.setArbAttr([n_name], 'aniHidden', 'yes', 'string')
                    val = self.getModifiedName(n_name)
                    maya2scenegraphXML.setArbAttr([linked_name], 'aniOverride',val, 'string')
        
        # if layout or animation moves cache assets in scn, we'll record the <ori></ori>ginal value to each ar node
        # attribute name is 'asb_wform'
        if pm.objExists('|assets|scn'):
            self.compare_asb_trans('|assets|scn')
        
        return

    def compare_asb_trans(self, start_node = '|assets|scn'):
        """
        """
        import lay.utilities.assembly_operations as ao;reload(ao)
        assets = ao.get_assemblies_scene_graph_xml(start_node, 'asb')
        ao.write_different_matrix_to_xml(assets)
        

    def clean_ar(self, l_ar):
        d_ar = {}
        for n_name in l_ar:
            t = len(n_name.split('|')[-1].split(':'))
            if not d_ar.has_key(t):
                d_ar[t] = []
            d_ar[t].append(n_name)

        for t in reversed(sorted(d_ar.keys())):
            for n_name in sorted(d_ar[t]):
                n = pm.PyNode(n_name)
                sgxml_edits = False
                delete_edits = False
                selectedAssembly = editUtils.makeDependNode(n_name)
                curAssembly = selectedAssembly
                assemblies = []
                while not curAssembly.isNull():
                    assemblies.append( curAssembly )
                    assemblyMFn = OpenMaya.MFnAssembly(curAssembly)
                    curAssembly = assemblyMFn.getParentAssembly()

                curAssembly = assemblies[-1]
                edits = PyEditItr(OpenMaya.MItEdits(curAssembly , selectedAssembly, OpenMaya.MItEdits.ALL_EDITS, OpenMaya.MItEdits.kReverse) )
                for edit in edits:
                    edit_str = edit.getString()
                    for key in ['.arbAttr_', ' arbAttr_', '.sgxml_', ' sgxml_']:
                        if key in edit_str:
                            sgxml_edits = True
                            if edit_str.startswith('deleteAttr '):
                                print '  delete attr:', edit_str
                                delete_edits = True

                if sgxml_edits:
                    print '  Delete AR edits for:', n_name.split("|")[-1], len(edits), sgxml_edits, delete_edits
                if delete_edits:
                    print '  Unload AR to delete sg xml attributes.'
                    the_label = n.getActiveLabel()
                    n.setActive('')

                for edit in edits:
                    edit_str = edit.getString()
                    for key in ['.arbAttr_', ' arbAttr_', '.sgxml_', ' sgxml_']:
                        if key in edit_str:
                            edits.removeCurrentEdit()

                if delete_edits:
                    n.setActive(the_label)

        return


    def in_ar_tree(self, n_name):
        tokens = n_name.split('|')
        if len(tokens) > 2 and tokens[2] in ['asb', 'scn']:
            return True
        else:
            return False


    def deleteTag(self):
        self.tagObj.deleteAllTags()

        #maya2scenegraphXML.deleteSgxmlAttrs("|assets")
        l_maya2017_ar = []
        for n_name in self.d_taged_nodes['ignored'].keys() + self.d_taged_nodes['ref'].keys() + self.d_taged_nodes['no_bound'].keys():
            try:
                m = pm.PyNode(n_name)
                if not (MAYA_VERSION in ['2017'] and self.in_ar_tree(n_name)) :
                    attributes = pm.listAttr(m, userDefined=True)
                    for attr in attributes:
                        if 'arbAttr_' in attr or 'sgxml_' in attr:
                            m.deleteAttr(attr)
                else:
                    if m.nodeType() == 'assemblyReference':
                        l_maya2017_ar.append(n_name)
            except:
                print traceback.format_exc()

        if len(l_maya2017_ar) > 0:
            self.clean_ar(l_maya2017_ar)

        return


    def parse_full_path(self, instance):
        full_path = ''
        for c in instance.getchildren():
            if c.tag == 'arbitraryList':
                for attr in c.getiterator('attribute'):
                    if attr.attrib['name'] == 'fullPath':
                        full_path = attr.attrib['value'] 

        return full_path


    def postProcessSgXML(self, scene_xml_path, topNode):
        '''
        modify xml, need input the path to xml file
        '''
        # Reset bounds and xform

        l_asb_full_pathes = [ar.fullPath() for ar in self.l_asb_ar]

        tree = ElementTree.parse(scene_xml_path)
        root = tree.getroot()

        l_instances = root.getiterator("instance")
        for i in l_instances:
            node_name = i.attrib['name']
            full_path = self.parse_full_path(i)

            # Camera Angle
            if node_name == 'assets' and full_path == '|assets':
                c1 = i.getchildren()[0]
                cam_angle = ElementTree.Element( 'cam_angle', {'value':str(self.get_cam_angle())})
                cam_angle.tail = c1.tail
                i.insert(0, cam_angle)

            # Reset bounds and xform
            if self.d_taged_nodes['ref'].has_key(full_path):
                bb = self.d_taged_nodes['ref'][full_path]['bb']
                if bb:
                    i.getiterator('bounds')[0].attrib = {'minx':str(bb.min()[0]), 'miny':str(bb.min()[1]), 'minz':str(bb.min()[2]), 'maxx':str(bb.max()[0]), 'maxy':str(bb.max()[1]), 'maxz':str(bb.max()[2])}

                xform = self.d_taged_nodes['ref'][full_path]['xform']
                if xform:
                    elem_xform = i.getiterator('xform')[0]
                    elem_xform.attrib = {'value': ' '.join([str(j) for j in xform])}
                    wform = self.d_taged_nodes['ref'][full_path]['wform']
                    elem_wform = ElementTree.Element( 'wform', {'value':' '.join([str(j) for j in wform])})
                    elem_wform.tail = elem_xform.tail
                    i.insert(2, elem_wform )

                # we only add isReferenceEdited and isHidden attribute when publishing layout xml
                if self.d_taged_nodes['ref'][full_path]['isReferenceEdited']:
                    i.set('isReferenceEdited', 'yes')
                    #del i.attrib['isReferenceEdited']

            if full_path in l_asb_full_pathes:
                i.attrib['groupType'] = 'assembly'

            # then we cut the tails of name, like :master
            if node_name.endswith(':master'):
                i.attrib['name'] = node_name[:-7]
            elif node_name.endswith('_AR'):
                i.attrib['name'] = node_name[:-3]

            node_name = i.attrib['name']
            i.attrib['name'] = node_name.replace(':','.')

        tree.write(scene_xml_path, "utf-8")

        return


    def exportXml(self, topNode, scene_xml_path, cam=None, l_frames=[1001], skip_unload = False):
        self.cam = cam
        self.l_frames = l_frames
        self.skip_unload = skip_unload
        self.start_time = [time.time()]
        self.export_time = [time.time()]
        if not pm.objExists(topNode):
            print 'Unable find root node, proceed was cancelled.'
            return
        maya2scenegraphXML.setBoundsWriteMode([topNode.fullPath()], value=False)
        self.elapseTime(self.start_time, 'set bounds')
        
        print 'Generating xml data structure...'
        self.addTag( topNode )
        self.elapseTime(self.start_time, 'add tags')

        print 'Added xml tags successfully, now export scene graph xml...'
        maya2scenegraphXML.maya2ScenegraphXML([topNode.name()], scene_xml_path, startFrame=1001, endFrame=1001, arbAttrs=['modVersion','rigVersion', 'lookPass', 'asbVersion', 'fullPath', 'hidden', 'viewable', 'angle', 'distance', 'asb_wform','aniOverride','aniHidden'])
        self.elapseTime(self.start_time, 'export xml')

        print 'Delete tagged attributes'
        self.deleteTag()
        self.elapseTime(self.start_time, 'delete tags')
        
        print 'Post processing xml file...'
        self.postProcessSgXML(scene_xml_path, topNode)
        self.elapseTime(self.start_time, 'post write xml')

        print 'Done!'
        self.elapseTime(self.export_time, 'generate the xml file')
        return

