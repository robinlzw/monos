/*
 * monos is written in C++.  It computes the weighted straight skeleton
 * of a monotone polygon in asymptotic n log n time and linear space.
 * Copyright (C) 2018 - Günther Eder - geder@cs.sbg.ac.at
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CGTYPES_H_
#define CGTYPES_H_

#include <iterator>
#include <array>
#include <algorithm>
#include <functional>
#include <queue>
#include <set>
#include <cmath>

#include "Definitions.h"


#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/Aff_transformation_2.h>
#include <CGAL/aff_transformation_tags.h>
#include <CGAL/intersections.h>
#include <CGAL/squared_distance_2.h>

using K 			 = CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt;

using Vector         = K::Vector_2;
using Point          = K::Point_2;
using Line           = K::Line_2;
using Ray            = K::Ray_2;
using Circle         = K::Circle_2;
using Direction      = K::Direction_2;
using Edge       	 = K::Segment_2;
using Intersect		 = K::Intersect_2;
using Transformation = CGAL::Aff_transformation_2<K>;
using Exact          = K::FT;


using InputPoints   	= std::vector<Point>;
using InputWeights  	= std::vector<Exact>;
using IndexEdge 		= std::array<uint,2>;
using Polygon   		= std::vector<IndexEdge>;
using Chain 			= std::list<uint>;
using ChainRef			= Chain::iterator;
using PartialSkeleton 	= std::list<uint>;
using PointIterator 	= std::vector<Point,std::allocator<Point>>::const_iterator;


#include "gml/BasicInput.h"
#include "gml/GMLGraph.h"

static Point ORIGIN = Point(0,0);

class Bisector {
public:
	Bisector(Ray r) :type(BisType::RAY), ray(r)  {}
	Bisector(Line l):type(BisType::LINE),line(l) {}

	BisType type;
	Ray     ray;
	Line    line;

	bool isRay()  const { return type == BisType::RAY; }
	bool isLine() const { return type == BisType::LINE;}

	Direction direction() const  { return (isRay()) ? ray.direction() : line.direction(); }
	Line supporting_line() const { return (isRay()) ? ray.supporting_line() : line; }
	Point point(uint i = 0) const { return supporting_line().point(i); }
	Vector to_vector() const { return (isRay()) ? ray.to_vector() : line.to_vector(); }

	void setRay(const Ray r) {ray = Ray(r); type = BisType::RAY;}

	void setPerpendicular() {perpendicular = true;}
	bool isPerpendicular() const {return perpendicular;}

	void changeDirection() {
		if(isRay()) {
			ray = ray.opposite();
		} else {
			line = line.opposite();
		}
	}

	void newSource(const Point s) {
		if(isRay()) {
			ray = Ray(s,direction());
		} else {
			line = Line(s,direction());
		}
	}

private:
	/* true if perpendicular to monotonicity line */
	bool perpendicular = false;

	friend std::ostream& operator<< (std::ostream& os, const Bisector& bis);
};


/** stores the indices of the three input points that define max/min x/y*/
struct BBox {
	BBox(uint _xMinIdx = 0, uint _xMaxIdx = 0, uint _yMinIdx = 0, uint _yMaxIdx = 0,
  		 Exact _xMin = 0, Exact _xMax = 0, Exact _yMin = 0, Exact _yMax = 0 ):
		 xMinIdx(_xMinIdx),xMaxIdx(_xMaxIdx), yMinIdx(_yMinIdx), yMaxIdx(_yMaxIdx),
		 xMin(_xMin),xMax(_xMax),yMin(_yMin),yMax(_yMax),
		 monotoneMinIdx(0),monotoneMaxIdx(0) {}

	uint xMinIdx, xMaxIdx, yMinIdx, yMaxIdx;
	Exact xMin, xMax, yMin, yMax;

	Edge top, bottom, left, right;

	uint  monotoneMinIdx, monotoneMaxIdx;
	Point monotoneMin,    monotoneMax;

	bool outside(const Point& p) const {return p.x() < xMin || xMax < p.x() ||
			       	   	   	   	  p.y() < yMin || yMax < p.y();  }
	bool onBoundary(const Point& p) const {return inside(p) && (p.x() == xMin || xMax == p.x() ||
			       	   	   	   	  p.y() == yMin || yMax == p.y());  }
	bool inside(const Point& p) const {return !outside(p);}
};

class Event {
	using EventEdges = std::array<uint, 3>;

public:
	Event(Exact time = 0, Point point = INFPOINT, uint edgeA = 0, uint edgeB = 0, uint edgeC = 0, ChainRef ref = ChainRef()):
		eventTime(time),eventPoint(point),edges{{edgeA,edgeB,edgeC}}, chainEdge(ref) {}

	bool isEvent()   const { return eventPoint != INFPOINT;}
	uint leftEdge()  const { return edges[0]; }
	uint mainEdge()  const { return edges[1]; }
	uint rightEdge() const { return edges[2]; }

	Exact         	eventTime;
	Point  			eventPoint;
	EventEdges		edges;

	ChainRef 		chainEdge;

	friend std::ostream& operator<< (std::ostream& os, const Event& event);
};

