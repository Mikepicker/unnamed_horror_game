import xml.etree.ElementTree as ET

INPUT_NAME = 'chiostro.dae'
OUTPUT_NAME = 'character'

# strip namespace from tags
tree = ET.ElementTree(file=INPUT_NAME)
for el in tree.iter():
    if '}' in el.tag:
        el.tag = el.tag.split('}', 1)[1]

# root node
root = tree.getroot()

# library nodes
library_geometries = root.find('library_geometries')
library_images = root.find('library_images')
library_materials = root.find('library_materials')
library_effects = root.find('library_effects')
library_vs = root.find('library_visual_scenes')
library_controllers = root.find('library_controllers')
library_anim = root.find('library_animations')

# ----------------- UTILS ----------------- #
def safe_split(l):
    res = l.replace('\n', ' ').split(' ')
    res = [x for x in res if x] # clean array from empty strings
    return res

# ----------------- MATERIALS DATA ----------------- #
def extract_newparams(effect_node):
    profile_common_node = effect_node.find('profile_COMMON')

    if (profile_common_node == None):
        print('[extract_newparams] Unsupported profile')
        exit()

    newparams = {}
    newparam_nodes = profile_common_node.findall('newparam')

    for n in newparam_nodes:
        newparam_id = n.attrib['sid']

        child_node = list(list(n)[0])[0]
        if 'source' in child_node.tag: # ref to another newparam
            newparams[newparam_id] = effect_node.find('.//newparam[@sid="' + child_node.text + '"]').find('.//init_from').text
        elif 'init_from' in child_node.tag:
            newparams[newparam_id] = child_node.text
        elif 'instance_image' in child_node.tag:
            newparams[newparam_id] = child_node.attrib['url'][1:]

    return newparams

def extract_technique(effect_node, newparams, textures):
    technique = []

    # get params
    phong_node = effect_node.find('.//phong') or effect_node.find('.//blinn')
    param_nodes = list(phong_node)
    for p_node in param_nodes:

        # can be texture, color, float
        texture_node = p_node.find('texture')
        color_node = p_node.find('color')
        float_node = p_node.find('float')

        p_tag = p_node.tag

        if texture_node != None:
            texture_id = newparams[texture_node.attrib['texture']]
            technique.append({ 'id': p_tag, 'value': textures[texture_id] , 'type': 'texture' })
        elif color_node != None:
            rgba = safe_split(color_node.text)
            technique.append({ 'id': p_tag, 'value': { 'r': rgba[0], 'g': rgba[1], 'b': rgba[2], 'a': rgba[3] }, 'type': 'rgba' })
        else:
            technique.append({ 'id': p_tag, 'value': float_node.text, 'type': 'float' })

    return technique

def extract_material(effects):
    material = { 'id': 0, 'params': [] }

    # get ids
    material['id'] = e_node.attrib['name']

def extract_materials():
    # get all textures
    textures = {}
    image_nodes = list(library_images)
    for i_node in image_nodes:
        init_from_node = i_node.find('init_from')
        ref_node = init_from_node.find('ref')
        textures[i_node.attrib['id']] = ref_node.text if ref_node != None else init_from_node.text

    # effects
    effects = {}
    effects_node = library_effects.findall('effect')
    for e_node in effects_node:
        effect_id = e_node.attrib['id']

        # parse newparams (needed to link textures)
        newparams = extract_newparams(e_node)

        # parse techniques (diffuse, ambient ecc..)
        effects[effect_id] = extract_technique(e_node, newparams, textures)


    # combine materials
    materials = []
    material_nodes = library_materials.findall('material')
    for m_node in material_nodes:
        effect_id = m_node.find('instance_effect').attrib['url'][1:]
        materials.append({ 'id': m_node.attrib['id'], 'name': m_node.attrib['name'], 'params': effects[effect_id] })

    return materials

# ----------------- GEOMETRY DATA ----------------- #
# vertices, normals, uvs, materials
def extract_positions(mesh_node, poly_node):
    data = []

    positions_source = poly_node.find('.//input[@semantic="VERTEX"]').attrib['source'][1:]
    positions_id = mesh_node.find('.//vertices').find('input[@semantic="POSITION"]').attrib['source'][1:]
    node = mesh_node.find('.//source[@id="' + positions_id + '"]').find('float_array')

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

    normals_id = poly_node.find('.//input[@semantic="NORMAL"]').attrib['source'][1:]
    node = mesh_node.find('.//source[@id="' + normals_id + '"]').find('float_array')

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

    uvs_id = poly_node.find('.//input[@semantic="TEXCOORD"]').attrib['source'][1:]
    node = mesh_node.find('.//source[@id="' + uvs_id + '"]').find('float_array')

    count = int(node.attrib['count'])
    array = safe_split(node.text)
    
    for i in range (int(count / 2)):
        u = float(array[i * 2])
        v = float(array[i * 2 + 1])
        data.append({ "u": u, "v": v })

    return data

