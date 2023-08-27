#include "Print.h"
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

