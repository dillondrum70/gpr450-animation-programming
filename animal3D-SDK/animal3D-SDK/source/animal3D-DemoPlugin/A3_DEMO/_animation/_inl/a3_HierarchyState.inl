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
	
	a3_HierarchyState.inl
	Implementation of inline transform hierarchy state operations.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Dillon Drummond, Neo Kattan, Joseph Lyons

	a3_DemoMode1_Animation-idle-update.c
	Implemented logic for functions relating to hierarchy states and hierarchy poses
*/


#ifdef __ANIMAL3D_HIERARCHYSTATE_H
#ifndef __ANIMAL3D_HIERARCHYSTATE_INL
#define __ANIMAL3D_HIERARCHYSTATE_INL

#include <stdio.h>
#include <stdlib.h>


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

// get offset to hierarchy pose in contiguous set
inline a3i32 a3hierarchyPoseGroupGetPoseOffsetIndex(const a3_HierarchyPoseGroup *poseGroup, const a3ui32 poseIndex)
{
	if (poseGroup && poseGroup->hierarchy)
		return (poseIndex * poseGroup->hierarchy->numNodes);
	return -1;
}

// get offset to single node pose in contiguous set
inline a3i32 a3hierarchyPoseGroupGetNodePoseOffsetIndex(const a3_HierarchyPoseGroup *poseGroup, const a3ui32 poseIndex, const a3ui32 nodeIndex)
{
	if (poseGroup && poseGroup->hierarchy)
		return (poseIndex * poseGroup->hierarchy->numNodes + nodeIndex);
	return -1;
}

//-----------------------------------------------------------------------------

// reset full hierarchy pose
inline a3i32 a3hierarchyPoseReset(const a3_HierarchyPose* pose_inout, const a3ui32 nodeCount)
{
	if (pose_inout && nodeCount)
	{
		//Reset each spatial pose
		for (a3ui32 i = 0; i < nodeCount; i++)
		{
			a3spatialPoseReset(pose_inout[i].sPoses);
		}

		return 1;
	}
	return -1;
}

// convert full hierarchy pose to hierarchy transforms
inline a3i32 a3hierarchyPoseConvert(const a3_HierarchyPose* pose_inout, const a3ui32 nodeCount, const a3_SpatialPoseChannel* channel, const a3_SpatialPoseEulerOrder order)
{
	if (pose_inout && nodeCount)
	{
		//Update transform matrix of every spatial pose in h pose
		for (a3ui32 i = 0; i < nodeCount; i++)
		{
			a3spatialPoseConvert(&(pose_inout->sPoses + i)->transform, (pose_inout->sPoses + i),
				channel[i], order);
		}

		return 1;
	}
	return -1;
}

// copy full hierarchy pose
inline a3i32 a3hierarchyPoseCopy(const a3_HierarchyPose* pose_out, const a3_HierarchyPose* pose_in, const a3ui32 nodeCount)
{
	if (pose_out && pose_in && nodeCount)
	{
		//Copy each spatial pose
		for (a3ui32 i = 0; i < nodeCount; i++)
		{
			//Copying values not addresses
			*(pose_out->sPoses + i) = *(pose_in->sPoses + i);
		}

		return 1;
	}
	return -1;
}

//Lerp between poses
inline a3i32 a3hierarchyPoseLerp(a3_HierarchyPose* pose_out, const a3_HierarchyPose* pose0, const a3_HierarchyPose* pose1,
	const a3real parameter, const a3ui32 numNodes)
{
	if (pose_out && pose0 && pose1 && numNodes)
	{

		//For each spatial pose
		for (a3ui32 i = 0; i < numNodes; i++)
		{
			//Lerp the pose0 and pose1 values using the parameter then stick them in pose_out
			//Translation
			a3real3Lerp(
				(pose_out->sPoses + i)->translation,
				(pose0->sPoses + i)->translation,
				(pose1->sPoses + i)->translation,
				parameter
			);

			//Rotation
			a3real3Lerp(
				(pose_out->sPoses + i)->rotation,
				(pose0->sPoses + i)->rotation,
				(pose1->sPoses + i)->rotation,
				parameter
			);

			//Scale
			a3real3Lerp(
				(pose_out->sPoses + i)->scale,
				(pose0->sPoses + i)->scale,
				(pose1->sPoses + i)->scale,
				parameter
			);
		}

		return 1;
	}
	return -1;
}

//step pose to given pose (just calls copy)
inline a3i32 a3hierarchyPoseStep(a3_HierarchyPose* pose_out, const a3_HierarchyPose* pose0, const a3ui32 numNodes)
{
	if (pose_out && pose0 && numNodes)
	{
		a3hierarchyPoseCopy(pose_out, pose0, numNodes);

		return 1;
	}
	return -1;
}

