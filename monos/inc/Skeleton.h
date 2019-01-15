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

class Skeleton {
public:
	Skeleton(const Data& _data, Wavefront& _wf) :
		data(_data),wf(_wf) {
	}
	~Skeleton() {}

	void MergeUpperLowerSkeleton();

	void printSkeleton() const;
	void writeOBJ(const Config& cfg) const;

private:
	bool findRayFaceIntersection(const uint& edgeIdx, const Ray& ray, const bool upperChain, uint& arcIdx, Point& intersection);
	bool nextArcOnPath(const uint& arcIdx, const uint& edgeIdx, uint& nextArcIdx) const;

	uint addNode(const Point& intersection);
	uint handleMerge(const std::vector<uint>& arcIndices, const uint& edgeIdxA, const uint& edgeIdxB, const Point& p, const Ray& bis);
	void updateArcTarget(const uint& arcIdx, const int& secondNodeIdx, const Point& edgeEndPoint);

	uint nextUpperChainIndex(const uint& idx) const;
	uint nextLowerChainIndex(const uint& idx) const;

	uint mergeStartNodeIdx() const {return data.e(wf.startLowerEdgeIdx)[0];}
	uint mergeEndNodeIdx() const {return data.e(wf.endLowerEdgeIdx)[1];}
	bool isMergeStartEndNodeIdx(const uint& idx) const {return idx == mergeStartNodeIdx() || idx == mergeEndNodeIdx();}

	Point& getSourceNodePoint() const { return wf.nodes[sourceNodeIdx].point; }
	Point intersectArcRay(const Arc& arc, const Ray& ray) const {
		return (arc.type == ArcType::NORMAL) ? intersectElements(ray, arc.edge) : intersectElements(ray, arc.ray);
	}

	const Data& data;
	Wavefront& wf;

	uint sourceNodeIdx;
	uint startIdxMergeNodes;
};

#endif /* SKELETON_H_ */