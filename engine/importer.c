#include "importer.h"

const int INIT_SIZE = 256 * 10000;
const char* TEXTURES_PATH = "/textures/";
const char* ASSETS_PATH = "assets/";

static vec3* temp_vertices;
static vec2* temp_uvs;
static vec3* temp_normals;
static GLuint* indices;

static dict* vh;
static dict* materials;

static int vsize, vcount;
static int vtsize, vtcount;
static int vnsize, vncount;
static int isize, icount;

static int total_vertices_size, total_vertices;
static vertex* vertices;

static int meshes_size, meshes_count;
static mesh* meshes;

typedef struct {
  int joint_ids[3];
  float weights[3];
  int count;
} vertex_weights;

static int has_skl_file;
static vertex_weights* vweights;

static animation* animations[OBJECT_MAX_ANIMS];
static int animation_count = 0;

static const char* get_filename_ext(const char* filename) {
  const char* dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

static int find_file_ext(const char* asset, const char* ext, char* out_path) {
  char dir[256];
  strcpy(dir, ASSETS_PATH);
  strcat(dir, asset);
  strcat(dir, "/");

  struct dirent *de;  // Pointer for directory entry 
  DIR* dr = opendir(dir); 

  if (dr == NULL) { 
    printf("[find_file_ext] Could not open directory: %s\n", dir); 
    return 0; 
  } 

  while ((de = readdir(dr)) != NULL) {
    if (strcmp(ext, get_filename_ext(de->d_name)) == 0) {
      if (out_path != NULL) {
        strcpy(out_path, dir);
        strcat(out_path, de->d_name);
      }
      closedir(dr);     
      return 1;
    }
  }

  closedir(dr);     
  return 0; 
}

static skeleton* import_skl(const char* asset) {
  // find asset/asset.skl
  char skl_path[256];
  if (!find_file_ext(asset, "skl", skl_path)) {
    printf("[importer] cannot find skl file\n");
    exit(1);
  }

  printf("[importer] found %s\n", skl_path);

  FILE* file = fopen(skl_path, "r");
  if (file == NULL) {
    printf("[importer] cannot find file: %s\n", skl_path);
    exit(1);
  }

  char line[256];

  skeleton* skl = skeleton_create();

  // 0 = none, 1 = joints, 2 = joints_inv, 3 = weights
  int state = 0;
   
  while (fgets(line, sizeof(line), file)) {
    if (strstr(line, "joints") != NULL) {
      state = 1;
    } else if (strstr(line, "bindpose_inv") != NULL) {
      state = 2;
    } else if (strstr(line, "weights") != NULL) {
      state = 3;

      int weights_size = 0;
      sscanf(line, "weights %d", &weights_size);

      vweights = malloc(weights_size * sizeof(vertex_weights));
      for (int i = 0; i < weights_size; i++) {
        vweights[i].count = 0;
      }
    } else {
      if (state == 1) { // joints
        int joint_id;
        char joint_name[256];
        int parent_id;
        mat4 t;

        sscanf(line, "%d %s %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
            &joint_id, joint_name, &parent_id,
            &t[0][0], &t[1][0], &t[2][0], &t[3][0],
            &t[0][1], &t[1][1], &t[2][1], &t[3][1],
            &t[0][2], &t[1][2], &t[2][2], &t[3][2],
            &t[0][3], &t[1][3], &t[2][3], &t[3][3]);

        skeleton_joint_add(skl, joint_id, joint_name, parent_id, t);
      } else if (state == 2) { // joints_inv
        int joint_id;
        mat4 t;

        sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
            &joint_id,
            &t[0][0], &t[1][0], &t[2][0], &t[3][0],
            &t[0][1], &t[1][1], &t[2][1], &t[3][1],
            &t[0][2], &t[1][2], &t[2][2], &t[3][2],
            &t[0][3], &t[1][3], &t[2][3], &t[3][3]);

        mat4_copy(skl->rest_pose.transforms_inv[joint_id], t);
      } else if (state == 3) { // weights
        int vertex_id, joint_id;
        float weight;
        sscanf(line, "%d %d %f", &vertex_id, &joint_id, &weight);

        vertex_weights* vw = &vweights[vertex_id];
        vw->joint_ids[vw->count] = joint_id;
        vw->weights[vw->count] = weight;
        vw->count++;
      }
    }
  }

  // compute world transform
  frame_gen_transforms(&skl->rest_pose);

  // copy rest pose to current frame
  frame_copy_to(&skl->rest_pose, &skl->current_frame);
  
  fclose(file);
  return skl;
}

