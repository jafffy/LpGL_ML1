#ifndef WIN32NATIVEWINDOW_H_
#define WIN32NATIVEWINDOW_H_

#include "App.h"

class Win32NativeWindowImpl;

class Win32NativeWindow
{
public:
	Win32NativeWindow(App* app);
	~Win32NativeWindow();

	int Start();

	void OnRender();

private:
	Win32NativeWindowImpl* impl
};

#endif // WIN32NATIVEWINDOW_H_