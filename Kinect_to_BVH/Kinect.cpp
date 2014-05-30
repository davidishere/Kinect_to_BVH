#include <windows.h>
#include <NuiApi.h>
#include <stddef.h>
#include <stdio.h>
#include "Kinect.h"

/*******定义一些内部使用的全局变量*******/
static HANDLE skeletonEvent;
static UserList myUserList;
static NUI_SKELETON_FRAME skeletonFrame = {};  //骨骼帧的定义 
/*************************************/

int KinectInit(){
	HRESULT hr;
	//1、初始化NUI，注意这里是USES_SKELETON
	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
	if (FAILED(hr))
	{
		printf("Kinect Initialize failed\n");
		return -1;
	}

	//2、定义骨骼信号事件句柄 
	skeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//3、打开骨骼跟踪事件
	hr = NuiSkeletonTrackingEnable(skeletonEvent, 0);
	if (FAILED(hr))
	{
		printf("Kinect Initialize failed\n");
		NuiShutdown();
		return -1;
	}
	printf("Kinect Initialize Succeed!\n");
	return 0;
}

UserList* getUserList(void){
	static long fr_timestamp;
	WaitForSingleObject(skeletonEvent, INFINITE);    //无限等待新的数据，等到后继续
	if (!SUCCEEDED(NuiSkeletonGetNextFrame(0, &skeletonFrame)))
		return NULL;    //得到骨骼帧 （包含6个骨架）
	// 用时间戳鉴别是否发生跳帧
	//	if (skeletonFrame.liTimeStamp  .getTimestamp() - fr_timestamp>40000)
	//		printf("发生跳帧！\n");
	//此处未完成
	NuiTransformSmooth(&skeletonFrame, NULL);    //平滑骨骼帧

	for (int i = 0; i < 6; i++){
		myUserList.user[i].TrackingState = (int)skeletonFrame.SkeletonData[i].eTrackingState;
		if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
			skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
		{
			myUserList.user[i].SkeletonAvailabel = 1;
			myUserList.user[i].sk = (void*)&skeletonFrame.SkeletonData[i];
		}
		else{
			myUserList.user[i].SkeletonAvailabel = 0;
			myUserList.user[i].sk = NULL;
		}
	}
	return &myUserList;
}

User* getTrackedUser(UserList* pl, int uid){
	if (pl->user[uid].TrackingState == 2 && pl->user[uid].SkeletonAvailabel){
		return &pl->user[uid];
	}
	else
		return NULL;
}

UserSkeleton getUserSkeleton(User* pu){
	UserSkeleton tmp = {};
	NUI_SKELETON_DATA* psk = (NUI_SKELETON_DATA*)pu->sk;
	(tmp).loc_x = psk->Position.x;
	(tmp).loc_y = psk->Position.y;
	(tmp).loc_z = psk->Position.z;//得到骨架坐标
	// 填写骨骼节点位置信息
	char* tp = (char*)(&tmp) + offsetof(UserSkeleton, hip_center);
	struct _point * pSkPoint = (struct _point *)tp;
	for (int i = 0; i < 20; i++){
		pSkPoint->loc_state = psk->eSkeletonPositionTrackingState[i];
		pSkPoint->loc_x = psk->SkeletonPositions[i].x;
		pSkPoint->loc_y = psk->SkeletonPositions[i].y;
		pSkPoint->loc_z = psk->SkeletonPositions[i].z;
		pSkPoint++;
	}
	// 计算节点旋转量
	NUI_SKELETON_BONE_ORIENTATION skeletonBoneOrientation[20] = {};
	int OrientationTransformFlag = NuiSkeletonCalculateBoneOrientations(psk, skeletonBoneOrientation);
	if (OrientationTransformFlag == S_OK){
		pSkPoint = (struct _point *)tp;
		pSkPoint->rot_state = 1;
		for (int i = 0; i < 20; i++){
			pSkPoint->absoluteRotation_x = skeletonBoneOrientation[i].absoluteRotation.rotationQuaternion.x;
			pSkPoint->absoluteRotation_y = skeletonBoneOrientation[i].absoluteRotation.rotationQuaternion.y;
			pSkPoint->absoluteRotation_z = skeletonBoneOrientation[i].absoluteRotation.rotationQuaternion.z;
			pSkPoint->absoluteRotation_w = skeletonBoneOrientation[i].absoluteRotation.rotationQuaternion.w;
			pSkPoint->hierarchicalRotation_x = skeletonBoneOrientation[i].hierarchicalRotation.rotationQuaternion.x;
			pSkPoint->hierarchicalRotation_y = skeletonBoneOrientation[i].hierarchicalRotation.rotationQuaternion.y;
			pSkPoint->hierarchicalRotation_z = skeletonBoneOrientation[i].hierarchicalRotation.rotationQuaternion.z;
			pSkPoint->hierarchicalRotation_w = skeletonBoneOrientation[i].hierarchicalRotation.rotationQuaternion.w;
			pSkPoint++;
		}
	}
	else
		printf("Rotation Calculate Error!");
	return tmp;
}

int KinectDestroy(void){
	NuiShutdown();
	return 0;
}
