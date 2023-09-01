#include "Print.h"
using namespace std;
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

void Print(Eigen::Vector3f v)
{
    cout << "Eigen::Vector3f" << endl;
    for (int i = 0; i < 3; i++)
        cout << v(i) << "\t";
    cout << endl;
}

