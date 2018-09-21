__author__ = 'lvyuedong'

import os
import pymel.core as pm
import traceback

class TagsInFrustum():
    def __init__(self):
        self.isPluginLoaded = self.loadPlugin()
        self.attribute = 'lca_viewable'
        self.masters_all = None

    def loadPlugin(self):
        try:
            pm.loadPlugin('frustumSelection', quiet=True)
            return True
        except:
            print "Unable to load frustumSelection plugin!"
            return False

    def getCamera(self):
        cam_name = os.path.basename( pm.sceneName() ).split('.')[0]

        if pm.objExists(cam_name+'_cam'):
            # eg. f76060_cam
            tmp = pm.PyNode(cam_name+'_cam')
            try:
                if type(tmp.getShape()) == pm.nt.Camera:
                    return [tmp]
                else:
                    pm.warning(cam_name+'_cam is not a valid camera!')
                    return []
            except:
                pm.warning('failed to find camera: '+ cam_name+'_cam')
                return []
        elif pm.objExists(cam_name):
            # or f76060
            tmp = pm.PyNode(cam_name)
            try:
                if type(tmp.getShape()) == pm.nt.Camera:
                    return [tmp]
                else:
                    pm.warning(cam_name+' is not a valid camera!')
                    return []
            except:
                pm.warning('failed to find camera: '+ cam_name)
                return []
        else:
            pm.warning('failed to find camera: '+cam_name+'_cam')
            return []
        return []

    def getCameraFromSelection(self):
        tmp = []
        for s in pm.ls(sl=True, dag=True, s=True):
            if type(s) == pm.nt.Camera:
                tmp.append( s.getParent() )
        if not tmp:
            pm.warning('please select camera(s)!')
            return []
        return tmp

    def filterObjects(self, obj):
        # convert obj to master
        masters = []
        if not obj:                 # if no obj in frustum, return an empty list
            return masters
        
        for o in obj:
            if pm.referenceQuery(o, inr=True):
                try:
                    ns = pm.referenceQuery(o, ns=True)
                except:
                    print 'can not find namespace for object: ' + o
                    continue
                if pm.objExists(ns+':master') and pm.objExists(ns+':poly'):
                    masters.append(ns+':master')

            else:
                if pm.nodeType(o) == 'assemblyReference':
                    masters.append(o)
                else:
                    tokens = o.split(':')
                    master_name = ':'.join(tokens[:-1])+':master'
                    if pm.objExists(master_name):
                        m_parent = pm.listRelatives(master_name, parent=True)
                        if len(m_parent) > 0 and m_parent[0].nodeType() == 'assemblyReference':
                            masters.append(m_parent[0].name())

        return [pm.PyNode(m) for m in list(set(masters))]

    def getObjectInFrustumShell(self, autoFunc, widthFunc, heightFunc, modeFunc, offsetFunc, invertFunc, startFunc, endFunc):
        frustum = self.getObjectInFrustum( autoGetCamera=autoFunc(), width=widthFunc(), height=heightFunc(), mode=modeFunc(), offset=offsetFunc(), invert=invertFunc(), start=startFunc(), end=endFunc() )
        pm.select(frustum, r=True)

    def getObjectInFrustum(self, autoGetCamera=True, width=2048, height=858, mode=1, offset=1, invert=0, start=1001, end=1100):
        # load frustum selection plugin
        if not self.isPluginLoaded:
            if not self.loadPlugin():
                return None

        # get camera
        if autoGetCamera:
            cameras = self.getCamera()
        else:
            cameras = self.getCameraFromSelection()
        if not cameras:
            pm.warning('Failed to find camera to proceed!')
            return None

        # cal overscan
        overscan = 1.0
        if offset>0:
            overscan = overscan + offset/100.0
        elif offset<0:
            overscan = overscan * abs(offset)

        # save original overscan and set to ours
        overscan_org = {}
        for c in cameras:
            overscan_org[c] = c.attr('overscan').get()
            try:
                c.attr('overscan').set( overscan )
            except:
                print 'Camera', c.name(), 'is locked. Skip.'

        # select cameras
        previous_sel = pm.ls(sl=True)
        pm.select(cameras, r=True)

        # get objects in frustum
        # be careful with returned type of frustumSelection command, it's a list of string, not object
        frustum = []
        if mode == 1:
            # time slider
            try:
                frustum = pm.mel.frustumSelection(vw=width, vh=height, uts=True, i=invert)
            except:
                traceback.format_exc()
        elif mode == 2:
            # start and end frame mode
            try:
                frustum = pm.mel.frustumSelection(vw=width, vh=height, sf=start, ef=end, i=invert)
            except:
                traceback.format_exc()
        elif mode == 3:
            # current time mode
            try:
                frustum = pm.mel.frustumSelection(vw=width, vh=height, i=invert)
            except:
                traceback.format_exc()

        frustum = self.filterObjects(frustum)

        # restore overscan
        for c in cameras:
            try:
                c.attr('overscan').set( overscan_org[c] )
            except:
                print 'Camera', c.name(), 'is locked. Skip.'

        # restore selection
        pm.select(previous_sel, r=True)

        return frustum


    def findAllAssets(self, top='|assets', ns_level='*:master'):
        masters = []
        if not pm.objExists(top):
            return masters

        masters_raw = [m for m in pm.ls(ns_level, rn=True) if m.isReferenced() and m.isChildOf(top)]
        if masters_raw:
            masters = [m for m in masters_raw if pm.objExists(m.name().replace(':master',':poly'))]
            masters.extend( self.findAllAssets(top, '*:'+ns_level) )

        # find assembly reference nodes
        l_ar = pm.listRelatives(top, ad=True, type='assemblyReference')
        l_asset_ar = []
        l_asb_ar = []
        for ar in l_ar:
            asset_ar = True
            l_nodes = pm.container(ar, q=True, nodeList=True)
            for n in l_nodes:
                if n.nodeType() == 'assemblyReference':
                    asset_ar = False

            if asset_ar:
                masters.append(ar)

        return masters


    def tagMasters(self, masters=[]):
        self.deleteAllTags()
        for m in masters:
            pm.addAttr(m, longName=self.attribute, at='bool', defaultValue=1)
        return


    def deleteAllTags(self):
        if self.masters_all is None:
            self.masters_all = self.findAllAssets()

        for m in self.masters_all:
            if pm.attributeQuery(self.attribute, node=m, exists=True):
                # delete visible tag
                pm.deleteAttr(m, attribute=self.attribute)
        return


    def window(self):
        win = pm.window(title='Frustum Selection', width=400, s=True, resizeToFitChildren=True)
        pm.columnLayout(adj=True)
        radio = pm.radioButtonGrp(nrb=3, l1='Use Time Slider', l2='Start/End', l3='Current Time', sl=1)
        frameRange = pm.intFieldGrp(nf=2, l='Frame Range: Start', cw1=100, v1=1001, el='End', v2=1100, en=False)
        radio.onCommand2(pm.Callback(frameRange.setEnable, True))
        radio.offCommand2(pm.Callback(frameRange.setEnable, False))
        screen_size = pm.intFieldGrp(nf=2, l='Screen Size', cw1=100, v1=2048, el='Height', v2=858)
        offset = pm.floatFieldGrp(nf=1, l='Selection Offset', cw1=100, el='%', v1=0)
        invert = pm.checkBoxGrp(ncb=1, l='Invert Selection', cw1=100, v1=0)
        auto = pm.checkBoxGrp(ncb=1, l='Auto Get Camera', cw1=100, v1=1)
        pm.text(label='Help: it is possible to select multiple cameras.\nfilmTranslate attribute on camera is NOT in effect on selecting, use filmOffset instead.\nPositive offset shrink selection while negative expanding.', align='left', fn='boldLabelFont', recomputeSize=True, wordWrap=True)
        #autoFunc, widthFunc, heightFunc, modeFunc, offsetFunc, invertFunc, startFunc, endFunc):
        pm.button(l='Select', command=pm.Callback(self.getObjectInFrustumShell, auto.getValue1, screen_size.getValue1, screen_size.getValue2, radio.getSelect, offset.getValue1, invert.getValue1, frameRange.getValue1, frameRange.getValue2))
        pm.button(l='Tag', command=pm.Callback(self.tagMasters))
        pm.button(l='Delete All Tags', command=pm.Callback(self.deleteAllTags))
        pm.separator(style='in', h=10)
        pm.button( label='Close', command= pm.Callback( pm.deleteUI, win.name() ) )
        pm.showWindow(win.name())
