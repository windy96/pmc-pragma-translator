#include <rose.h>

using namespace std;
using namespace SageInterface;
//using namespace AstFromString;


class MyVisitor: public AstSimpleProcessing {
	protected:
		void virtual visit(SgNode* node);
};


void MyVisitor::visit(SgNode* node) {
	cout << node->class_name() << endl;
}

int main(int argc, char *argv[]) {
	SgProject *astNode = frontend(argc, argv);
	MyVisitor v;
	v.traverseInputFiles(astNode, preorder);
}

