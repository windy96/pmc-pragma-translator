// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jul 1, 2013

#include "rose.h"
#include "pmcSupport.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using namespace SageInterface;
using namespace SageBuilder;
using namespace AstFromString;	// for parser
using namespace boost;


namespace PMCSupport
{

	string PMCPragmaEnumString[] = {
		"none",
		"shared",
		"private",
		"readonly",
		"rwshared",
		"locked",
		"atomic",
		"first sharing",
		"last sharing",
		"thread local live in",
		"thread local live out",
		"write dense",
		"write first",
		"write once",
		"unknown",
		"out of bound"
	};

	vector<string> sharedVarVec; 

	string PMCPragmaInfo::toString()
	{
		string result;
		result = PMCPragmaEnumString[pmcCmd] + " for " + content;
		return result;
	}


	string PMCPragmaAttribute::toString()
	{
		string result;
		vector<PMCPragmaInfo>::iterator		pragmaVecIt;
 
		for (pragmaVecIt = pragmaVec.begin(); pragmaVecIt != pragmaVec.end(); ++pragmaVecIt)
		{
			if (pragmaVecIt != pragmaVec.begin())
				result += ", ";
			result += pragmaVecIt->toString();		
		}

		return result;
	}


	PMCPragmaAttribute& PMCPragmaAttribute::operator+=(PMCPragmaAttribute &rhs) 
	{
		vector<PMCPragmaInfo>::iterator		it;

		for (it = rhs.pragmaVec.begin(); it != rhs.pragmaVec.end(); ++it)
			pragmaVec.push_back(*it);
	}


	void CheckerTraversal::visit(SgNode* node)
	{

		if (node->attributeExists("PMCAttribute")) {
			PMCPragmaAttribute *p = dynamic_cast<PMCPragmaAttribute *> (node->getAttribute("PMCAttribute"));
			vector<PMCPragmaInfo>::iterator	it;

			cout << "Node '" << node->unparseToString() << "' has pmc attributes." << endl;
			for (it = p->pragmaVec.begin(); it != p->pragmaVec.end(); ++it)
			{
				if (it != p->pragmaVec.begin())
					cout << ", ";
				else
					cout << "\t";
				cout << it->toString();
			}
			cout << endl << endl;
		}

	}

	void PropagateTraversal::visit(SgNode* node)
	{

		if (node->attributeExists("PMCAttribute")) {
			PMCPragmaAttribute *p = dynamic_cast<PMCPragmaAttribute *> (node->getAttribute("PMCAttribute"));
			vector<PMCPragmaInfo>::iterator	it;

			cout << "Node '" << node->unparseToString() << "' has pmc attributes." << endl;
			cout << node->class_name() << endl;
			
			SgStatement	*st, *nst;
			SgFunctionDeclaration *func;
			SgBasicBlock *func_body;
			switch (node->variantT()) {
			case V_SgFunctionDeclaration:
				//st = getEnclosingStatement(node);
				//cout << "enclosing: " << st->unparseToString() << endl;
				//nst = getNextStatement(st);
				//cout << "next: " << nst->unparseToString() << endl;
				func = isSgFunctionDeclaration(node);
				func_body = func->get_definition()->get_body();
				cout << "func body: " << func_body->unparseToString() << endl;
				break;

			case V_SgForStatement:
				st = getEnclosingStatement(node);
				//cout << "enclosing: " << st->unparseToString() << endl;
				//nst = getNextStatement(st);
				cout << "for: " << st->unparseToString() << endl;
				break;

			case V_SgVariableDeclaration:
				st = getEnclosingStatement(node);
				cout << "variable declaration: " << st->unparseToString() << endl;
				break;
			
			default:
				cout << "not identified for " << node->class_name() << endl;
				break;	

			}
		}

	}

	void CoherenceTrackingTraversal::visit(SgNode* node)
	{

		if (node->attributeExists("PMCAttribute")) {
			PMCPragmaAttribute *p = dynamic_cast<PMCPragmaAttribute *> (node->getAttribute("PMCAttribute"));
			vector<PMCPragmaInfo>::iterator	it;

			cout << "Node '" << node->unparseToString() << "' has pmc attributes." << endl;
			cout << node->class_name() << endl;
			
			SgStatement	*st, *nst;
			SgFunctionDeclaration *func;
			SgBasicBlock *func_body;
			switch (node->variantT()) {
			case V_SgFunctionDeclaration:
				//st = getEnclosingStatement(node);
				//cout << "enclosing: " << st->unparseToString() << endl;
				//nst = getNextStatement(st);
				//cout << "next: " << nst->unparseToString() << endl;
				func = isSgFunctionDeclaration(node);
				func_body = func->get_definition()->get_body();
				cout << "func body: " << func_body->unparseToString() << endl;
				break;

			case V_SgForStatement:
				st = getEnclosingStatement(node);
				//cout << "enclosing: " << st->unparseToString() << endl;
				//nst = getNextStatement(st);
				cout << "for: " << st->unparseToString() << endl;
				break;

			case V_SgVariableDeclaration:
				st = getEnclosingStatement(node);
				cout << "variable declaration: " << st->unparseToString() << endl;
			
			default:
				cout << "not identified for " << node->class_name() << endl;
				break;	

			}
		}

	}


