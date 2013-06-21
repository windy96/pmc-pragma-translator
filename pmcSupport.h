// Pragma Translator for Programmer Managed Cache
// Written by Wooil Kim
// Last updated at Jun 20, 2013

#ifndef PMC_SUPPORT_H
#define PMC_SUPPORT_H

namespace PMCSupport
{

  enum PMCPragmaEnum {
    NONE,
    PMC_SHARED,
    PMC_NO_LIVE_IN,
    PMC_NO_LIVE_OUT,
    PMC_DATAFLOW_IN,
    PMC_DATAFLOW_OUT,
    PMC_WRITE_DENSE,
    PMC_WRITE_FIRST,
    PMC_WRITE_ONCE,
    PMC_NUM_PRAGMAS
  };


  class PMCPragmaAttribute: public AstAttribute
  {
    public:
      SgNode* node;
      enum PMCPragmaEnum pragmaType;

      PMCPragmaAttribute(SgNode* n, PMCPragmaEnum pType)
        : node(n), pragmaType(pType) { }

      virtual std::string toString();
  };

  class ParsingTraversal: public AstSimpleProcessing
  {
    protected:
      virtual void visit(SgNode* n);
      void parseTriplet(PMCPragmaAttribute* attr);

  };


  PMCPragmaAttribute* parsePMCPragma(SgPragmaDeclaration* pPragmaDecl);


}  // end of namespace

#endif