//Step pose to nearest
inline a3i32 a3hierarchyPoseNearest(a3_HierarchyPose* pose_out, const a3_HierarchyPose* pose0, const a3_HierarchyPose* pose1,
	const a3real parameter, const a3ui32 numNodes)
{
	if (pose_out && pose0 && numNodes)
	{
		//Latter half of keyframe, nearest is the next pose (pose1)
		if (parameter > .5)
		{
			//Copy pose
			a3hierarchyPoseCopy(pose_out, pose1, numNodes);
		}
		else
		{
			//Copy pose
			a3hierarchyPoseCopy(pose_out, pose0, numNodes);
		}
		
		return 1;
	}
	return -1;
}

//Smoothstep pose between given poses
inline a3i32 a3hierarchyPoseSmoothstep(a3_HierarchyPose* pose_out, const a3_HierarchyPose* pose0, const a3_HierarchyPose* pose1,
	const a3real parameter, const a3ui32 numNodes)
{
	if (pose_out && pose0 && pose1 && numNodes)
	{
		if (parameter >= 1) //Clamp to pose1
		{
			//Copy pose
			a3hierarchyPoseCopy(pose_out, pose1, numNodes);
		}
		else if (parameter <= 0) //Clamp to pose0
		{
			//Copy pose
			a3hierarchyPoseCopy(pose_out, pose0, numNodes);
		}
		else //Smoothstep between pose0 and pose1
		{
			//For each spatial pose
			for (a3ui32 i = 0; i < numNodes; i++)
			{
				//Translation X
				a3realSmoothstep(&((pose_out->sPoses + i)->translation[0]), (pose0->sPoses + i)->translation[0],
					(pose1->sPoses + i)->translation[0], parameter);

				//Translation Y
				a3realSmoothstep(&((pose_out->sPoses + i)->translation[1]), (pose0->sPoses + i)->translation[1],
					(pose1->sPoses + i)->translation[1], parameter);

				//Translation Z
				a3realSmoothstep(&((pose_out->sPoses + i)->translation[2]), (pose0->sPoses + i)->translation[2],
					(pose1->sPoses + i)->translation[2], parameter);


				//Rotation X
				a3realSmoothstep(&((pose_out->sPoses + i)->rotation[0]), (pose0->sPoses + i)->rotation[0],
					(pose1->sPoses + i)->rotation[0], parameter);

				//Rotation Y
				a3realSmoothstep(&((pose_out->sPoses + i)->rotation[1]), (pose0->sPoses + i)->rotation[1],
					(pose1->sPoses + i)->rotation[1], parameter);

				//Rotation Z
				a3realSmoothstep(&((pose_out->sPoses + i)->rotation[2]), (pose0->sPoses + i)->rotation[2],
					(pose1->sPoses + i)->rotation[2], parameter);


				//Scale X
				a3realSmoothstep(&((pose_out->sPoses + i)->scale[0]), (pose0->sPoses + i)->scale[0],
					(pose1->sPoses + i)->scale[0], parameter);

				//Scale Y
				a3realSmoothstep(&((pose_out->sPoses + i)->scale[1]), (pose0->sPoses + i)->scale[1],
					(pose1->sPoses + i)->scale[1], parameter);

				//Scale Z
				a3realSmoothstep(&((pose_out->sPoses + i)->scale[2]), (pose0->sPoses + i)->scale[2],
					(pose1->sPoses + i)->scale[2], parameter);
			}
		}

		return 1;
	}
	return -1;
}

//Executes smoothstep and returns value in val_out
inline a3i32 a3realSmoothstep(a3real* val_out, const a3real val0, const a3real val1, const a3real parameter)
{
	if (val_out)
	{
		//Smoothstep is just lerp with a (clamped) smoothstep function in the parameter
		*val_out = val0 + ((val1 - val0) * a3clamp(0, 1, (parameter * parameter * (3 - (2 * parameter))))); //(-2x^3 + 3x^2)

		return 1;
	}
	return -1;
}

