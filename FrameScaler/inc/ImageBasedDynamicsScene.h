#ifndef IMAGEBASEDDYNAMICSCENE_H_
#define IMAGEBASEDDYNAMICSCENE_H_

#include "App.h"

class ImageBasedDynamicSceneImpl;

class ImageBasedDynamicsScene : public App {
public:
	ImageBasedDynamicsScene();
	~ImageBasedDynamicsScene();

	bool InitContents();
	void DestroyContents();

	int GetTargetFrameRate();

	void OnRender(int cameraIndex, float dt);

	void OnPressed();

private:
	ImageBasedDynamicSceneImpl* impl = nullptr;
};

#endif // IMAGEBASEDDYNAMICSCENE_H_