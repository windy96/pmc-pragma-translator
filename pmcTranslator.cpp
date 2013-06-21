// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jun 20, 2013

#include <rose.h>
#include <iostream>
#include "pmcSupport.h"


using namespace std;
using namespace PMCSupport;


int main(int argc, char *argv[])
{
  // Build the AST used by ROSE
  SgProject *project = frontend(argc, argv);

  // Parse PMC pragmas
  ParsingTraversal pmcParser;
  pmcParser.traverseInputFiles(project, preorder);

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

