/*
 ReadOnlyGraph -> Graph -> EdgeGraph/VertexGraph
 ComplementGraph
 PairedGraph/Debruijn



 interface ReadonlyGraph {
 NodeIt begin_node();
 NodeIt end_node();

 void outgoing_edges(const NodeId &n, EdgeIt &begin, EdgeIt &end);

 bool IsLast(const NodeId &n);

 bool IsFirst(const NodeId &n);

 NodeId start(const EdgeId &e);
 NodeId end(const EdgeId &e);

 };

 interface Graph {

 };

 interface EdgeGraph {
 //	const NodeId addNode(const NodeData& n_d);

 const NodeId addNode();

 const EdgeId addEdge(const EdgeData& e_d, const NodeId& n1, const NodeId& n2);
 const EdgeData& getEdgeData(const EdgeId &e);
 }

 interface ComplementGraph {
 const NodeId complement_node(const& NodeId);
 const EdgeId complement_edge(const& EdgeId);
 }

 interface NodeGraph {
 const NodeData& getNodeData(const NodeId &n);
 }

 interface DeBruijnGraph {
 const EdgeData* getNodeData(const NodeId &n, char c);
 }

 */

#ifndef EDGE_GRAPH_HPP_
#define EDGE_GRAPH_HPP_

#include <vector>
#include <set>
//#include <ext/hash_map>
#include <tr1/unordered_map>
#include <cstring>
#include "seq.hpp"
#include "graphVisualizer.hpp"
#include "sequence.hpp"
#include "logging.hpp"
#include "nucl.hpp"
#include "debruijn.hpp"
#include "strobe_read.hpp"

using namespace std;

namespace edge_graph {
LOGGER("d.edge_graph");

class Vertex;

class Edge {
	Sequence nucls_;
	Vertex* end_;
	size_t coverage_;
	size_t incoming_coverage_;
	size_t outgoing_coverage_;

	friend class EdgeGraph;
	Edge(const Sequence& nucls, Vertex* end) :
		nucls_(nucls), end_(end), coverage_(0) {
	}

	//	void set_coverage(size_t cov) {
	//		coverage_ = cov;
	//	}
	//
	//	size_t coverage() {
	//		return coverage_;
	//	}

	Vertex* end() const {
		return end_;
	}

	const Sequence& nucls() const {
		return nucls_;
	}

	size_t size() const {
		return nucls_.size();
	}

	//	void inc_coverage(char nucl) {
	//		++edge_coverage_[(int) nucl];
	//	}

};

class Vertex {
public:
	typedef vector<Edge *>::const_iterator EdgeIterator;
private:
	vector<Edge *> outgoing_edges_;

	Vertex* complement_;

	friend class EdgeGraph;

	void set_complement(Vertex* complement) {
		complement_ = complement;
	}

	EdgeIterator begin() const {
		return outgoing_edges_.begin();
	}

	EdgeIterator end() const {
		return outgoing_edges_.end();
	}

	Vertex() {
	}

	size_t OutgoingEdgeCount() const {
		return outgoing_edges_.size();
	}

	bool IsDeadend() {
		return outgoing_edges_.size() == 0;
	}

	void AddOutgoingEdge(Edge* e) {
		outgoing_edges_.push_back(e);
	}

	bool RemoveOutgoingEdge(const Edge* e) {
		vector<Edge *>::iterator it = outgoing_edges_.begin();
		while (it != outgoing_edges_.end() && *it != e) {
			++it;
		}
		if (it == outgoing_edges_.end()) {
			return false;
		}
		outgoing_edges_.erase(it);
		return true;
	}

	//
	//	void RemoveOutgoingEdge(vector<Edge *>::iterator iter) {
	//		assert(iter != outgoing_edges_.end());
	//		outgoing_edges_.erase(iter);
	//	}

	Vertex* complement() const {
		return complement_;
	}

	~Vertex() {
		assert(outgoing_edges_.size() == 0);
	}

};

class GraphActionHandler {
public:

