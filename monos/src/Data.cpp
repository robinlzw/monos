/* monos is written in C++.  It computes the weighted straight skeleton
 * of a monotone polygon in asymptotic n log n time and linear space.
 *
 * Copyright 2018, 2019 Günther Eder - geder@cs.sbg.ac.at
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

#include "BasicInput.h"

#include "Data.h"
#include "tools.h"

std::ostream& operator<< (std::ostream& os, const MonotoneVector& mv) {
	os << mv.vector << " id(" << mv.id << ")";
	return os;
}


bool Data::ensureMonotonicity() {
	assert(input.edges().size() > 2);

	LOG(INFO) << "checking for monotonicity";

	auto edgeIt = input.edges().begin();
	auto edgeB  = edgeIt;
	auto edgeA  = edgeB;

	Vector vA, vB, intervalA, intervalB;
	Point corner;

	vB 		= input.get_segment(*edgeIt).to_vector();
	++edgeIt;


	intervalA = vA;
	intervalB = vA;

	std::set<MonotoneVector,MonVectCmp> intervals;
	ul idCnt = 0;

	/* iterate over polygon and obtaining the 'monotonicity angle'
	 * if it exists. */
	do {
		edgeA = edgeB;
		edgeB = edgeIt;
		vA = vB;
		vB = input.get_segment(*edgeIt).to_vector();
		corner = v(edgeA->v).p;
		/* ensure the vertex is reflex */
		if(CGAL::right_turn(corner-vA,corner,corner+vB)) {
			MonotoneVector a(vA,MonotoneType::START, idCnt);
			MonotoneVector b(vB,MonotoneType::END,   idCnt);
			intervals.insert(a);
			intervals.insert(b);

			a = MonotoneVector(Vector(-vA.x(),-vA.y()),MonotoneType::START,  idCnt+1);
			b = MonotoneVector(Vector(-vB.x(),-vB.y()),MonotoneType::END,    idCnt+1);
			intervals.insert(a);
			intervals.insert(b);

			idCnt+=2;
		}
	} while(++edgeIt != input.edges().end());

	if(intervals.empty()) {
		/* polygon is convex, let us choose the x-axis */
		setMonotonicity(Line(ORIGIN, ORIGIN + Vector(1,0)));
		return true;
	}

	/* iterate to first START vector */
	auto itStart  = intervals.begin();
	auto it 	  = itStart;
	int activeCnt = 0;		/* keep track how many intervals are active */
	std::vector<bool> activeIntervals(intervals.size(),false);

	for(;it != intervals.end(); ++it) {
		if(it->type == MonotoneType::END) {
			if(activeIntervals[it->id]) {
				activeIntervals[it->id] = false;
				--activeCnt;
			}
		}
		if(it->type == MonotoneType::START) {
			if(!activeIntervals[it->id]) {
				activeIntervals[it->id] = true;
				++activeCnt;
			}
		}
	}

	it 			    = itStart;
	bool success 	= false;

	if(activeCnt == 0) {
		success = true;
		Vector a = intervals.rbegin()->vector;
		Vector b = intervals.begin()->vector;
		Line line = getMonotonicityLineFromVector(a,b);
		if(testMonotonicityLineOnPolygon(line)) {
			setMonotonicity(line);
			return true;
		} else {
			assert(false);
		}
	} else {
		do {
			if(it->type == MonotoneType::END) {
				if(activeIntervals[it->id]) {
					activeIntervals[it->id] = false;
					--activeCnt;
				}
			}
			/* we found a window*/
			if(activeCnt == 0) {
				/* TODO VERIFY CORRECT LINE! */
				success = true;
			} else if(it->type == MonotoneType::START) {
				if(!activeIntervals[it->id]) {
					activeIntervals[it->id] = true;
					++activeCnt;
				}
			}

			if(success) {
				/* in case of a rectilinear monotone polygon we have two possible monotonoicity lines
				 * due to the angle intervals we stored, therfore we have to check if we found the  right
				 * one */
				Vector a = it->vector;
				++it;
				Vector b = it->vector;
				Line line = getMonotonicityLineFromVector(a,b);

				LOG(INFO) << "monotonicity Line " << line << " dir: "
						  << line.direction().to_vector() << " found ... testing.";

				if(testMonotonicityLineOnPolygon(line)) {
					setMonotonicity(line);
					return true;
					break;
				} else {
					success = false;
				}
			}

			/* iterate */
			if(++it == intervals.end()) {it = intervals.begin();}

		} while(!success && it != itStart);
	}

	/* construct the monotonicity line */
	if(!success) {
		LOG(WARNING) << "Polygon not monotone!";
		return false;
	}

	return true;
}

