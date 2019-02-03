// %BANNER_BEGIN%
// ---------------------------------------------------------------------
// %COPYRIGHT_BEGIN%
//
// Copyright (c) 2018 Magic Leap, Inc. All Rights Reserved.
// Use of this file is governed by the Creator Agreement, located
// here: https://id.magicleap.com/creator-terms
//
// %COPYRIGHT_END%
// ---------------------------------------------------------------------
// %BANNER_END%

#include <ml_perception.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>
#include <ml_head_tracking.h>
#include <ml_eye_tracking.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <chrono>
#include <thread>

#include "util.h"

const char application_name[] = "com.magicleap.simpleapp";

struct application_context_t {
	int dummy_value;
};

static void onStop(void* application_context)
{
	((struct application_context_t*)application_context)->dummy_value = 0;
	ML_LOG(Info, "%s: On stop called.", application_name);
}

static void onPause(void* application_context)
{
	((struct application_context_t*)application_context)->dummy_value = 1;
	ML_LOG(Info, "%s: On pause called.", application_name);
}

static void onResume(void* application_context)
{
	((struct application_context_t*)application_context)->dummy_value = 2;
	ML_LOG(Info, "%s: On resume called.", application_name);
}

int main() {
	MLLifecycleCallbacks lifecycle_callbacks = {};
	lifecycle_callbacks.on_stop = onStop;
	lifecycle_callbacks.on_pause = onPause;
	lifecycle_callbacks.on_resume = onResume;

	struct application_context_t application_context;
	application_context.dummy_value = 2;

	if (MLResult_Ok != MLLifecycleInit(&lifecycle_callbacks, (void*)&application_context)) {
		ML_LOG(Error, "%s: Failed to initialize lifecyle.", application_name);
		return -1;
	}

	MLPerceptionSettings perception_settings;
	if (MLResult_Ok != MLPerceptionInitSettings(&perception_settings)) {
		ML_LOG(Error, "%s: Failed to initialize perception.", application_name);
		return -1;
	}

	if (MLResult_Ok != MLPerceptionStartup(&perception_settings)) {
		ML_LOG(Error, "%s: Failed to startup perception.", application_name);
		return -1;
	}

	if (MLResult_Ok != MLLifecycleSetReadyIndication()) {
		ML_LOG(Error, "%s: Failed to indicate lifecycle ready.", application_name);
		return -1;
	}

	// Initialization about head tracker
	MLHandle head_tracker;
	MLResult head_tracker_result = MLHeadTrackingCreate(&head_tracker);
	MLHeadTrackingStaticData head_static_data;
	{
		if (MLResult_Ok == head_tracker_result && MLHandleIsValid(head_tracker)) {
			MLHeadTrackingGetStaticData(head_tracker, &head_static_data);
		}
		else {
			ML_LOG(Error, "%s: Failed to create head tracker.", application_name);
		}
	}

	// Initialization about eye tracker
	MLHandle eye_tracking;
	MLResult eye_tracker_result = MLEyeTrackingCreate(&eye_tracking);
	MLEyeTrackingStaticData eye_static_data;
	{
		if (MLResult_Ok == eye_tracker_result && MLHandleIsValid(eye_tracking)) {
			MLEyeTrackingGetStaticData(eye_tracking, &eye_static_data);
		}
		else {
			ML_LOG(Error, "%s: Failed to create eye tracker.", application_name);
		}
	}

	double timer = 0.0;
	constexpr double k_running_time = 5;
	auto start = std::chrono::high_resolution_clock::now();

	ML_LOG(Info, "%s: the loop is just launched.", application_name);

	while (timer < k_running_time)
	{
		MLSnapshot* current_snapshot = nullptr;
		if (MLResult_Ok != MLPerceptionGetSnapshot(&current_snapshot)) {
			ML_LOG(Error, "%s: Failed to get a current perception snapshot.", application_name);
			continue;
		}

		// Getting Head Tracking Data
		{
			// TODO: Consider the MLHeadTrackingState for the error pruning.
			MLTransform head_transform;

			if (MLResult_Ok != MLSnapshotGetTransform(current_snapshot, &head_static_data.coord_frame_head, &head_transform)) {
				ML_LOG(Error, "%s: Failed to get a head tracking transform from the current snapshot.", application_name);
			}
			else {
				ML_LOG_TAG(Info, "head_tracking", "%s", toString(head_transform).c_str());
			}
		}

		// Getting Eye Tracking Data
		{
			// TODO: Consider the MLEyeTrackingState for the error pruning.
			MLTransform fixation_transform;
			MLTransform left_center_transform;
			MLTransform right_center_transform;

			if (MLResult_Ok != MLSnapshotGetTransform(current_snapshot, &eye_static_data.fixation, &fixation_transform)) {
				ML_LOG(Error, "%s: Failed to get a eye tracking transform(fixation) from the current snapshot.", application_name);
			}
			else {
				ML_LOG_TAG(Info, "eye_tracking", "fixation: %s", toString(fixation_transform).c_str());
			}
			if (MLResult_Ok != MLSnapshotGetTransform(current_snapshot, &eye_static_data.left_center, &left_center_transform)) {
				ML_LOG(Error, "%s: Failed to get a eye tracking transform(left_center) from the current snapshot.", application_name);
			}
			else {
				ML_LOG_TAG(Info, "eye_tracking", "left_center: %s", toString(left_center_transform).c_str());
			}
			if (MLResult_Ok != MLSnapshotGetTransform(current_snapshot, &eye_static_data.right_center, &right_center_transform)) {
				ML_LOG(Error, "%s: Failed to get a eye tracking transform(right_center) from the current snapshot.", application_name);
			}
			else {
				ML_LOG_TAG(Info, "eye_tracking", "right_center: %s", toString(right_center_transform).c_str());
			}
		}

		// Release snapshot
		MLPerceptionReleaseSnapshot(current_snapshot);

		// Counting the time for termination
		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = end - start;

		timer = elapsed.count();

		// using namespace std::chrono_literals; ML1 doesn't provide it. Jesus.
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	ML_LOG(Info, "It ends! (by %lf)", timer);

	MLHeadTrackingDestroy(head_tracker);

	MLPerceptionShutdown();

	return 0;
}