//Add translation, add rotation, multiply scale, applies delta pose changes to base pose and returns pose_out
inline a3i32 a3hierarchyPoseConcat(a3_HierarchyPose* pose_out, const a3_HierarchyPose* basePose,
	const a3_HierarchyPose* deltaPose, const a3ui32 numNodes)
{
	if (pose_out && basePose && deltaPose && numNodes)
	{
		//Copy each spatial pose
		for (a3ui32 i = 0; i < numNodes; i++)
		{
			//Modify values not addresses, dereference
			
			//Add translation
			a3spatialPoseSetTranslation(&(pose_out->sPoses[i]),
				(basePose->sPoses[i]).translation[0] + (deltaPose->sPoses[i]).translation[0],
				(basePose->sPoses[i]).translation[1] + (deltaPose->sPoses[i]).translation[1],
				(basePose->sPoses[i]).translation[2] + (deltaPose->sPoses[i]).translation[2]
			);
			//Add rotation
			a3spatialPoseSetRotation(&(pose_out->sPoses[i]),
				(basePose->sPoses[i]).rotation[0] + (deltaPose->sPoses[i]).rotation[0],
				(basePose->sPoses[i]).rotation[1] + (deltaPose->sPoses[i]).rotation[1],
				(basePose->sPoses[i]).rotation[2] + (deltaPose->sPoses[i]).rotation[2]
			);
			//Multiply scale
			a3spatialPoseSetScale(&(pose_out->sPoses[i]),
				(basePose->sPoses[i]).scale[0] * (deltaPose->sPoses[i]).scale[0],
				(basePose->sPoses[i]).scale[1] * (deltaPose->sPoses[i]).scale[1],
				(basePose->sPoses[i]).scale[2] * (deltaPose->sPoses[i]).scale[2]
			);
		}

		return 1;
	}
	return -1;
}


//Prints out all spatial pose data
inline a3i32 a3hierarchyPosePrint(a3_HierarchyPose* pose, const a3ui32 numNodes)
{
	if (pose && numNodes)
	{
		//For each spatial pose
		for (a3ui32 i = 0; i < numNodes; i++)
		{
			//Pretty print
			printf("\nSpatial Pose\nTransform:\n%f  %f  %f  %f\n%f  %f  %f  %f\n%f  %f  %f  %f\n%f  %f  %f  %f\n\nTranslation: (%f, %f, %f)\n\nRotation: (%f, %f, %f)\n\nScale: (%f, %f, %f)\n\n",
				pose->sPoses[i].transform.x0, pose->sPoses[i].transform.x1, pose->sPoses[i].transform.x2, pose->sPoses[i].transform.x3,
				pose->sPoses[i].transform.y0, pose->sPoses[i].transform.y1, pose->sPoses[i].transform.y2, pose->sPoses[i].transform.y3,
				pose->sPoses[i].transform.z0, pose->sPoses[i].transform.z1, pose->sPoses[i].transform.z2, pose->sPoses[i].transform.z3,
				pose->sPoses[i].transform.w0, pose->sPoses[i].transform.w1, pose->sPoses[i].transform.w2, pose->sPoses[i].transform.w3,
				pose->sPoses[i].translation[0], pose->sPoses[i].translation[1], pose->sPoses[i].translation[2],
				pose->sPoses[i].rotation[0], pose->sPoses[i].rotation[1], pose->sPoses[i].rotation[2],
				pose->sPoses[i].scale[0], pose->sPoses[i].scale[1], pose->sPoses[i].scale[2]
				);
		}

		return 1;
	}
	return -1;
}


//-----------------------------------------------------------------------------

// update inverse object-space matrices
inline a3i32 a3hierarchyStateUpdateObjectInverse(const a3_HierarchyState *state)
{
	if (state && state->inverseObjectSpace && state->objectSpace)
	{
		//Loop through all spatial poses and get inverse
		for (a3ui32 i = 0; i < state->hierarchy->numNodes; i++)
		{
			a3real4x4GetInverse(
				state->inverseObjectSpace->sPoses[i].transform.m, 
				state->objectSpace->sPoses[i].transform.m
			);
		}

		return 1;
	}
	return -1;
}

//Update objectBindToCurrent matrix
inline a3i32 a3hierarchyStateUpdateObjectBindToCurrent(a3_HierarchyState* activeHS, const a3_HierarchyState* baseHS)
{
	if (activeHS && baseHS)
	{
		//Loop through all spatial poses and get delta spatial pose transform * inverse base pose transform
		for (a3ui32 i = 0; i < baseHS->hierarchy->numNodes; i++)
		{
			a3real4x4Product(
				activeHS->objectSpaceBindToCurrent->sPoses[i].transform.m, 
				activeHS->objectSpace->sPoses[i].transform.m,
				baseHS->inverseObjectSpace->sPoses[i].transform.m
				);
		}

		return 1;
	}
	return -1;
}


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------


#endif	// !__ANIMAL3D_HIERARCHYSTATE_INL
#endif	// __ANIMAL3D_HIERARCHYSTATE_H