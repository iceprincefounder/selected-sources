
def registerSpeedTreeIn():
    """
    Registers a new SpeedTreeIn node type using the NodeTypeBuilder utility
    class.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute, FnGeolibServices

    def buildSpeedTreeInOpChain(node, interface):
        """
        Defines the callback function used to create the Ops chain for the
        node type being registered.

        @type node: C{Nodes3DAPI.NodeTypeBuilder.SpeedTreeIn}
        @type interface: C{Nodes3DAPI.NodeTypeBuilder.BuildChainInterface}
        @param node: The node for which to define the Ops chain
        @param interface: The interface providing the functions needed to set
            up the Ops chain for the given node.
        """
        # Get the current frame time
        frameTime = interface.getGraphState().getTime()

        # Set the minimum number of input ports
        interface.setMinRequiredInputs(0)

        argsGb = FnAttribute.GroupBuilder()

        # Parse node parameters
        locationParam = node.getParameter('location')
        argsGb.set("location",FnAttribute.StringAttribute(locationParam.getValue(frameTime)))

        srtFileParam = node.getParameter( "srtFile" )
        argsGb.set("srtFile",FnAttribute.StringAttribute(srtFileParam.getValue(frameTime)))

        globalMotionParam = node.getParameter('useGlobalMotion')
        argsGb.set("useGlobalMotion",FnAttribute.IntAttribute(globalMotionParam.getValue(frameTime)))

        currentFrameParam = node.getParameter('currentFrame')
        argsGb.set("currentFrame",FnAttribute.FloatAttribute(currentFrameParam.getValue(frameTime)))

        globalMotionParam = node.getParameter('enableMotionBlur')
        argsGb.set("enableMotionBlur",FnAttribute.IntAttribute(globalMotionParam.getValue(frameTime)))

        motionSamplesParam = node.getParameter('motionSamples')
        motionSamples = []
        for ci in range(0, motionSamplesParam.getNumChildren()):
            motionSamples = motionSamples + [ motionSamplesParam.getChildByIndex(ci).getValue(frameTime) ]
        if motionSamples is not None and len(motionSamples) > 0:
            opMotionSamplesParam = FnAttribute.FloatAttribute(motionSamples)
            argsGb.set( "motionSamples", opMotionSamplesParam )

        fpsParam = node.getParameter('fps')
        argsGb.set("fps",FnAttribute.FloatAttribute(fpsParam.getValue(frameTime)))

        globalFrequencyParam = node.getParameter('globalFrequency')
        argsGb.set("globalFrequency",FnAttribute.FloatAttribute(globalFrequencyParam.getValue(frameTime)))

        gustFrequencyParam = node.getParameter('gustFrequency')
        argsGb.set("gustFrequency",FnAttribute.FloatAttribute(gustFrequencyParam.getValue(frameTime)))

        windSpeedParam = node.getParameter('windSpeed')
        argsGb.set("windSpeed",FnAttribute.FloatAttribute(windSpeedParam.getValue(frameTime)))

        windDirectionGb = FnAttribute.GroupBuilder()
        windDirectionGb.set(
            'x',
            FnAttribute.FloatAttribute(
                node.getParameter('windDirection.x').getValue(frameTime)))
        windDirectionGb.set(
            'y',
            FnAttribute.FloatAttribute(
                node.getParameter('windDirection.y').getValue(frameTime)))
        windDirectionGb.set(
            'z',
            FnAttribute.FloatAttribute(
                node.getParameter('windDirection.z').getValue(frameTime)))
        argsGb.set( "windDirection", windDirectionGb.build())
        
        windTypeParam = node.getParameter('windType')
        argsGb.set("windType",FnAttribute.IntAttribute(windTypeParam.getValue(frameTime)))

        LODTypeParam = node.getParameter('LODType')
        argsGb.set("LODType",FnAttribute.IntAttribute(LODTypeParam.getValue(frameTime)))

        LODSmoothTypeParam = node.getParameter('LODSmoothType')
        argsGb.set("LODSmoothType",FnAttribute.IntAttribute(LODSmoothTypeParam.getValue(frameTime)))

        speedKeyFrameParam = node.getParameter('speedKeyFrame')
        speedKeyFrames = []
        for ci in range(0, speedKeyFrameParam.getNumChildren()):
            speedKeyFrames = speedKeyFrames + [ speedKeyFrameParam.getChildByIndex(ci).getValue(frameTime) ]
        if speedKeyFrames is not None and len(speedKeyFrames) > 0:
            opSpeedKeyFrameParam = FnAttribute.FloatAttribute(speedKeyFrames)
            argsGb.set( "speedKeyFrame", opSpeedKeyFrameParam )

        speedResponseTimeParam = node.getParameter('speedResponseTime')
        speedResponseTime = []
        for ci in range(0, speedResponseTimeParam.getNumChildren()):
            speedResponseTime = speedResponseTime + [ speedResponseTimeParam.getChildByIndex(ci).getValue(frameTime) ]
        if speedResponseTime is not None and len(speedResponseTime) > 0:
            opSpeedResponseTimeParam = FnAttribute.FloatAttribute(speedResponseTime)
            argsGb.set( "speedResponseTime", opSpeedResponseTimeParam )

        speedKeyValueParam = node.getParameter('speedKeyValue')
        speedKeyValue = []
        for ci in range(0, speedKeyValueParam.getNumChildren()):
            speedKeyValue = speedKeyValue + [ speedKeyValueParam.getChildByIndex(ci).getValue(frameTime) ]
        if speedKeyValue is not None and len(speedKeyValue) > 0:
            opSpeedKeyValueParam = FnAttribute.FloatAttribute(speedKeyValue)
            argsGb.set( "speedKeyValue", opSpeedKeyValueParam )

        direResponseTimeParam = node.getParameter('direResponseTime')
        direResponseTime = []
        for ci in range(0, direResponseTimeParam.getNumChildren()):
            direResponseTime = direResponseTime + [ direResponseTimeParam.getChildByIndex(ci).getValue(frameTime) ]
        if direResponseTime is not None and len(direResponseTime) > 0:
            opDireResponseTimeParam = FnAttribute.FloatAttribute(direResponseTime)
            argsGb.set( "direResponseTime", opDireResponseTimeParam )

        direKeyFrameParam = node.getParameter('direKeyFrame')
        direKeyFrame = []
        for ci in range(0, direKeyFrameParam.getNumChildren()):
            direKeyFrame = direKeyFrame + [ direKeyFrameParam.getChildByIndex(ci).getValue(frameTime) ]
        if direKeyFrame is not None and len(direKeyFrame) > 0:
            opDireKeyFrameParam = FnAttribute.FloatAttribute(direKeyFrame)
            argsGb.set( "direKeyFrame", opDireKeyFrameParam)

        direKeyValueParam = node.getParameter( "direKeyValue" )
        direKeyValue = []
        for ci in range(0, direKeyValueParam.getNumChildren()):
            direKeyValue = direKeyValue + [ direKeyValueParam.getChildByIndex(ci).getValue(frameTime) ]
        if direKeyValue is not None and len(direKeyValue) > 0:
            opDireKeyValueParam = FnAttribute.FloatAttribute(direKeyValue,3)
            argsGb.set( "direKeyValue", opDireKeyValueParam)


        # Add the SpeedTree_In Op to the Ops chain
        rootLocation = locationParam.getValue( frameTime )

        sscb = FnGeolibServices.OpArgsBuilders.StaticSceneCreate()
        sscb.addSubOpAtLocation( rootLocation, "SpeedTreeIn", argsGb.build() )

        interface.appendOp( "StaticSceneCreate", sscb.build() )


        # set expression on the currentFrame parameter
        node.getParameter('currentFrame').setExpression("frame")


    # Create a NodeTypeBuilder to register the new type
    nodeTypeBuilder = Nodes3DAPI.NodeTypeBuilder('SpeedTree_In')

    # Build the node's parameters
    gb = FnAttribute.GroupBuilder()
    gb.set('location',
           FnAttribute.StringAttribute('/root/world/geo/SpeedTreeProc'))
    gb.set('srtFile', FnAttribute.StringAttribute(""))

    gb.set('useGlobalMotion', FnAttribute.IntAttribute(0))
    gb.set('currentFrame', FnAttribute.FloatAttribute(1001.0))
    gb.set('enableMotionBlur', FnAttribute.IntAttribute(1))
    gb.set( "motionSamples", FnAttribute.FloatAttribute([-0.25,0.25], 1) )


    gb.set('fps', FnAttribute.FloatAttribute(24.0))
    gb.set('globalFrequency', FnAttribute.FloatAttribute(0.0))
    gb.set('gustFrequency', FnAttribute.FloatAttribute(0.0))
    
    gb.set('windSpeed', FnAttribute.FloatAttribute(0.0))
    gb.set('windDirection.x', FnAttribute.FloatAttribute(1.0))
    gb.set('windDirection.y', FnAttribute.FloatAttribute(0.0))
    gb.set('windDirection.z', FnAttribute.FloatAttribute(0.0))

    gb.set('windType', FnAttribute.IntAttribute(6))
    gb.set('LODType', FnAttribute.IntAttribute(0))
    gb.set('LODSmoothType', FnAttribute.IntAttribute(0))

    gb.set( "speedKeyFrame", FnAttribute.FloatAttribute([], 1) )
    gb.set( "speedResponseTime", FnAttribute.FloatAttribute([], 1) )
    gb.set( "speedKeyValue", FnAttribute.FloatAttribute([], 1) )
    gb.set( "direKeyFrame", FnAttribute.FloatAttribute([], 1) )
    gb.set( "direResponseTime", FnAttribute.FloatAttribute([], 1) )
    gb.set( "direKeyValue", FnAttribute.FloatAttribute([], 3 ) )

    nodeTypeBuilder.setParametersTemplateAttr(gb.build())

    # Set parameter hints

    nodeTypeBuilder.setHintsForNode( { 'help' : '<p>Create Arnold procedurals suitable for SpeedTree.</p>' +
                                                '<p>Read and prase srt file data into arnold origin geometry'})
    nodeTypeBuilder.setHintsForParameter('location',
        {'widget':'scenegraphLocation','help':'The location of rendererProcedural node.'})
    nodeTypeBuilder.setHintsForParameter('srtFile', 
        {'widget':'assetIdInput','sequenceListing' : False, 'fileTypes':'srt', 'help':'SpeedTree srt file path'})

    nodeTypeBuilder.setHintsForParameter('useGlobalMotion',
        {'widget':'checkBox', 'help' : 'SpeedTree Procedutal useGlobalMotion'})
    nodeTypeBuilder.setHintsForParameter('currentFrame',
        {'help' : 'SpeedTree Procedutal currentFrame'})
    nodeTypeBuilder.setHintsForParameter('enableMotionBlur',
        {'widget':'checkBox', 'help' : 'SpeedTree Procedutal enableMotionBlur'})
    nodeTypeBuilder.setHintsForParameter( "motionSamples",
        {'widget' : 'sortableArray', 'help' : 'SpeedTree Procedutal motionSamples'})

    nodeTypeBuilder.setHintsForParameter('fps',
        {'help' : 'SpeedTree Procedutal fps'})
    nodeTypeBuilder.setHintsForParameter('globalFrequency',
        {'help' : 'SpeedTree Procedutal globalFrequency'})
    nodeTypeBuilder.setHintsForParameter('gustFrequency',
        {'help' : 'SpeedTree Procedutal gustFrequency'})
    nodeTypeBuilder.setHintsForParameter('windSpeed',
        {'help' : 'SpeedTree Procedutal windSpeed'})

    nodeTypeBuilder.setHintsForParameter('windDirection',
        {'widget':'multi','help' : 'SpeedTree Procedutal'})

    nodeTypeBuilder.setHintsForParameter('windType',
        {'help' : 'SpeedTree Procedutal windType'})
    nodeTypeBuilder.setHintsForParameter('LODType',
        {'help' : 'SpeedTree Procedutal LODType'})
    nodeTypeBuilder.setHintsForParameter('LODSmoothType',
        {'help' : 'SpeedTree Procedutal LODSmoothType'})

    nodeTypeBuilder.setHintsForParameter( "speedKeyFrame",
        {'widget' : 'sortableArray', 'help' : 'SpeedTree Procedutal speedKeyFrame'})
    nodeTypeBuilder.setHintsForParameter( "speedResponseTime",
        { 'widget' : 'sortableArray','help' : 'SpeedTree Procedutal speedResponseTime'})
    nodeTypeBuilder.setHintsForParameter( "speedKeyValue",
        { 'widget' : 'sortableArray','help' : 'SpeedTree Procedutal speedKeyValue'})
    nodeTypeBuilder.setHintsForParameter( "direKeyFrame",
        { 'widget' : 'sortableArray','help' : 'SpeedTree Procedutal direKeyFrame'})
    nodeTypeBuilder.setHintsForParameter( "direResponseTime",
        { 'widget' : 'sortableArray','help' : 'SpeedTree Procedutal direKeyFrame'})
    nodeTypeBuilder.setHintsForParameter( "direKeyValue",
        { 'widget' : 'dynamicArray','help' : 'SpeedTree Procedutal direKeyValue'})

    # Set the callback responsible to build the Ops chain
    nodeTypeBuilder.setBuildOpChainFnc(buildSpeedTreeInOpChain)


    # Build the new node type
    nodeTypeBuilder.build()

# Register the node
registerSpeedTreeIn()
