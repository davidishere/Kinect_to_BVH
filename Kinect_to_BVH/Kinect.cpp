#include <windows.h>
#include <NuiApi.h>
#include <stddef.h>
#include <stdio.h>
#include "Kinect.h"

/*******����һЩ�ڲ�ʹ�õ�ȫ�ֱ���*******/
static HANDLE skeletonEvent;
static UserList myUserList;
static NUI_SKELETON_FRAME skeletonFrame = {};  //����֡�Ķ��� 
/*************************************/

int KinectInit(){
	HRESULT hr;
	//1����ʼ��NUI��ע��������USES_SKELETON
	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
	if (FAILED(hr))
	{
		printf("Kinect Initialize failed\n");
		return -1;
	}

	//2����������ź��¼���� 
	skeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//3���򿪹��������¼�
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
	WaitForSingleObject(skeletonEvent, INFINITE);    //���޵ȴ��µ����ݣ��ȵ������
	if (!SUCCEEDED(NuiSkeletonGetNextFrame(0, &skeletonFrame)))
		return NULL;    //�õ�����֡ ������6���Ǽܣ�
	// ��ʱ��������Ƿ�����֡
	//	if (skeletonFrame.liTimeStamp  .getTimestamp() - fr_timestamp>40000)
	//		printf("������֡��\n");
	//�˴�δ���
	NuiTransformSmooth(&skeletonFrame, NULL);    //ƽ������֡

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
	(tmp).loc_z = psk->Position.z;//�õ��Ǽ�����
	// ��д�����ڵ�λ����Ϣ
	char* tp = (char*)(&tmp) + offsetof(UserSkeleton, hip_center);
	struct _point * pSkPoint = (struct _point *)tp;
	for (int i = 0; i < 20; i++){
		pSkPoint->loc_state = psk->eSkeletonPositionTrackingState[i];
		pSkPoint->loc_x = psk->SkeletonPositions[i].x;
		pSkPoint->loc_y = psk->SkeletonPositions[i].y;
		pSkPoint->loc_z = psk->SkeletonPositions[i].z;
		pSkPoint++;
	}
	// ����ڵ���ת��
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
