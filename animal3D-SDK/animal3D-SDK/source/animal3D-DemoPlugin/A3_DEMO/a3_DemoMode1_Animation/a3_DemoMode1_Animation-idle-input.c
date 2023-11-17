/*
	Copyright 2011-2020 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein

	a3_DemoMode1_Animation-idle-input.c
	Demo mode implementations: animation scene.

	********************************************
	*** INPUT FOR ANIMATION SCENE MODE       ***
	********************************************
*/

//-----------------------------------------------------------------------------

#include "../a3_DemoMode1_Animation.h"

//typedef struct a3_DemoState a3_DemoState;
#include "../a3_DemoState.h"

#include "../_a3_demo_utilities/a3_DemoMacros.h"


//-----------------------------------------------------------------------------
// CALLBACKS

// main demo mode callback
void a3animation_input_keyCharPress(a3_DemoState const* demoState, a3_DemoMode1_Animation* demoMode, a3i32 const asciiKey, a3i32 const state)
{
	switch (asciiKey)
	{
		// toggle render program
		//a3demoCtrlCasesLoop(demoMode->render, animation_render_max, 'k', 'j');

		// toggle display program
		//a3demoCtrlCasesLoop(demoMode->display, animation_display_max, 'K', 'J');

		// toggle active camera
		//a3demoCtrlCasesLoop(demoMode->activeCamera, animation_camera_max, 'v', 'c');

		// toggle pipeline mode
		a3demoCtrlCasesLoop(demoMode->pipeline, animation_pipeline_max, ']', '[');

		// toggle target
		a3demoCtrlCasesLoop(demoMode->targetIndex[demoMode->pass], demoMode->targetCount[demoMode->pass], '}', '{');

		// toggle pass to display
		a3demoCtrlCasesLoop(demoMode->pass, animation_pass_max, ')', '(');

		// toggle control target
		a3demoCtrlCasesLoop(demoMode->ctrl_target, animation_ctrlmode_max, '\'', ';');

		// toggle position input mode
		a3demoCtrlCasesLoop(demoMode->ctrl_position, animation_inputmode_max, '=', '-');

		// toggle rotation input mode
		a3demoCtrlCasesLoop(demoMode->ctrl_rotation, animation_inputmode_max, '+', '_');
	}
}

void a3animation_input_keyCharHold(a3_DemoState const* demoState, a3_DemoMode1_Animation* demoMode, a3i32 const asciiKey, a3i32 const state)
{
//	switch (asciiKey)
//	{
//
//	}
}



void a3animation_tryStartJump(a3_DemoMode1_Animation* demoMode)
{
	if (demoMode->isJumping) return;

	demoMode->isJumping = true;

	// Reset to beginning of jump clip
	demoMode->jumpClipCtrl->keyframeIndex = demoMode->jumpClipCtrl->clip->keyframeIndex_first;
	demoMode->jumpClipCtrl->keyframe = demoMode->jumpClipCtrl->clipPool->keyframe + demoMode->jumpClipCtrl->keyframeIndex;
	demoMode->jumpClipCtrl->playback_sec = 0;
	demoMode->jumpClipCtrl->clipTime_sec = 0;
	demoMode->jumpClipCtrl->keyframeTime_sec = 0;


}



//-----------------------------------------------------------------------------

void a3demo_input_controlObject(
	a3_DemoState* demoState, a3_DemoSceneObject* object,
	a3f64 const dt, a3real ctrlMoveSpeed, a3real ctrlRotateSpeed);
void a3demo_input_controlProjector(
	a3_DemoState* demoState, a3_DemoProjector* projector,
	a3f64 const dt, a3real ctrlMoveSpeed, a3real ctrlRotateSpeed, a3real ctrlZoomSpeed);

