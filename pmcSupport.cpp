#include <rose.h>
#include "pmcSupport.h"


using namespace std;
using namespace SageInterface;
using namespace SageBuilder;
using namespace AstFromString;	// for parser


namespace PMCSupport
{


  void ParsingTraversal::parseTriplet(PMCPragmaAttribute *attr)
  {
//    if (afs_match_char(

  }


  void ParsingTraversal::visit(SgNode* node)
  {
    if (node->variantT() != V_SgPragmaDeclaration)
       return;
    // Now we found a pragma declaration.

    SgPragmaDeclaration* pDecl = isSgPragmaDeclaration(node);
    ROSE_ASSERT(pDecl != NULL);

    string key = extractPragmaKeyword(pDecl);
    if (key != "pmc")
      return;
    // Now we have a PMC pragma declaration.
    cout << "We found a PMC pragma." << endl;

    string content = pDecl->get_pragma()->get_pragma();
    content = content.substr(3, content.size() - 3);
    //cout << "pragma is: " << content << endl;

    PMCPragmaAttribute* pAttribute = NULL;
    c_char = content.c_str();

    if (afs_match_substr("write_dense")) {
      cout << "write_dense is detected." << endl;
      pAttribute = new PMCPragmaAttribute(node, PMC_WR_DENSE);
      parseTriplet(pAttribute);
    }
    else if (afs_match_substr("write_first")) {
      cout << "write_first is detected." << endl;
      pAttribute = new PMCPragmaAttribute(node, PMC_WR_FIRST);
      parseTriplet(pAttribute);
    }
    else if (afs_match_substr("write_once")) {
      cout << "write_once is detected." << endl;
      pAttribute = new PMCPragmaAttribute(node, PMC_WR_ONCE);
      parseTriplet(pAttribute);
    }
    else {
      cerr << "Error: Unrecognizable PMC pragma: " << content << "." << endl;
      ROSE_ASSERT(false);
    }
 
    node->addNewAttribute("PMCAttribute", pAttribute);

  }
}; // end of namespace
