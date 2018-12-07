#include "ImageBasedDynamicsScene.h"

#include "glm/glm.hpp"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_logging.h>

#include <algorithm>
#include <cmath>

// #define PSNR

class ImageBasedDynamicSceneImpl
{
public:
	std::vector<glm::u8vec3> lastFrame;
	std::vector<glm::u8vec3> buf;
	bool isFirst = true;

	ImageBasedDynamicSceneImpl()
	{
		buf.resize(1280 * 960);
		lastFrame.resize(1280 * 960);
	}

	~ImageBasedDynamicSceneImpl()
	{
	}
};

ImageBasedDynamicsScene::ImageBasedDynamicsScene()
{
	impl = new ImageBasedDynamicSceneImpl();
}

ImageBasedDynamicsScene::~ImageBasedDynamicsScene()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

bool ImageBasedDynamicsScene::InitContents()
{
	return true;
}

void ImageBasedDynamicsScene::DestroyContents()
{

}

int ImageBasedDynamicsScene::GetTargetFrameRate()
{
	return 60;
}

void ImageBasedDynamicsScene::OnRender(int cameraIndex, float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (cameraIndex == 0) {
		ML_LOG(Info, "SSIM_Start");
		// ML_LOG(Info, "glReadPixels_Start");

		glReadPixels(0, 0, 1280, 960, GL_RGB, GL_UNSIGNED_BYTE, impl->buf.data());

		// ML_LOG(Info, "glReadPixels_End");

		if (impl->isFirst) {
			impl->isFirst = false;
		}
		else {
#ifdef PSNR
			float tot = 0.0f;

			for (int x = 0; x < 1280; ++x) {
				for (int y = 0; y < 960; ++y) {
					const auto& u = impl->buf[x * 960 + y];
					const auto& v = impl->lastFrame[x * 960 + y];

					tot += (v.x - u.x) * (v.x - u.x) + (v.y - u.y) * (v.y - u.y) + (v.z - u.z) * (v.z - u.z);
				}
			}
			ML_LOG(Info, "%lf", tot / 1280 / 960);
#else
			float ux = 0.0f;
			float uy = 0.0f;
			float vx = 0.0f;
			float vy = 0.0f;
			float uxy = 0.0f;

			constexpr float k1 = 0.01;
			constexpr float k2 = 0.03;
			constexpr float L = 256 - 1;
			constexpr float c1 = k1 * k1 * L * L;
			constexpr float c2 = k2 * k2 * L * L;

			for (int x = 0; x < 1280; ++x) {
				for (int y = 0; y < 960; ++y) {
					const auto& v1 = impl->buf[x * 960 + y];
					const auto& v2 = impl->lastFrame[x * 960 + y];

					float y1 = 0.299 * v1.x + 0.587 * v1.y + 0.114 * v1.z;
					float y2 = 0.299 * v2.x + 0.587 * v2.y + 0.114 * v2.z;

					ux += y1;
					uy += y2;
					vx += y1 * y1;
					vy += y2 * y2;
					uxy += y1 * y2;
				}
			}

			const float size = 1280 * 960;

			ux /= size;
			uy /= size;
			vx /= size;
			vy /= size;
			uxy /= size;

			vx = vx - ux * ux;
			vy = vy - uy * uy;
			uxy = uxy - ux * uy;

			const float ssim = (2 * ux * uy + c1) * (2 * uxy + c2) / ((ux*ux + uy * uy + c1) * (vx*vx + vy * vy + c2));
			ML_LOG(Info, "SSIM: %f", ssim);
#endif
		}

		std::swap(impl->buf, impl->lastFrame);

		ML_LOG(Info, "SSIM_End");
		// ML_LOG(Info, "End");
	}
}

void ImageBasedDynamicsScene::OnPressed()
{

}

