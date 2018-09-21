from xml.etree import ElementTree

def get_path(node):
    for c in node.getchildren():
        if c.tag == 'arbitraryList':
            for attr in c.getchildren():
                if attr.attrib['name'] == "fullPath" and attr.attrib.has_key('value'):
                    return attr.attrib['value']
    return ""


def relink(p_instance, c_instance):
    for c in p_instance.getchildren():
        if c.tag == 'instanceList':
            p_instance_list = c

    for c in c_instance.getchildren():
        if c.tag == 'instanceList':
            c_instance_list = c
    p_instance.remove(p_instance_list)
    p_instance.append(c_instance_list)
    return


def parse_node(parent_instance, node):
    if node.tag =='instance':
        node_path = get_path(node)
        if node_path.endswith(":master") and node.attrib['name'] == parent_instance.attrib['name']:
            relink(parent_instance, node)
        elif node_path.endswith(":asb"):
            relink(parent_instance, node)
        else:
            parent_instance = node

    for c in node.getchildren():
        parse_node(parent_instance, c)
    return


def shorten_name(root):
    l_instances = root.getiterator("instance")
    for i in l_instances:
        if not i.attrib.has_key('refFile'):
            i.attrib['name'] = i.attrib['name'].split('.')[-1]
    return


def main(src_xml, dst_xml):
    tree = ElementTree.parse(src_xml)
    xml_root = tree.getroot()
    parse_node(None, xml_root)
    shorten_name(xml_root)
    tree.write(dst_xml, "utf-8")
    return



