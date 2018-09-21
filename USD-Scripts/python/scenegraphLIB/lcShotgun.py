# coding=utf-8
"""
    lcShotgun.py
    get and store shot gun info from project and shot name
"""

import sys
import os
import shutil
global ET
#we cannot use lxml in maya2013. use default xmltree
try:
    import maya.cmds as cmds
except:
    pass
    
try:
    from lxml import etree as ET
except:
    import xml.etree.ElementTree as ET

import string
import re
import production.pipeline.lcProdProj as clpp
import traceback
import tempfile
import production.pipeline.utils as cu
import production.pipeline.color_log as ppcl
import logging
import production.pipeline.fastCache as ppfc
reload(ppfc)

if os.environ.has_key('LCTOOLSET'):
    sys.path.append(os.path.join(os.environ['LCTOOLSET'],'lib/production'))
else:
    sys.path.append('/mnt/utility/toolset/lib/production')

global sg
sg=None

def init_shotgun():
    global sg
    if not sg:
        from production.shotgun_connection import Connection
        sg = Connection('get_project_info').get_sg()

def uninit_shotgun():
    if sg:sg.close()

def localMsgOut(msg,level=0):
    prefix=['Info:   ','Warn:   ','Error:   ']
    print (prefix[level]+msg)
    if level==2:
        raise Exception(msg)

def outMsg(msg,level=0):
    try:
        import lcKatanaUtils as lcku
        lcku.katanaPrint(msg,level)
    except:
        localMsgOut(msg,level)

def removeAssetDigtal(asset):
    res=re.search(r"\d*$",asset)
    return asset[:res.start()].lower()

def getRealNamespace(nameList,originalName):
    oldAssetName = removeAssetDigtal( string.split(originalName,'.')[-1] )
    realAssetName=oldAssetName
    i=1
    while True:
        if realAssetName not in nameList:
            nameList.append(realAssetName)
            return realAssetName
        else:
            realAssetName=oldAssetName+str(i)
            i+=1
#-----------------------------------------shotgun functions
def findShotgunProjects(text=False):
    init_shotgun()
    shotgunProj = sg.find('Project',[['sg_status','is','Active']],['name'])
    if text==False:
        return sorted(shotgunProj)
    else:
        result=[]
        for i in range(1,len(shotgunProj)):
            result.append(shotgunProj[i]['name'])
        return sorted(result)

def findShotgunSequence(proj,text=False):
    init_shotgun()
    filter = [
                ['name', 'is', proj]
            ]
    projInfo = sg.find_one('Project', filter)

    seqFilter=[
        ['project', 'is', {'type': 'Project', 'id': projInfo['id']}],
    ]
    sequences=sg.find('Sequence', seqFilter, ['id','code'])
    if text==False:
        return sorted(sequences)
    else:
        result=[]
        for p in sequences:
            result.append(p['code'])
        return sorted(result)

def findShotgunShots(proj=None,seq=None,text=False):
    init_shotgun()
    filter = [
                ['name', 'is', proj]
            ]
    projInfo = sg.find_one('Project', filter)

    shotFilter=[]
    if seq is None:
        shotFilter=[
            ['project', 'is', {'type': 'Project', 'id': projInfo['id']}],
        ]
    else:
        allSeq=findShotgunSequence(proj)
        seqId=None
        for s in allSeq:
            if s['code']==seq:
                seqId=s
        if seqId is not None:
            shotFilter=[
                    ['project', 'is', {'type': 'Project', 'id': projInfo['id']}],
                    ['sg_sequence', 'is', {'type': 'Sequence', 'id': seqId['id']}]
            ]
    shots=sg.find('Shot', shotFilter, ['id','code'])

    if shots is None:
        return None

    if text==False:
        return sorted(shots)
    else:
        result=[]
        for p in shots:
            result.append(p['code'])
        return sorted(result)

