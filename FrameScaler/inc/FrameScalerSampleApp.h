#ifndef FRAMESCALERSAMPLEAPP_H_
#define FRAMESCALERSAMPLEAPP_H_

#include "glm/glm.hpp"

class FrameScalerSampleAppImpl;

class FrameScalerSampleApp
{
public:
	FrameScalerSampleApp();
	~FrameScalerSampleApp();

	int Start();
	void Cleanup();

	bool InitContents();
	void DestroyContents();

	void Update(float dt);
	void OnRender();
	void Draw(int camera_number, glm::mat4 VP);

private:
	FrameScalerSampleAppImpl* impl = nullptr;
};

#endif // FRAMESCALERSAMPLEAPP_H_