	virtual void HandleAdd(Vertex* v) {
	}

	virtual void HandleAdd(Edge* e) {
	}

	virtual void HandleDelete(Vertex* v) {
	}

	virtual void HandleDelete(Edge* e) {
	}

};

class EdgeGraph {
	//Is there any other way to let Edge and Vertex class know value of k?
	size_t k_;

	bool CheckIfNoIncoming(Vertex* v) const;

	bool CanBeDeleted(Vertex* v) const;

	Edge* AddSingleEdge(Vertex* v1, Vertex* v2, const Sequence& s);

//	void DeleteSingleEdge(const Edge* edge);

	GraphActionHandler* action_handler_;

	set<Vertex*> vertices_;

public:

	typedef set<Vertex *>::const_iterator VertexIterator;

	/**
	 * Constructs empty graph to work with k-mers.
	 *
	 * @param k Main parameter that defines the size of k-mers
	 * @param action_handler Graph actions handler
	 */
	EdgeGraph(size_t k, GraphActionHandler* action_handler = new GraphActionHandler()) {
		k_ = k;
		assert(action_handler != NULL);
		action_handler_ = action_handler;
	}

	/**
	 * Deletes action_handler.
	 */
	~EdgeGraph() {
		while(!vertices().empty()) {
			ForceDeleteVertex(*vertices().begin());
		}
		delete action_handler_;
	}

	const set<Vertex*>& vertices() const {
		return vertices_;
	}

	size_t k() {
		return k_;
	}

	void set_action_handler(GraphActionHandler* action_handler) {
		delete action_handler_;
		assert(action_handler != NULL);
		action_handler_ = action_handler;
	}

	void OutgoingEdges(const Vertex* v, Vertex::EdgeIterator &begin,
			Vertex::EdgeIterator &end) const;

	Edge* OutgoingEdge(const Vertex* v, char nucl) const;

	size_t OutgoingEdgeCount(Vertex *v) const {
		return v->OutgoingEdgeCount();
	}

	Edge *GetUniqueEdge(const Vertex *v) const {
		assert(v->OutgoingEdgeCount() == 1);
		return *(v->begin());
	}

	Edge *ComplementEdge(const Edge* edge) const;

	const Sequence &EdgeNucls(Edge *edge) const {
		return edge->nucls();
	}

	/*
	 * Can not return vector iterators for vector which does not exist
	 * Possible solutions:
	 * 1. return vector(and create it every time)
	 * 2. store vector of incoming edges
	 * 3. stop supporting incoming edges: incoming are outcoming for rc vertex
	 */
	//	void IncomingEdges(const Vertex* v, Vertex::EdgeIterator &begin,
	//			Vertex::EdgeIterator &end) const;

	/**
	 * adds vertex and its complement
	 */
	Vertex* AddVertex();

	Sequence vertexNucls(const Vertex *v) const;

	/**
	 * deletes vertex and its complement
	 */
	void DeleteVertex(Vertex* v);

	void ForceDeleteVertex(Vertex* v);

	Edge* AddEdge(Vertex* v1, Vertex* v2, const Sequence &nucls);

	void DeleteEdge(Edge* edge);

	bool AreLinkable(Vertex* v1, Vertex* v2, const Sequence &nucls) const;

	bool IsDeadEnd(Vertex* v) const {
		return v->IsDeadend();
	}

	bool IsDeadStart(Vertex* v) const {
		return IsDeadEnd(v->complement());
	}

	Vertex *edgeStart(const Edge *edge) const;

	Vertex *edgeEnd(const Edge *edge) const;

	Vertex *ComplementVertex(const Vertex* v) const {
		return v->complement();
	}
};

//////////////////////////////////////////////////////////////////

/**
 * @brief Base class for condensed graph traversals.
 */
class Traversal {
public:

