#ifndef PMC_SUPPORT_H
#define PMC_SUPPORT_H

namespace PMCSupport
{

  enum PMCPragmaEnum {
    PMC_WR_DENSE,
    PMC_WR_FIRST,
    PMC_WR_ONCE,
    PMC_NUM_PRAGMAS
  };


  class PMCPragmaAttribute: public AstAttribute
  {
    public:
      SgNode* node;
      enum PMCPragmaEnum pragmaType;

      PMCPragmaAttribute(SgNode* n, PMCPragmaEnum pType)
        : node(n), pragmaType(pType) {};
  };

  class ParsingTraversal: public AstSimpleProcessing
  {
    protected:
      virtual void visit(SgNode* n);
      void parseTriplet(PMCPragmaAttribute* attr);

  };

}  // end of namespace

#endif

