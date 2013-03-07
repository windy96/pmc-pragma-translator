
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

  // Transform PMC pragmas
  // not implemented yet


  // Test for consistency
  AstTests::runAllTests(project);

  // Invoke a backend compiler
  return backend(project);
}

