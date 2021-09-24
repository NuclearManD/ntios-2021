
#include "ntios_math.h"

double* Quaternion::H_product(double* q, double* r, double* dest) {
	dest[0] = r[0]*q[0]-r[1]*q[1]-r[2]*q[2]-r[3]*q[3];
	dest[1] = r[0]*q[1]+r[1]*q[0]-r[2]*q[3]+r[3]*q[2];
	dest[2] = r[0]*q[2]+r[1]*q[3]+r[2]*q[0]-r[3]*q[1];
	dest[3] = r[0]*q[3]-r[1]*q[2]+r[2]*q[1]+r[3]*q[0];
	
	return dest;
}

Vector3 Quaternion::rotateVector(Vector3& vector) {

	double accelQ[4] = {0, 
		vector.x,
		vector.y,
		vector.z
	};
	double rotQ[4]   = {w, x, y, z};
	double rotNQ[4]  = {w, -x, -y, -z};
	double intermediate[4];
	double output[4];

	H_product(rotQ, accelQ, intermediate);
	H_product(intermediate, rotNQ, output);

	return Vector3(output[1], output[2], output[3]);
}