	/**
	 * Stub base class for handling graph primitives during traversal.
	 */
	class Handler {
	public:
		virtual void HandleStartVertex(const Vertex* v) {
		}
		virtual void HandleEndVertex(const Vertex* v) {
		}
		virtual void HandleEdge(const Edge* e) {
		}
	};

	Traversal(const EdgeGraph& g) :
		g_(g) {
	}

	/**
	 *
	 */
	virtual void Traverse(Handler& h) = 0;

protected:
	const EdgeGraph& g_;
};

class DFS: public Traversal {
	set<Vertex*> visited_;
	void ProcessVertex(Vertex* v, vector<Vertex*>& stack, Handler& h);
public:
	DFS(const EdgeGraph& g) :
		Traversal(g) {

	}
	virtual void Traverse(Handler& h);
};

class GraphVisualizer {
public:
	virtual void Visualize(const EdgeGraph& g) = 0;
};

class SimpleGraphVisualizer: public GraphVisualizer {
	gvis::GraphPrinter<const Vertex*>& gp_;
public:
	SimpleGraphVisualizer(gvis::GraphPrinter<const Vertex*>& gp) :
		gp_(gp) {
	}

	virtual void Visualize(const EdgeGraph& g);
};

class ComplementGraphVisualizer: public GraphVisualizer {
	gvis::PairedGraphPrinter<const Vertex*>& gp_;
public:
	ComplementGraphVisualizer(gvis::PairedGraphPrinter<const Vertex*>& gp) :
		gp_(gp) {
	}

	virtual void Visualize(const EdgeGraph& g);
};

class SimpleStatCounter: public Traversal::Handler {
	size_t v_count_;
	size_t e_count_;
public:
	SimpleStatCounter() :
		v_count_(0), e_count_(0) {
	}
	virtual void HandleStartVertex(const Vertex* v) {
		v_count_++;
	}
	virtual void HandleEdge(const Vertex* v1, const Vertex* v2) {
		e_count_++;
	}

	size_t v_count() const {
		return v_count_;
	}

	size_t e_count() const {
		return e_count_;
	}
};

class CountHandler: public Traversal::Handler {
	tr1::unordered_map<const Vertex*, size_t>& map_;
	size_t count_;
public:

	CountHandler(tr1::unordered_map<const Vertex*, size_t>& map) :
		map_(map), count_(0) {
	}

	virtual void HandleStartVertex(const Vertex* v) {
		map_.insert(make_pair(v, count_++));
	}
};

//class VisHandler: public Traversal::Handler {
//	gvis::GraphPrinter<const Vertex*>& pr_;
//public:
//
//	VisHandler(gvis::GraphPrinter<const Vertex*>& pr) :
//		pr_(pr) {
//	}
//
//	virtual void HandleStartVertex(const Vertex* v) {
//		stringstream ss;
//		ss << v->nucls().size();
//		pr_.addVertex(v, ss.str());
//	}
//
//	virtual void HandleEdge(const Vertex* v1, const Vertex* v2) {
//		pr_.addEdge(v1, v2, "");
//	}
//
//};
//
//class ComplementVisHandler: public Traversal::Handler {
//	gvis::PairedGraphPrinter<const Vertex*>& pr_;
//public:
//
//	ComplementVisHandler(gvis::PairedGraphPrinter<const Vertex*>& pr) :
//		pr_(pr) {
//	}
//
//	virtual void HandleStartVertex(const Vertex* v) {
//		stringstream ss;
//		ss << v->nucls().size();
//
//		//todo delete after debug
//		stringstream ss2;
//		ss2 << v->complement()->nucls().size();
//		pr_.addVertex(v, ss.str(), v->complement(), ss2.str());
//	}
//
//	virtual void HandleEdge(const Vertex* v1, const Vertex* v2) {
//		pr_.addEdge(make_pair(v1, v1->complement()),
//				make_pair(v2, v2->complement()), "");
//	}
//
//};
}
#endif /* EDGE_GRAPH_HPP_ */
