#include "LpGLEngine.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "Experiment.h"

#include <functional>

class LpGLEngineImpl
{
public:
	std::vector<const BoundingBox2D*> last_bbs;
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

int LpGLEngine::Update(std::vector<ModelObj *> &models, int currentFPS, float dt) {
	if (!this->IsOn) {
		for (auto *model : models) {
			model->InitalizeLpGLForLpGL();
		}

		return 60;
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

      float t = 0.5f;

			boundingBox2D.AddPoint(result.x, result.y);
			boundingBox2DForFocus.AddPoint(result.x * t, result.y * t);
		}

		boundingBox2D.Build();
		boundingBox2DForFocus.Build();

		float k_culling_radius = 4;

		if ((boundingBox2D.Min.y > k_culling_radius || boundingBox2D.Max.y < -k_culling_radius
				 || boundingBox2D.Min.x > k_culling_radius || boundingBox2D.Max.x < -k_culling_radius))
		{
			model->SetCulled(true);
		}
		else
		{
			model->SetCulled(false);

			float kReductionLevel2 = fabsf(LOD_LV1 / CULLING_FOV);
			float kReductionLevel1 = fabsf(LOD_LV2 / CULLING_FOV);

			if (boundingBox2DForFocus.Min.y > kReductionLevel2 || boundingBox2DForFocus.Max.y < -kReductionLevel2
					|| boundingBox2DForFocus.Min.x > kReductionLevel2 || boundingBox2DForFocus.Max.x < -kReductionLevel2) {
				model->SetReductionLevel(2);
			} else if (boundingBox2DForFocus.Min.y > kReductionLevel1 || boundingBox2DForFocus.Max.y < -kReductionLevel1
								 || boundingBox2DForFocus.Min.x > kReductionLevel1 || boundingBox2DForFocus.Max.x < -kReductionLevel1) {
				model->SetReductionLevel(1);
			} else {
				model->SetReductionLevel(0);
			}

			if (!model->IsLastCulled() && !model->IsCulled()) {
				const auto& a = model->GetLastProjectedPosition();
				const auto& b = boundingBox2D.mid();

				float x = a.x - b.x;
				float y = a.y - b.y;
				float distanceSQ = x * x + y * y;

				if (dynamic_score < distanceSQ) {
					dynamic_score = distanceSQ;
				}
			}
		}

		model->SetLastProjectedPosition(boundingBox2D.mid());
	}

	constexpr float level1 = 0.01f / 120.0f;
	constexpr float level2 = 0.005f / 120.0f;

	if (currentFPS == 60) {
		if (dynamic_score < level1) {
			recommended_fps = 30;
		}
	} else if (currentFPS == 30) {
		if (dynamic_score > level1) {
			recommended_fps = 60;
		} else if (dynamic_score < level2) {
			recommended_fps = 15;
		}
	} else {
		if (dynamic_score > level2) {
			recommended_fps = 30;
		}
	}

	return recommended_fps;
}

LpGLEngine& LpGLEngine::instance()
{
	static LpGLEngine s_instance;
	return s_instance;
}