/* test if given line is a line where the input polygon is monotone to */
bool Data::testMonotonicityLineOnPolygon(const Line line) const {
	auto dir = line.direction().perpendicular(CGAL::POSITIVE);

	ul startIdx = 0;
	Point pStart  = eA(startIdx);
	auto testLine = Line(pStart,-dir);
	for(ul i = startIdx+1; i < input.edges().size(); ++i) {
		auto p = eA(i);
		if(!testLine.has_on_positive_side(p)) {
			startIdx = i; pStart = p;
			testLine = Line(pStart,-dir);
		}
	}

	/* startIdx is 'leftmost' edge, start walking 'rightwards' until we violate the monotonicity */
	bool monotone  = true, rightward = true;
	auto idxIt = startIdx;
	do {
		testLine = Line(eA(idxIt),-dir);
		if(rightward && !testLine.has_on_positive_side(eB(idxIt))) {
			rightward = false;
		} else if(!rightward && !testLine.has_on_negative_side(eB(idxIt))) {
			monotone = false;
		}

		if(++idxIt >= input.edges().size()) {idxIt = 0;}
	} while(monotone && idxIt != startIdx);

	return monotone;
}

Line Data::getMonotonicityLineFromVector(const Vector a, const Vector b) const {
	auto c = CGAL::bisector(Line(ORIGIN,a),Line(ORIGIN,b));
	Line l = c.perpendicular(ORIGIN);

	if(l.to_vector().x() < 0.0) {
		l = l.opposite();
	} else if(l.is_vertical() && l.to_vector().y() < 0.0) {
		l = l.opposite();
	}

	return l;
}


void Data::assignBoundingBox() {
	auto *xMin   = &getVertices()[0];
	auto *xMax   = &getVertices()[0];
	auto *yMin   = &getVertices()[0];
	auto *yMax   = &getVertices()[0];
	auto *monMin = &getVertices()[0];
	auto *monMax = &getVertices()[0];

	for(auto e = getPolygon().begin(); e != getPolygon().end(); ++e ) {
		if(e->u != xMin->id && v(e->u).p.x() < xMin->p.x()) {xMin = &v(e->u);}
		if(e->u != xMax->id && v(e->u).p.x() > xMax->p.x()) {xMax = &v(e->u);}
		if(e->u != yMin->id && v(e->u).p.y() < yMin->p.y()) {yMin = &v(e->u);}
		if(e->u != yMax->id && v(e->u).p.y() > yMax->p.y()) {yMax = &v(e->u);}

		if(v(e->u).id != monMin->id && v(e->u).p < monMin->p) {monMin = &v(e->u);}
		if(v(e->u).id != monMax->id && monMax->p < v(e->u).p) {monMax = &v(e->u);}
	}

	bbox = new BBox {
			{xMin->p, xMin->id},
			{xMax->p, xMax->id},
			{yMin->p, yMin->id},
			{yMax->p, yMax->id},
			{monMin->p, monMin->id},
			{monMax->p, monMax->id}
	};

	LOG(INFO)<< "monmin: " << *monMin << ", monMax: "  << *monMax;
}


void Data::printInput() const {
	std::stringstream ss;
	ss << "Input Vertices: " << std::endl;
	for(auto v : input.vertices()) {
		ss << "(" << v.p.x() << "," << v.p.y() << ") ";
	}
	ss << std::endl << "Input Polygon: " << std::endl;
	ul cnt = 0;
	for(auto edge : input.edges()) {
		auto seg = input.get_segment(edge);
		ss << "[" << edge.id <<  "](" << seg.source() << " -> " << seg.target() << ") | ";
	}
	LOG(INFO) << ss.str();
}


/*** true if a lies above b relative to monotonicity line */
//bool Data::isAbove(const Point& a, const Point &b) const {
//	return a.y > b.y;
////	bool aAbove = monotonicityLine.has_on_positive_side(a);
////	bool bAbove = monotonicityLine.has_on_positive_side(b);
////	auto distA = CGAL::squared_distance(monotonicityLine,a);
////	auto distB = CGAL::squared_distance(monotonicityLine,b);
////	return ( aAbove && !bAbove) ||
////		   ( aAbove &&  bAbove && distA > distB) ||
////		   (!aAbove && !bAbove && distA < distB);
//}

void Data::printLineFormat() {
	for(auto e : getPolygon()) {
		std::cout << p(e.u) << std::endl;
	}
	std::cout << p(getPolygon().back().v) << std::endl;
}

/*
 * NOTE: this only works if writeOBJ was invoced before, as we only
 * add a face with the indices that require the vertices already in
 * the file
 * */
void Data::addPolyToOBJ(const Config& cfg) const {
	double xt, yt, zt, xm, ym, zm;
	getNormalizer(*bbox,xt,xm,yt,ym,zt,zm);

	std::ofstream outfile (cfg.outputFileName,std::ofstream::binary | std::ofstream::app);

	outfile << "f";
	for(auto e : getPolygon()) {
		outfile << " " << e.u+1;
	}

	outfile << std::endl;
	outfile.close();
}
