
#include "MyAtom.h"
#include "Utilities.h"
using namespace mdbr;


void MyProposition::write(ostream &sout){
	originalLiteral->write(sout);
}

