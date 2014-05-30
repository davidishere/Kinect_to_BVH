#include "Kinect.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	KinectInit();
	while (1){
		UserList* p = getUserList();
		if (p){
			for (int i = 0; i < 6; i++){
				User* q = getTrackedUser(p, i);
				if (q){
					UserSkeleton a = getUserSkeleton(q);
					printf("--%f--\t", a.hip_center.loc_x);
					printf("--%f--\t", a.hip_center.loc_y);
					printf("--%f--\t\n", a.hip_center.loc_z);
					printf("--%f--\t", a.spine.absoluteRotation_x);
					printf("--%f--\t", a.spine.absoluteRotation_y);
					printf("--%f--\t\n", a.spine.absoluteRotation_z);
				}
			}
		}
	}
	KinectDestroy();
	return 0;
}