#ifndef FIDELITYSCENE_H_
#define FIDELITYSCENE_H_

#include "App.h"

class FidelitySceneImpl;

class FidelityScene : public App
{
public:
	FidelityScene();
	~FidelityScene();

	bool InitContents();
	void DestroyContents();

	int GetTargetFrameRate();

	void OnRender(int cameraIndex, float dt);

	void OnPressed();

private:
	FidelitySceneImpl* impl = nullptr;
};

#endif // FIDELITYSCENE_H_