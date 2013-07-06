// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jul 1, 2013

#include <rose.h>
#include "pmcSupport.h"

using namespace SageInterface;
using namespace SageBuilder;
using namespace AstFromString;	// for parser


namespace PMCSupport
{

  std::string PMCPragmaAttribute::toString()
  {
    std::string result;

	vector<PMCPragmaInfo>::iterator		pmcVecIt;
 
	for (pmcVecIt = pmcVec.begin(); pmcVecIt != pmcVec.end(); ++pmcVecIt)
	{
		
    switch ((*pmcVecIt)->pmcCmd)
    {
       case PMC_SHARED:       result += "shared";  
         result += " for " + content; break;
       case PMC_FIRST_SHARING:   result += "first sharing";  break;
       case PMC_LAST_SHARING: result += "last sharing";  break;
       case PMC_DATAFLOW_IN:  result += "dataflow in";  break;
       case PMC_DATAFLOW_OUT: result += "dataflow out";  break;
       case PMC_WRITE_DENSE:  result += "write dense";  break;
       case PMC_WRITE_FIRST:  result += "write first";  break;
       case PMC_WRITE_ONCE:   result += "write once";  break;
       default:  break;
     }
	result += endl;
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



  void ParsingPMCTraversal::preOrderVisit(SgNode* node)
  {
    if (node->variantT() == V_SgPragmaDeclaration) {
      //cout << "I met pragma" << endl;
      SgPragmaDeclaration* pDecl = isSgPragmaDeclaration(node);
      ROSE_ASSERT(pDecl != NULL);
      string key = extractPragmaKeyword(pDecl);
      if ((key != "pmc") && (key != "PMC"))
        return;
       // Now we have a PMC pragma declaration.

    
      string content = pDecl->get_pragma()->get_pragma();
      content = content.substr(4, content.size() - 4);
      cout << "pragma is recognized: " << content << endl;
      SgStatement *pragmaStmt = getEnclosingStatement(node);
      SgStatement *stmt;
      stmt = getNextStatement(pragmaStmt);
      while (isSgPragmaDeclaration(stmt))
        stmt = getNextStatement(stmt);
      cout << "applied to: " << stmt->unparseToString() << endl << endl;
      PMCPragmaAttribute *pAttribute = parsePMCPragma(pDecl);
    
      // push this pragma into vector
      pendingPMCPragmas.push_back(pAttribute);

      //node->addNewAttribute("PMCAttribute", pAttribute);
      //ROSE_ASSERT(pAttribute);
      //cout << pAttribute->toString() << endl;
    }
    else if (node->variantT() == V_SgBasicBlock) {
      SgBasicBlock *pBB = isSgBasicBlock(node);
      ROSE_ASSERT(pBB != NULL);

      cout << endl;
      cout << "I met a basic block:" << endl;
      cout << pBB->unparseToString() << endl;
      cout << endl;

      // if pragma vector is not null
      //if (currentPMCPragmas.size() > 0) {
      for (pendingIt = pendingPMCPragmas.begin(); pendingIt != pendingPMCPragmas.end(); pendingIt++)
      {
        struct pragmaBBPair *pPair;
        pPair = new struct pragmaBBPair (*pendingIt, pBB);
        
        currentPMCPragmas.push_back(pPair);
        cout << "pragma " << (*pendingIt)->toString() << " is added to this bb" << endl;
      }
      pendingPMCPragmas.clear();
      cout << endl;

    }
    else if (node->variantT() == V_SgVarRefExp) {
    //else if (node->variantT() == 698) {
      SgVarRefExp *pVar = isSgVarRefExp(node);
      //cout << "variable " << pVar->unparseToString() << endl;
      //cout << currentPMCPragmas.size() << endl;

      for (pairIt = currentPMCPragmas.begin(); pairIt != currentPMCPragmas.end(); pairIt++)
      {
        if ((*pairIt)->pragma->pragmaType == PMC_SHARED) {
          if (pVar->unparseToString() == (*pairIt)->pragma->content) {
            cout << "*** found " << pVar->unparseToString() << endl;
            cout << currentPMCPragmas.size() << " pragmas applied" << endl;
            // append statemen
            // temporay break;
            break;
          }
          else {
            /*
            cout << "difference found" << endl;
            cout << pVar->unparseToString() << endl;
            cout << (*pairIt)->pragma->content << endl;
            */
          }
        }
      }
    }
    else {
      //cout << "what? " << node->class_name() << " number=" << node->variantT() << endl;
    }
  }

  void ParsingPMCTraversal::postOrderVisit(SgNode* node)
  {
    if (node->variantT() == V_SgBasicBlock) {
      SgBasicBlock *pBB = isSgBasicBlock(node);
      ROSE_ASSERT(pBB != NULL);

      cout << endl;
      cout << "exiting BB" << endl;

      for (pairIt = currentPMCPragmas.begin(); pairIt != currentPMCPragmas.end(); pairIt++)
      {
        if ( (*pairIt)->pBB == pBB) {
          cout << "pragma " << (*pairIt)->pragma->toString() << " is found while exiting bb" << endl;
          
          //pendingPMCPragmas.erase( (*pairIt)->pragma );
          for (pendingIt = pendingPMCPragmas.begin(); pendingIt != pendingPMCPragmas.end(); pendingIt++)
          {
            if (*pendingIt == (*pairIt)->pragma) {
              pendingPMCPragmas.erase(pendingIt);
              break;
            }
          }
          //currentPMCPragmas.erase(pairIt);
        }
      }
      cout << endl;
    }
    else if (node->variantT() == V_SgVarRefExp) {
      SgVarRefExp *pVar = isSgVarRefExp(node);
      //cout << "post variable " << pVar->unparseToString() << endl;
      //cout << currentPMCPragmas.size() << endl;
    //else if (node->variantT() == 698) {
     /*
      SgVarRefExp *pVar = isSgVarRefExp(node);
      cout << "variable " << pVar->unparseToString() << endl;
      cout << currentPMCPragmas.size() << endl;

      for (pairIt = currentPMCPragmas.begin(); pairIt != currentPMCPragmas.end(); pairIt++)
      {
        if ((*pairIt)->pragma->pragmaType == PMC_SHARED) {
          if (pVar->unparseToString() == (*pairIt)->pragma->content) {
            cout << "*** found " << pVar->unparseToString() << endl;
          }
          else {
            cout << "difference found" << endl;
            cout << pVar->unparseToString() << endl;
            cout << (*pairIt)->pragma->content << endl;
          }
        }
      }
     */
    }
  }




  PMCPragmaAttribute* parsePMCPragma(SgPragmaDeclaration* pPragmaDecl)
  {
    PMCPragmaAttribute *result = NULL;
    ROSE_ASSERT(pPragmaDecl);
    ROSE_ASSERT(pPragmaDecl->get_pragma());

    string pragmaContent = pPragmaDecl->get_pragma()->get_pragma();
    pragmaContent = pragmaContent.substr(4, pragmaContent.size() - 4);

    //c_sgnode = getNextStatement(pPragmaDecl);
    c_sgnode = getEnclosingStatement(pPragmaDecl);
    c_char = pragmaContent.c_str();

    //cout << "parsing" << endl;
    //cout << c_sgnode->unparseToString() << endl;
    string content;
    if (afs_match_substr("shared")) {
      content = pragmaContent.substr(7, pragmaContent.size() - 7, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_SHARED);
    }
    else if (afs_match_substr("first_sharing")) {
      content = pragmaContent.substr(14, pragmaContent.size() - 14, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_FIRST_SHARING);
    }
    else if (afs_match_substr("last_sharing")) {
      content = pragmaContent.substr(13, pragmaContent.size() - 13, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_LAST_SHARING);
    }
    else if (afs_match_substr("from_the_same_thread")) {
      content = pragmaContent.substr(21, pragmaContent.size() - 21, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_DATAFLOW_IN);
    }
    else if (afs_match_substr("to_the_same_thread")) {
      content = pragmaContent.substr(19, pragmaContent.size() - 19, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_DATAFLOW_OUT);
    }
    else if (afs_match_substr("write_dense")) {
      content = pragmaContent.substr(12, pragmaContent.size() - 12, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_WRITE_DENSE);
    }
    else if (afs_match_substr("write_first")) {
      content = pragmaContent.substr(12, pragmaContent.size() - 12, content);
      result = new PMCPragmaAttribute(c_sgnode, PMC_WRITE_FIRST);
    }
    else if (afs_match_substr("write_once")) {
      content = pragmaContent.substr(11, pragmaContent.size() - 11, content);
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
