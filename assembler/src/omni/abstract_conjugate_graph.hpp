#ifndef ABSTRACT_CONJUGATE_GRAPH_HPP_
#define ABSTRACT_CONJUGATE_GRAPH_HPP_

namespace omnigraph {

template<typename VertexData, typename EdgeData, class DataMaster>
class AbstractConjugateGraph {
public:
	class Vertex;
	typedef Vertex* VertexId;
	class Edge;
	typedef Edge* EdgeId;

	class Vertex {
	public:
		typedef vector<EdgeId>::const_iterator EdgeIterator;
	private:
		friend class AbstractConjugateGraph<VertexData, EdgeData> ;

		vector<EdgeId> outgoing_edges_;

		VertexId conjugate_;

		VertexData data_;

		void set_conjugate(VertexId conjugate) {
			conjugate_ = conjugate;
		}

		size_t OutgoingEdgeCount() const {
			return outgoing_edges_.size();
		}

		const vector<EdgeId> OutgoingEdges() const {
			return outgoing_edges_;
		}

		size_t IncomingEdgeCount() const {
			return conjugate_->OutgoingEdgeCount();
		}

		const vector<EdgeId> IncomingEdges() const {
			vector<EdgeId> result = conjugate_->OutgoingEdges();
			for(size_t i = 0; i < result.size(); i++) {
				result[i] = result[i].conjugate();
			}
			return result;
		}

		Vertex(VertexData data) :
			data_(data) {
		}

		VertexData &data() {
			return data_;
		}

		void set_data(VertexData data) {
			data_ = data;
		}

		bool IsDeadend() {
			return outgoing_edges_.size() == 0;
		}

		void AddOutgoingEdge(EdgeId e) {
			outgoing_edges_.push_back(e);
		}

		bool RemoveOutgoingEdge(const EdgeId e) {
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

		VertexId conjugate() const {
			return conjugate_;;
		}

		~Vertex() {
			assert(outgoing_edges_.size() == 0);
		}
	};

	class Edge {
		VertexId end_;

		EdgeData data_;

		EdgeId conjugate_;

		Edge(Vertex* end, const EdgeData &data) :
			end_(end), data_(data) {
		}

		EdgeData &data() {
			return data_;
		}

		VertexId end() const {
			return end_;
		}

		EdgeId conjugate() {
			return conjugate_;
		}

		void set_conjugate(EdgeId conjugate) {
			conjugate_ = conjugate;
		}

		~Edge() {
		}
	};

	const PairedHandlerApplier<AbstractConjugateGraph<VertexData, EdgeData> > applier_;

	vector<ActionHandler*> action_handler_list_;

	set<Vertex*> vertices_;

	DataMaster master_;

	VertexId EdgeGraph::HiddenAddVertex(VertexData &data) {
		VertexId v1 = new Vertex(data);
		VertexId v2 = new Vertex(!data);
		v1->set_conjugate(v2);
		v2->set_conjugate(v1);
		vertices_.insert(v1);
		vertices_.insert(v2);
		return v1;
	}

	EdgeId EdgeGraph::HiddenAddEdge(VertexId v1, VertexId v2, const EdgeData &data) {
		assert(vertices_.find(v1) != vertices_.end() && vertices_.find(v2) != vertices_.end());
		//	assert(OutgoingEdge(v1, nucls[k_]) == NULL);
		EdgeId result = AddSingleEdge(v1, v2, data);
		EdgeId rcEdge = result;
		if (data != !data) {
			rcEdge = AddSingleEdge(v2->conjugate(), v1->conjugate(), !data);
		}
		result->set_conjugate(rcEdge);
		rcEdge->set_conjugate(result);
		return result;
	}

	EdgeId EdgeGraph::AddSingleEdge(VertexId v1, VertexId v2, const EdgeData &data) {
		EdgeId newEdge = new Edge(v2, data);
		v1->AddOutgoingEdge(newEdge);
		return newEdge;
	}

	void DeleteAllOutgoing(VertexId v) {
		vector<EdgeId> out = v->outgoing_edges_;
		for (vector<EdgeId>::iterator it = out.begin(); it != out.end(); ++it) {
			DeleteEdge(*it);
		}
	}

	void EdgeGraph::FireAddVertex(VertexId v) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplyAdd(*it, v);
		}
	}

