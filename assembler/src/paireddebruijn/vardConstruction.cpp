#include "constructHashTable.hpp"
#include "common.hpp"
#include "graphConstruction.hpp"
#include "seq.hpp"
#include "graphVisualizer.hpp"
#include "pairedGraph.hpp"
#include "graphio.hpp"
#include "vardConstruction.hpp"

LOGGER("p.vardConstruction");

using namespace paired_assembler;
namespace vard {
inline ll leftkmer(ll kmer) {
	return kmer >> 2;
}
inline ll rightkmer(ll kmer) {
	return kmer & (~((ll)3 << (2*k-2)));
}
inline ll subkmer(ll kmer, int direction) {
	if (direction == LEFT)
		return leftkmer(kmer);
	else if (direction == RIGHT)
		return rightkmer(kmer);
	else assert(0);
}

/*
 * @param direction  LEFT if we look from leftmost end
 *
 * @return index of vertex, if this k-1 -seq can be traced to it.
 * If there are multiple, return -2,(we hope there will be no such situation:)
 * if no- returns -1
 */

int findPossibleVertex(ll kmer, Sequence &down, edgesMap &edges, verticesMap &verts){
	verticesMap::iterator v = verts.find(kmer);
	TRACE("findPossibleVert: "<< kmer <<" vertssize" << verts.size());
	int count = 0;
	int res = -1;
	if (v != verts.end()) {
		TRACE(" kmer FOUND");

		forn(i, v->second.size()) {
			Sequence* cur_seq =  v->second[i]->lower;
			int position = v->second[i]->position;
			int tmp_pos;
			if ((tmp_pos = down.str().find(cur_seq->Subseq(position, position + k-1).str())) != string::npos){
				res =  v->second[i]->VertexId;
				DEBUG("vert found " << kmer << " " << cur_seq->str() << " " << tmp_pos<< " at position " << position);
				DEBUG("For " << kmer << " " << down.str() );
				count++;
			}
		}
	}
	if (count > 1) res = -2;
	TRACE ("result :" <<res);
	if (count == 1) DEBUG("vertex found");
	return res;
}
//go left until vertex or fork.
// Go right until vertex or fork, adding nucleotides to curEdge strings.
/*\
 *\
 *
 * @return coverage of resulting edge when expanding or 0.
 */
Sequence* SubSeq(Sequence Seq, int direction){
	if (direction == LEFT)
		return new Sequence(Seq.Subseq(0, Seq.size()-1));
	else if (direction == RIGHT)
		return new Sequence(Seq.Subseq(1));
	else {assert(0);}

}
int expandDirected(edgesMap &edges, protoEdgeType &curEdge, verticesMap &verts, ll &curKmer, Sequence* &curSeq, int &EdgeCoverage, int direction){
	assert(direction == LEFT || direction == RIGHT );
	TRACE("expanding" << direction << " kmer "<< curKmer);
	while( (findPossibleVertex(subkmer(curKmer, direction), *SubSeq(*curSeq, direction), edges, verts) == -1) ){
		pair <char, EdgePrototype*> otherdir_res = findUniqueWay(edges, curKmer, curSeq, otherDirection(direction), true);
		pair <char, EdgePrototype*> dir_res = findUniqueWay(edges, curKmer, curSeq, direction , false);

		if ((otherdir_res.second == NULL) ) {
			DEBUG("Other dir NULL");
			break;
		}
		if   (dir_res.second == NULL) {
			DEBUG("This dir NULL");
			break;
		}
		goUniqueWay(edges, curKmer, curSeq, dir_res, EdgeCoverage, direction);
		if (direction == RIGHT) {
			dir_res.second->used = true;
			string tmp = decompress(curKmer, k);
			curEdge.first+=(tmp[k-1]);
			curEdge.second.append(curSeq->Subseq(k-1,k).str());
			//TODO:: do it, save nucleo/
		}
	}
	return 0;
}

pair<char, EdgePrototype*> findUniqueWay(edgesMap &edges, ll curKmer, Sequence *curSeq , int direction, bool replace){
	assert(direction == LEFT || direction == RIGHT );
	int count = 0;
	TRACE("Find uniqueness" << direction);
//	cerr << "findUniqueWay" << endl;
	pair <char, EdgePrototype*> res = make_pair(0, (EdgePrototype *)NULL);

	for (int Nucl = 0; Nucl < 4; Nucl++) {
		ll tmpcurKmer;
		if (!replace)
			tmpcurKmer = subkmer(curKmer, direction);
		else
			tmpcurKmer = subkmer(curKmer, otherDirection(direction));
		ll tmpKmer = pushNucleotide(tmpcurKmer, k - 1, direction, Nucl);

		edgesMap::iterator iter = edges.find(tmpKmer);
		TRACE("FROM " << curKmer << " Trying to find " << tmpKmer);
		if (iter != edges.end()) {
			for (vector<EdgePrototype *>::iterator it = iter->second.begin(); it != iter->second.end(); ++it) {
				//TODO: minIntersect?
//				if (curSeq->similar(*((*it)->lower), minIntersect, direction)) {
				if (curSeq->similar(*((*it)->lower), minIntersect, 0)) {
					count++;
					TRACE("FOUND " << (*it)->lower->str());
					if (count > 1) {
						DEBUG("multiple: ");
						DEBUG("Nucl "<<(int)Nucl<<" Seq "<< (*it)->lower->str());
						DEBUG("Nucl "<<(int)res.first<<" Seq "<< res.second->lower->str());
						return make_pair(0, (EdgePrototype *)NULL);
					} else {
						res = make_pair(Nucl, *it);
					}

				}
			}
		}
	}
	return res;
}

//while going left we don't mark anything as used, we just find leftmost possible vert
int goUniqueWay(edgesMap &edges, ll &curKmer, Sequence* &curSeq, pair<char, EdgePrototype*> findResult, int &EdgeCoverage, int direction) {
	assert(direction == LEFT || direction == RIGHT );
	TRACE ("going " << direction << " from  " << curKmer << " ");
	ll tmpKmer = subkmer(curKmer,direction);
	TRACE(tmpKmer <<" " << (int)findResult.first);
	curKmer = pushNucleotide(tmpKmer, k-1,  direction, findResult.first);
	TRACE (curKmer);
	EdgeCoverage += findResult.second->coverage;
	curSeq = new Sequence(*findResult.second->lower);//PossibleSequence;
	findResult.second->used = 1;
	return 0;
}

int countWays(vector<EdgePrototype *> &v, Sequence *finishSeq, int direction) {
	int count = 0;
//	cerr <<" countWays started"<< endl;
	for (vector<EdgePrototype *>::iterator it = v.begin(); it != v.end(); ++it) {
//TODO: minIntersect?
		if (finishSeq->similar(*((*it)->lower), minIntersect, direction)) {
			count++;
			if (count > 1) {
				return count;
			}
		}
	}
	return count;
}



void createVertices(edgesMap &edges, PairedGraph &graph) {
	int count = 0;
	for (edgesMap::iterator iter = edges.begin(); iter != edges.end();) {
		int size = iter->second.size();
		ll kmer = iter->fi;
		DEBUG("Starting from k-mer " << kmer);
		forn(i, size) {
			if ((!(iter->se)[i]->used)) {
				DEBUG("Starting seq " << kmer);

				int length = 0;
				int EdgeCoverage;
				count++;
				protoEdgeType curEdge;
				EdgePrototype* curEdgePrototype = (iter->se)[i];
				//curEdgePrototype->used = true;
				Sequence * startSeq = curEdgePrototype->lower;
				DEBUG("Starting seq " << startSeq->str());
				int curshift = 0;
				ll startKmer = kmer;
	//					subkmer(kmer, LEFT);


				expandDirected(edges, curEdge, graph.verts, startKmer, startSeq, EdgeCoverage, LEFT);




				//todo: rewrite
				DEBUG("Start find edge");
				int findCnt = 0;
				edgesMap::iterator cur_iter = edges.find(startKmer);
				if (cur_iter != edges.end()) {
					for (vector<EdgePrototype *>::iterator it = cur_iter->second.begin(); it != cur_iter->second.end(); ++it) {
						//TODO: minIntersect?
						if ((*it)->lower->size()>=startSeq->size())
						if (startSeq->similar(*((*it)->lower), startSeq->size(), 0)) {
							findCnt++;
							assert(findCnt<2);
							(*it)->used = true;
						}
					}
				}
				DEBUG("Finish find edge");





				Sequence *startSubSeq = SubSeq(*startSeq, LEFT);
				curEdge.first = "";
				curEdge.second = "";
				int startVertId = findPossibleVertex(subkmer(startKmer, LEFT), *startSubSeq, edges, graph.verts);

				LOG_ASSERT((startVertId != -2), " on " << subkmer(startKmer, LEFT));
				DEBUG("LEFT EDGE K_MER:" <<startKmer<< " seq "<<startSeq->str());
				ll finKmer = startKmer;
				Sequence *finSeq = new Sequence(*startSeq);
				curEdge.first = decompress(startKmer, k);
				curEdge.second = finSeq->str();
				expandDirected(edges, curEdge, graph.verts, finKmer, finSeq, EdgeCoverage, RIGHT);
				Sequence *finSubSeq = SubSeq(*finSeq, RIGHT);
				DEBUG("RIGHT VERTEX K_MER:" <<finKmer<<" seq "<<finSeq->str());
				int finVertId = findPossibleVertex(subkmer(finKmer, RIGHT), *finSubSeq, edges, graph.verts);
				assert(finVertId != -2);
				//TODO: what about loops?
				if (startVertId < 0) {
					startVertId = storeVertex(graph, subkmer(startKmer, LEFT), startSubSeq, true);
					TRACE("adding startVertId" <<  subkmer(startKmer, LEFT)<<" edge "<< startKmer);
				}
				if (finVertId < 0) {
					finVertId = storeVertex(graph, subkmer(finKmer, RIGHT), finSubSeq, true);
					TRACE("adding finVertId " << subkmer(finKmer, RIGHT)<<" edge "<<finKmer);

				}

				Edge* newEdge = new Edge(curEdge, startVertId, finVertId, graph.EdgeId, EdgeCoverage);
				graph.addEdge(newEdge);
				DEBUG("adding edge of length"<< curEdge.first.length()+1-k);
//				if (curEdge.first.length() <1000)
//					TRACE(curEdge.first);
//				assert(0);
				//expandDirected(edges, curEdge, graph.verts, startKmer, startSeq, EdgeCoverage, LEFT);
				if (!(iter->se)[i]->used) i--;
			}
		}
		(iter->second).clear();
		//TODO: clear memory
		edges.erase(iter++);
	}
}



}