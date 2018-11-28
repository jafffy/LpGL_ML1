#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_

#define CULLING

#define QDS
#define QDS_DEPTH 10

#define LOD

// #define LOG_DYNAMIC_SCORE

// --------------------------

#define TARGET_FRAME_RATE 60

#define NUM_OBJECTS 5

#define TARGET_ASSET_BASEPATH "assets/"
#define TARGET_MODEL_BASEPATH TARGET_ASSET_BASEPATH "models/"
#define TARGET_MODEL_FILEPATH TARGET_MODEL_BASEPATH "standard_sphere.obj"
#define TARGET_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "standard_sphere.obj"
#define TARGET_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "standard_sphere.obj"

#define SHADING

#define TARGET_SHADER_BASEPATH TARGET_ASSET_BASEPATH "shaders/"

#ifdef SHADING
#define VS_FILE_PATH TARGET_SHADER_BASEPATH "normal.vert"
#define FS_FILE_PATH TARGET_SHADER_BASEPATH "normal.frag"
#else
#define VS_FILE_PATH TARGET_SHADER_BASEPATH "basic3D.vert"
#define FS_FILE_PATH TARGET_SHADER_BASEPATH "basic3D.frag"
#endif

#endif // EXPERIMENT_H_