static animation* import_anm(const char* anim_path, const char* anim_name, skeleton* s) {
  // find asset/asset.anm
  FILE* file = fopen(anim_path, "r");
  if (file == NULL) {
    printf("[importer] cannot find file: %s\n", anim_path);
    exit(1);
  }

  char line[256];

  animation* anm = animation_create(anim_name);

  // 0 = none, 1 = keyframes, 2 = animations
  int state = 0;
  int keyframe_id = 0;
  while (fgets(line, sizeof(line), file)) {
    if (strstr(line, "keyframes") != NULL) {
      state = 1;
    } else if (strstr(line, "time") != NULL) {
      if (state == 1) {
        keyframe_id = 0;
        anm->frame_count = anm->keyframe_count;
      }
      state = 2; 
      sscanf(line, "time %d", &keyframe_id);
    } else {
      if (state == 1) {
        float time;
        sscanf(line, "%f", &time);
        animation_add_keyframe(anm, time);
        frame_copy_to(&s->rest_pose, &anm->frames[keyframe_id]);
        keyframe_id++;
      } else if (state == 2) {

        int joint_id;
        mat4 t;

        sscanf(line, "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
            &joint_id,
            &t[0][0], &t[1][0], &t[2][0], &t[3][0],
            &t[0][1], &t[1][1], &t[2][1], &t[3][1],
            &t[0][2], &t[1][2], &t[2][2], &t[3][2],
            &t[0][3], &t[1][3], &t[2][3], &t[3][3]);

        // set rotation
        quat_from_mat4(anm->frames[keyframe_id].joint_rotations[joint_id], t);

        // set position
        vec3 position = { t[3][0], t[3][1], t[3][2] };
        vec3_copy(anm->frames[keyframe_id].joint_positions[joint_id], position);

      }
    }
  }

  fclose(file);
  return anm;
}

static void import_animations(const char* asset, skeleton* s) {
  char dir[256];
  strcpy(dir, ASSETS_PATH);
  strcat(dir, asset);
  strcat(dir, "/");

  struct dirent *de;  // Pointer for directory entry 
  DIR* dr = opendir(dir); 

  if (dr == NULL) { 
    printf("[find_file_ext] Could not open directory: %s\n", dir); 
    return;
  } 

  char anim[256];

  int i = 0;
  while ((de = readdir(dr)) != NULL) {
    if (strcmp("anm", get_filename_ext(de->d_name)) == 0) {
      printf("[importer] found %s\n", de->d_name);
      strcpy(anim, dir);
      strcat(anim, de->d_name);
      animations[animation_count] = import_anm(anim, strtok(de->d_name, "."), s);
      animation_count++;
    }
  }

  closedir(dr);     
}

static void import_mtl(const char* asset) {
  // find asset/asset.mtl
  char mtl_path[256];
  if (!find_file_ext(asset, "mtl", mtl_path)) {
    printf("[importer] cannot find mtl file\n");
    exit(1);
  }

  printf("[importer] found %s\n", mtl_path);

  FILE* file = fopen(mtl_path, "r");
  if (file == NULL) {
    printf("[importer] cannot find file: %s\n", mtl_path);
    exit(1);
  }

  char tex_path[256];
  strcpy(tex_path, ASSETS_PATH);
  strcat(tex_path, asset);
  strcat(tex_path, TEXTURES_PATH);

  char line[256];
  material* current_mat = NULL;
  int first = 1;
  while (fgets(line, sizeof(line), file)) {
    // new material
    if (strstr(line, "newmtl ") != NULL) {
      if (!first) {
        dict_insert(materials, current_mat->name, current_mat);
      } else {
        first = 0;
      }
      current_mat = (material*)malloc(sizeof(material));
      material_init(current_mat);
      sscanf(line, "newmtl %s", current_mat->name);
    }
    // texture path
    else if (strstr(line, "map_Kd ") != NULL) {
      // assets/[asset]/textures/[diffuse].png
      char diffuse_path[256];
      sscanf(line, "map_Kd %s", diffuse_path);
      strcpy(current_mat->texture_path, tex_path);
      strcat(current_mat->texture_path, diffuse_path);
    }
    // normal map path
    else if (strstr(line, "map_Kn ") != NULL) {
      // assets/[asset]/textures/[normal].png
      char normal_path[256];
      sscanf(line, "map_Kn %s", normal_path);
      strcpy(current_mat->normal_map_path, tex_path);
      strcat(current_mat->normal_map_path, normal_path);
    }
    // specular map path
    else if (strstr(line, "map_Ks ") != NULL) {
      // assets/[asset]/textures/[specular].png
      char specular_path[256];
      sscanf(line, "map_Ks %s", specular_path);
      strcpy(current_mat->specular_map_path, tex_path);
      strcat(current_mat->specular_map_path, specular_path);
    }
    // mask map path
    else if (strstr(line, "map_d ") != NULL) {
      // assets/[asset]/textures/[normal].png
      char mask_path[256];
      sscanf(line, "map_d %s", mask_path);
      strcpy(current_mat->mask_map_path, tex_path);
      strcat(current_mat->mask_map_path, mask_path);
    }
    // diffuse
    else if (strstr(line, "Kd ") != NULL) {
      sscanf(line, "Kd %f %f %f\n", &current_mat->diffuse[0], &current_mat->diffuse[1], &current_mat->diffuse[2]);
    }
    // specular
    else if (strstr(line, "Ks ") != NULL) {
      sscanf(line, "Ks %f\n", &current_mat->specular, &current_mat->specular, &current_mat->specular);
    }
    else if (strstr(line, "r ") != NULL) {
      sscanf(line, "r %f\n", &current_mat->reflectivity);
    }
  }

  if (current_mat != NULL) {
    dict_insert(materials, current_mat->name, current_mat);
  }

  fclose(file);
}