void a3animation_input(a3_DemoState* demoState, a3_DemoMode1_Animation* demoMode, a3f64 const dt)
{
	// TEST - Manipulating ctrlNode
	/*a3ret horizontal = a3keyboardGetState(demoState->keyboard, a3key_rightArrow) - a3keyboardGetState(demoState->keyboard, a3key_leftArrow);
	a3ret vertical = a3keyboardGetState(demoState->keyboard, a3key_upArrow) - a3keyboardGetState(demoState->keyboard, a3key_downArrow);

	a3spatialPoseSetTranslation(demoMode->ctrlNode, demoMode->ctrlNode->translate.x + horizontal, demoMode->ctrlNode->translate.y + vertical, demoMode->ctrlNode->translate.z);*/


	a3_DemoProjector* projector = demoMode->projector + demoMode->activeCamera;
	a3_DemoSceneObject* sceneObject;


	// right click to ray pick
	if (a3mouseGetState(demoState->mouse, a3mouse_right) == a3input_down)
	{
		// get window coordinates
		a3i32 const x = a3mouseGetX(demoState->mouse) + demoState->frameBorder;
		a3i32 const y = a3mouseGetY(demoState->mouse) + demoState->frameBorder;

		// transform to NDC
		a3vec4 coord = a3vec4_one;
		coord.x = +((a3real)x * demoState->frameWidthInv * a3real_two - a3real_one);
		coord.y = -((a3real)y * demoState->frameHeightInv * a3real_two - a3real_one);
		coord.z = -a3real_one;

		// transform to view space
		a3real4Real4x4Mul(projector->projectionMatInv.m, coord.v);
		a3real4DivS(coord.v, coord.w);

		// transform to world space
		a3real4Real4x4Mul(projector->sceneObject->modelMat.m, coord.v);
	}
	
	// choose control target
	switch (demoMode->ctrl_target)
	{
	case animation_ctrl_camera:
		// move camera
		a3demo_input_controlProjector(demoState, projector,
			dt, projector->ctrlMoveSpeed, projector->ctrlRotateSpeed, projector->ctrlZoomSpeed);
		break;
	case animation_ctrl_character:
		// capture axes
		if (a3XboxControlIsConnected(demoState->xcontrol))
		{
			// get directly from joysticks
			a3XboxControlGetJoysticks(demoState->xcontrol, demoMode->axis_l, demoMode->axis_r);
		}
		else
		{
			//Movement
			// calculate normalized vectors given keyboard state
			demoMode->axis_l[0] = (a3real)a3keyboardGetDifference(demoState->keyboard, a3key_D, a3key_A);
			demoMode->axis_l[1] = (a3real)a3keyboardGetDifference(demoState->keyboard, a3key_W, a3key_S);

			//Normalize if magnitude is not 0
			if (demoMode->axis_l[0] != 0 && demoMode->axis_l[1] != 0)
			{
				a3real shortAxis[2];
				shortAxis[0] = (a3real)demoMode->axis_l[0];
				shortAxis[1] = (a3real)demoMode->axis_l[1];
				a3real2Normalize(shortAxis);
				demoMode->axis_l[0] = (a3f64)shortAxis[0];
				demoMode->axis_l[1] = (a3f64)shortAxis[1];
			}

			//Rotation
			a3real pitch = -(a3real)a3mouseGetDeltaY(demoState->mouse);
			a3real yaw = -(a3real)a3mouseGetDeltaX(demoState->mouse);

			demoMode->axis_r[0] = yaw;
			demoMode->axis_r[1] = pitch;
		}
		break;
	case animation_ctrl_neckLookat:
	case animation_ctrl_wristEffector_r:
	case animation_ctrl_wristConstraint_r:
		sceneObject = demoMode->obj_skeleton_ctrl + demoMode->ctrl_target - animation_ctrl_character;
		a3demo_input_controlObject(demoState, sceneObject, dt, a3real_one, a3real_zero);
		/*	// capture axes
		if (a3XboxControlIsConnected(demoState->xcontrol))
		{
			// ****TO-DO:
			// get directly from joysticks

		}
		else
		{
			// ****TO-DO:
			// calculate normalized vectors given keyboard state

		}
	*/break;
	}

	// allow the controller, if connected, to change control targets
	if (a3XboxControlIsConnected(demoState->xcontrol))
	{
		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_DPAD_right))
			a3demoCtrlIncLoop(demoMode->ctrl_target, animation_ctrlmode_max);
		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_DPAD_left))
			a3demoCtrlDecLoop(demoMode->ctrl_target, animation_ctrlmode_max);

		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_B))
			a3demoCtrlIncLoop(demoMode->ctrl_position, animation_inputmode_max);
		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_X))
			a3demoCtrlDecLoop(demoMode->ctrl_position, animation_inputmode_max);

		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_DPAD_up))
			a3demoCtrlIncLoop(demoMode->ctrl_rotation, animation_inputmode_max);
		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_DPAD_down))
			a3demoCtrlDecLoop(demoMode->ctrl_rotation, animation_inputmode_max);

		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_A))
		{

		}

		if (a3XboxControlIsPressed(demoState->xcontrol, a3xbox_start))
		{
			demoMode->ctrl_target = animation_ctrl_character;
			demoMode->ctrl_position = animation_input_interpolate2;
			demoMode->ctrl_rotation = animation_input_interpolate2;
		}
	}
}