def getShotgunPathVersion(path):
    if path is None or path=='':
        return ''

    sep=string.split(path,'/')
    i=0
    for k in sep:
        if k=='publish':
            break
        i+=1

    if (i+1)<len(sep):
        sepp=string.split(sep[i+1],'.')
        if len(sepp)==4:
            return sepp[2]+'.'+sepp[3]

    return ''
#-----------------------------------------shotgun functions


class lcGunBase:
    """
    lcshotgun operation base class
    """
    lcBuildVersion=0.2

    def __init__(self):
        self.logger=ppcl.init_logger(__name__)

        return None

    def sortFolderInfoByCode(self,foldInfo):
        """
        sort the fold path by code
        """
        sortFolder = {}
        for folder in foldInfo:
            sortFolder[folder['code']] = folder

        #sort the key
        sortedkey = []
        for key in sorted(sortFolder.iterkeys()):
            sortedkey.append(key)

        return sortedkey

class lcShotgun(lcGunBase):
    """
    Give me a project and shot, i will collect relevant info for you.
    You can query the sequence, assets, in time, out time .etc
    And make a new xml file from the layout scene graph xml file.
    The new xml file will contains the latest assets and their lookfiles, and project,shot, in and out time info.
    Use lcMakeKatanaScene.py to make a katana scene from the new xml file.

    Example:
    sgInfo=lcShotgun('GTA','a10010')
    sgInfo.initShotgunInfo()
    """
    UNUSED_ATTRS_IN_INSTANCE=['fullPath','hidden','angle','distance','asb_wform']
    def __init__(self, project=None, shot=None):
        lcGunBase.__init__(self)
        if not project:
            return

        init_shotgun()

        self.set(project=project, shot=shot)
        
        self.fast_cache=ppfc.FastCache(projInfo=self.projInfo)
        self.fast_cache.set_shot(shot)

        self.assets_pool={}
        self.log_txt=[]
        
        self.instance_parent_filter=[]
        self.xml_update_type='ani'

    def __del__(self):
        uninit_shotgun()

    def __getLatestVersion(self):

        #sort the key
        sortedkey = lcGunBase.sortFolderInfoByCode(self,self.shotgunFolderInfo)
        self.shotgunFolderkey=[]
        self.shotgunFolderPath={}
        for key in sortedkey:
            splitKey = string.split(key, '.')
            self.shotgunFolderPath[splitKey[1]]=[]

        for key in sortedkey:
            splitKey = string.split(key, '.')
            for folderHandle in self.shotgunFolderInfo:
                if folderHandle['code'] == key and folderHandle['sg_version_folder'] and folderHandle['sg_version_folder'].has_key('local_path'):
                    self.shotgunFolderPath[splitKey[1]].append( folderHandle['sg_version_folder']['local_path'] )

    def __initProj(self):
        #####get project handle
        if self.proj is not None:
            projFilter = [
                ['name', 'is', self.proj]
            ]
            self.shotgunProjInfo = sg.find_one('Project', projFilter)
            if self.shotgunProjInfo is None:
                outMsg( 'lcShotgun : '+ 'Cannot find project ' + self.proj,2)
        else:
            outMsg( 'No project name is given',2)

    def __initShot(self,shot=None):
        if shot is not None:
            self.shot=shot
        if not self.shot:
            return

        outMsg( 'lcShotgun: Init shotgun info for Project: '+ self.proj+ '    Shot: '+self.shot)

        #####get shot handle
        shotFilter = [
            ['code', 'is', self.shot],
            ['project', 'is', {'type': 'Project', 'id': self.shotgunProjInfo['id']}]
        ]
        self.shotgunShotInfo = sg.find_one('Shot', shotFilter, ['sg_sequence', 'sg_cut_in', 'sg_cut_out', 'sg_ani_cut_in', 'sg_ani_cut_out'])
        if self.shotgunShotInfo is None:
            outMsg('lcShotgun : '+'Cannot find shot ' +self.proj+' ' +self.shot,2)

        #####get shot folder handle
        folderInfoFilter = [
            ['entity', 'is', self.shotgunShotInfo],
            ['project', 'is', {'type': 'Project', 'id': self.shotgunProjInfo['id']}]
        ]
        self.shotgunFolderInfo = sg.find('Version', folderInfoFilter, ['code', 'sg_version_folder'])
        # print '------',self.shotgunFolderInfo
        if self.shotgunFolderInfo is None:
            outMsg('lcShotgun : '+ 'Cannot find version folders for ' + self.shot,2)

        self.__getLatestVersion()

    #---------------------------------------xml remake
    @staticmethod
    def get_node_arbitrary_attrs(node):
        arbitrary_attr = {}
        for rr in node.getiterator('attribute'):
            if rr.attrib.get('name',''):
                arbitrary_attr[rr.attrib['name']]= rr.attrib.get('value', '')
        return arbitrary_attr

    @staticmethod
    def get_asset_xml_arbitrary_attrs(file):
        tree = ET.parse(file)
        root = tree.getroot()
        arbitrary_attr = None
        for r in root.getiterator('arbitraryList'):
            arbitrary_attr = lcShotgun.get_node_arbitrary_attrs(r)
        return arbitrary_attr

    @staticmethod
    def add_mod_level_info(node):
        ref_file=node.attrib.get('refFile','')
        abc_info=clpp.lcProdProj.get_cache_asset(ref_file)
        if abc_info:
            level_info=abc_info.keys()
            if len(level_info)>1:
                for r in node.getiterator('arbitraryList'):

                    #add modLevel
                    attr_node=None
                    current_mod_level='md'
                    for rr in r.getiterator('attribute'):
                        if rr.attrib.get('name', '') == 'modLevel':
                            attr_node=rr
                        if rr.attrib.get('name','')=='fullPath' and 'plt_proxy_grp' in  rr.attrib.get('value',''):
                            current_mod_level='lo'
                            

                    if not attr_node:
                        attr_node=ET.SubElement(r,'attribute')
                        attr_node.set('name','modLevel')
                        attr_node.set('type','string')
                    
                    attr_node.set('value',','.join(level_info))

                    #add currentModLevel
                    curretn_modlevel_node=None
                    for rr in r.getiterator('attribute'):
                        if rr.attrib.get('name', '') == 'currentModLevel':
                            curretn_modlevel_node=rr

                    if not curretn_modlevel_node:
                        curretn_modlevel_node=ET.SubElement(r,'attribute')
                        curretn_modlevel_node.set('name','currentModLevel')
                        curretn_modlevel_node.set('type','string')
                    
                    curretn_modlevel_node.set('value',current_mod_level if current_mod_level in level_info else level_info[0])
                    
    @staticmethod
    def set_node_arbitrary(node,arbit):
        if arbit:
            for r in node.getiterator('arbitraryList'):
               for rr in r.getiterator('attribute'):
                   if rr.attrib.get('name', '') == 'modVersion':
                       rr.set('value', arbit.get('modVersion',''))
                   if rr.attrib.get('name', '') == 'rigVersion':
                       rr.set('value', arbit.get('rigVersion',''))

    def __set_cache_attributes(self,node,cache_asset):
        if cache_asset.get('isCache',False) or cache_asset.get('isTransCache',False):
            cache_status=self.projInfo.getAniAssetStatus(self.shot,node.attrib['name'])
            lcShotgun.xmlSetXformDefault(node,
                                                            cache_status,
                                                            cache_asset.get('isCache',False))
            
        if cache_asset.get('isCache',False):
            lcShotgun.xmlRemoveNodeBounds(node)
            self.assets_pool[node.attrib['name']]= cache_asset.get('data','')

    def __set_refFile_path(self,node,cache_asset):
        newAssetPath =cache_asset.get('data','')

        #set 'name' to asset namespace,
        #set 'refFile' to ani asset or srf asset
        if newAssetPath:
            node.attrib['refFile']=str(newAssetPath)
            node.attrib['type']='reference'
            if '/mod/publish/' in node.attrib['refFile']:
                lcShotgun.add_mod_level_info(node)

            a = lcShotgun.get_asset_xml_arbitrary_attrs(newAssetPath)
            lcShotgun.set_node_arbitrary(node,a)

    #replace the node refFile with srf refFile.
    def __xmlResetAsset_elementTree_Srf(self,node):
        # lcShotgun.xmlRemoveNodeBounds(node)
        srfAssetPath=None
        srfSearch=self.fast_cache.get_fast_srf_data(node)
        realAssetName = removeAssetDigtal(string.split(node.attrib['name'], '.')[-1])

        # search in 'srf'
        if srfSearch  and srfSearch.get('xml') :
            srfAssetPath=srfSearch['xml']
        else:
            msg='Can not find srf xml file for '+realAssetName
            if srfAssetPath is None:
                srfAssetPath=self.projInfo.getModAsset(realAssetName)
                if srfAssetPath is not None:
                    msg+=',   Use mod xml file '+getShotgunPathVersion(srfAssetPath)
                else:
                    msg+=',   Use asset in layout xml.'

            outMsg(msg,1)

        if srfAssetPath :
            node.attrib['refFile']=srfAssetPath
            node.attrib['type']='reference'

            a = lcShotgun.get_asset_xml_arbitrary_attrs(srfAssetPath)
            lcShotgun.set_node_arbitrary(node,a)
        # else:
        #     node.attrib['type']='group'
            
    def __xmlResetAsset_elementTree(self,node):
        mod_type=re.findall('/projects/[a-z]{3}/asset/([a-z]{3})/', node.attrib.get('refFile',''))
        if mod_type:
            lcShotgun.set_arbitrary_attrs(node, {'modelType':mod_type[0]})

        # get cache data
        cache_asset=self.fast_cache.get_fast_cache_data(node,ani_type=self.xml_update_type)
        if not cache_asset:
            outMsg( 'Cannot find asset data'+node.attrib['name'],1)
            return
        
        self.__set_cache_attributes(node,cache_asset)

        self.__set_refFile_path(node, cache_asset)

    @staticmethod
    def rollbackInstance(node,new_name,old_name):
        its=node.findall(old_name)
        if its:
            key_value={}
            for it in its:
                for k,v in it.attrib.items():
                    key_value[k]=v
            
            its = node.findall(new_name)
            new_node=None
            if its:
                for it in its:
                    new_node=it
            else:
                new_node = ET.SubElement(node,new_name)
            
            for k,v in key_value.items():
                new_node.set(k,v)
            
            return new_node

    @staticmethod
    def storeOldInstanceValue(node,name,key_value):
        its=node.findall(name)
        if not its:
            new_ele = ET.SubElement(node,name)
            for k,v in key_value.items():
                new_ele.set(k,v)
            
            return new_ele

    @staticmethod
    def xmlRemoveNodeBounds(node):
        its=node.findall('bounds')
        bounds_old={}
        for t in its:
            for k,v in t.attrib.items():
                bounds_old[k]=v
            node.remove(t)
        lcShotgun.storeOldInstanceValue(node,'bounds_old',bounds_old)

    @staticmethod
    def xmlSetXformDefault(node,cache_status='',is_cache=True):
        its=node.findall('xform')
        xform_old={}
        for it in its:
            xform_old['value']=it.attrib['value']
            it.attrib['value']= '1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1'
        
        lcShotgun.storeOldInstanceValue(node,'xform_old',xform_old)

        lcShotgun.set_is_cache(node,is_cache,status=cache_status)

    @staticmethod
    def set_arbitrary_attrs(node,attr_dict):
        arb_list = node.find('arbitraryList')
        if arb_list is None:
            arb_list = ET.SubElement(node,'arbitraryList')

        arb_attr = arb_list.findall('attribute')
        attr_nodes={}
        for a in arb_attr:
            attr_nodes[a.attrib.get('name')]=a

        for k,v in attr_dict.items():
            if k in attr_nodes.keys():
                attr_nodes[k].set(k,v)
            else:
                cache_node=ET.SubElement(arb_list,'attribute')
                cache_node.set('name',k)
                cache_node.set('type','string')
                cache_node.set('value',v)

    @staticmethod
    def set_is_cache(node,is_cache,status=''):
        if not is_cache:
            lcShotgun.add_mod_level_info(node)

        lcShotgun.set_arbitrary_attrs(node, {'isCache':['no','yes'][is_cache]})
        if status: lcShotgun.set_arbitrary_attrs(node, {'cacheStatus':status})

    #---------------------------------------xml remake

    def set(self, project=None, shot=None):
        if project is None :
            return
        self.proj = project
        self.projInfo=clpp.lcProdProj()
        self.projInfo.setProj(self.proj,context=shot)

        #in shot mode
        self.shot = shot

        #shotgun data
        self.shotgunProjInfo = None
        self.shotgunShotInfo = None
        self.shotgunFolderInfo = None

        #shot path
        self.shotgunFolderkey = None
        #list of path{'lay':'...','ani':'...'}
        self.shotgunFolderPath = None

        self.assetTableList=None


        self.lay_xml=None

    def initShotgunInfo(self):
        self.__initProj()
        self.__initShot()
        
    def getAniCutIn(self):
        if self.shotgunShotInfo is not None:
            return self.shotgunShotInfo['sg_ani_cut_in']
    
    def getAniCutOut(self):
        if self.shotgunShotInfo is not None:
            return self.shotgunShotInfo['sg_ani_cut_out']

    #get cut in time
    def getTimeIn(self):
        if self.shotgunShotInfo is not None:
            cut_in=self.shotgunShotInfo['sg_cut_in']
            return cut_in

    #get cut out time
    def getTimeOut(self):
        if self.shotgunShotInfo is not None:
            cut_out=self.shotgunShotInfo['sg_cut_out']
            return cut_out

    #get cut in time
    def getCutIn(self):
        if self.shotgunShotInfo is not None:
            cut_in=self.shotgunShotInfo['sg_cut_in']
            return cut_in

    #get cut out time
    def getCutOut(self):
        if self.shotgunShotInfo is not None:
            cut_out=self.shotgunShotInfo['sg_cut_out']
            return cut_out

    def  __is_instance_under_group(self,all_attributes):
        for at in all_attributes:
            if at.attrib.get('name') =='fullPath':
                full_p=at.attrib.get('value')
                if full_p:
                    for c in self.instance_parent_filter:
                        if c in full_p:return True
                break

        return False

    def instance_xml(self,inst_num,exclude=[],exclude_reverse=False):
        tree = ET.parse(self.lay_xml[0])
        root = tree.getroot()

        self.fast_cache.asset_mod_xml_node={}
        for r in root.getiterator('instance'):
            if r.attrib.get('type', '')=='reference':
                self.fast_cache.get_fast_cache_data(r)
                
        instance_source_xml_node = self.__add_instance_source(root)
        for k,v in self.fast_cache.asset_mod_xml_node.items():
            if len(v)<inst_num :
                continue
            if not exclude_reverse and k in exclude:
                continue
            if exclude_reverse and k not in exclude:
                continue

            mod_type=self.projInfo.getAssetType(k)
        
            ast_count=0
            for vv in v:
                arb_list = vv.find('arbitraryList')
                attr_set=arb_list.findall('attribute')
                if self.instance_parent_filter and not self.__is_instance_under_group(attr_set):
                    continue

                vv.attrib['groupType']='instance'
                vv.attrib['type']='group'
                del vv.attrib['refFile']
                del vv.attrib['refType']

                for at in attr_set:
                    if at.attrib.get('name') in ['modVersion','rigVersion']:
                        arb_list.remove(at)

                lcShotgun.set_arbitrary_attrs(vv, {'autoInstance':'yes'})
                lcShotgun.set_arbitrary_attrs(vv, {'modelType':mod_type})

                ast_count+=1
    
            if ast_count:            
                self.__add_assets_to_scr(instance_source_xml_node,\
                                         k,\
                                         v[0])

        print 'Build Shot : Mark instance assets...'
        cu.indent_xml(root,0)
        tree.write(self.lay_xml[0])

    def __add_assets_to_scr(self,parent_node,asset_name,old_asset_node):
        asset_node=ET.SubElement(parent_node,'instance')
        asset_node.set('name',asset_name)
        
        old_ref_file=old_asset_node.attrib.get('refFile','')
        asset_node.set('refType','xml')
        asset_node.set('type','reference')
        asset_node.set('refFile',old_ref_file)

        asset_node.set('instsrc','yes')

        arblist_node=ET.SubElement(asset_node,'arbitraryList')

        attrs_nodes =old_asset_node.getiterator('attribute')
        for an in attrs_nodes:
            if an.attrib.get('name','') not in lcShotgun.UNUSED_ATTRS_IN_INSTANCE:
                arb_attr = ET.SubElement(arblist_node,'attribute')
                arb_attr.set('name',an.attrib.get('name',''))
                arb_attr.set('type',an.attrib.get('type',''))
                arb_attr.set('value',an.attrib.get('value',''))

    def __add_instance_source(self,root):
        inst=root.getiterator('instance')
        for ins in inst:
            if ins.attrib.get('name', '')=='assets' and \
                ins.attrib.get('type', '')=='group':
                asset_inst_list = ins.find('instanceList')
                inst_source_node=ET.SubElement(asset_inst_list,'instance')
                inst_source_node.set('name','inst_src')
                inst_source_node.set('type','group')

                xform_node=ET.SubElement(inst_source_node,'xform')
                xform_node.set('value','1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0')

                inst_source_list_node=ET.SubElement(inst_source_node,'instanceList')
                return inst_source_list_node
    
    @staticmethod
    def get_parent_path(node):
        location=''
        while node is not None:
            name = node.attrib.get('name')
            if name and node.tag=='instance':
                location=name+'/'+location
            node = node.getparent()
        return '/'+location

    def get_lay_xml(self,custom_xml=None,filter_remove=None,clear_assets=False):
        if custom_xml and os.path.isfile(custom_xml):
            temp_xml=tempfile.mktemp()+ '.tmp.xml'
            shutil.copy(custom_xml,temp_xml)
            self.lay_xml=[temp_xml,'customLayXml']
        else:
            self.lay_xml = self.projInfo.lcGetLayXml(self.shot,clear_assets=clear_assets)
            if not self.lay_xml:
                outMsg('Can not find layout xml file.',2)

        if filter_remove and self.lay_xml[0] and os.path.isfile(self.lay_xml[0]):
            tree = ET.parse(self.lay_xml[0])
            root = tree.getroot()
            all_list=root.getiterator('instanceList')

            for l in all_list:
                local_instance=l.findall('instance')
                for li in local_instance:
                    if li.attrib.get('type','')=='reference' and \
                       li.attrib.get('refType','')=='xml':
                        for fs in filter_remove:
                            if re.search(fs,li.attrib.get('name','')):
                                l.remove(li)

            base_name=os.path.basename(self.lay_xml[0]).split('.')[0]
            temp_d=tempfile.gettempdir()
            new_xml_path=os.path.join(temp_d,base_name+'_filter.xml')
            tree.write(new_xml_path)
            self.lay_xml=[new_xml_path,self.lay_xml[1]]
            print 'Filtered lay xml',self.lay_xml
    
        if self.lay_xml:
            temp_xml=tempfile.mktemp()+'.'+self.shot+'.'+self.proj+'.srf_assets_collector.xml'
            clpp.lcProdProj.collect_assets_from_xml(self.lay_xml[0],temp_xml)
            self.logger.info('Collect assets into srf xml :'+temp_xml)
            self.lay_xml.append(temp_xml)

    def __update_root_attrib(self,root):
        root.attrib['proj']=self.proj
        root.attrib['shot']=self.shot

        root.attrib['layStartFrame']=str(self.getCutIn())
        root.attrib['layEndFrame']=str(self.getCutOut())

        root.attrib['startFrame']=str(self.getTimeIn())
        root.attrib['endFrame']=str(self.getTimeOut())

        root.attrib['sourceXml']=self.lay_xml[1]

        root.attrib['xmlType']=self.xml_update_type

        root.attrib['buildVersion']=str(lcGunBase.lcBuildVersion)

    def __get_working_xml(self):

        current_xml=self.lay_xml[0]
        
        if self.xml_update_type=='srf' and len(self.lay_xml)==3:
            current_xml=self.lay_xml[2]
        
        return current_xml

    def __replace_assets(self,root):
        for r in root.getiterator('instance'):
            realName = r.attrib.get('name','').split('.')[-1]

            if r.attrib.get('type', '')=='reference':
                if self.xml_update_type in ['ani','transform']:
                    self.__xmlResetAsset_elementTree(r)
                elif self.xml_update_type == 'srf':
                    self.__xmlResetAsset_elementTree_Srf(r)
                
                if '.' in r.attrib.get('name',''):    
                    r.set('namespace',str(r.attrib['name']))
                    r.set('name', str(realName))
                
                refFile=r.attrib.get('refFile','')
                mod_type=re.findall('/projects/[a-z]{3}/asset/([a-z]{3})/', refFile)
                if mod_type: r.set('modelType',mod_type[0])

            if r.attrib.get('groupType')=='instance':
            # if '.' in r.attrib.get('name',''):
                r.set('namespace',str(r.attrib['name']))
                r.set('name', str(realName))
        
        print 'Build Shot : End replace assets in',self.xml_update_type,'part'

    def __save_xml_file(self,tree,newPath):
        if newPath:
            newPath = os.path.join(newPath,self.shot+'_'+self.xml_update_type+'_sg.xml')
            try:
                tree.write(newPath)
                print 'Build Shot : Write',self.xml_update_type,'xml to',newPath
                return newPath
            except:
                raise Exception('Cannot write xml '+newPath+ ' Error'+str(traceback.format_exc()))
                # raise Exception('Cannot write xml '+newPath)
        else:
            return None

    #-------------------------------------------------
    def xmlRemake_elementTree(self,newPath=None,fileType='ani'):
        self.xml_update_type=fileType

        working_xml=self.__get_working_xml()

        tree = ET.parse(working_xml)
        root = tree.getroot()

        self.__update_root_attrib(root)

        if working_xml!=self.lay_xml[0]:
            root.attrib['tinySrf']='yes'

        self.__replace_assets(root)

        return self.__save_xml_file(tree,newPath)

    def __save_xml_file2(self,tree,newPath):
        if newPath:
            # newPath = os.path.join(newPath,self.shot+'_'+self.xml_update_type+'_sg.xml')
            try:
                tree.write(newPath)
                print 'Build Shot : Write',self.xml_update_type,'xml to',newPath
                return newPath
            except:
                raise Exception('Cannot write xml '+newPath+ ' Error'+str(traceback.format_exc()))
                # raise Exception('Cannot write xml '+newPath)
        else:
            return None

    #-------------------------------------------------
    def xmlRemake_elementTree2(self,input_path, newPath,fileType='ani'):
        self.xml_update_type=fileType

        working_xml=self.__get_working_xml()
        working_xml = input_path

        tree = ET.parse(working_xml)
        root = tree.getroot()

        self.__update_root_attrib(root)

        if working_xml!=self.lay_xml[0]:
            root.attrib['tinySrf']='yes'

        self.__replace_assets(root)

        return self.__save_xml_file2(tree,newPath)