	void EdgeGraph::FireAddEdge(EdgeId edge) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplyAdd(*it, edge);
		}
	}

	void EdgeGraph::FireDeleteVertex(VertexId v) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplyDelete(*it, v);
		}
	}

	void EdgeGraph::FireDeleteEdge(EdgeId edge) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplyDelete(*it, edge);
		}
	}

	void EdgeGraph::FireMerge(vector<EdgeId> oldEdges, EdgeId newEdge) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplyMerge(*it, oldEdges, newEdge);
		}
	}

	void EdgeGraph::FireGlue(EdgeId edge1, EdgeId edge2) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplyGlue(*it, edge1, edge2);
		}
	}

	void EdgeGraph::FireSplit(EdgeId edge, EdgeId newEdge1, EdgeId newEdge2) {
		for (vector<ActionHandler*>::iterator it = action_handler_list_.begin(); it
				!= action_handler_list_.end(); ++it) {
			applier_.ApplySplit(*it, edge, newEdge1, newEdge2);
		}
	}

public:

	VertexIterator begin() const {
		return vertices_.begin();
	}

	VertexIterator end() const {
		return vertices_.end();
	}

	template<typename Comparator = std::less<VertexId> >
	SmartVertexIterator<EdgeGraph, Comparator> SmartVertexBegin(
			const Comparator& comparator = Comparator()) {
		return SmartVertexIterator<EdgeGraph, Comparator> (*this, true,
				comparator);
	}

	template<typename Comparator = std::less<VertexId> >
	SmartVertexIterator<EdgeGraph, Comparator> SmartVertexEnd(
			const Comparator& comparator = Comparator()) {
		return SmartVertexIterator<EdgeGraph, Comparator> (*this, false,
				comparator);
	}

	template<typename Comparator = std::less<EdgeId> >
	SmartEdgeIterator<EdgeGraph, Comparator> SmartEdgeBegin(
			const Comparator& comparator = Comparator()) {
		return SmartEdgeIterator<EdgeGraph, Comparator> (*this, true,
				comparator);
	}

	template<typename Comparator = std::less<EdgeId> >
	SmartEdgeIterator<EdgeGraph, Comparator> SmartEdgeEnd(
			const Comparator& comparator = Comparator()) {
		return SmartEdgeIterator<EdgeGraph, Comparator> (*this, false,
				comparator);
	}

	size_t size() {
		return vertices_.size();
	}

	AbstractConjugateGraph(DataMaster master) : applier_(*this), master_(master) {
	}

	~AbstractConjugateGraph() {
		while (!vertices_.empty()) {
			ForceDeleteVertex(*vertices_.begin());
		}
	}

	void AddActionHandler(ActionHandler* action_handler) {
		TRACE("Action handler added");
		action_handler_list_.push_back(action_handler);
	}

	bool RemoveActionHandler(ActionHandler* action_handler) {
		TRACE("Trying to remove action handler");
		for (vector<ActionHandler*>::iterator it =
				action_handler_list_.begin(); it != action_handler_list_.end(); ++it) {
			if (*it == action_handler) {
				action_handler_list_.erase(it);
				TRACE("Action handler removed");
				return true;
			}
		}
		return false;
	}

	const vector<EdgeId> EdgeGraph::OutgoingEdges(VertexId v) const {
		return v->OutGoingEdges();
	}

	const vector<EdgeId> EdgeGraph::IncomingEdges(VertexId v) const {
		return v->IncomingEdges();
	}

	size_t OutgoingEdgeCount(VertexId v) const {
		return v->OutgoingEdgeCount();
	}

	bool CheckUniqueOutgiongEdge(VertexId v) const {
		return v->OutgoingEdgeCount() == 1;
	}

	EdgeId GetUniqueOutgoingEdge(VertexId v) const {
		assert(CheckUniqueOutgiongEdge(v));
		return (v->OutgoingEdges())[0];
	}

	bool CheckUniqueIncomingEdge(VertexId v) const {
		return CheckUniqueOutgiongEdge(v->conjugate());
	}

	EdgeId GetUniqueIncomingEdge(VertexId v) const {
		return conjugate(GetUniqueOutgoingEdge(v->conjugate()));
	}

	const EdgeData& data(EdgeId edge) const {
		return edge->data();
	}

	const VertexData& data(VertexId v) const {
		return v->data();
	}

	VertexId AddVertex() {
		VertexId result = HiddenAddVertex();
		FireAddVertex(result);
		return result;
	}

	VertexId AddVertex() {
		VertexId result = HiddenAddVertex();
		FireAddVertex(result);
		return result;
	}

	void ForceDeleteVertex(VertexId v) {
		DeleteAllOutgoing(v);
		DeleteAllOutgoing(v->conjugate());
		DeleteVertex(v);
	}

	EdgeId AddEdge(VertexId v1, VertexId v2, const EdgeData &data) {
		EdgeId result = HiddenAddEdge(v1, v2, data);
		FireAddEdge(result);
		return result;
	}

	void DeleteEdge(EdgeId edge) {
		FireDeleteEdge(edge);
		EdgeId rcEdge = conjugate(edge);
		VertexId rcStart = conjugate(edge->end());
		VertexId start = conjugate(rcEdge->end());
		start->RemoveOutgoingEdge(edge);
		rcStart->RemoveOutgoingEdge(rcEdge);
		if (edge != rcEdge) {
			delete rcEdge;
		}
		delete edge;
	}

	bool IsDeadEnd(VertexId v) const {
		return v->IsDeadend();
	}

	bool IsDeadStart(VertexId v) const {
		return IsDeadEnd(v->conjugate());
	}

	VertexId EdgeStart(EdgeId edge) const {
		return edge->conjugate()->end()->conjugate();
	}

	VertexId EdgeEnd(EdgeId edge) const {
		return edge->end();
	}

	VertexId conjugate(VertexId v) const {
		return v->conjugate();
	}

	EdgeId conjugate(EdgeId edge) const {
		return edge->conjugate();
	}

	bool CanCompressVertex(VertexId v) const {
		return v->OutgoingEdgeCount() == 1 && v->conjugate()->OutgoingEdgeCount()
				== 1;
	}

	void CompressVertex(VertexId v) {
		//assert(CanCompressVertex(v));
		if (CanCompressVertex(v)) {
			Merge(GetUniqueIncomingEdge(v), GetUniqueOutgoingEdge(v));
		}
	}

	void CompressVertex(VertexId v) {
		//assert(CanCompressVertex(v));
		if (CanCompressVertex(v)) {
			Merge(GetUniqueIncomingEdge(v), GetUniqueOutgoingEdge(v));
		}
	}

	EdgeId MergePath(const vector<EdgeId>& path) {
		assert(!path.empty());
		SequenceBuilder sb;
		VertexId v1 = EdgeStart(path[0]);
		VertexId v2 = EdgeEnd(path[path.size() - 1]);
		vector<EdgeData&> toMerge;
		for (vector<EdgeId>::const_iterator it = path.begin(); it != path.end(); ++it) {
			toMerge.push_back((*it)->data());
		}
		EdgeId newEdge = HiddenAddEdge(v1, v2, master_.MergeData(toMerge));
		FireMerge(path, newEdge);
		DeleteEdge(path[0]);
		for (size_t i = 0; i + 1 < path.size(); i++) {
			VertexId v = EdgeStart(path[i + 1]);
			DeleteEdge(path[i + 1]);
			DeleteVertex(v);
		}
		FireAddEdge(newEdge);
		return newEdge;
	}
};

