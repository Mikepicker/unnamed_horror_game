import xml.etree.ElementTree as ET

root = ET.parse('Walking.dae').getroot()

# ----------------- UTILS ----------------- #
def safe_split(l):
    res = l.replace('\n', ' ').split(' ')
    res = [x for x in res if x] # clean array from empty strings
    return res

# newmtl Material
# Ns 323.999994
# Ka 1.000000 1.000000 1.000000
# Kd 0.800000 0.800000 0.800000
# Ks 0.500000 0.500000 0.500000
# Ke 0.000000 0.000000 0.000000
# Ni 1.450000
# d 1.000000
# illum 2

# ----------------- MATERIALS DATA ----------------- #
library_images = root.find('{*}library_images')
library_materials = root.find('{*}library_materials')
library_effects = root.find('{*}library_effects')

def extract_materials():
    materials = []

    # textures
    textures = {}
    image_nodes = list(library_images)
    for i_node in image_nodes:
        textures[i_node.attrib['name']] = i_node.find('{*}init_from').text

    # effects
    effects_node = library_effects.findall('{*}effect')
    for e_node in effects_node:
        material = { 'id': 0, 'params': [] }

        # get ids
        material['id'] = e_node.attrib['name']

        # get params
        phong_node = e_node.find('.//{*}phong')
        param_nodes = list(phong_node)
        for p_node in param_nodes:

            # can be texture, color, float
            texture_node = p_node.find('{*}texture')
            color_node = p_node.find('{*}color')
            float_node = p_node.find('{*}float')

            p_tag = p_node.tag.split('}')[1]

            if texture_node != None:
                texture_id = texture_node.attrib['texture'].split('-')[0]
                material['params'].append({ 'id': p_tag, 'value': textures[texture_id] , 'type': 'texture' })
            elif color_node != None:
                rgba = safe_split(color_node.text)
                material['params'].append({ 'id': p_tag, 'value': { 'r': rgba[0], 'g': rgba[1], 'b': rgba[2], 'a': rgba[3] }, 'type': 'rgba' })
            else:
                material['params'].append({ 'id': p_tag, 'value': float_node.text, 'type': 'float' })

        materials.append(material)

    return materials

# ----------------- GEOMETRY DATA ----------------- #
# vertices, normals, uvs, materials

library_geometries = root.find('{*}library_geometries')

def extract_positions(mesh_node, poly_node):
    data = []

    positions_source = poly_node.find('.//{*}input[@semantic="VERTEX"]').attrib['source'][1:]
    positions_id = mesh_node.find('.//{*}vertices').find('{*}input[@semantic="POSITION"]').attrib['source'][1:]
    node = mesh_node.find('.//{*}source[@id="' + positions_id + '"]').find('{*}float_array')

    count = int(node.attrib['count'])
    array = safe_split(node.text)
    
    for i in range(int(count / 3)):
        x = float(array[i * 3 + 0])
        y = float(array[i * 3 + 1])
        z = float(array[i * 3 + 2])
        data.append({ "x": x, "y": y, "z": z })

    return data

def extract_normals(mesh_node, poly_node):
    data = []

    normals_id = poly_node.find('.//{*}input[@semantic="NORMAL"]').attrib['source'][1:]
    node = mesh_node.find('.//{*}source[@id="' + normals_id + '"]').find('{*}float_array')

    count = int(node.attrib['count'])
    array = safe_split(node.text)
    
    for i in range(int(count / 3)):
        x = float(array[i * 3])
        y = float(array[i * 3 + 1])
        z = float(array[i * 3 + 2])
        data.append({ "x": x, "y": y, "z": z })

    return data

def extract_uvs(mesh_node, poly_node):
    data = []

    uvs_id = poly_node.find('.//{*}input[@semantic="TEXCOORD"]').attrib['source'][1:]
    node = mesh_node.find('.//{*}source[@id="' + uvs_id + '"]').find('{*}float_array')

    count = int(node.attrib['count'])
    array = safe_split(node.text)
    
    for i in range (int(count / 2)):
        u = float(array[i * 2])
        v = float(array[i * 2 + 1])
        data.append({ "u": u, "v": v })

    return data

