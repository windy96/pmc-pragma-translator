#include <rose.h>
#include <deque>
#include <map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <CallGraph.h>
#include <iostream>


using namespace std;
using namespace SageInterface;


SgProject *project;
CallGraphBuilder *CGBuilder;
SgIncidenceDirectedGraph *graph;
typedef boost::unordered_map<SgFunctionDeclaration *, SgGraphNode * > graphMapType;
typedef boost::unordered_map<SgFunctionDeclaration *, SgGraphNode * >::iterator graphMapTypeIterator;
graphMapType	graphMap;
graphMapTypeIterator	graphMapIterator;
SgGraphNode *mainNode;


bool checkIfPropagatedDirectedGraphEdgeExists(string srcName, string dstName);
bool checkIfPropagatedDirectedGraphEdgeExists(SgGraphNode *srcNode, SgGraphNode *dstNode);

// class OmpStack
// to track program structure, OmpStack keeps current information as a stack.
class OmpStack
{
private:
	deque<SgNode* >	stack;
	deque<SgNode* >::iterator it;
	deque<SgNode* >::reverse_iterator rit;

public:
	void push_back(SgNode *node) {
		stack.push_back(node);
	}

	void pop_back() {
		stack.pop_back();
	}

	SgNode* back() {
		return stack.back();
	}

	void printStack() {
		for (it = stack.begin(); it != stack.end(); ++it) {
			cout << (*it)->unparseToString() << endl;
		}
	}

	void printStackReverse() {
		for (rit = stack.rbegin(); rit != stack.rend(); ++rit) {
			cout << (*rit)->unparseToString() << endl;
		}
	}
};

OmpStack	ompStack;



// map data structure for function-num of omp nodes
// set in MeaningfulOmpVisitor
map<string, int>			functionOmp;
map<string, int>::iterator	functionOmpIterator;


// class OmpSkeletonAttribute
// This attribute class shows how many Omp statements this node has.
class OmpSkeletonAttribute: public AstAttribute
{
public:
	SgNode *node;
	int		numOmpChild;

	OmpSkeletonAttribute() { node = NULL; }
	OmpSkeletonAttribute(SgNode *nd, int n) { node = nd; numOmpChild = n;}
};


// class NumOmpNodeSynthesis
// This synthesis class is used for storing number of Omp nodes,
// and passing the information from bottom to up.
class NumOmpNodeSynthesis {
public:
	int		m_numOmp;
	NumOmpNodeSynthesis() { m_numOmp = 0; }
	NumOmpNodeSynthesis(int n) { m_numOmp = n; }
};


// class MeaningfulOmpVisitor
// This bottom-up traversal visitor can traverse from the bottom leaf node
// to find how many omp nodes are under this node.
// meaningful means omp node has implicit/explicit
class MeaningfulOmpVisitor: public AstBottomUpProcessing<NumOmpNodeSynthesis> {
protected:
	NumOmpNodeSynthesis	evaluateSynthesizedAttribute(SgNode *ast, SubTreeSynthesizedAttributes synthesizedList);
	// filter function defines what is a meaningful node.
	bool filterMeaningful(SgNode *ast);
};


// filter for this class is defined like this.
// Omp parallel, for, sections, single: which has implicit barrier.
// Omp barrier: which has explicit barrier.
bool MeaningfulOmpVisitor::filterMeaningful(SgNode *ast)
{
	SgOmpParallelStatement *sg_omp_parallel = isSgOmpParallelStatement(ast);
	SgOmpForStatement *sg_omp_for = isSgOmpForStatement(ast);
	SgOmpSectionsStatement *sg_omp_sections = isSgOmpSectionsStatement(ast);
	SgOmpSingleStatement *sg_omp_single = isSgOmpSingleStatement(ast);
	SgOmpBarrierStatement *sg_omp_barrier = isSgOmpBarrierStatement(ast);
	if (sg_omp_parallel || sg_omp_for || sg_omp_sections || sg_omp_single || sg_omp_barrier)
		return true;
}


