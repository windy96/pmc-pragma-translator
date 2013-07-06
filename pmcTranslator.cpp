// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jun 20, 2013

#include <rose.h>
#include "DefUseAnalysis.h"
#include <string>
#include <iostream>
#include "pmcSupport.h"


using namespace std;
using namespace PMCSupport;
using namespace SageInterface;
using namespace SageBuilder;
using namespace AstFromString;


int main(int argc, char *argv[])
{
	// Build the AST used by ROSE
	SgProject *project = frontend(argc, argv);

	// Step 1. Query 
	// [TODO] separate this routine to a function
	Rose_STL_Container<SgNode *> pragmaDeclarationList = NodeQuery::querySubTree(project, V_SgPragmaDeclaration);
	Rose_STL_Container<SgNode *>::iterator pragmaIt;

	for (pragmaIt = pragmaDeclarationList.begin(); pragmaIt != pragmaDeclarationList.end(); ++pragmaIt)
	{
		SgPragmaDeclaration *pDecl;
		pDecl = isSgPragmaDeclaration(*pragmaIt);
		string	key = extractPragmaKeyword(pDecl);
		if ((key != "pmc") && (key != "PMC"))
			continue;

		string	content = pDecl->get_pragma()->get_pragma();
		content = content.substr(4, content.size() - 4);
		cout << "pragma is recognized: " << content << endl;

		SgStatement *pragmaStmt = getEnclosingStatement(pDecl);
		SgStatement *stmt;
		stmt = getNextStatement(pragmaStmt);
		ROSE_ASSERT(stmt);
		while (isSgPragmaDeclaration(stmt)) {
			stmt = getNextStatement(stmt);
			ROSE_ASSERT(stmt);
		}

		cout << "    applied to: " << stmt->unparseToString() << endl;
		PMCPragmaAttribute *pAttribute = parsePMCPragma(pDecl);	
//		if (stmt->attributeExists("PMCAttribute"
		stmt->addNewAttribute("PMCAttribute", pAttribute);
		cout << "    attribute " << pAttribute->toString() << " is added." << endl << endl;
	}



	// 2. 

  // Parse PMC pragmas
  //ParsingTraversal pmcParser;
  //pmcParser.traverseInputFiles(project, preorder);

/*
  ParsingPMCTraversal pmcParser;
  pmcParser.traverseInputFiles(project);

  // Call the Def-Use Analysis
  DFAnalysis* defuse = new DefUseAnalysis(project);
  bool debug = false;
  defuse->run(debug);
*/

  // Transform PMC pragmas into PMC API calls
  /*
  std::vector <SgPragmaDeclaration*>::reverse_iterator listElement;
  for (listElement = pmcPragmaList.rbegin(); listElement != pmcPragmaList.rend(); listElement++) {
    SgPragmaDeclaration* p_decl = *listElement;
    transformPMCPragma(p_decl);
  }
  */

  // Test for consistency
  AstTests::runAllTests(project);

  // Invoke a backend compiler
  return backend(project);
}

