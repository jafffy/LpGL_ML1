#ifndef UDPSCENE_H_
#define UDPSCENE_H_

#include "App.h"

class UDPSceneImpl;

class UDPScene : public App
{
public:
	UDPScene();
	~UDPScene();

	bool InitContents();
	void DestroyContents();


	int GetTargetFrameRate() { return 60; }

	void OnRender(int cameraIndex, float dt);

	void OnPressed();

private:
	UDPSceneImpl* impl = nullptr;
};

#endif // UDPSCENE_H_