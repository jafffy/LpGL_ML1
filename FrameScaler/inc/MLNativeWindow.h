#ifndef MLNATIVEWINDOW_H_
#define MLNATIVEWINDOW_H_

#include "App.h"

class MLNativeWindowImpl;

class MLNativeWindow
{
public:
	MLNativeWindow(App* app);
	~MLNativeWindow();

	int Start();

	void OnRender(float dt);

private:
	MLNativeWindowImpl* impl = nullptr;
};

#endif // MLNATIVEWINDOW_H_