def extract_faces(mesh_node, poly_node):
    data = []

    type_count = len(poly_node.findall('{*}input'))
    index_data = safe_split(poly_node.find('{*}p').text)

    for i in range(int(len(index_data) / type_count)):
        p_index = index_data[i * type_count + 0]
        n_index = index_data[i * type_count + 1]
        u_index = index_data[i * type_count + 2]
        data.append({ "p_index": p_index, "n_index": n_index, "u_index": u_index })

    return data

def extract_geometry():
    geometry = []

    mesh_node = library_geometries.find('{*}geometry').find('{*}mesh')

    poly_nodes = mesh_node.findall('{*}triangles') or mesh_node.findall('{*}polylist')

    for poly_node in poly_nodes:
        # raw data
        positions = extract_positions(mesh_node, poly_node)
        normals = extract_normals(mesh_node, poly_node)
        uvs = extract_uvs(mesh_node, poly_node)

        # faces
        faces = extract_faces(mesh_node, poly_node)

        # material
        material_id = poly_node.attrib['material']

        geometry.append({ "positions": positions, "normals": normals, "uvs": uvs, "faces": faces, "material_id": material_id })

    return geometry

# ----------------- SKIN DATA ----------------- #
# joints list and vertex weights

library_controllers = root.find('{*}library_controllers')

def shrink_vertex_data(vertex_data):
    vertex_data = sorted(vertex_data, key=lambda k: k['weight'], reverse=True)
    shrinked = vertex_data[:3]
    if (len(vertex_data) > 3):
        remaining = 1 - sum(list(map(lambda x: float(x['weight']), shrinked)))
        for i in range(3):
            shrinked[i] = float(shrinked[i]['weight']) + (remaining / 3)
    return shrinked

def extract_joints():
    joints_data = []
    controller = library_controllers.find('{*}controller')
    joints_tag = root.find('.//{*}Name_array')
    joints = joints_tag.text.split()

    i = 0
    for joint_name in joints:
        data = joint_name
        joints_data.insert(i, data)
        i += 1

    return joints_data

def extract_vertex_weights():
    weights_data = []

    weights_data_id = library_controllers.find('.//{*}input[@semantic="WEIGHT"]').attrib['source'][1:]
    weights = library_controllers.find('.//{*}source[@id="' + weights_data_id + '"]').find('.//{*}float_array').text.split()

    vertex_weights_tag = root.find('.//{*}vertex_weights')
    counts = vertex_weights_tag.find('{*}vcount').text.split()
    weights_map = vertex_weights_tag.find('{*}v').text.split() # [joint_id, weight_id]

    pointer = 0
    vertex_id = 0
    for count in counts:
        vertex_data = []
        for w in range(int(count)):
            joint_id = weights_map[vertex_id]
            pointer += 1
            weight_id = weights_map[vertex_id]
            pointer += 1
            vertex_data.append({ "vertex_id": vertex_id, "joint_id": joint_id, "weight": weights[int(weight_id)] })

        # get only first 3 weights
        vertex_data = shrink_vertex_data(vertex_data)

        weights_data += vertex_data

        vertex_id += 1

# ----------------- SKELETON DATA ----------------- #
# joints hierarchy and transforms

library_vs = root.find('{*}library_visual_scenes')

def extract_joint_data(joints, joint_node):
    joint_name = joint_node.attrib['id']
    
    if joint_name not in joints:
        print("[extract_joint_data] skipping " + joint_name)
        return None

    index = joints.index(joint_name)
    matrix_data = joint_node.find('{*}matrix').text.split(' ')

    children_data = []
    children_nodes = joint_node.findall('{*}node')
    for child in children_nodes:
        res = extract_joint_data(joints, child)
        if res != None:
            res["parent_id"] = index
            children_data.append(res)

    return { "joint_id": index, "matrix": matrix_data, "children": children_data }

def extract_skeleton(joints):
    skeleton = []

    skeleton_node = library_vs.find('.//{*}node').find('.//{*}skeleton')
    head_joint_node = library_vs.find('.//{*}node[@id="' + skeleton_node.text[1:] + '"]')

    head_joint_data = extract_joint_data(joints, head_joint_node) 
    skeleton.append(head_joint_data)

    return skeleton