NumOmpNodeSynthesis MeaningfulOmpVisitor::evaluateSynthesizedAttribute(SgNode *ast, SubTreeSynthesizedAttributes synthesizedList) {
	NumOmpNodeSynthesis syn;

	SubTreeSynthesizedAttributes::iterator it;
	int num = 0;
	// accumulating numbers from the bottom
	for (it = synthesizedList.begin(); it != synthesizedList.end(); ++it) 
		num += it->m_numOmp;
	syn.m_numOmp = num;	

	// filtering the node
	if (filterMeaningful(ast))
		syn.m_numOmp++;
	
	// adding or updating OmpSkeleton attribute
	if (ast->attributeExists("OmpSkeleton")) {
		OmpSkeletonAttribute *p = dynamic_cast<OmpSkeletonAttribute *> (ast->getAttribute("OmpSkeleton"));
		p->numOmpChild = syn.m_numOmp;
		ast->updateAttribute("OmpSkeleton", p);
	}
	else {
		OmpSkeletonAttribute *p = new OmpSkeletonAttribute(ast, syn.m_numOmp);
		ast->addNewAttribute("OmpSkeleton", p);
	}

	// if the node is function, specifically function definition,
	// it calculates the number of meaningful omp nodes in the function
	// and saves it to functionOmp map.
	SgFunctionDefinition	*fd = isSgFunctionDefinition(ast);
	if (fd) {
		string s = fd->get_declaration()->get_name().getString();
		functionOmp[s] = syn.m_numOmp;
	}

	return syn;
}



// class SimpleVisitor
// to check OmpSkeleton attribute is properly given to each node
// If properly done, outermost for loop statement will show its OmpSkeleton attribute.
class SimpleVisitor: public AstSimpleProcessing {
protected:
	void virtual visit(SgNode* node);
};


void SimpleVisitor::visit(SgNode* node) {
	// skip some global or upper-level nodes
	if (isSgProject(node))
		return;
	if (isSgFileList(node))
		return;
	if (isSgSourceFile(node))
		return;
	if (isSgGlobal(node))
		return;
	if (isSgFunctionDeclaration(node))
		return;
	if (isSgFunctionDefinition(node))
		return;
	if (isSgBasicBlock(node))
		return;

	if (node->attributeExists("OmpSkeleton")) {
		OmpSkeletonAttribute *p = dynamic_cast<OmpSkeletonAttribute *> (node->getAttribute("OmpSkeleton"));
		if (p->numOmpChild > 0) {
			cout << "=== " << node->class_name() << endl;
			cout << p->numOmpChild << " : " << node->unparseToString() << endl;
		}
	}
}



vector<SgIncidenceDirectedGraph *> executionGraphPerFunction;
map<string, int>	executionGraphMap;
map<string, SgGraphNode*>	executionGraphFirstNodeMap;
map<string, SgGraphNode*>	executionGraphLastNodeMap;
SgIncidenceDirectedGraph *currentGraph;
SgGraphNode *currentNode;
SgGraphNode *currentFirstNode;


// class ExecutionGraphVisitor
// this class should be used with command line arguement '-rose:openmp:ast_only'

class ExecutionGraphVisitor: public AstPrePostProcessing {
protected:
	void virtual preOrderVisit(SgNode* node);
	void virtual postOrderVisit(SgNode* node);

	string		currentFunctionName;
	SgNode		*currentFunctionNode;
	bool		insideOmpParallel;
	bool		insideOmpFor;
	bool		insideOmpSections;
	bool		insideOmpSingle;
};


