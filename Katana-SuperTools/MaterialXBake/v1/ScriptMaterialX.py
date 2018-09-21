import os,sys
import logging
log = logging.getLogger('MaterialXBakeNode')
try:
    import MaterialX as mx
except ImportError:
    log.error("Can`t find MaterialX -- %s"% sys.exc_info()[0])
try:
    from Katana import NodegraphAPI
except ImportError:
    print "Can`t find Katana"

ARNOLD_NODEDEFS=os.path.join(os.path.dirname(__file__), 'arnold', 'nodedefs.mtlx')

COLOR3 = ['out', 'out.r', 'out.g', 'out.b']
COLOR4 = ['out', 'out.r', 'out.g', 'out.b', 'out.a']
SWIZZLESUFFIX = ['.r', '.g', '.b', '.a', '.x', '.y', '.z']

def __traverseUpstreamMaterialNetworks(node, sets):
    log.info("Traverse current node -%s"%node)
    up_nodes = __getConnectedUpstreamNodes(node)
    for node in up_nodes:
        sets.append(node)
        __traverseUpstreamMaterialNetworks(node, sets)

# Get upstream nodes which connected to current node.
def __getConnectedUpstreamNodes(node):
    result_nodes = []
    inputports = node.getInputPorts()
    for i_port in inputports:
        port = i_port.getConnectedPort(0)
        if not port:
            continue
        shader_name = port.getNode()
        result_nodes.append(shader_name)
    return result_nodes

# Check out if this port connected with any other ports.
def __isPortConnected(port):
    ports = port.getConnectedPorts()
    if ports:
        return True
    else:
        return False

# Get the node which connected to the input port.
def __getPortConnectedNode(port):
    if __isPortConnected(port):
        return port.getConnectedPort(0).getNode()
    else:
        return None


def __getValue(node, parameter):
    def __getType(node, parameter):
        doc = mx.createDocument()
        mx.readFromXmlFile(doc, ARNOLD_NODEDEFS)
        # Traverse the document tree in depth-first order.
        nodeinput_type = node.getParameter('nodeType').getValue(0)
        return doc.getNodeDef(nodeinput_type).getInput(parameter).getType()
    para_type = node.getParameter("parameters.%s.value"%parameter).getType()
    input_type = __getType(node, parameter)
    if para_type == "string":
        return "string", node.getParameter("parameters.%s.value"%parameter).getValue(0)
    elif para_type == "number":
        if input_type == "integer":
            return "integer", node.getParameter("parameters.%s.value"%parameter).getValue(0)
        elif input_type == "boolean":
            if int(node.getParameter("parameters.%s.value"%parameter).getValue(0)):
                return "boolean", "true"
            else:
                return "boolean", "false"
        elif input_type == "float":
            return "float", node.getParameter("parameters.%s.value"%parameter).getValue(0)
    elif para_type == "numberArray":
        # Find out the tuple type,color or vector?
        _size =  node.getParameter("parameters.%s.value"%parameter).getTupleSize()
        _tuple = []
        for i in range(0, _size):
            _tuple.append(node.getParameter("parameters.%s.value.i%i"%(parameter, i)).getValue(0))
        if input_type == "color2":
            return "color2", mx.Color2(_tuple[0], _tuple[1])
        elif input_type == "color3":
            return "color3", mx.Color3(_tuple[0], _tuple[1], _tuple[2])
        elif input_type == "color4":
            return "color4", mx.Color4(_tuple[0], _tuple[1], _tuple[2], _tuple[3])
        elif input_type == "vector2":
            return "vector2", mx.Vector2(_tuple[0], _tuple[1])
        elif input_type == "vector3":
            return "vector3", mx.Vector3(_tuple[0], _tuple[1], _tuple[2])
        elif input_type == "vector4":
            return "vector4", mx.Vector4(_tuple[0], _tuple[1], _tuple[2], _tuple[3])

# Since Arnold`s NodeDefs don`t define output type so we can`t
# get this attribute directlly.
def __getNodeOutputType(node):
    output_ports = node.getOutputPorts()
    output_port_names = []
    for port in output_ports:
        output_port_names.append(port.getName())
    if output_port_names == COLOR3:
        return "color3"
    elif output_port_names == COLOR4:
        return "color4"
    else:
        ## To Do: return Closure
        return "float"

def __filteringSwizzleSuffix(parameter):
    suffix_list = SWIZZLESUFFIX
    for suffix in suffix_list:
        if parameter.endswith(suffix ):
            parameter = parameter[:-2]
    return parameter

#########################################################################
# Since now you COULDN`T connect multiple output into one input!
def __setChannels(mxnode, node, parameter):
    # print parameter,node.getInputPort(parameter),upstream_node
    mx_input = mxnode.getInput(parameter)

    channel = node.getInputPort(parameter).getConnectedPort(0).getName().split(".")[-1]
    if not channel == "out":
        mx_input.setChannels(channel)

