#ifndef VARYINGFOCUSANGLESCENE_H_
#define VARYINGFOCUSANGLESCENE_H_

#include "App.h"

class VaryingFocusAngleSceneImpl;

class VaryingFocusAngleScene : public App
{
public:
	VaryingFocusAngleScene();
	~VaryingFocusAngleScene();

	bool InitContents();
	void DestroyContents();

	int GetTargetFrameRate();

	void OnRender(int cameraIndex, float dt);

	void OnPressed();

private:
	VaryingFocusAngleSceneImpl* impl = nullptr;
};

#endif // VARYINGFOCUSANGLESCENE_H_