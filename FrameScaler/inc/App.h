#ifndef APP_H_
#define APP_H_

class App
{
public:
	virtual ~App() {}

	virtual bool InitContents() = 0;
	virtual void DestroyContents() = 0;

	virtual int GetTargetFrameRate() = 0;

	virtual void OnRender(int cameraIndex) = 0;
};

#endif // APP_H_