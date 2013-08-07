#include <rose.h>
#include <deque>
#include <set>

using namespace std;
using namespace SageInterface;
using namespace OmpSupport;

namespace FindSharedVariable {

// scopeStack to track current scope history
// maintained as deque, used like a stack.
// each scopeStack element has either scope statement or omp body statement.
deque< pair<SgScopeStatement *, SgOmpClauseBodyStatement*> > scopeStack;
deque< pair<SgScopeStatement *, SgOmpClauseBodyStatement*> >::iterator scopeStackIt;


static SgStatement *reservedNextStmt;	// reserved next statement to handle for statement
static bool justAfterOmpFor;	// when omp for is visited, next for statement will not be added to stack.
// This is related to how Rose OpenMP implementation deals with for statement.
// If we push for statement after omp for, this will make error when we call getNextStaement with for statement.
static SgIfStmt *currentIfStmt;
static int currentIfStmtPhase;	// trakcing if statement processing


SgStatement* getSrcLevelFirstStatement(SgStatement* stmt)
{
	SgStatement *resultStmt;
	SgScopeStatement *scopeStmt;
	SgOmpParallelStatement *scopeParallelStmt;
	SgOmpForStatement *scopeForStmt;
	pair< SgScopeStatement*, SgOmpClauseBodyStatement * > element;

	// check if statement is not a real expression statement
	scopeStmt = isSgScopeStatement(stmt);
	scopeParallelStmt = isSgOmpParallelStatement(stmt);
	scopeForStmt = isSgOmpForStatement(stmt);

	// if given stmt is not scope, omp parallel, omp for,
	// this is not what expected.
	// [TODO] omp section needs to be added.
	if ((scopeStmt == NULL) && (scopeParallelStmt == NULL) && (scopeForStmt == NULL)) {
		cout << "[Error] in getSrcLevelFirstStatement: input parameter is not a scope statement." << endl;
		return NULL;
	}

	resultStmt = stmt;	// use resultStmt as intermediate stmt for processing
	while (1) {
		if (scopeStmt) {
			//cout << "scope stmt:" << scopeStmt->class_name() << endl;
			SgForStatement *forStmt;
			forStmt = isSgForStatement(resultStmt);
			if (forStmt) {
				// cout << "\tfor" << endl;
				// cout << "\t\t" << forStmt->get_for_init_stmt()->unparseToString();

				if (justAfterOmpFor == false) {
					// only when for statement comes not after omp for statement, we push this scope.
					element.first = scopeStmt;
					element.second = NULL;
					scopeStack.push_back(element);
					//cout << "\tpush for" << endl;
				}
				else {
					// [TODO] may need to check this works okay
					justAfterOmpFor = false;
				}

				// current 'nextStmt' is for init statement,
				// and next 'nextStmt' is set to the first statement in for body.
				reservedNextStmt = getFirstStatement(scopeStmt, false);
				resultStmt = forStmt->get_for_init_stmt();
				return resultStmt;
			}
			// [TODO] need to add support for while
			else {
				resultStmt = getFirstStatement(scopeStmt, false);
			}
			/*
			resultStmt = getFirstStatement(scopeStmt, false);
			cout << "first stmt (scope): " << resultStmt->unparseToString() << endl << endl << endl;
			return resultStmt;
			*/
		}
		else if (scopeParallelStmt) {		// when omp parallel
			// [TODO] need to check if this work fine
			/*
			element.first = NULL;
			element.second = scopeParallelStmt;
			scopeStack.push_back(element);
			*/
			//cout << "\tpush omp parallel" << endl;

			resultStmt = scopeParallelStmt->get_body();
		}
		else if (scopeForStmt) {			// when omp for
			element.first = NULL;
			element.second = scopeForStmt;
			scopeStack.push_back(element);

			/*
			SgForStatement *forStmt;
			forStmt = isSgForStatement(scopeForStmt->get_body());
			if (forStmt) {
				cout << "\tomp for" << endl;
				cout << "\t\t" << forStmt->get_for_init_stmt()->unparseToString() << endl;

				reservedNextStmt = getFirstStatement(forStmt, false);
				resultStmt = forStmt->get_for_init_stmt();
				return resultStmt;
			}
			*/

			justAfterOmpFor = true;
			//cout << "\tjust after omp for is turned on." << endl;
			//cout << "\tpush omp for (now=" << scopeStack.size() << ")" << endl;

			// next statement to omp for cannot be found with getFirst or getNextStatement.
			// we need to use get_traversalSuccssorContainer to find next statement in Src level.
			vector<SgNode *> nextOne;
			nextOne = scopeForStmt->get_traversalSuccessorContainer();
			resultStmt = isSgStatement(nextOne[0]);
			//cout << "omp for's next stmt is " << resultStmt->unparseToString() << endl;
		}
		else {
			// not scope statement (including for statement), not omp parallel, not omp for
			// Then, this is the first real statement that we are looking for.
			return resultStmt;
		}

		// check next statement
		scopeStmt = isSgScopeStatement(resultStmt);
		scopeParallelStmt = isSgOmpParallelStatement(resultStmt);
		scopeForStmt = isSgOmpForStatement(resultStmt);
	}
}


SgStatement* getSrcLevelNextStatement(SgStatement *stmt)
{
	SgStatement *nextStmt;
	SgExpression *nextExpr;
	pair< SgScopeStatement*, SgOmpClauseBodyStatement * > element;

	if (reservedNextStmt) {
		nextStmt = reservedNextStmt;
		reservedNextStmt = NULL;
	}
	else if (currentIfStmtPhase > 0) {		
		if (currentIfStmtPhase == 1) {
			nextStmt = currentIfStmt->get_true_body();
			if (currentIfStmt->get_false_body()) {
				currentIfStmtPhase = 2;
			}
			else {
				currentIfStmtPhase = 3;
			}
		}
		else if (currentIfStmtPhase == 2) {
			nextStmt = currentIfStmt->get_false_body();
		}
		else if (currentIfStmtPhase == 3) {
			currentIfStmtPhase = 0;
			currentIfStmt = NULL;
			nextStmt = getNextStatement(scopeStack.back().first);
			scopeStack.pop_back();
		}
	}
	else {
		cout << "cur stmt: " << stmt->unparseToString() << endl;
		nextStmt = getNextStatement(stmt);
		if (nextStmt)
			cout << "next stmt: " << nextStmt->unparseToString() << endl;
	}

	// if next statement is null, try to find pushed scope in the stack
	if (nextStmt == NULL) {
		if (scopeStack.size() > 0) {
			do {
				//cout << "checkpoint" << endl;
				// when we come to the last
				if (scopeStack.size() == 0)
					return nextStmt;

				if (scopeStack.back().first) {
					//cout << "checkpoint 1" << endl;
					cout << scopeStack.back().first->unparseToString() << endl;
					SgScopeStatement *scope;
					scope = getScope(scopeStack.back().first);
					//cout << "scope is " << scope->unparseToString() << endl;
					nextStmt = getNextStatement(scopeStack.back().first);
					if (!nextStmt)
						cout << "nextStmt is null again." << endl;
				}
				else {
					//cout << "checkpoint 2" << endl;
					nextStmt = getNextStatement(scopeStack.back().second);
				}
				scopeStack.pop_back();
				//cout << "\tpop (now=" << scopeStack.size() << ")" << endl;
			} while (nextStmt == NULL);
		}
		else
			return nextStmt;	// null
	}

/*
	while (1) {
		SgOmpParallelStatement *ompParStmt;
		ompParStmt = isSgOmpParallelStatement(nextStmt);
		SgOmpForStatement *ompForStmt;
		ompForStmt = isSgOmpForStatement(nextStmt);

		if (ompParStmt) {
			cout << "\tskip omp parallel" << endl;
			vector<SgNode *> nextOne;
			nextOne = ompParStmt->get_traversalSuccessorContainer();
			nextStmt = isSgStatement(nextOne[0]);
		}
		else if (ompForStmt) {
			cout << "\tskip omp for" << endl;
			vector<SgNode *> nextOne;
			nextOne = ompForStmt->get_traversalSuccessorContainer();
			nextStmt = isSgStatement(nextOne[0]);
		}
		else
			break;
	}
*/

	cout << "next stmt class_name:" << nextStmt->class_name() << endl << endl;
	
	SgScopeStatement *nextScope;
	nextScope = isSgScopeStatement(nextStmt);

	SgOmpParallelStatement *ompParallelStmt;
	ompParallelStmt = isSgOmpParallelStatement(nextStmt);
	SgOmpForStatement *ompForStmt;
	ompForStmt = isSgOmpForStatement(nextStmt);
	//cout << " scope: " << (nextScope != NULL);
	//cout << " omp parallel: "<< (ompParallelStmt != NULL);
	//cout << " omp for: " << (ompForStmt != NULL) << endl;
	if (nextScope || ompParallelStmt || ompForStmt) {
		// [TODO] sections need to be concerned.
		//cout << "\tpush scope" << endl;

		if (ompParallelStmt) {
			element.first = NULL;
			element.second = ompParallelStmt;
			scopeStack.push_back(element);
			//cout << "\tpush (now=" << scopeStack.size() << ")" << endl;
		}
		else if (ompForStmt) {
			element.first = NULL;
			element.second = ompForStmt;
			scopeStack.push_back(element);
			//cout << "\tpush omp for" << endl;
			justAfterOmpFor = true;
			//cout << "\tjust after omp for is turned on." << endl;
			//cout << "\tpush (now=" << scopeStack.size() << ")" << endl;

			vector<SgNode *> nextOne;
			nextOne = ompForStmt->get_traversalSuccessorContainer();
			nextScope = isSgScopeStatement(nextOne[0]);
			nextStmt = isSgStatement(nextOne[0]);
			//cout << "omp for's next stmt is " << nextStmt->unparseToString() << endl;
		}
		else if (justAfterOmpFor == false) {
			element.first = nextScope;
			element.second = NULL;
			scopeStack.push_back(element);
			//cout << "\tpush normal" << endl;
			//cout << "normal scope: " << nextScope->unparseToString() << endl;
			//cout << "\tpush (now=" << scopeStack.size() << ")" << endl;
		}
		else {
			// this must be for statement with justAfterOmpFor.
			justAfterOmpFor = true;
			//cout << "\t not push because this is just after omp for" << endl;
			element.first = nextScope;
			element.second = NULL;
			scopeStack.push_back(element);
			//cout << "\tpush normal even though just after omp for" << endl;
			//cout << "normal scope: " << nextScope->unparseToString() << endl;
			//cout << "\tpush (now=" << scopeStack.size() << ")" << endl;
		}
		//cout << "\t" << nextScope->unparseToString() << endl;

		SgIfStmt *ifStmt;
		ifStmt = isSgIfStmt(nextStmt);
		if (ifStmt) {
			nextStmt = ifStmt->get_conditional();
			currentIfStmt = ifStmt;
			currentIfStmtPhase = 1;
			return nextStmt;

			//nextStmt = ifStmt->get_true_body();
			reservedNextStmt = ifStmt->get_true_body();
			//cout << "\ttrue: " << nextStmt->unparseToString() << endl;
			nextStmt = ifStmt->get_false_body();
			if (nextStmt)
				cout << "\tif false body found: " << nextStmt->unparseToString() << endl;
			
			nextStmt = getNextStatement(scopeStack.back().first);
			scopeStack.pop_back();
			return nextStmt;
		}

		SgForStatement *forStmt;
		forStmt = isSgForStatement(nextStmt);
		if (forStmt) {
			//cout << "\tfor" << endl;
			nextStmt = forStmt->get_for_init_stmt();
			//cout << "\t\t" << nextStmt->unparseToString();
			nextStmt = forStmt->get_test();
			//cout << "\t" << nextStmt->unparseToString();
			nextExpr = forStmt->get_increment();
			//cout << "\t" << nextExpr->unparseToString() << endl;

			reservedNextStmt = getFirstStatement(nextScope, false);
			//cout << "first stmt (scope)2: " << nextScope->unparseToString() << endl;
			nextStmt = forStmt->get_for_init_stmt();
			return nextStmt;
		}

		nextStmt = getFirstStatement(nextScope, false);
		//cout << "first stmt (scope)3: " << nextScope->unparseToString() << endl;
	}

	return nextStmt;
}


}	// end of namespace FindSharedVariable

