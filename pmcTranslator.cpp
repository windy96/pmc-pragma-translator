// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated on Jul 19, 2013

#include "rose.h"
#include <string>
#include <iostream>
#include "pmcSupport.h"
//#include "DefUseAnalysis.h"


using namespace std;
using namespace SageInterface;
using namespace SageBuilder;
using namespace AstFromString;
using namespace PMCSupport;


int main(int argc, char *argv[])
{
	// Build the AST
	SgProject *project = frontend(argc, argv);

	// Step 1. Read PMC pragmas, and convert to AST attributes
	convertPMCPragmasToAttributes(project);

	// Step 2. Check AST attributes for debugging
	CheckerTraversal pmcParser;
	pmcParser.traverseInputFiles(project, preorder);

	// Step 3.
	applyPMCAttributesToStatements(project);

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
	//return backend(project);
	return 0;
}