	void VarRefTraversal::visit(SgNode* node)
	{
		SgVarRefExp* refExp;
		SgVariableSymbol *symbol;
		SgVariableSymbol *symbol2;
		SgName name;
		SgFunctionDefinition* fd;
		SgSymbol *sym;
		SgStatement *st;
		SgScopeStatement *sc;
		SgGlobal *sglobal;
		SgBasicBlock *bb;

		switch (node->variantT()) {
		case V_SgVarRefExp:
			refExp = isSgVarRefExp(node);
			symbol = refExp->get_symbol();
			name = symbol->get_name();
			cout << endl;
			cout << "\tname = " << name.getString() << endl;
			// get_name for SgVarRef returns 
			//cout << "\tvar ref name = " << get_name(node) << endl;
			sc = symbol->get_scope();
			ROSE_ASSERT(sc);


			bb = isSgBasicBlock(sc);
			if (bb) {
				cout << "scope's first statement = " << sc->firstStatement()->unparseToString() << endl;
				cout << "scope's last statement = " << sc->lastStatement()->unparseToString() << endl;
				cout << "scope name = " << symbol->get_scope()->get_qualified_name().getString() << endl;
			}
			fd = isSgFunctionDefinition(sc);
			if (fd) {
				cout << "scope's first statement = " << sc->firstStatement()->unparseToString() << endl;
				cout << "scope's last statement = " << sc->lastStatement()->unparseToString() << endl;
				cout << "scope name = " << symbol->get_scope()->get_qualified_name().getString() << endl;
			}
			sglobal = isSgGlobal(sc);
			if (sglobal) {
				cout << "global scope" << endl;
			}

			/*
			symbol2 = lookupVariableSymbolInParentScopes(name, getScope(node));
			if (symbol2 == NULL)
				cout << "\tdoes not return symbol" << endl;
			else
				cout << "\tfound symbol in parent scopes:" << symbol2->get_name().getString() << endl;

			cout << "local symbol table" << endl;	
			outputLocalSymbolTables(node);

			fd = getEnclosingFunctionDefinition(node);
			cout << "\tin the function definition, " << "exists? " << fd->symbol_exists(name) << endl;
			sym = fd->lookup_symbol(name);
			cout << "\tin the function definition, " << "exists? " << (sym != NULL) << endl << endl;
			symbol2 = fd->lookup_var_symbol(name);
			st = getEnclosingStatement(node);
			sc = isSgScopeStatement(st);
			if (sc == NULL)
				cout << "sc is null" << endl;
			else {
				symbol2 = sc->lookup_var_symbol(name);
				if (symbol2 == NULL)
					cout << "\tdoes not return symbol" << endl;
				else
					cout << "\tfound symbol in parent scopes:" << symbol2->get_name().getString() << endl;
			}
			*/	
			
			break;
			
		default:
			break;	
		}
	}



	


/*
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
        if ((*pairIt)->pragma->pmcCmd == PMC_SHARED) {
          if (pVar->unparseToString() == (*pairIt)->pragma->content) {
            cout << "*** found " << pVar->unparseToString() << endl;
            cout << currentPMCPragmas.size() << " pragmas applied" << endl;
            // append statemen
            // temporay break;
            break;
          }
          else {
            // / *
            cout << "difference found" << endl;
            cout << pVar->unparseToString() << endl;
            cout << (*pairIt)->pragma->content << endl;
            // * /
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
     // / *
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
     // * /
    }
  }

*/




