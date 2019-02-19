#ifndef QDSEVALUATIONSCENE_H_
#define QDSEVALUATIONSCENE_H_

#include "App.h"

class QDSEvaluationSceneImpl;

class QDSEvaluationScene : public App
{
public:
	QDSEvaluationScene();
	~QDSEvaluationScene();

	bool InitContents();
	void DestroyContents();

	int GetTargetFrameRate();

	void OnRender(int cameraIndex, float dt);

	void OnPressed();

private:
	QDSEvaluationSceneImpl* impl = nullptr;
};

#endif // QDSEVALUATIONSCENE_H_