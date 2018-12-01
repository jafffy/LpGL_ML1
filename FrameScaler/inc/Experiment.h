#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_

// Tag definition
#define LATENCY "LATENCY"
#define FIDELITY_LATENCY "FIDELITY_" LATENCY
#define HIT_ACCURACY "HIT_ACCURACY"

#define CULLING
#define CULLING_FOV float(M_PI / 180 * 80.0f)

#define QDS
#define QDS_DEPTH 10

#define LOD

#define LOD_LV1 float(M_PI / 180 * 30.0f)
#define LOD_LV2 float(M_PI / 180 * 10.0f)

// #define TRANSIT_LPGL_STATE

// #define LOG_DYNAMIC_SCORE

#define INITIAL_X 1.75

// #define DYNAMIC_SCENE
#define FIDELITY_SCENE

// --------------------------

#define TARGET_FRAME_RATE 60

#define NUM_OBJECTS 16

#define TARGET_ASSET_BASEPATH "assets/"
#define TARGET_MODEL_BASEPATH TARGET_ASSET_BASEPATH "models/"

#ifdef FIDELITY_SCENE
#define ABNORMAL_MODEL_FILEPATH TARGET_MODEL_BASEPATH "69K_abnormal_2.obj"
#define ABNORMAL_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "69K_abnormal_2.obj"
#define ABNORMAL_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "69K_abnormal_2.obj"
#define ABNORMAL2_MODEL_FILEPATH TARGET_MODEL_BASEPATH "69K_abnormal2_2.obj"
#define ABNORMAL2_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "69K_abnormal2_2.obj"
#define ABNORMAL2_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "69K_abnormal2_2.obj"
#define ABNORMAL3_MODEL_FILEPATH TARGET_MODEL_BASEPATH "69K_abnormal3_2.obj"
#define ABNORMAL3_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "69K_abnormal3_2.obj"
#define ABNORMAL3_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "69K_abnormal3_2.obj"
#define TARGET_MODEL_FILEPATH TARGET_MODEL_BASEPATH "69K.obj"
#define TARGET_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "69K_1.obj"
#define TARGET_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "69K_2.obj"
#else
#define TARGET_MODEL_FILEPATH TARGET_MODEL_BASEPATH "69K_0.5.obj"
#define TARGET_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "69K_0.5.obj"
#define TARGET_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "69K_0.5.obj"
#endif

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