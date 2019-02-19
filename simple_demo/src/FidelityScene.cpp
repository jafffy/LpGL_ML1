#include "FidelityScene.h"

class FidelitySceneImpl
{

};

FidelityScene::FidelityScene()
{
	impl = new FidelitySceneImpl();
}

FidelityScene::~FidelityScene()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

bool FidelityScene::InitContents()
{
	return true;
}

void FidelityScene::DestroyContents()
{}

int FidelityScene::GetTargetFrameRate()
{}

void FidelityScene::OnRender(int cameraIndex, float dt)
{

}

void FidelityScene::OnPressed()
{}
