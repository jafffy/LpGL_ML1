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

#include "MLNativeWindow.h"
#include "FrameScalerSampleApp.h"
#include "VaryingFocusAngleScene.h"

int main() {
	// FrameScalerSampleApp app;
	VaryingFocusAngleScene app;
	MLNativeWindow nativeWindow(&app);

	nativeWindow.Start();

  return 0;
}