typedef struct _TimeEdge {
	_TimeEdge(Exact t, uint e):time(t),edgeIdx(e) {}
	Exact time;
	uint  edgeIdx;
} TimeEdge;

struct TimeEdgeCmp {
	bool operator()(const TimeEdge &left, const TimeEdge &right) const {
		return (left.time) < (right.time) || (left.time == right.time && left.edgeIdx < right.edgeIdx);
	}
};

class Arc {
public:
	Arc(ArcType t, uint firstNode, uint leftEdge, uint rightEdge, Ray r):
		type(t), firstNodeIdx(firstNode), secondNodeIdx(MAX),
		leftEdgeIdx(leftEdge), rightEdgeIdx(rightEdge),
		edge(Edge()),ray(r) {}
	Arc(ArcType t, uint firstNode, uint secondNode, uint leftEdge, uint rightEdge, Edge e):
		type(t), firstNodeIdx(firstNode), secondNodeIdx(secondNode),
		leftEdgeIdx(leftEdge), rightEdgeIdx(rightEdge),
		edge(e),ray(Ray()) {}

	void disable() {type=ArcType::DISABLED;}
	bool isDisable() {return type == ArcType::DISABLED;}

	bool adjacent(const Arc& arc) const {
		return firstNodeIdx  == arc.firstNodeIdx || firstNodeIdx  == arc.secondNodeIdx ||
			   secondNodeIdx == arc.firstNodeIdx || secondNodeIdx == arc.secondNodeIdx;
	}

	uint getSecondNodeIdx(const uint idx) const { return (idx == firstNodeIdx) ? secondNodeIdx : firstNodeIdx; }

	Line supporting_line() const {return (isEdge()) ? edge.supporting_line() : ray.supporting_line();}

	bool isRay() const { return type == ArcType::RAY; }
	bool isEdge() const { return type == ArcType::NORMAL; }

	void setPerpendicular() {perpendicular = true;}
	bool isPerpendicular() const { return perpendicular; }

	ArcType type;
	uint firstNodeIdx, secondNodeIdx;
	uint leftEdgeIdx,  rightEdgeIdx;

	Edge edge;
	Ray  ray;

private:
	bool perpendicular = false;

	friend std::ostream& operator<< (std::ostream& os, const Arc& arc);
};

using ArcList		= std::vector<Arc>;

struct ArcCmp {
	ArcCmp(const ArcList& list):arcList(list) {}
	const ArcList& arcList;
	bool operator()(const uint &left, const uint &right) const {
		auto leftArc  = &(arcList)[left];
		auto rightArc = &(arcList)[right];
		return (   (leftArc->firstNodeIdx  == rightArc->firstNodeIdx  || leftArc->secondNodeIdx  == rightArc->secondNodeIdx) && leftArc->rightEdgeIdx == rightArc->leftEdgeIdx)
				|| (leftArc->firstNodeIdx  == rightArc->secondNodeIdx && leftArc->rightEdgeIdx   == rightArc->rightEdgeIdx)
				|| (leftArc->secondNodeIdx == rightArc->firstNodeIdx  && leftArc->leftEdgeIdx    == rightArc->leftEdgeIdx);
	}
};


struct Node {
	Node(const NodeType t, const Point p, Exact time): type(t), point(p), time(time) {}

	NodeType 		type;
	Point			point;
	Exact			time;

	/* all incident arcs, i.e., the indices to them */
	std::vector<uint> 	arcs;

	void disable() {type = NodeType::DISABLED;}
	bool isDisabled() const { return type == NodeType::DISABLED;}
	bool isTerminal() const { return type == NodeType::TERMINAL;}

	void sort(const ArcList& arcList) {
		std::sort(arcs.begin(), arcs.end(), ArcCmp(arcList));
	}
};

/* the inputPoints index equals the terminal node index,
 * as from every vertex emits an arc */
using Nodes 		= std::vector<Node>;
using PathFinder    = std::vector<IndexEdge>;

Exact normalDistance(const Line& l, const Point& p);

template<class T, class U>
Point intersectElements(const T& a, const U& b);

template<class T, class U>
bool isLinesParallel(const T& a, const U& b);

template<class T, class U>
Point intersectElements(const T& a, const U& b) {
	Point intersectionPoint = INFPOINT;

	std::cout << "(" << a << " -- " << b; fflush(stdout);
	auto result = CGAL::intersection(a, b);
	std::cout << ")" << std::endl; fflush(stdout);
	if (result) {
		if (const Point* p = boost::get<Point>(&*result)) {
			return Point(*p);
		} else {
			LOG(WARNING) << "Intersection forms a segment/ray/line";
			const Edge* s = boost::get<Edge>(&*result);
			std::cout << *s << std::endl;
			return intersectionPoint;
		}
	}
	return intersectionPoint;
}

Point intersectRayArc(const Ray& ray, const Arc& arc);
Point intersectBisectorArc(const Bisector& bis, const Arc& arc);

template<class T, class U>
bool isLinesParallel(const T& a, const U& b) {
	return CGAL::parallel(Line(a),Line(b));
}

bool do_intersect(const Bisector& ray, const Arc& arc);
bool do_intersect(const Bisector& ray, const Edge& edge);

#endif /* CGALTYPES_H_ */
