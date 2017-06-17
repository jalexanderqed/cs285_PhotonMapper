#include "PolyBound.h"

#define MAX_DEPTH 12
#define MIN_SIZE 5

void PolyBound::addPolygon(const PolygonIO* p) {
	polySet.push_back(p);
	boundBox.apply(BoundingBox(p));
}

PolyBound::~PolyBound() {
	for (PolyBound* p : children) {
		delete p;
	}
}

PolyBound::PolyBound(const ObjIO* p, int d) : parentObject{ p },
depth{ d }
{ }

void PolyBound::split() {
	if (depth >= MAX_DEPTH ||
		polySet.size() < MIN_SIZE) {
		return;
	}

	glm::vec3 axisAverage = boundBox.vMin + 0.5f * (boundBox.vMax - boundBox.vMin);
	int axisCounts[3][3] = { {0, 0, 0},
	{ 0, 0, 0 },
	{ 0, 0, 0 } };

	for (const PolygonIO* poly : polySet) {
		BoundingBox polyBox(poly);
		for (int axis = 0; axis < 3; axis++) {
			if (polyBox.vMax[axis] < axisAverage[axis]) {
				axisCounts[axis][0]++;
			}
			else if (polyBox.vMin[axis] > axisAverage[axis]) {
				axisCounts[axis][2]++;
			}
			else {
				axisCounts[axis][1]++;
			}
		}
	}

	int minCross = INT_MAX;
	for (int axis = 0; axis < 3; axis++) {
		minCross = min(minCross, axisCounts[axis][1]);
	}
	if (minCross <= 0) minCross = 1;

	float scores[3];
	for (int axis = 0; axis < 3; axis++) {
		scores[axis] = (((float)axisCounts[axis][1]) / minCross) +
			(((float)max(axisCounts[axis][0], axisCounts[axis][2])) / min(axisCounts[axis][0], axisCounts[axis][2]));
	}

	int bestAxis = -1;
	for (int axis = 0; axis < 3; axis++) {
		if (
			(bestAxis == -1 ||
				scores[axis] < scores[bestAxis])
			) {
			bestAxis = axis;
		}
	}

	PolyBound* less = new PolyBound(parentObject, depth + 1);
	PolyBound* cross = new PolyBound(parentObject, depth + 1);
	PolyBound* more = new PolyBound(parentObject, depth + 1);
	for (const PolygonIO* poly : polySet) {
		BoundingBox polyBox(poly);
		if (polyBox.vMax[bestAxis] < axisAverage[bestAxis]) {
			less->addPolygon(poly);
		}
		else if (polyBox.vMin[bestAxis] > axisAverage[bestAxis]) {
			more->addPolygon(poly);
		}
		else {
			cross->addPolygon(poly);
		}
	}
	children.push_back(less);
	children.push_back(cross);
	children.push_back(more);

	for (PolyBound* p : children) {
		p->split();
	}
}

bool PolyBound::mightIntersect(const glm::vec3& vec, const glm::vec3& origin, glm::vec3& res) {
	if (polySet.size() == 0) {
		return false;
	}
	else {
		return boundBox.intersect(vec, origin, res);
	}
}

struct DistPolyPair {
	float dist;
	PolyBound* poly;
};

IntersectionPoint PolyBound::intersect(const glm::vec3& vec, const glm::vec3& origin) {
	IntersectionPoint finalPoint;
	if (children.size() == 0) {
		for (const PolygonIO* poly : polySet) {
			PolyIntersectionPoint currPoint = intersectPoly(vec, origin, poly);
			if (currPoint.poly != NULL && (finalPoint.object == NULL ||
				glm::distance2(currPoint.position, origin) < glm::distance2(finalPoint.position, origin))) {
				finalPoint = IntersectionPoint(currPoint.position, parentObject, currPoint);
			}
		}
	}
	else {		
		float finalDist = INFINITY;
		for (PolyBound* p : children) {
			glm::vec3 interPoint;
			if (p->mightIntersect(vec, origin, interPoint)) {
				IntersectionPoint currPoint = p->intersect(vec, origin);
				float currDist = currPoint.object == NULL ? 0 : glm::distance2(currPoint.position, origin);
				if (currPoint.object != NULL && (finalPoint.object == NULL ||
					currDist < finalDist)) {
					finalPoint = currPoint;
					finalDist = currDist;
				}
			}
		}
	}

	return finalPoint;
}

bool PolyBound::insertPhoton(Photon photon) {
	if (children.size() == 0) {
		photons.push_back(photon);
		return true;
	}
	else {
		for (PolyBound* p : children) {
			if (p->boundBox.inside(photon.position) &&
				p->insertPhoton(photon))
				return true;
		}
	}
	return false;
}

list<Photon> PolyBound::getPhotons(const glm::vec3& position, float radius) {
	list<Photon> res;
	if (children.size() == 0) {
		float radius2 = radius * radius;
		for (Photon photon : photons) {
			if (glm::distance2(photon.position, position) <= radius2) {
				res.push_back(photon);
			}
		}
	}
	else {
		for (PolyBound* p : children) {
			if (p->boundBox.intersectsBox(position, radius)) {
				list<Photon> pRes = p->getPhotons(position, radius);
				if (pRes.size() > 0) res.splice(res.end(), pRes);
			}
		}
	}
	return res;
}