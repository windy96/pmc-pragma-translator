#include <rose.h>
#include <stack>
#include <set>

using namespace std;
using namespace SageInterface;


stack<SgScopeStatement *> scopeStack;

SgStatement* getSrcLevelFirstStatement(SgScopeStatement* scope)
{
	//scopeStack.push(scope);
	SgStatement *stmt;
	stmt = getFirstStatement(scope, false);
	// if stmt is scope statement,

	return stmt;
}

SgStatement* getSrcLevelNextStatement(SgStatement *stmt)
{
	SgStatement *nextStmt;
	SgExpression *nextExpr;
	static SgStatement *reservedNextStmt;

	if (reservedNextStmt) {
		nextStmt = reservedNextStmt;
		reservedNextStmt = NULL;
	}
	else
		nextStmt = getNextStatement(stmt);

	// if next statement is null, try to find pushed scope in the stack
	if (nextStmt == NULL) {
		if (scopeStack.size() > 0) {
			nextStmt = getNextStatement(scopeStack.top());
			scopeStack.pop();
			cout << "\tpop" << endl;
		}
		else
			return nextStmt;	// null
	}
	
	SgScopeStatement *nextScope;
	nextScope = isSgScopeStatement(nextStmt);
	if (nextScope) {
		cout << "\tpush" << endl;
		scopeStack.push(nextScope);
		//cout << "\t" << nextScope->unparseToString() << endl;

		SgIfStmt *ifStmt;
		ifStmt = isSgIfStmt(nextStmt);
		if (ifStmt) {
			nextStmt = ifStmt->get_conditional();
			cout << "\t\t" << nextStmt->unparseToString() << endl;
			nextStmt = ifStmt->get_true_body();
			cout << "\t" << nextStmt->unparseToString() << endl;
			nextStmt = ifStmt->get_false_body();
			cout << "\t" << nextStmt->unparseToString() << endl;
			
			nextStmt = getNextStatement(scopeStack.top());
			scopeStack.pop();
			return nextStmt;
		}

		SgForStatement *forStmt;
		forStmt = isSgForStatement(nextStmt);
		if (forStmt) {
			cout << "\tfor" << endl;
			nextStmt = forStmt->get_for_init_stmt();
			cout << "\t\t" << nextStmt->unparseToString();
			nextStmt = forStmt->get_test();
			cout << "\t" << nextStmt->unparseToString();
			nextExpr = forStmt->get_increment();
			cout << "\t" << nextExpr->unparseToString() << endl;

			reservedNextStmt = getFirstStatement(nextScope, false);
			nextStmt = forStmt->get_for_init_stmt();
			return nextStmt;
		}

		nextStmt = getFirstStatement(nextScope, false);
	}

	return nextStmt;
}

int main(int argc, char *argv[]) 
{
	SgProject *project = frontend(argc, argv);

	SgFunctionDeclaration *mainFunc = findMain(project);
	SgBasicBlock *mainFuncBody = mainFunc->get_definition()->get_body();

	SgStatement *stmt;
	stmt = getSrcLevelFirstStatement(mainFuncBody);

	int i = 1;
	set<SgInitializedName *> readVars;
	set<SgInitializedName *> writeVars;
	bool result;

	while (stmt) {
		cout << "[" << i++ << "]" << stmt->unparseToString() << endl;

		result = collectReadWriteVariables(stmt, readVars, writeVars);

		if (result == true) {
			cout << "read variables" << endl;
			for (set<SgInitializedName *>::iterator it = readVars.begin(); it != readVars.end(); ++it)
			{
				cout << (*it)->get_name().getString() << endl;
			}

			cout << "write variables" << endl;
			for (set<SgInitializedName *>::iterator it = writeVars.begin(); it != writeVars.end(); ++it)
			{
				cout << (*it)->get_name().getString() << endl;
			}
		} 
		stmt = getSrcLevelNextStatement(stmt);
	}

	return 0;
}
