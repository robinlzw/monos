#ifndef SKELETON_H_
#define SKELETON_H_

#include <algorithm>
#include <vector>

/* we store the skeletal structure in two types: nodes and arcs
 * a node holds a vector with the incidences to the incident arcs
 * these arcs are/can be sorted CCW around a node
 * an arc references its 'first' and 'second' node as well as
 * the left and right 'face' (edge, since every edge has only one face) */

#include "tools.h"
#include "Config.h"

#include "cgTypes.h"

#include "Data.h"
#include "Wavefront.h"


/* FIRST is UpperChainIntersection / SECOND is LowerChainIntersection*/
using IntersectionPair = std::pair<Point,Point>;

class Skeleton {
public:
	Skeleton(Data& _data, Wavefront& _wf) :
		data(_data),wf(_wf) {}

	~Skeleton() {}

	/* running the merge by one call */
	void MergeUpperLowerSkeleton();

	void initMerge();
	bool SingleMergeStep();
	void finishMerge();

	void writeOBJ(const Config& cfg) const;

	bool computationFinished = false;

	void storeChains(Chain upper, Chain lower) {
		upperChain = upper;
		lowerChain = lower;
	}

private:
	/* return std::pair upper/lower intersection Point */
	IntersectionPair findNextIntersectingArc(const Line& bis);

	void removePath(const ul& arcIdx, const ul& edgeIdx);

	ul handleMerge(const IntersectionPair& intersectionPair);
	void updateArcTarget(const ul& arcIdx, const ul& edgeIdx, const int& secondNodeIdx, const Point& edgeEndPoint);

	bool EndOfBothChains() const {return EndOfUpperChain() && EndOfLowerChain();  }
	bool EndOfUpperChain() const {return upperChainIndex == upperChain.front(); }
	bool EndOfLowerChain() const {return lowerChainIndex == lowerChain.back(); }
	bool EndOfChain(ChainType t) { return (t == ChainType::UPPER) ? EndOfUpperChain() : EndOfLowerChain();}

	void initPathForEdge(ChainType type);

	inline bool isIntersecting(const Line& l, const Arc& arc) {
		bool a = l.has_on_positive_side(arc.source());
		bool b = l.has_on_positive_side(arc.target());
		return (a && !b) || (!a && b);
	}

	/* true is formward, false is backwards */
	bool decideDirection(ChainType type, const Line& bis) const;

	/* for the merge to keep track of the current state */
	ul upperPath = MAX, lowerPath = MAX;

	Data& 		data;
	Wavefront& 	wf;

	Chain  		upperChain, lowerChain;

	/* using as state for the merge */
	Node* sourceNode = nullptr;
	ul sourceNodeIdx = 0, newNodeIdx = 0;

	ul upperChainIndex = 0, lowerChainIndex = 0;

	/*********************************************************/
	/* 		state variables of findNextIntersectingArc 		 */
//	Point Pu = INFPOINT;
//	Point Pl = INFPOINT;
//	Point uPa, uPb, lPa, lPb;
//	bool doneU = false, doneL = false;
//	bool iterateForwardU = true, iterateForwardL = true;
//	Arc* upperArc = nullptr; Arc* lowerArc = nullptr;
//	ChainType searchChain;
	/*********************************************************/
};

#endif /* SKELETON_H_ */
