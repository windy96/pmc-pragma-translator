#include <rose.h>
#include <string.h>

using namespace std;
using namespace SageInterface;
//using namespace AstFromString;


class MyVisitor: public AstPrePostProcessing {
	protected:
		void virtual preOrderVisit(SgNode* node);
		void virtual postOrderVisit(SgNode* node);
};


void MyVisitor::preOrderVisit(SgNode* node) {
	cout << node->class_name() << endl;
        if (!strcmp(node->class_name().c_str(), "SgVarRefExp")) {
	        cout << "number= " << node->variantT() << endl;
		cout << V_SgVarRefExp << endl;
	}
}

void MyVisitor::postOrderVisit(SgNode* node) {
}

int main(int argc, char *argv[]) {
	SgProject *astNode = frontend(argc, argv);
	MyVisitor v;
	v.traverseInputFiles(astNode);
}

