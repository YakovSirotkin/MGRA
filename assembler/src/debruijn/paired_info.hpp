#ifndef PAIRED_INFO_HPP_
#define PAIRED_INFO_HPP_
#include "utils.hpp"
#include "strobe_reader.hpp"
#include "sequence.hpp"

#define MERGE_DATA_ABSOLUTE_DIFFERENCE 3
#define MERGE_DATA_RELATIVE_DIFFERENCE 0.3

namespace de_bruijn {

template<size_t kmer_size, class Stream, class Graph>
class PairedInfoIndex: public GraphActionHandler<Graph> {
public:
	typedef typename Graph::EdgeId EdgeId;
	typedef typename Graph::VertexId VertexId;

	struct PairInfo {
		EdgeId first_;
		EdgeId second_;
		int d_;//distance between starts. Can be negative
		double weight_;
		PairInfo(EdgeId first, EdgeId second, int d, double weight) :
			first_(first), second_(second), d_(d), weight_(weight) {
		}
	};

	//	template<size_t kmer_size>
	PairedInfoIndex(Graph &g, const SimpleIndex<kmer_size + 1, EdgeId>& index,
			Stream stream) :
		graph_(g) {
		CollectData/*<kmer_size> */(index, stream);
		g.AddActionHandler(this);
	}

private:
	Graph& graph_;
	map<EdgeId, vector<PairInfo> > data_;

	size_t CorrectLength(const de_bruijn::Path<EdgeId>& path, size_t index) {
		if (index == 0) {
			return graph_.length(path[index]) - path.start_pos();
		}
		if (index == path.size() - 1) {
			return path.end_pos();
		}
		return graph_.length(path[index]);
	}

	//	template<size_t kmer_size>
	void CollectData(const SimpleIndex<kmer_size + 1, EdgeId>& index,
			Stream &stream) {
		//todo
		size_t d = 100;

		typedef Seq<kmer_size + 1> KPOMer;
		de_bruijn::SimpleReadThreader<kmer_size, Graph> read_threader(graph_,
				index);
		while (!stream.eof()) {
			vector<Read> reads;
			stream >> reads;
			de_bruijn::Path<EdgeId> path1 = read_threader.ThreadRead(
					reads[0].getSequence());
			de_bruijn::Path<EdgeId> path2 = read_threader.ThreadRead(
					reads[1].getSequence());
			//walken path lengths
			size_t length1 = 0;
			size_t length2 = 0;
			for (size_t i = 0; i < path1.size(); ++i) {
				for (size_t j = 0; j < path2.size(); ++j) {
					AddPairInfo(path1[i], path2[j], d + length2 - length1,
							CorrectLength(path1, i) * CorrectLength(path2, j));
					if (length2 == 0) {
						length2 += kmer_size;
					}
					length2 += CorrectLength(path2, j);
				}
				if (length1 == 0) {
					length1 += kmer_size;
				}
				length1 += CorrectLength(path1, i);
			}
		}
	}

	void UpdateInfo(PairInfo &info, const int d, const double weight) {
		for (typename vector<PairInfo>::iterator it =
				data_[info.second_].begin(); it != data_[info.second_].end(); ++it) {
			if (it->second_ == info.first_ && it->d_ == info.d_ && it->weight_
					== info.weight_) {
				it->d_ = info.d_;
				it->weight_ = info.weight_;
				break;
			}
		}
		info.d_ = d;
		info.weight_ = weight;
	}

	bool MergeData(PairInfo info, const int d, const double weight) {
		if (std::abs(d - info.d_) <= MERGE_DATA_ABSOLUTE_DIFFERENCE
				&& std::abs(d - info.d_) <= info.d_
						* MERGE_DATA_RELATIVE_DIFFERENCE) {
			double newWeight = info.weight_ + weight;
			int newD = std::floor(
					(info.d_ * info.weight_ + d * weight) / weight + 0.5);
			UpdateInfo(info, newD, newWeight);
			return true;
		}
		return false;
	}

	void AddPairInfoToData(const EdgeId first, const EdgeId second,
			const int d, const double weight) {
		data_[first].push_back(PairInfo(first, second, d, weight));
		data_[second].push_back(PairInfo(second, first, -d, weight));
	}

	void AddPairInfo(const EdgeId first, const EdgeId second, const int d,
			const double weight) {
		typename map<EdgeId, vector<PairInfo> >::iterator it =
				data_.find(first);
		if (it == data_.end()) {
			AddPairInfoToData(first, second, d, weight);
		} else {
			vector<PairInfo> &edgeData = (*it).second;
			for (size_t i = 0; i < edgeData.size(); i++)
				if (edgeData[i].second_ == second) {
					if (MergeData(edgeData[i], d, weight)) {
						break;
					}
				}
		}
	}

	void RemoveEdgeInfo(const EdgeId edge) {
		data_.erase(edge);
	}

public:
	vector<const PairInfo> GetEdgeInfo(EdgeId edge) {
		typename map<EdgeId, vector<PairInfo> >::iterator it = data_.find(edge);
		if (it == data_.end()) {
			vector<const PairInfo> res;
			return res;
		} else {
			return vector<const PairInfo> (it->begin(), it->end());
		}
	}

	vector<const PairInfo> GetEdgePairInfo(EdgeId first, EdgeId second) {
		typename map<EdgeId, vector<PairInfo> >::iterator it =
				data_.find(first);
		vector<const PairInfo> res;
		if (it == data_.end()) {
			return res;
		} else {
			for (typename vector<PairInfo>::iterator dataIt = it->begin(); it
					!= it->end(); ++it) {
				assert(dataIt->first == first);
				if (dataIt->second_ == second) {
					res.push_back(*it);
				}
			}
			return res;
		}
	}

	virtual void HandleDelete(EdgeId e) {
		this->RemoveEdgeInfo(e);
	}

	virtual void HandleMerge(vector<EdgeId> oldEdge, EdgeId newEdge) {
		//TODO
	}

	virtual void HandleGlue(EdgeId oldEdge, EdgeId newEdge) {
		//TODO
	}

	virtual void HandleSplit(EdgeId oldEdge, EdgeId newEdge1, EdgeId newEdge2) {
		//TODO
	}

	virtual ~PairedInfoIndex() {
		graph_.RemoveActionHandler(this);
	}

};
}

#endif /* PAIRED_INFO_HPP_ */
