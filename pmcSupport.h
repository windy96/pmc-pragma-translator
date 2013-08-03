// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated on Aug 2, 2013

#ifndef PMC_SUPPORT_H
#define PMC_SUPPORT_H

#include "rose.h"
#include <string>

using namespace std;


namespace PMCSupport
{

	enum PMCPragmaEnum {
		NONE	= 0,
	    PMC_SHARED,
		PMC_PRIVATE,
		PMC_READONLY,
		PMC_RWSHARED,
		PMC_LOCKED,
		PMC_ATOMIC,
		PMC_FIRST_SHARING,
		PMC_LAST_SHARING,
		PMC_THREAD_LOCAL_LIVE_IN,
		PMC_THREAD_LOCAL_LIVE_OUT,
		PMC_WRITE_DENSE,
	    PMC_WRITE_FIRST,
		PMC_WRITE_ONCE,
		UNKNOWN,
	    PMC_NUM_PRAGMAS
	};

	extern string PMCPragmaEnumString[];


	extern vector<string>	sharedVarVec;


	class PMCPragmaInfo
	{
	public:
		PMCPragmaEnum	pmcCmd;
		string			content;

		PMCPragmaInfo(PMCPragmaEnum p, string s)
			: pmcCmd(p), content(s) {}

		string toString();
	};


	class PMCPragmaAttribute: public AstAttribute
	{
	public:
		SgNode *node;
		vector<PMCPragmaInfo>	pragmaVec;

		PMCPragmaAttribute()
		{
			node = NULL;
		}

		PMCPragmaAttribute(SgNode *n, PMCPragmaEnum pType, string con)
		{
			node = n;
			pragmaVec.push_back(PMCPragmaInfo(pType, con));
		}

		virtual std::string toString();

		// add pragma attribute to current attribute
		PMCPragmaAttribute& operator+=(PMCPragmaAttribute &rhs);
	};


	class CheckerTraversal: public AstSimpleProcessing
	{
	protected:
		virtual void visit(SgNode* n);
		void parseTriplet(PMCPragmaAttribute* attr);
	};

	class PropagateTraversal: public AstSimpleProcessing
	{
	protected:
		virtual void visit(SgNode* n);
	};


	class CoherenceTrackingTraversal: public AstSimpleProcessing
	{
	protected:
		virtual void visit(SgNode* n);
	};

	class VarRefTraversal: public AstSimpleProcessing
	{
	protected:
		virtual void visit(SgNode* n);
	};
/*
	class ParsingPMCTraversal: public AstPrePostProcessing
	{
	protected:
		virtual void preOrderVisit(SgNode* n);
		virtual void postOrderVisit(SgNode* n);

	private:
		vector<struct pragmaBBPair *> currentPMCPragmas;
		vector<struct pragmaBBPair *>::iterator pairIt;	Rose_STL_Container<SgNode *> pragmaDeclarationList = NodeQuery::querySubTree(project, V_SgPragmaDeclaration);
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

		cout << "    attribute " << attributeToAdd.toString() << " is added." << endl << endl;
	}


		vector<PMCPragmaAttribute * > pendingPMCPragmas;
		vector<PMCPragmaAttribute *>::iterator pendingIt;
	};
*/  
	void convertPMCPragmasToAttributes(SgProject *proj);
	PMCPragmaAttribute parsePMCPragma(SgPragmaDeclaration* pPragmaDecl);
	void applyPMCAttributesToStatements(SgProject *proj);

}  // end of namespace

#endif