static void push_index(const char* vkey) {
  // search face in the hashmap
  int* found = dict_search(vh, vkey);

  // get index from hashtable (or insert it if not present)
  if (found != NULL) {
    indices[icount] = *found;
  } else {
    // get vertex indices (vertex, texcoords, normals)
    int v_index, vt_index, vn_index = -1;
    int matches = sscanf(vkey, "%d/%d/%d", &v_index, &vt_index, &vn_index);
    if (matches != 3) {
      v_index = vt_index = vn_index = -1;
      matches = sscanf(vkey, "%d//%d", &v_index, &vn_index);
    }

    v_index--; vt_index--; vn_index--;

    // push vertex
    vertices[total_vertices].x = temp_vertices[v_index][0];
    vertices[total_vertices].y = temp_vertices[v_index][1];
    vertices[total_vertices].z = temp_vertices[v_index][2];
    vertices[total_vertices].u = vt_index >= 0 ? temp_uvs[vt_index][0] : 0.0f;
    vertices[total_vertices].v = vt_index >= 0 ? temp_uvs[vt_index][1] : 0.0f;
    vertices[total_vertices].nx = vn_index >= 0 ? temp_normals[vn_index][0] : 0.0f;
    vertices[total_vertices].ny = vn_index >= 0 ? temp_normals[vn_index][1] : 0.0f;
    vertices[total_vertices].nz = vn_index >= 0 ? temp_normals[vn_index][2] : 0.0f;

    if (has_skl_file) {
      vertices[total_vertices].jx = vweights[v_index].joint_ids[0];
      vertices[total_vertices].jy = vweights[v_index].joint_ids[1];
      vertices[total_vertices].jz = vweights[v_index].joint_ids[2];
      vertices[total_vertices].wx = vweights[v_index].weights[0];
      vertices[total_vertices].wy = vweights[v_index].weights[1];
      vertices[total_vertices].wz = vweights[v_index].weights[2];
    }
    
    // update indices
    int* index = malloc(sizeof(int));
    *index = total_vertices;
    dict_insert(vh, vkey, index);
    indices[icount] = total_vertices;

    total_vertices++;

    // Increase vertices size
    if (total_vertices >= total_vertices_size) {
      total_vertices_size *= 2;
      vertices = realloc(vertices, total_vertices_size * sizeof(vertex));
    }
  }
  icount++;
}

static void init_structures() {
  // temp vertices
  vsize = INIT_SIZE;
  vcount = 0;
  temp_vertices = malloc(vsize * sizeof(vec3));

  // temp uvs
  vtsize = INIT_SIZE;
  vtcount = 0;
  temp_uvs = malloc(vtsize * sizeof(vec2));
  
  // temp normals
  vnsize = INIT_SIZE;
  vncount = 0;
  temp_normals = malloc(vnsize * sizeof(vec3));

  // temp indices
  isize = INIT_SIZE;
  icount = 0;
  indices = malloc(isize * sizeof(GLuint));

  // vertices to return
  total_vertices_size = INIT_SIZE;
  total_vertices = 0;
  vertices = malloc(total_vertices_size * sizeof(vertex));

  // init hastable (it will be resized if needed)
  vh = dict_new(INIT_SIZE);

  // meshes
  meshes_count = 0;
  meshes_size = INIT_SIZE;
  meshes = malloc(meshes_size * sizeof(mesh));
}