def __setParameters(mxnode, node, tag="NodeGraph"):
    _parameter_valued = []
    _parameter_connected = []
    for input_port in node.getInputPorts():
        paramter_name = input_port.getName()
        if __isPortConnected(input_port):
            if not paramter_name in _parameter_connected:
                _parameter_connected.append(paramter_name)   
        else:
            paramter_name = __filteringSwizzleSuffix(paramter_name)
            # If port 'out' has connected to any node, we should skip the
            # children port like 'out.r', 'out.g', 'out.b'.
            parent_port = node.getInputPort(paramter_name)
            if not paramter_name in _parameter_valued \
                                and not __isPortConnected(parent_port):
                _parameter_valued.append(paramter_name)
    if tag == "NodeGraph":
        for parameter in _parameter_connected:
            upstream_node = __getPortConnectedNode(node.getInputPort(parameter))
            mxnode.setConnectedNodeName(parameter, upstream_node.getName())
            __setChannels(mxnode, node, parameter)
        for parameter in  _parameter_valued:
            _type, _value = __getValue(node, parameter)
            mxnode.setInputValue(parameter, _value, _type)
    elif tag == "ShaderRef":
        for parameter in _parameter_connected:
            _bind_input = mxnode.addBindInput(parameter)
            upstream_node = __getPortConnectedNode(node.getInputPort(parameter))
            _bind_input.setNodeGraphString("NodeGraph__"+upstream_node.getName())
        for parameter in  _parameter_valued:
            _type, _value = __getValue(node, parameter)
            _bind_input = mxnode.addBindInput(parameter, _type)
            _bind_input.setValue(_value)
    else:
        return

def createMXMaterial(document, node):
    material_name = node.getOutputPortByIndex(0).getConnectedPort(0).getNode().getName()
    mx_material = document.getMaterial("Material__"+material_name)
    if not mx_material:
        mx_material = document.addMaterial("Material__"+material_name)
    mx_shader_ref = createMXShaderRef(mx_material, node)
    return mx_material

def createMXShaderRef(material, node):
    shader_ref_name = node.getName()
    shader_refinput_type = node.getParameter('nodeType').getValue(0)
    mx_shader_ref = material.getShaderRef("ShaderRef__"+shader_ref_name)    
    if not mx_shader_ref:
        mx_shader_ref = material.addShaderRef("ShaderRef__"+shader_ref_name, shader_refinput_type)
        __setParameters(mx_shader_ref, node, "ShaderRef")
    return mx_shader_ref

def createMXNodeGraph(document, node):
    node_graph_name = node.getName()
    mx_node_graph = document.getNodeGraph("NodeGraph__"+node_graph_name)
    if not mx_node_graph:
        mx_node_graph = document.addNodeGraph("NodeGraph__"+node_graph_name)

        ng_root_node_name = node.getName()
        ng_root_nodeinput_type = node.getParameter('nodeType').getValue(0)
        ng_root_node = mx_node_graph.addNode( ng_root_nodeinput_type, name=ng_root_node_name)
        __setParameters(ng_root_node, node, "NodeGraph")
        
        upstream_nodes = []
        __traverseUpstreamMaterialNetworks(node, upstream_nodes)
        for next_node in upstream_nodes:
            ng_next_node_name = next_node.getName()
            ng_next_node_type = next_node.getParameter('nodeType').getValue(0)
            ng_next_node = mx_node_graph.getNode( ng_next_node_name )
            if not ng_next_node:
                ng_next_node = mx_node_graph.addNode( ng_next_node_type, name=ng_next_node_name)
                __setParameters(ng_next_node, next_node, "NodeGraph")
        output = mx_node_graph.addOutput('out')
        output.setConnectedNode(ng_root_node)
    return mx_node_graph

def export(sets, saveTo):
    # Create a document.
    doc = mx.createDocument()
    # Include Arnold nodedefs.
    mx.prependXInclude(doc, ARNOLD_NODEDEFS)

    for look_name in sets:
        node_sets = sets[look_name]

        for node_set in node_sets:
            collection_node_name = node_set[0]
            network_material_node_name = node_set[1]
            node = NodegraphAPI.GetNode(network_material_node_name)
            # The material node might be surafceShader or displacementShader
            # so we need to record it all.
            material_node_list = __getConnectedUpstreamNodes(node)
            for meterial_node in material_node_list:
                mx_material = createMXMaterial(doc, meterial_node)
                mx_shader_ref = mx_material.getShaderRef("ShaderRef__"+meterial_node.getName())

                input_port_list = meterial_node.getInputPorts()
                node_graph_node_list = []
                # Get nodeGraph root node list
                for input_port in input_port_list:
                    if __isPortConnected(input_port):
                        node_graph_node = __getPortConnectedNode(input_port)
                        if not node_graph_node in node_graph_node_list:
                            node_graph_node_list.append(node_graph_node)
                # Create nodeGraph node
                for node_graph_node in node_graph_node_list:
                    mx_node_graph = createMXNodeGraph(doc,node_graph_node)


        # Create a look.
        look = doc.addLook(look_name)
        for node_set in node_sets:
            collection_node_name = node_set[0]
            network_material_node_name = node_set[1]

            # Create a collection.
            collection = doc.addCollection("Colloction__"+look_name+"_"+collection_node_name.replace("/","_"))
            collectionAdd = collection.addCollectionAdd("CollectionAdd__"+look_name+"_"+collection_node_name.replace("/","_"))
            collectionAdd.setGeom("*"+collection_node_name)

            materialAssign = look.addMaterialAssign("MaterialAssign__"+look_name+"_"+collection_node_name.replace("/","_"))
            materialAssign.setCollection(collection)
            materialAssign.setMaterial("Material__"+network_material_node_name)

    mx.writeToXmlFile(doc, saveTo)