#ifndef VARDCONSTRUCTION_H_
#define VARDCONSTRUCTION_H_
#include "pairedGraph.hpp"
#include "common.hpp"
#include "graphVisualizer.hpp"

using namespace paired_assembler;

namespace vard{


void createVertices(edgesMap &edges, PairedGraph &graph);
//int expandRight(edgesMap &edges, verticesMap &verts, ll &finishKmer, Sequence* &finishSeq, int &EdgeCoverage);
int expandDirected(edgesMap &edges, protoEdgeType &curEdge, verticesMap &verts, ll &curKmer, Sequence* &curSeq, int &EdgeCoverage, int direction);
pair<char, EdgePrototype*> findUniqueWay(edgesMap &edges, ll finishKmer, Sequence *finishSeq , int direction, bool replace = false);
int goUniqueWay(edgesMap &edges, ll &finishKmer, Sequence* &finishSeq, pair<char, EdgePrototype*>, int &EdgeCoverage, int direction);
}


#endif /*VARDCONSTRUCTION_H_*/