object* importer_load(const char* asset) {
  // find asset/asset.obj
  char obj_path[256];
  if (!find_file_ext(asset, "obj", obj_path)) {
    printf("[importer] cannot find obj file\n");
    exit(1);
  }

  printf("[importer] found %s\n", obj_path);

  /* parse vertices */
  FILE* file = fopen(obj_path, "r");
  char line[256];

  if (file == NULL) {
    printf("[importer] cannot find file: %s\n", obj_path);
    exit(1);
  }

  init_structures();
  
  // import skl and anm
  skeleton* skel = NULL;
  has_skl_file = 0;
  if (find_file_ext(asset, "skl", NULL)) {
    has_skl_file = 1;
    skel = import_skl(asset);
    import_animations(asset, skel);
  }

  // materials dictionary
  materials = dict_new(INIT_SIZE);

  int first_mesh = 1;
  while (fgets(line, sizeof(line), file)) {

    // realloc indices list
    if (icount + 3 >= isize) {
      isize = isize * 2;
      indices = realloc(indices, isize * sizeof(GLuint));
    }

    // new vertex
    if (strstr(line, "v ") != NULL) {
      sscanf(line, "v %f %f %f", &temp_vertices[vcount][0], &temp_vertices[vcount][1], &temp_vertices[vcount][2]);
      vcount++;

      // realloc vertices list
      if (vcount >= vsize) {
        vsize *= 2;
        temp_vertices = realloc(temp_vertices, vsize * sizeof(vec3));
      }
    }

    // new texcoords
    if (strstr(line, "vt ") != NULL) {
      sscanf(line, "vt %f %f", &temp_uvs[vtcount][0], &temp_uvs[vtcount][1]);
      vtcount++;

      // realloc uvs list
      if (vtcount >= vtsize) {
        vtsize *= 2;
        temp_uvs = realloc(temp_uvs, vtsize * sizeof(vec2));
      }
    }

    // new normal
    if (strstr(line, "vn ") != NULL) {
      sscanf(line, "vn %f %f %f", &temp_normals[vncount][0], &temp_normals[vncount][1], &temp_normals[vncount][2]);
      vncount++;

      // realloc vertices list
      if (vncount >= vnsize) {
        vnsize *= 2;
        temp_normals = realloc(temp_normals, vnsize * sizeof(vec3));
      }
    }

    // new face
    if (strstr(line, "f ") != NULL) {

      // parse vertices of current face
      char v1[128];
      char v2[128];
      char v3[128];
      sscanf(line, "f %s %s %s", v1, v2, v3);

      // update indices
      push_index(v1);
      push_index(v2);
      push_index(v3);
    }

    // load mtl
    if (strstr(line, "mtllib ") != NULL) {
      import_mtl(asset);
    }

    // use mtl
    if (strstr(line, "usemtl") != NULL) {
      if (!first_mesh) {
        meshes[meshes_count].vertices = vertices;
        meshes[meshes_count].indices = indices;
        meshes[meshes_count].num_vertices = total_vertices;
        meshes[meshes_count].num_indices = icount;
        mesh_compute_tangent(&meshes[meshes_count]);
        meshes_count++;
      } else {
        first_mesh = 0;
      }

      char mtl_name[128];
      sscanf(line, "usemtl %s\n", mtl_name);
      meshes[meshes_count].mat = *((material*)dict_search(materials, mtl_name));
    }
  }

  // push last mesh
  meshes[meshes_count].vertices = vertices;
  meshes[meshes_count].indices = indices;
  meshes[meshes_count].num_vertices = total_vertices;
  meshes[meshes_count].num_indices = icount;
  mesh_compute_tangent(&meshes[meshes_count]);
  meshes_count++;

  fclose(file);

  dict_free(vh);
  free(temp_vertices);
  free(temp_uvs);
  free(temp_normals);
  dict_free(materials);

  if (has_skl_file) {
    free(vweights);
  }

  object* o = object_create(NULL, 1.0f, meshes, meshes_count, 1, skel);

  temp_vertices = NULL;
  temp_uvs = NULL;
  temp_normals = NULL;
  indices = NULL;
  vertices = NULL;
  meshes = NULL;
  vweights = NULL;

  for (int i = 0; i < animation_count; i++) {
    object_add_animation(o, animations[i]);
    animations[i] = NULL;
  }
  animation_count = 0;

  return o;
}
