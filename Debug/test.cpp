//In the name of God


#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

class MySomethign {
public:
	void test (int a){
		cout << a << endl;
	}
	void test2(){
		vector <int> b;
		for (int i = 0; i < 10; i++){
			b.push_back(i);
		}
		for_each (b.begin(), b.end(), this->test);
	}
};


int main(){
	MySomethign c;
	c.test2();
	return 0;
}