using namespace FindSharedVariable;


int main(int argc, char *argv[]) 
{
	SgProject *project = frontend(argc, argv);

	// 1. For main function
	//SgFunctionDeclaration *mainFunc = findMain(project);
	//SgFunctionDeclaration *mainFunc = findFunctionDeclaration(project, "jacobi", NULL, true);
	//SgBasicBlock *mainFuncBody = mainFunc->get_definition()->get_body();

	// 2. For Omp Parallel
	Rose_STL_Container<SgNode *> nodeList = NodeQuery::querySubTree(project, V_SgOmpParallelStatement);
	Rose_STL_Container<SgNode *>::iterator it;

	it = nodeList.begin();
	SgOmpParallelStatement *ompParStmt;
	ompParStmt = isSgOmpParallelStatement(*it);
	SgStatement *bodyStmt = ompParStmt->get_body();
	SgBasicBlock *ompParallelBody;
	ompParallelBody = isSgBasicBlock(bodyStmt);

	// 3. For Omp for
	/*
	Rose_STL_Container<SgNode *> nodeList = NodeQuery::querySubTree(project, V_SgOmpForStatement);
	Rose_STL_Container<SgNode *>::iterator it;

	it = nodeList.begin();
	it++;
	SgOmpForStatement *ompForStmt;
	ompForStmt = isSgOmpForStatement(*it);
	SgStatement *bodyStmt = ompForStmt->get_body();
	if (ompForStmt == NULL)
		cout << "omp for stmt is null." << endl;
	if (bodyStmt == NULL)
		cout << "body stmt is null." << endl;
	cout << bodyStmt->unparseToString() << endl;
	*/

	SgStatement *stmt;
	// 1. main func
	//cout << "jacobi body" << endl;
	//stmt = getSrcLevelFirstStatement(mainFuncBody);
	// 2. omp parallel body
	cout << "omp parallel body is: " << endl;
	//cout << ompParallelBody->unparseToString() << endl << endl << endl;
	//stmt = getSrcLevelFirstStatement(ompParallelBody);
	cout << ompParStmt->unparseToString() << endl << endl << endl;
	stmt = getSrcLevelFirstStatement(ompParStmt);
	// 3. omp for body: this is not possible
	//cout << "omp for body is: " << endl;
	//cout << bodyStmt->unparseToString() << endl << endl << endl;
	//stmt = getSrcLevelFirstStatement(bodyStmt);
	// 3. omp for
	//cout << "omp for is: " << endl;
	//cout << ompForStmt->unparseToString() << endl << endl << endl;
	//stmt = getSrcLevelFirstStatement(ompForStmt);


	int i = 1;
	set<SgInitializedName *> readVars;
	set<SgInitializedName *> writeVars;
	bool result;
	Rose_STL_Container<SgInitializedName *> privateClauses;
	Rose_STL_Container<SgInitializedName *>::iterator privateClausesIt;
	set<SgInitializedName *> privateClausesSet;
	set<SgInitializedName *>::iterator privateClausesSetIt;

	set<SgInitializedName *> sharedReadSet;
	set<SgInitializedName *>::iterator sharedReadSetIt;
	set<SgInitializedName *> sharedWriteSet;
	set<SgInitializedName *>::iterator sharedWriteSetIt;

	while (stmt) {
		cout << endl << endl;
		cout << "[" << i++ << "]" << stmt->unparseToString() << endl;
		readVars.clear();
		writeVars.clear();

		Rose_STL_Container<SgNode *> nodeList = NodeQuery::querySubTree(stmt, V_SgFunctionCallExp);
		if (nodeList.size() > 0) {
			cout << "statement has function call. skip this statement." << endl;
			cout << endl;
			stmt = getSrcLevelNextStatement(stmt);
			continue;
		}
			
		result = collectReadWriteVariables(stmt, readVars, writeVars);
		SgGlobal *globalScope = getGlobalScope(stmt);
		SgFunctionDefinition *functionDefScope = getEnclosingFunctionDefinition(stmt);
		SgFunctionDeclaration *functionDeclScope = getEnclosingFunctionDeclaration(stmt);
		SgBasicBlock *functionBBScope = functionDefScope->get_body();

		// read through scopeStack to find private variable list
		// [TODO] need to add firstprivate, lastprivate, threadprivate
		for (scopeStackIt = scopeStack.begin(); scopeStackIt != scopeStack.end(); ++scopeStackIt)
		{
			if (scopeStackIt->second) {
				privateClauses = collectClauseVariables(scopeStackIt->second, V_SgOmpPrivateClause);
				for (privateClausesIt = privateClauses.begin(); privateClausesIt != privateClauses.end(); ++privateClausesIt) 
					privateClausesSet.insert(*privateClausesIt);
			}
		}

		//for (privateClausesSetIt = privateClausesSet.begin(); privateClausesSetIt != privateClausesSet.end(); ++privateClausesSetIt)
		//	cout << "private: " << (*privateClausesSetIt)->get_name().getString() << endl;

		if (result == true) {
			cout << "read variables:   ";
			for (set<SgInitializedName *>::iterator it = readVars.begin(); it != readVars.end(); ++it)
			{
				SgScopeStatement *currentScope = (*it)->get_scope();

				if (privateClausesSet.find(*it) != privateClausesSet.end()) {
					//cout << "private variable " << (*it)->get_name().getString() << " detected" << endl;
					continue;
				}
						
				if (currentScope == (SgScopeStatement*) globalScope)
					cout << (*it)->get_name().getString() << " in (global)" << endl;
				else if (currentScope == (SgScopeStatement*) functionBBScope)
					cout << (*it)->get_name().getString() << " in (function)" << endl;
				else
					cout << (*it)->get_name().getString() << " in (" << (*it)->get_scope()->unparseToString() << ")" << endl;

				sharedReadSet.insert(*it);
			}
			cout << endl;

			cout << "write variables:  ";
			for (set<SgInitializedName *>::iterator it = writeVars.begin(); it != writeVars.end(); ++it)
			{
				SgScopeStatement *currentScope = (*it)->get_scope();
				if (currentScope == (SgScopeStatement*) globalScope)
					cout << (*it)->get_name().getString() << " in (global)" << endl;
				else if (currentScope == (SgScopeStatement*) functionBBScope)
					cout << (*it)->get_name().getString() << " in (function)" << endl;
				else
					cout << (*it)->get_name().getString() << " in (" << (*it)->get_scope()->unparseToString() << ")" << endl;

				sharedWriteSet.insert(*it);
			}
			cout << endl;
		} 
		cout << endl << endl;
		stmt = getSrcLevelNextStatement(stmt);
	}


	cout << "stack size = " << scopeStack.size() << endl;

	cout << "-- shared read only set --" << endl;
	for (sharedReadSetIt = sharedReadSet.begin(); sharedReadSetIt != sharedReadSet.end(); ++sharedReadSetIt)
	{
		if (sharedWriteSet.find(*sharedReadSetIt) == sharedWriteSet.end())
			cout << (*sharedReadSetIt)->get_name().getString() << endl;	
	}
	cout << "-- shared read set --" << endl;
	for (sharedReadSetIt = sharedReadSet.begin(); sharedReadSetIt != sharedReadSet.end(); ++sharedReadSetIt)
	{
		cout << (*sharedReadSetIt)->get_name().getString() << endl;	
	}
	cout << "-- shared write set --" << endl;
	for (sharedWriteSetIt = sharedWriteSet.begin(); sharedWriteSetIt != sharedWriteSet.end(); ++sharedWriteSetIt)
	{
		cout << (*sharedWriteSetIt)->get_name().getString() << endl;	
	}
	return 0;
}