void ExecutionGraphVisitor::preOrderVisit(SgNode* node) {
	SgGraphNode *newNode;

	if (isSgOmpParallelStatement(node)) {
		string name = "omp parallel begin";
		ROSE_ASSERT(currentGraph);
		newNode = currentGraph->addNode(name, node);
		cout << "\tomp parallel edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
		ROSE_ASSERT(currentNode);
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		insideOmpParallel = true;
		ompStack.push_back(node);
	}
	else if (isSgOmpForStatement(node)) {
		newNode = currentGraph->addNode("omp for begin", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		//	cout << "\tedge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
		cout << "\tomp for edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
		currentNode = newNode;

		insideOmpFor = true;
		ompStack.push_back(node);
	}
	else if (isSgOmpSectionsStatement(node)) {
		newNode = currentGraph->addNode("omp sections begin", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		insideOmpSections = true;
		ompStack.push_back(node);
	}
	else if (isSgOmpSingleStatement(node)) {
		newNode = currentGraph->addNode("omp single begin", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		insideOmpSingle = true;
		ompStack.push_back(node);
	}
	else if (isSgOmpBarrierStatement(node)) {
		newNode = currentGraph->addNode("omp barrier", node);
		cout << "\tbarrier edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		ompStack.push_back(node);
	}
	else if (isSgWhileStmt(node)) {
		if (node->attributeExists("OmpSkeleton")) {
			OmpSkeletonAttribute *p = dynamic_cast<OmpSkeletonAttribute *> (node->getAttribute("OmpSkeleton"));
			if (p->numOmpChild > 0) {
				newNode = currentGraph->addNode("while", node);
				currentGraph->addDirectedEdge(currentNode, newNode);
				cout << "\twhile edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
				currentNode = newNode;

				ompStack.push_back(node);
			}
		}
	}
	else if (isSgForStatement(node)) {
		if (node->attributeExists("OmpSkeleton")) {
			OmpSkeletonAttribute *p = dynamic_cast<OmpSkeletonAttribute *> (node->getAttribute("OmpSkeleton"));
			if (p->numOmpChild > 0) {
				newNode = currentGraph->addNode("for", node);
				currentGraph->addDirectedEdge(currentNode, newNode);
				cout << "\tfor edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
				currentNode = newNode;

				ompStack.push_back(node);
			}
		}
	}
	//else if (isSgFunctionCallExp(node)) {
	else if (isSgFunctionRefExp(node)) {
		// if the function has meaningful stuffs
		
		cout << "function ref: ";
		cout << node->unparseToString() << endl;
		string s = node->unparseToString();
		if (functionOmp[s] > 0) {
			newNode = currentGraph->addNode(s, node);
			currentGraph->addDirectedEdge(currentNode, newNode);
			currentNode = newNode;
		}
		else {
			bool found = false;
			for (functionOmpIterator = functionOmp.begin(); functionOmpIterator != functionOmp.end(); ++functionOmpIterator)
			{
			
				if (functionOmpIterator->second > 0) {
					found = checkIfPropagatedDirectedGraphEdgeExists(s, functionOmpIterator->first);
					cout << s << " -> " << functionOmpIterator->first << " : " << found << endl;
					if (found)
						break;
				}
			}
			if (found) {
				cout << "function call " << s << " survived" << endl;
				newNode = currentGraph->addNode(s, node);
				currentGraph->addDirectedEdge(currentNode, newNode);
				currentNode = newNode;
			}
		}
	}
	else if (isSgFunctionDefinition(node)) {
		currentGraph = new SgIncidenceDirectedGraph();	
		ROSE_ASSERT(currentGraph);

		string s = isSgFunctionDefinition(node)->get_declaration()->get_name().getString();
		cout << "A new graph is created for function " << s << "." << endl;

		newNode = currentGraph->addNode("function begin", node);
		ROSE_ASSERT(newNode);
		currentNode = newNode;
		currentFirstNode = newNode;
		currentFunctionName = s;
		currentFunctionNode = node;
	}	
}


void ExecutionGraphVisitor::postOrderVisit(SgNode* node) {
	SgGraphNode *newNode;
	// should be used with command line arguement '-rose:openmp:ast_only'
	if (isSgOmpParallelStatement(node)) {
		newNode = currentGraph->addNode("omp parallel end", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		cout << "\tomp parallel edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
		currentNode = newNode;

		insideOmpParallel = false;
		ompStack.pop_back();
	}
	else if (isSgOmpForStatement(node)) {
		// currently assuming there is no 'nowait'

		newNode = currentGraph->addNode("omp for end", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		cout << "\tomp for edge from " << currentNode->get_name() << " to " << newNode->get_name() << endl;
		currentNode = newNode;

		insideOmpFor = false;
		ompStack.pop_back();
	}
	else if (isSgOmpSectionsStatement(node)) {
		newNode = currentGraph->addNode("omp sections end", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		insideOmpSections = false;
		ompStack.pop_back();
	}
	else if (isSgOmpSingleStatement(node)) {
		newNode = currentGraph->addNode("omp single end", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		insideOmpSingle = false;
		ompStack.pop_back();
	}
	else if (isSgOmpBarrierStatement(node)) {
		ompStack.pop_back();
	}
	else if (isSgWhileStmt(node)) {
		if (node->attributeExists("OmpSkeleton")) {
			OmpSkeletonAttribute *p = dynamic_cast<OmpSkeletonAttribute *> (node->getAttribute("OmpSkeleton"));
			if (p->numOmpChild > 0) {
				vector<SgGraphNode *> predVec, prevVec, prevPredVec;
				vector<SgGraphNode *>::iterator predVecIt, prevVecIt, prevPredVecIt;
				bool found;
				SgGraphNode *foundGraphNode;

				// currentNode should not be NULL.
				ROSE_ASSERT(currentNode);
				currentGraph->getPredecessors(currentNode, predVec);
				//cout << "initially, predVec size = " << predVec.size() << endl;
				found = false;
				while (predVec.size() > 0) {
					prevVec.clear();
					prevPredVec.clear();
					for (predVecIt = predVec.begin(); predVecIt != predVec.end(); ++predVecIt)
					{
						if ((*predVecIt)->get_name() == "while") {
							found = true;
							foundGraphNode = *predVecIt;
							break;
						}
						currentGraph->getPredecessors(*predVecIt, prevVec);
						for (prevVecIt = prevVec.begin(); prevVecIt != prevVec.end(); ++prevVecIt)
							prevPredVec.push_back(*prevVecIt);
					}
					if (found == true)
						break;
					predVec = prevPredVec;
					//cout << "next, predVec size = " << predVec.size() << endl;
				}
				
				// found should be true!!
				ROSE_ASSERT(found);

				currentGraph->addDirectedEdge(currentNode, foundGraphNode);
				cout << "\twhile edge from " << currentNode->get_name() << " to " << foundGraphNode->get_name() << endl;
				currentNode = foundGraphNode;

				ompStack.pop_back();
			}
			else cout << "while num child = 0" << endl;
		}
		else cout << "while without OmpSkeleton" << endl;
	}
	else if (isSgForStatement(node)) {
		if (node->attributeExists("OmpSkeleton")) {
			OmpSkeletonAttribute *p = dynamic_cast<OmpSkeletonAttribute *> (node->getAttribute("OmpSkeleton"));
			if (p->numOmpChild > 0) {
				vector<SgGraphNode *> predVec, prevVec, prevPredVec;
				vector<SgGraphNode *>::iterator predVecIt, prevVecIt, prevPredVecIt;
				bool found;
				SgGraphNode *foundGraphNode;

				// currentNode should not be NULL.
				ROSE_ASSERT(currentNode);
				currentGraph->getPredecessors(currentNode, predVec);
				//cout << "initially, predVec size = " << predVec.size() << endl;
				found = false;
				while (predVec.size() > 0) {
					prevVec.clear();
					prevPredVec.clear();
					for (predVecIt = predVec.begin(); predVecIt != predVec.end(); ++predVecIt)
					{
						if ((*predVecIt)->get_name() == "for") {
							found = true;
							foundGraphNode = *predVecIt;
							break;
						}
						currentGraph->getPredecessors(*predVecIt, prevVec);
						for (prevVecIt = prevVec.begin(); prevVecIt != prevVec.end(); ++prevVecIt)
							prevPredVec.push_back(*prevVecIt);
					}
					if (found == true)
						break;
					predVec = prevPredVec;
					//cout << "next, predVec size = " << predVec.size() << endl;
				}
				
				// found should be true!!
				ROSE_ASSERT(found);

				currentGraph->addDirectedEdge(currentNode, foundGraphNode);
				cout << "\tfor edge from " << currentNode->get_name() << " to " << foundGraphNode->get_name() << endl;
				currentNode = foundGraphNode;

				ompStack.pop_back();
			}
		}
	}
	else if (isSgFunctionDefinition(node)) {
		newNode = currentGraph->addNode("function end", node);
		currentGraph->addDirectedEdge(currentNode, newNode);
		currentNode = newNode;

		string s = isSgFunctionDefinition(node)->get_declaration()->get_name().getString();
		int n = executionGraphPerFunction.size();
		executionGraphPerFunction.push_back(currentGraph);	
		executionGraphMap[s] = n;
		executionGraphFirstNodeMap[s] = currentFirstNode;
		executionGraphLastNodeMap[s] = currentNode;


		SgIncidenceDirectedGraph *g = executionGraphPerFunction[n];
		//if (start == NULL) {
		if (g->numberOfGraphNodes() == 2) {
			// this condition should be changed to the evaluation of the number of nodes.
			cout << "function " << s << " does not have any omp meaningful node" << endl << endl << endl;
			return;
		}

		vector<SgGraphNode *> succVec, nextVec, nextSuccVec;
		vector<SgGraphNode *>::iterator succVecIt, nextVecIt, nextSuccVecIt;

		cout << endl;
		cout << "BFS for function " << s << endl;
		cout << "num nodes = " << currentGraph->numberOfGraphNodes() << " edges = " << currentGraph->numberOfGraphEdges() << endl;
		g->getSuccessors(currentFirstNode, succVec);


		set<SgGraphNode *> visitedNodeSet;
		visitedNodeSet.clear();

		while (succVec.size() > 0) {
			nextVec.clear();
			nextSuccVec.clear();
			for (succVecIt = succVec.begin(); succVecIt != succVec.end(); ++succVecIt)
			{
				//SgFunctionDeclaration *fd;
				//fd = isSgFunctionDeclaration( (*succVecIt)->get_SgNode() );
				//cout << fd->get_name().getString() << endl;
				visitedNodeSet.insert(*succVecIt);
				cout << (*succVecIt)->get_name() << ",";
				if ((*succVecIt)->get_name() == "function end")
					continue;
				g->getSuccessors(*succVecIt, nextVec);
				cout << "(size=" << nextVec.size() << ") ";
				//cout << "\t" << "next vec size is " << nextVec.size() << endl;
				for (nextVecIt = nextVec.begin(); nextVecIt != nextVec.end(); ++nextVecIt) {
					if (visitedNodeSet.find(*nextVecIt) == visitedNodeSet.end())
						nextSuccVec.push_back(*nextVecIt);
				}
			}
			cout << endl;
			succVec = nextSuccVec;
		}
		cout << endl << endl;

	    AstDOTGeneration dotgen;
		SgIncidenceDirectedGraph *g2;
		dotgen.writeIncidenceGraphToDOTFile( g, s+".dot");
	}



	//cout << node->class_name() << endl;
	else if (isSgStatement(node)) {
		//if (!isSgBasicBlock(node))
		//	justAfterOmpConstruct = false;
	}
	//cout << node->class_name() << endl;
}


void buildCallGraph(SgProject *prj)
{
	CGBuilder = new CallGraphBuilder(prj);
	CGBuilder->buildCallGraph(builtinFilter());

    AstDOTGeneration dotgen;
    SgFilePtrList file_list = prj->get_fileList();
	string firstFileName = StringUtility::stripPathFromFileName(file_list[0]->getFileName());
	graph = CGBuilder->getGraph();
    dotgen.writeIncidenceGraphToDOTFile( graph, firstFileName+"_callGraph.dot");

	graphMap = CGBuilder->getGraphNodesMapping();

	BOOST_FOREACH(graphMapType::value_type i, graphMap) {
		if ((i.first)->get_name().getString() == "main") {
			mainNode = i.second;
			cout << "main found in the graph" << endl;
		}
	}
}


bool checkIfPropagatedDirectedGraphEdgeExists(string srcName, string dstName)
{
	SgGraphNode *srcNode, *dstNode;
	srcNode = NULL;
	dstNode = NULL;
	for (graphMapIterator = graphMap.begin(); graphMapIterator != graphMap.end(); ++graphMapIterator)
	{
		if (graphMapIterator->first->get_name().getString() == srcName)
			srcNode = graphMapIterator->second;
		if (graphMapIterator->first->get_name().getString() == dstName)
			dstNode = graphMapIterator->second;
	}

	if (srcNode == NULL)
		return false;
	if (dstNode == NULL)
		return false;

	ROSE_ASSERT(srcNode);
	ROSE_ASSERT(dstNode);
	cout << "propagated test " << endl;
	cout << srcNode->get_name() << " to " << dstNode->get_name() << endl;

	return checkIfPropagatedDirectedGraphEdgeExists(srcNode, dstNode);
}


bool checkIfPropagatedDirectedGraphEdgeExists(SgGraphNode *srcNode, SgGraphNode *dstNode)
{
	// did not consider cycle due to recursive calls
	bool b1 = graph->checkIfDirectedGraphEdgeExists(srcNode, dstNode);
	if (b1 == true)
		return true;

	vector<SgGraphNode *> succVec, nextVec, nextSuccVec;
	vector<SgGraphNode *>::iterator succVecIt, nextVecIt;
	graph->getSuccessors(srcNode, succVec);


	while (succVec.size() > 0) {
		nextVec.clear();
		nextSuccVec.clear();
		for (succVecIt = succVec.begin(); succVecIt != succVec.end(); ++succVecIt)
		{
			if (dstNode == *succVecIt)
				return true;

			//SgFunctionDeclaration *fd;
			//fd = isSgFunctionDeclaration( (*succVecIt)->get_SgNode() );
			//cout << fd->get_name().getString() << endl;
			graph->getSuccessors(*succVecIt, nextVec);
			//cout << nextVec.size() << endl;
			for (nextVecIt = nextVec.begin(); nextVecIt != nextVec.end(); ++nextVecIt)
				nextSuccVec.push_back(*nextVecIt);
		}
		succVec = nextSuccVec;
	}

	return false;
}


int main(int argc, char *argv[]) {
	project = frontend(argc, argv);

	// if you need to traverse from main function
	SgFunctionDeclaration *mainFunc = findMain(project);
	SgBasicBlock *mainFuncBody = mainFunc->get_definition()->get_body(); 

	// if you need to traverse from a specific function
	// findFunctionDeclaration has a problem in finding function with definition when there is pre-declaration.
	/*
	SgFunctionDeclaration *jacobiFunc = findFunctionDeclaration(project, "jacobi", NULL, true);
	ROSE_ASSERT(jacobiFunc);
	SgFunctionDefinition *jacobiFuncDefi = jacobiFunc->get_definition();
	ROSE_ASSERT(jacobiFuncDefi);
	SgBasicBlock *jacobiFuncBody = jacobiFuncDefi->get_body();
	ROSE_ASSERT(jacobiFuncBody);
	*/

	// First, count meaningful omp section and store information as an attribute.
	MeaningfulOmpVisitor ompVisitor;
	NumOmpNodeSynthesis  mySynthesized;
	//mySynthesized = ompVisitor.traverse(jacobiFuncBody);
	mySynthesized = ompVisitor.traverse(project);
	// print the total omp count
	//cout << mySynthesized.m_numOmp << endl;

	cout << endl << endl;
	cout << "Function Omp List" << endl;
	for (functionOmpIterator = functionOmp.begin(); functionOmpIterator != functionOmp.end(); ++functionOmpIterator)
	{
		cout << "function " << functionOmpIterator->first << " : " << functionOmpIterator->second << endl;
	}

	cout << endl << endl;
	cout << "Section Omp List" << endl;
	SimpleVisitor sVisitor;
	sVisitor.traverse(project, preorder);
	//sVisitor.traverse(jacobiFuncBody, preorder);


	

	buildCallGraph(project);

	BOOST_FOREACH(graphMapType::value_type i, graphMap) {
		string fname;
		//std::cout<<i.first<<","<<i.second<<"\n";
		fname = (i.first)->get_name().getString();
		cout << fname << ", " << i.second << endl;
		bool b1 = graph->checkIfDirectedGraphEdgeExists(mainNode, i.second);
		bool b2 = graph->checkIfDirectedGraphEdgeExists(i.second, mainNode);
		cout << "main to " << fname << " : " << b1 << endl;
		cout << "main from " << fname << " : " << b2 << endl << endl;
	}


	cout << endl << endl;

	BOOST_FOREACH(graphMapType::value_type i, graphMap) {
		string fname;
		//std::cout<<i.first<<","<<i.second<<"\n";
		fname = (i.first)->get_name().getString();
		cout << fname << ", " << i.second << endl;
		bool b1 = checkIfPropagatedDirectedGraphEdgeExists(mainNode, i.second);
		bool b2 = checkIfPropagatedDirectedGraphEdgeExists(i.second, mainNode);
		cout << "main to " << fname << " : " << b1 << endl;
		cout << "main from " << fname << " : " << b2 << endl << endl;
	}



/*	cout << graph->get_numberOfTraversalSuccessors() << endl;
	vector<SgNode *> vec = graph->get_traversalSuccessorContainer();
	vector<SgNode *>::iterator it;
	for (it = vec.begin(); it != vec.end(); ++it) {
		cout << (*it)->class_name() << endl;
	}
*/
	//graph->checkIfDirectedEdgeExists(,);




	cout << endl << endl;
	cout << "Execution graph" << endl;
	ExecutionGraphVisitor v;
	v.traverseInputFiles(project);
	//v.traverse(mainFuncBody);
	//v.traverse(jacobiFuncBody);
	return backend(project);
}

