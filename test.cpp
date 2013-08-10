//In the name of God



#include <iostream>
#include <map>
#include <list>
#include <set>


using namespace std;


class A{
public:
	A(){
		cout << "AAA" << endl;
	}
	virtual ~A(){
		cout << "~~AA" << endl;
	}
};

class B: public A{
public:
	B():A(){
		cout << "BBB" << endl;
	}
	virtual ~B(){
		cout << "~~BB" << endl;
	}
};





int main(){
	set <int> a;
	for (int i = 1; i < 10; i++){
		a.insert(i);
	}	

	set <int> b;
	b = a;
	a.insert(100);
	set <int>::iterator it1, itEnd1;
	it1 = a.begin();
	itEnd1 = a.end();
	for (; it1 != itEnd1; ++it1){
		cout << "a " << *it1 << endl;
	}
	it1 = b.begin();
	itEnd1 = b.end();
	for (; it1 != itEnd1; ++it1){
		cout << "b " << *it1 << endl;
	}	
	return 0;
}