# ----------------- ANIMATION DATA ----------------- #
# keyframes list, joint transforms for each keyframes, total duration

library_anim = root.find('{*}library_animations')

def extract_animations(joints):
    keyframes = []
    transforms = []
    duration = 0
     
    keyframes = library_anim.find('{*}animation').find('{*}source').find('{*}float_array').text.replace('\n', ' ').split(' ')
    keyframes = [x for x in keyframes if x] # clean array from empty strings

    duration = keyframes[len(keyframes) - 1]

    animation_nodes = library_anim.findall('{*}animation')
    for joint_node in animation_nodes:
        joint_name = joint_node.find('{*}channel') .attrib['target'].split('/')[0]
        joint_data_id = joint_node.find('{*}sampler').find('.//{*}input[@semantic="OUTPUT"]').attrib['source'][1:]
        joint_transform = joint_node.find('.//{*}float_array').text.replace('\n', ' ').split(' ')
        joint_transform = [x for x in joint_transform if x] # clean array from empty strings

        transforms.append({ "joint_id": joints.index(joint_name), "transform": joint_transform })

    return { "keyframes": keyframes, "transforms": transforms, "duration": duration }


# ----------------- EXPORT OBJ ----------------- #
def export_obj(geometry, materials):
    obj_out = open('output.obj', 'w')

    # output mtl reference
    if len(materials) > 0:
        obj_out.write('mtllib output.mtl\n')
    
    for group in geometry:
        if group['material_id'] != None:
            obj_out.write('usemtl ' + group['material_id'] + '\n')

        # output positions
        for p in group['positions']:
            obj_out.write('v ' + str(p['x']) + ' ' + str(p['y']) + ' ' + str(p['z']) + '\n') 

        # output normals
        for n in group['normals']:
            obj_out.write('vn ' + str(n['x']) + ' ' + str(n['y']) + ' ' + str(n['z']) + '\n') 

        # output texcoords
        for u in group['uvs']:
            obj_out.write('vt ' + str(u['u']) + ' ' + str(u['v']) + '\n') 

        # output faces
        faces = group['faces']

        for i in range(int(len(faces) / 3)):
            line = 'f'
            line += ' ' + str(int(faces[i * 3 + 0]['p_index']) + 1) + '/' + str(int(faces[i * 3 + 0]['u_index']) + 1) + '/' + str(int(faces[i * 3 + 0]['n_index']) + 1)
            line += ' ' + str(int(faces[i * 3 + 1]['p_index']) + 1) + '/' + str(int(faces[i * 3 + 1]['u_index']) + 1) + '/' + str(int(faces[i * 3 + 1]['n_index']) + 1)
            line += ' ' + str(int(faces[i * 3 + 2]['p_index']) + 1) + '/' + str(int(faces[i * 3 + 2]['u_index']) + 1) + '/' + str(int(faces[i * 3 + 2]['n_index']) + 1)
            line += '\n'
            obj_out.write(line)

    obj_out.close()

    # output mtl
    mtl_out = open('output.mtl', 'w')

    conv_map = {
        'emission': 'Ke',
        'ambient': 'Ka',
        'diffuse': 'Kd',
        'specular': 'Ks',
        'shininess': 'Ns',
        'transparency': 'd'
    }

    skip_list = ['reflectivity', 'reflective', 'transparent']

    for m in materials:
        mtl_out.write('newmtl ' + str(m['id']) + '\n')

        for p in m['params']:
            if p['id'] in skip_list:
                continue

            if p['type'] == 'texture':
                mtl_out.write('map_' + conv_map[p['id']] + ' ' + p['value'] + '\n')
            elif p['type'] == 'rgba':
                mtl_out.write(conv_map[p['id']] + ' ' + p['value']['r'] + ' ' + p['value']['g'] + ' ' + p['value']['b'] + '\n')
            elif p['type'] == 'float':
                mtl_out.write(conv_map[p['id']] + ' ' + p['value'] + '\n')

    mtl_out.close()

# ----------------- MAIN ----------------- #

geometry = extract_geometry()
materials = extract_materials()
export_obj(geometry, materials)

'''
joints = extract_joints()
weights = extract_vertex_weights()

skeleton = extract_skeleton(joints)

animations = extract_animations(joints)
'''
