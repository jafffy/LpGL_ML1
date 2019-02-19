#ifndef OBJECTQUALITYScENE_H_
#define OBJECTQUALITYScENE_H_

#include "App.h"

class ObjectQualitySceneImpl;

class ObjectQualityScene : public App
{
public:
	ObjectQualityScene();
	~ObjectQualityScene();

	bool InitContents();
	void DestroyContents();

	int GetTargetFrameRate();

	void OnRender(int cameraIndex, float dt);

	void OnPressed();

private:
	ObjectQualitySceneImpl* impl = nullptr;
};

#endif // OBJECTQUALITYScENE_H_