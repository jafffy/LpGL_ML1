#include "LpGLEngine.h"
#include "BoundingBox.h"
#include "ModelObj.h"
#include "Camera.h"
#include "Experiment.h"

#include <vector>
#include <functional>

#include <ml_logging.h>

#define HIGH
#ifdef LOW
static struct {
	float level1 = 0.7f;
	float level2 = 0.5f;
} threshold;
#endif
#ifdef HIGH
static struct {
	float level1 = 0.2f;
	float level2 = 0.1f;
} threshold;
#endif
#ifdef ORIGIN
static struct {
	float level1 = 0.0f;
	float level2 = 0.0f;
} threshold;
#endif

class LpGLEngineImpl
{
public:
	std::vector<const BoundingBox2D*> last_bbs;
	float angle = 0.0f;
};

LpGLEngine::LpGLEngine()
{
	impl = new LpGLEngineImpl();
}

LpGLEngine::~LpGLEngine()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

int LpGLEngine::Update(int currentState, std::vector<ModelObj*>& models, int currentFPS, float dt)
{
	if (currentState == eels_without_lpgl) {
		return currentFPS;
	}

	int recommended_fps = currentFPS;

	float dynamic_score = 0;

	for (auto* model : models) {
		if (!model->IsVisible())
			continue;

		std::vector<glm::vec3> boundingVertices = model->GetBoundingBox();
		BoundingBox2D boundingBox2D;
		BoundingBox2D boundingBox2DForFocus;

		for (const auto& v : boundingVertices) {
			glm::vec4 result = Camera::Instance().P_for_LpGL * (Camera::Instance().V_for_LpGL * glm::vec4(v, 1.0f));

			boundingBox2D.AddPoint(result.x, result.y);
			boundingBox2DForFocus.AddPoint(result.x * 0.25f, result.y * 0.25f);
		}

		if ((currentState == eels_with_culling || currentState == eels_with_full_lpgl)
			&& (boundingBox2D.Min.y > 1 || boundingBox2D.Max.y < -1
				|| boundingBox2D.Min.x > 1 || boundingBox2D.Max.x < -1))
		{
			model->SetCulled(true);
		}
		else
		{
			model->SetCulled(false);

			if (currentState == eels_with_meshsimp || currentState == eels_with_full_lpgl) {
				float kReductionLevel2 = LOD_LV1 / CULLING_FOV;
				float kReductionLevel1 = LOD_LV2 / CULLING_FOV;

				if (boundingBox2DForFocus.Min.y > kReductionLevel2 || boundingBox2DForFocus.Max.y < -kReductionLevel2
					|| boundingBox2DForFocus.Min.x > kReductionLevel2 || boundingBox2DForFocus.Max.x < -kReductionLevel2) {
					model->SetReductionLevel(2);
				}
				else if (boundingBox2DForFocus.Min.y > kReductionLevel1 || boundingBox2DForFocus.Max.y < -kReductionLevel1
					|| boundingBox2DForFocus.Min.x > kReductionLevel1 || boundingBox2DForFocus.Max.x < -kReductionLevel1) {
					model->SetReductionLevel(1);
				}
				else {
					model->SetReductionLevel(0);
				}
			}

			if ((currentState == eels_with_ds || currentState == eels_with_full_lpgl)
				&& !model->IsLastCulled() && !model->IsCulled()) {
				glm::vec2 diff = model->GetLastProjectedPosition() - boundingBox2D.mid();
				float distanceSQ = glm::dot(diff, diff);

				if (dynamic_score < distanceSQ) {
					dynamic_score = distanceSQ;
				}
			}
		}

		model->SetLastProjectedPosition(boundingBox2D.mid());
	}

	constexpr float level1 = 0.01f / 15.0f;
	constexpr float level2 = 0.005f / 15.0f;

	if (currentState == eels_with_ds || currentState == eels_with_full_lpgl) {
		if (currentFPS == 60) {
			if (dynamic_score < level1) {
				recommended_fps = 30;
			}
		}
		else if (currentFPS == 30) {
			if (dynamic_score > level1) {
				recommended_fps = 60;
			}
			else if (dynamic_score < level2) {
				recommended_fps = 15;
			}
		}
		else {
			if (dynamic_score > level2) {
				recommended_fps = 30;
			}
		}
	}

	return recommended_fps;
}

void LpGLEngine::SetLODSensitivity(float angle) {
	impl->angle = angle;
}

LpGLEngine& LpGLEngine::instance()
{
	static LpGLEngine s_instance;
	return s_instance;
}

