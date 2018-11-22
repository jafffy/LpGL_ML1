#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_

#define CULLING

#define QDS_DEPTH 10

// --------------------------

#define TARGET_FRAME_RATE 60

#define NUM_OBJECTS 1

#define TARGET_MODEL_FILEPATH "assets/models/69K.obj"

#define SHADING

#ifdef SHADING
#define VS_FILE_PATH "assets/shaders/normal.vert"
#define FS_FILE_PATH "assets/shaders/normal.frag"
#else
#define VS_FILE_PATH "assets/shaders/basic3D.vert"
#define FS_FILE_PATH "assets/shaders/basic3D.frag"
#endif

#endif // EXPERIMENT_H_