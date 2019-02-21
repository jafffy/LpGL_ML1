#ifndef FRAMESCALERSAMPLEAPP_H_
#define FRAMESCALERSAMPLEAPP_H_

#include "App.h"

class SimpleDemoAppImpl;

class SimpleDemoApp : public App
{
public:
  SimpleDemoApp();

  ~SimpleDemoApp() override;

  bool InitContents() override;

  void DestroyContents() override;

	void Update(float dt);

  void OnRender(int cameraIndex, float dt) override;

  int GetTargetFrameRate() override;

  void OnPressed() override;

private:
  SimpleDemoAppImpl *impl = nullptr;
};

#endif // FRAMESCALERSAMPLEAPP_H_