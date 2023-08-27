#include "Print.h"
void Print(glm::mat4 m)
{
	cout << "glm::mat4" << endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << setprecision(2)<< m[i][j] << "\t";
		}
		cout << endl;
	}
	cout << endl;
}

void Print(glm::vec3 v)
{
	cout << "glm::vec3" << endl;
	cout << v.x << "\t" << v.y << "\t" << v.z << "\t" <<  endl;
}

void Print(Eigen::Matrix4f mat)
{
    cout << "Eigen::Matrix4f" << endl;
    for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << setprecision(2)<< mat(i,j) << "\t";
		}
		cout << endl;
	}
	cout << endl;

    
}