pair<EdgeId, EdgeId> SplitEdge(EdgeId edge, size_t position) {
	assert(edge != conjugate(edge));
	pair<VertexData, pair<EdgeData, EdgeData>> newData = master_.SplitData(edge->data(), position);
	VertexId splitVertex = HiddenAddVertex(newData.first);
	EdgeId newEdge1 = HiddenAddEdge(this->EdgeStart(edge), splitVertex, newData.second.first);
	EdgeId newEdge2 = HiddenAddEdge(splitVertex, this->EdgeEnd(edge), newData.second.second);
	FireSplit(edge, newEdge1, newEdge2);
	FireAddVertex(splitVertex);
	FireAddEdge(newEdge1);
	FireAddEdge(newEdge2);
	DeleteEdge(edge);
	return make_pair(newEdge1, newEdge2);
}

void EdgeGraph::GlueEdges(EdgeId edge1, EdgeId edge2) {
	FireDeleteEdge(edge2);
	FireGlue(edge1, edge2);
	edge2->set_data(master_.GlueData(edge1->data(), edge2->data()));
	FireAddEdge(edge2);
	VertexId start = EdgeStart(edge1);
	VertexId end = EdgeEnd(edge1);
	DeleteEdge(edge1);
	if (IsDeadStart(start) && IsDeadEnd(start)) {
		DeleteVertex(start);
	}
	if (IsDeadStart(end) && IsDeadEnd(end)) {
		DeleteVertex(end);
	}
}

}
#endif /* ABSTRUCT_CONJUGATE_GRAPH_HPP_ */