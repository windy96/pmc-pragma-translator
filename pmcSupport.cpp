// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jun 20, 2013

#include <rose.h>
#include "pmcSupport.h"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;
using namespace AstFromString;	// for parser


namespace PMCSupport
{

  std::string PMCPragmaAttribute::toString()
  {
    std::string result;

    switch (pragmaType)
    {
       case PMC_SHARED:       result += "shared";  break;
       case PMC_NO_LIVE_IN:   result += "no live in";  break;
       case PMC_NO_LIVE_OUT:  result += "no live out";  break;
       case PMC_DATAFLOW_IN:  result += "dataflow in";  break;
       case PMC_DATAFLOW_OUT: result += "dataflow out";  break;
       case PMC_WRITE_DENSE:  result += "write dense";  break;
       case PMC_WRITE_FIRST:  result += "write first";  break;
       case PMC_WRITE_ONCE:   result += "write once";  break;
       default:  break;
     }

     return result;
  }


  void ParsingTraversal::parseTriplet(PMCPragmaAttribute *attr)
  {
//    if (afs_match_char(

  }


  void ParsingTraversal::visit(SgNode* node)
  {
    if (node->variantT() != V_SgPragmaDeclaration)
       return;

    SgPragmaDeclaration* pDecl = isSgPragmaDeclaration(node);
    ROSE_ASSERT(pDecl != NULL);
    string key = extractPragmaKeyword(pDecl);
    if ((key != "pmc") && (key != "PMC"))
      return;
    // Now we have a PMC pragma declaration.

    
    string content = pDecl->get_pragma()->get_pragma();
    content = content.substr(4, content.size() - 4);
    cout << "pragma is recognized: " << content << endl;
    
    PMCPragmaAttribute *pAttribute = parsePMCPragma(pDecl);
    node->addNewAttribute("PMCAttribute", pAttribute);
    ROSE_ASSERT(pAttribute);
    cout << pAttribute->toString() << endl;

/*
    printf("checkpoint 1\n");
    AstAttribute *pAttribute = parsePMCPragma(pDecl);
    printf("checkpoint 2\n");
    //node->addNewAttribute("PMCAttribute", pAttribute);
    printf("checkpoint 3\n");
    PMCPragmaAttribute *pPMCAttribute = dynamic_cast<PMCPragmaAttribute *> (pAttribute);
    printf("checkpoint 4\n");
    cout << pAttribute->toString() << endl;
    printf("checkpoint 5\n");
*/

/*
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
 */
//    node->addNewAttribute("PMCAttribute", pAttribute);

  }





  PMCPragmaAttribute* parsePMCPragma(SgPragmaDeclaration* pPragmaDecl)
  {
    PMCPragmaAttribute *result = NULL;
    ROSE_ASSERT(pPragmaDecl);
    ROSE_ASSERT(pPragmaDecl->get_pragma());

    string pragmaContent = pPragmaDecl->get_pragma()->get_pragma();
    pragmaContent = pragmaContent.substr(4, pragmaContent.size() - 4);

    c_sgnode = getNextStatement(pPragmaDecl);
    c_char = pragmaContent.c_str();

    if (afs_match_substr("shared")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_SHARED);
    }
    else if (afs_match_substr("no_live_in")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_NO_LIVE_IN);
    }
    else if (afs_match_substr("no_live_out")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_NO_LIVE_OUT);
    }
    else if (afs_match_substr("dataflow_in")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_DATAFLOW_IN);
    }
    else if (afs_match_substr("dataflow_out")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_DATAFLOW_OUT);
    }
    else if (afs_match_substr("write_dense")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_WRITE_DENSE);
    }
    else if (afs_match_substr("write_first")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_WRITE_FIRST);
    }
    else if (afs_match_substr("write_once")) {
      result = new PMCPragmaAttribute(c_sgnode, PMC_WRITE_ONCE);
    }
    else {
      cerr << "error in recognizing PMC pragma" << endl;
    }

    ROSE_ASSERT(result);
    
/*

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
*/
    return result;
  }


}; // end of namespace