	void convertPMCPragmasToAttributes(SgProject *proj)
	{
		Rose_STL_Container<SgNode *> pragmaDeclarationList = NodeQuery::querySubTree(proj, V_SgPragmaDeclaration);
		Rose_STL_Container<SgNode *>::iterator pragmaIt;

		for (pragmaIt = pragmaDeclarationList.begin(); pragmaIt != pragmaDeclarationList.end(); ++pragmaIt)
		{
			SgPragmaDeclaration *pDecl;
			pDecl = isSgPragmaDeclaration(*pragmaIt);
			string	key = extractPragmaKeyword(pDecl);
			if (boost::iequals(key, "pmc")) {

				string	content = pDecl->get_pragma()->get_pragma();
				content = content.substr(4, content.size() - 4);
				cout << "pragma is recognized: " << content << endl;

				SgStatement *pragmaStmt = getEnclosingStatement(pDecl);
				SgStatement *stmt;
				stmt = getNextStatement(pragmaStmt);
				ROSE_ASSERT(stmt);
				while (isSgPragmaDeclaration(stmt)) {
					// skip other pragma declarations
					stmt = getNextStatement(stmt);
					ROSE_ASSERT(stmt);
				}

				SgVariableDeclaration *varDecl = isSgVariableDeclaration(stmt);	
				if (varDecl) {
					Rose_STL_Container<SgInitializedName *> varList;
					Rose_STL_Container<SgInitializedName *>::iterator varListIt;
					varList = varDecl->get_variables();
					for (varListIt = varList.begin(); varListIt != varList.end(); ++varListIt) {
						string s = (*varListIt)->get_name().getString();	
						sharedVarVec.push_back(s);
						cout << "\t" << s << " is added to shared variable vector." << endl;
					}
				}

				cout << "\tapplied to: " << stmt->unparseToString() << endl;
				PMCPragmaAttribute attributeToAdd = parsePMCPragma(pDecl);	
				if (stmt->attributeExists("PMCAttribute")) {
					PMCPragmaAttribute *p = dynamic_cast<PMCPragmaAttribute *> (stmt->getAttribute("PMCAttribute"));
					*p += attributeToAdd;	
					stmt->updateAttribute("PMCAttribute", p);
				}
				else {
					PMCPragmaAttribute *pNewAttribute = new PMCPragmaAttribute();
					*pNewAttribute += attributeToAdd;
					stmt->addNewAttribute("PMCAttribute", pNewAttribute);
				}

				cout << "\tattribute " << attributeToAdd.toString() << " is added." << endl << endl;
			}	// if pmc
		}	// end for
	}


	PMCPragmaAttribute parsePMCPragma(SgPragmaDeclaration* pPragmaDecl)
	{
		PMCPragmaAttribute result;
		ROSE_ASSERT(pPragmaDecl);
		ROSE_ASSERT(pPragmaDecl->get_pragma());
		string pragmaContent = pPragmaDecl->get_pragma()->get_pragma();

		// Used boost tokenizer and string comparator instead of afs_match_substr
		// to get more flexible handling of pragma content

		// Set separator as '(', ')', ',', and white space.
		// because pragmas can be given like '#pragma pmc shared   (a, b)'
		char_separator<char> sep("(), ");
		tokenizer< char_separator<char> > tok(pragmaContent, sep);
		tokenizer< char_separator<char> >::iterator it;

		it = tok.begin();
		++it;		// skip pmc identifier
		PMCPragmaEnum	pmcCmd;
		if (boost::iequals(*it, "shared")) 
			pmcCmd = PMC_SHARED;
		else if (boost::iequals(*it, "private")) 
			pmcCmd = PMC_PRIVATE;
		else if (boost::iequals(*it, "readonly")) 
			pmcCmd = PMC_READONLY;
		else if (boost::iequals(*it, "rwshared")) 
			pmcCmd = PMC_RWSHARED;
		else if (boost::iequals(*it, "locked")) 
			pmcCmd = PMC_LOCKED;
		else if (boost::iequals(*it, "atomic")) 
			pmcCmd = PMC_ATOMIC;
		else if (boost::iequals(*it, "first_sharing")) 
			pmcCmd = PMC_FIRST_SHARING;
		else if (boost::iequals(*it, "last_sharing")) 
			pmcCmd = PMC_LAST_SHARING;
		else if (boost::iequals(*it, "thread_local_live_in")) 
			pmcCmd = PMC_THREAD_LOCAL_LIVE_IN;
		else if (boost::iequals(*it, "thread_local_live_out")) 
			pmcCmd = PMC_THREAD_LOCAL_LIVE_OUT;
		else if (boost::iequals(*it, "write_dense"))
			pmcCmd = PMC_WRITE_DENSE;
		else if (boost::iequals(*it, "write_first")) 
			pmcCmd = PMC_WRITE_FIRST;
		else if (boost::iequals(*it, "write_once")) 
			pmcCmd = PMC_WRITE_ONCE;
		else 
			pmcCmd = UNKNOWN;

		// skip pmc command, and iterate remaining tokens
		for (++it; it != tok.end(); ++it)
		{
			PMCPragmaAttribute temp(NULL, pmcCmd, *it);
			result += temp;
		}

		return result;
	}


	NodeQuerySynthesizedAttributeType querySolver(SgNode *node)
	{
		NodeQuerySynthesizedAttributeType result;

		if (node->attributeExists("PMCAttribute"))
			result.push_back(node); 

		return result;
	}


	void applyPMCAttributesToStatements(SgProject *proj)
	{
		Rose_STL_Container<SgNode *> nodeList = NodeQuery::querySubTree(proj, &querySolver);
		Rose_STL_Container<SgNode *>::iterator it;

		cout << "query" << endl;
		VarRefTraversal varref;
		for (it = nodeList.begin(); it != nodeList.end(); ++it)
		{
			cout << (*it)->unparseToString() << endl;

			varref.traverse(*it, preorder);
		}

		
		cout << endl << "propagate" << endl;
		PropagateTraversal propagate;
		for (it = nodeList.begin(); it != nodeList.end(); ++it)
		{
			propagate.traverse(*it, preorder);
			cout << endl;
		}

		


	}

}; // end of namespace