def extract_faces(mesh_node, poly_node):
    data = []

    type_count = len(poly_node.findall('input'))
    index_data = safe_split(poly_node.find('p').text)

    for i in range(int(len(index_data) / type_count)):
        p_index = index_data[i * type_count + 0]
        n_index = index_data[i * type_count + 1]
        u_index = index_data[i * type_count + 2]
        data.append({ "p_index": p_index, "n_index": n_index, "u_index": u_index })

    return data

def extract_geometry():
    res = []

    geometries = library_geometries.findall('geometry')

    for geometry in geometries:
        mesh_node = geometry.find('mesh')
        poly_node = mesh_node.find('triangles') or mesh_node.find('polylist')

        # raw data
        positions = extract_positions(mesh_node, poly_node)
        normals = extract_normals(mesh_node, poly_node)
        uvs = extract_uvs(mesh_node, poly_node)

        # faces
        faces = extract_faces(mesh_node, poly_node)

        # material
        material_symbol = poly_node.attrib['material']
        material_id = library_vs.find('.//instance_material').attrib['target'][1:]

        res.append({ "positions": positions, "normals": normals, "uvs": uvs, "faces": faces, "material_id": material_id })

    return res

# ----------------- SKIN DATA ----------------- #
# joints list and vertex weights

def shrink_vertex_data(vertex_data):
    vertex_data = sorted(vertex_data, key=lambda k: k['weight'], reverse=True)
    shrinked = vertex_data[:3]
    if (len(vertex_data) > 3):
        remaining = 1 - sum(list(map(lambda x: float(x['weight']), shrinked)))
        for i in range(3):
            shrinked[i]['weight'] = float(shrinked[i]['weight']) + (remaining / 3)
    return shrinked

def extract_joints():
    joints_data = []
    controller = library_controllers.find('controller')
    joints_tag = root.find('.//Name_array')
    joints = joints_tag.text.split()

    i = 0
    for joint_name in joints:
        data = joint_name
        joints_data.insert(i, data)
        i += 1

    return joints_data

def extract_vertex_weights():
    weights_data = []

    weights_data_id = library_controllers.find('.//input[@semantic="WEIGHT"]').attrib['source'][1:]
    weights = library_controllers.find('.//source[@id="' + weights_data_id + '"]').find('.//float_array').text.split()

    vertex_weights_tag = root.find('.//vertex_weights')
    counts = vertex_weights_tag.find('vcount').text.split()
    weights_map = vertex_weights_tag.find('v').text.split() # [joint_id, weight_id]

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

    return weights_data

# ----------------- SKELETON DATA ----------------- #
# joints hierarchy and transforms

def extract_joint_data(joints, joint_node):
    joint_name = joint_node.attrib['id']
    
    if joint_name not in joints:
        print("[extract_joint_data] skipping " + joint_name)
        return None

    index = joints.index(joint_name)
    matrix_data = joint_node.find('matrix').text.split(' ')

    children_data = []
    children_nodes = joint_node.findall('node')
    for child in children_nodes:
        res = extract_joint_data(joints, child)
        if res != None:
            res["parent_id"] = index
            children_data.append(res)

    return { "joint_id": index,  "joint_name": joint_name, "transform": matrix_data, "parent_id": -1, "children": children_data }

def extract_skeleton(joints):
    skeleton = []

    skeleton_node = library_vs.find('.//node').find('.//skeleton')
    head_joint_node = library_vs.find('.//node[@id="' + skeleton_node.text[1:] + '"]')

    head_joint_data = extract_joint_data(joints, head_joint_node) 
    skeleton.append(head_joint_data)

    return skeleton

# ----------------- ANIMATION DATA ----------------- #
# keyframes list, joint transforms for each keyframes, total duration
def extract_animations(joints):
    keyframes = []
    transforms = []
    duration = 0
     
    keyframes = safe_split(library_anim.find('animation').find('source').find('float_array').text.replace('\n', ' '))

    duration = keyframes[len(keyframes) - 1]

    animation_nodes = library_anim.findall('animation')
    for joint_node in animation_nodes:
        joint_name = joint_node.find('channel') .attrib['target'].split('/')[0]
        joint_data_id = joint_node.find('sampler').find('.//input[@semantic="OUTPUT"]').attrib['source'][1:]
        joint_transforms = safe_split(joint_node.find('.//float_array').text.replace('\n', ' '))

        # transform matrix for each keyframe
        transforms_out = []
        for i in range(int(len(joint_transforms) / 4)):
            t = []
            t.append(joint_transforms[i * 4 + 0])
            t.append(joint_transforms[i * 4 + 1])
            t.append(joint_transforms[i * 4 + 2])
            t.append(joint_transforms[i * 4 + 3])
            transforms_out.append(t)

        transforms.append({ "joint_id": joints.index(joint_name), "transforms": transforms_out })

    return { "keyframes": keyframes, "animations": transforms, "duration": duration }


