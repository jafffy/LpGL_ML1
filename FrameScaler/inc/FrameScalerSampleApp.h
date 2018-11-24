#ifndef FRAMESCALERSAMPLEAPP_H_
#define FRAMESCALERSAMPLEAPP_H_

#include "App.h"

class FrameScalerSampleAppImpl;

class FrameScalerSampleApp : public App
{
public:
	FrameScalerSampleApp();
	~FrameScalerSampleApp();

	int Start();
	void Cleanup();

	bool InitContents();
	void DestroyContents();

	void Update(float dt);
	void OnRender(int cameraIndex);

	int GetTargetFrameRate();
	void SetTargetFrameRate(int targetFrameRate);

private:
	FrameScalerSampleAppImpl* impl = nullptr;
};

#endif // FRAMESCALERSAMPLEAPP_H_