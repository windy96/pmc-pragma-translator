// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jun 20, 2013

#ifndef PMC_SUPPORT_H
#define PMC_SUPPORT_H

#include <rose.h>

using namespace std;

namespace PMCSupport
{

	enum PMCPragmaEnum {
		NONE,
    PMC_SHARED,
    PMC_FIRST_SHARING,
    PMC_LAST_SHARING,
    PMC_DATAFLOW_IN,
    PMC_DATAFLOW_OUT,
    PMC_WRITE_DENSE,
    PMC_WRITE_FIRST,
    PMC_WRITE_ONCE,
    PMC_NUM_PRAGMAS
  };



struct PMCPragmaInfo
{
	PMCPragmaEnum	pmcCmd;
	string			content;

	PMCPragmaInfo(PMCPragmaEnum p, string s)
		: pmcCmd(p), content(s) {}
};

class PMCPragmaAttribute: public AstAttribute
{
public:
	SgNode* node;
	vector<PMCPragmaInfo>	pragmaVec;

	PMCPragmaAttribute(SgNode* n, PMCPragmaEnum pType, string con)
	{
		node = n;
		pragmaVec.push_back(PMCPragmaInfo(pType, con));
	}

	virtual std::string toString();
};


  struct pragmaBBPair {
    PMCPragmaAttribute *pragma;
    SgBasicBlock *pBB;

    pragmaBBPair(PMCPragmaAttribute *p, SgBasicBlock *b) {
      pragma = p;
      pBB = b;
    }
  };

  class ParsingTraversal: public AstSimpleProcessing
  {
    protected:
      virtual void visit(SgNode* n);
      void parseTriplet(PMCPragmaAttribute* attr);

  };

  class ParsingPMCTraversal: public AstPrePostProcessing
  {
    protected:
      virtual void preOrderVisit(SgNode* n);
      virtual void postOrderVisit(SgNode* n);

    private:

      vector<struct pragmaBBPair *> currentPMCPragmas;
      vector<struct pragmaBBPair *>::iterator pairIt;
      vector<PMCPragmaAttribute * > pendingPMCPragmas;
      vector<PMCPragmaAttribute *>::iterator pendingIt;
  };
    
	  
		

  PMCPragmaAttribute* parsePMCPragma(SgPragmaDeclaration* pPragmaDecl);


}  // end of namespace

#endif