# ----------------- EXPORT OBJ ----------------- #
def export_obj(geometry, materials):
    obj_out = open(OUTPUT_NAME + '.obj', 'w')

    has_materials = len(materials) > 0

    # output mtl reference
    if has_materials:
        obj_out.write('mtllib ' + OUTPUT_NAME + '.mtl\n')
    
    for group in geometry:
        # output positions
        for p in group['positions']:
            obj_out.write('v ' + str(p['x']) + ' ' + str(p['y']) + ' ' + str(p['z']) + '\n') 

    for group in geometry:
        # output normals
        for n in group['normals']:
            obj_out.write('vn ' + str(n['x']) + ' ' + str(n['y']) + ' ' + str(n['z']) + '\n') 

    for group in geometry:
        # output texcoords
        for u in group['uvs']:
            obj_out.write('vt ' + str(u['u']) + ' ' + str(u['v']) + '\n') 

    geom_index = 0
    p_offset = 0 # position offset 
    u_offset = 0 # texcoord offset
    n_offset = 0 # normal offset
    for group in geometry:
        # output material
        if has_materials and group['material_id'] != None:
            obj_out.write('usemtl ' + group['material_id'] + '\n')

        # output faces
        faces = group['faces']

        for i in range(int(len(faces) / 3)):
            line = 'f'
            line += ' ' + str(int(faces[i * 3 + 0]['p_index']) + 1 + p_offset) + '/' + str(int(faces[i * 3 + 0]['u_index']) + 1 + u_offset) + '/' + str(int(faces[i * 3 + 0]['n_index']) + 1 + n_offset)
            line += ' ' + str(int(faces[i * 3 + 1]['p_index']) + 1 + p_offset) + '/' + str(int(faces[i * 3 + 1]['u_index']) + 1 + u_offset) + '/' + str(int(faces[i * 3 + 1]['n_index']) + 1 + n_offset)
            line += ' ' + str(int(faces[i * 3 + 2]['p_index']) + 1 + p_offset) + '/' + str(int(faces[i * 3 + 2]['u_index']) + 1 + u_offset) + '/' + str(int(faces[i * 3 + 2]['n_index']) + 1 + n_offset)
            line += '\n'
            obj_out.write(line)

        geom_index += 1
        last_geom = geometry[geom_index - 1]
        p_offset += len(last_geom['positions'])
        u_offset += len(last_geom['uvs'])
        n_offset += len(last_geom['normals'])

    obj_out.close()

    # output mtl
    mtl_out = open(OUTPUT_NAME + '.mtl', 'w')

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

# ----------------- EXPORT SKL ----------------- #
def write_skeleton(skeleton, skl_out):
    for j in skeleton:
        skl_out.write(str(j['joint_id']) + ' ' + str(j['joint_name']) + ' ' + str(j['parent_id']) + ' ' + ' '.join(j['transform']) + '\n')
        write_skeleton(j['children'], skl_out)

def export_skl(weights, skeleton):
    skl_out = open(OUTPUT_NAME + '.skl', 'w')

    # joint_id joint_name parent_id transform
    skl_out.write('joints\n')
    write_skeleton(skeleton, skl_out)

    # vertex_id joint_id weight
    skl_out.write('weights\n')
    for w in weights:
        skl_out.write(str(w['vertex_id']) + ' ' + str(w['joint_id']) + ' ' + str(w['weight']) + '\n')

    skl_out.close()

# ----------------- EXPORT ANM ----------------- #
def export_anm(animations):
    anm_out = open(OUTPUT_NAME + '.anm', 'w')

    # duration
    anm_out.write('duration ' + animations['duration'] + '\n')

    # keyframes list
    anm_out.write('keyframes\n' + '\n'.join(animations['keyframes']) + '\n')

    # joint_id keyframe_id transform
    anm_out.write('animations\n')
    for a in animations['animations']:
        keyframe_id = 0
        for t in a['transforms']:
            anm_out.write(str(keyframe_id) + ' ' + str(a['joint_id']) + ' ' + ' '.join(t) + '\n') 
            keyframe_id += 1

    anm_out.close()

# ----------------- MAIN ----------------- #
geometry = extract_geometry()
materials = extract_materials()
export_obj(geometry, materials)

exit()

joints = extract_joints()

weights = extract_vertex_weights()
skeleton = extract_skeleton(joints)
export_skl(weights, skeleton)

animations = extract_animations(joints)
export_anm(animations)
