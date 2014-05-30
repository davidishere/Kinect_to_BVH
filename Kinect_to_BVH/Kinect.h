#ifndef _Kinect 
#define _Kinect 1

typedef struct _user{
	int TrackingState;
	int SkeletonAvailabel;
	void* sk;
}User;

typedef struct _user_list{
	struct _user user[6];
}UserList;

struct _point{
	int loc_state;
	float loc_x, loc_y, loc_z;
	int rot_state;
	float absoluteRotation_x, absoluteRotation_y, absoluteRotation_z, absoluteRotation_w;
	float hierarchicalRotation_x, hierarchicalRotation_y, hierarchicalRotation_z, hierarchicalRotation_w;
};

typedef struct _user_sk{
	float loc_x, loc_y, loc_z;
	struct _point
		hip_center,
		spine,
		shoulder_center,
		head,
		shoulder_left,
		elbow_left,
		wrist_left,
		hand_left,
		shoulder_right,
		elbow_right,
		wrist_right,
		hand_right,
		hip_left,
		knee_left,
		ankle_left,
		foot_left,
		hip_right,
		knee_right,
		ankel_right,
		foot_right;
}UserSkeleton;

extern int KinectInit(void);
extern UserList* getUserList(void);
extern User* getTrackedUser(UserList*, int);
extern UserSkeleton getUserSkeleton(User*);
extern int KinectDestroy(void);

#endif