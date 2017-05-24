#include "IntersectionPrimitives.h"

PolyIntersectionPoint::PolyIntersectionPoint() :
	poly{ NULL } {}

PolyIntersectionPoint::PolyIntersectionPoint(const PolyIntersectionPoint& p) :
	position{ p.position }, poly{ p.poly } {}

PolyIntersectionPoint::PolyIntersectionPoint(const glm::vec3& p, const PolygonIO* py) :
	position{ p }, poly{ py } {}

IntersectionPoint::IntersectionPoint() :
	object{ NULL } {}

IntersectionPoint::IntersectionPoint(const glm::vec3& p, const ObjIO *o, const PolyIntersectionPoint& pi) :
	position{ p }, object{ o }, polyIntersect{ pi } {}

IntersectionPoint intersectSphere(const glm::vec3& vec, const glm::vec3& origin, const ObjIO* sphere) {
	IntersectionPoint point;

	SphereIO* objData = (SphereIO*)sphere->data;

	glm::vec3 center(objData->origin[0], objData->origin[1], objData->origin[2]);
	glm::vec3 oToC = center - origin;
	float rayProj = glm::dot(vec, oToC);
	if (rayProj < 0) return point;

	float distFromCenter = sqrt(glm::length2(oToC) - rayProj * rayProj);
	if (distFromCenter > objData->radius) return point;
	float halfChordLength = sqrt(objData->radius * objData->radius - distFromCenter * distFromCenter);
	float distToIntersect;
	if (glm::length(oToC) > objData->radius) distToIntersect = rayProj - halfChordLength;
	else distToIntersect = rayProj + halfChordLength;

	point = { (distToIntersect * vec) + origin, sphere, PolyIntersectionPoint() };

	return point;
}

inline void cross(const glm::vec3& v1, const float* v2, float* res) {
	res[0] = v1[1] * v2[2] - v1[2] * v2[1];
	res[1] = v1[2] * v2[0] - v1[0] * v2[2];
	res[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

inline void cross(const float* v1, const float* v2, float* res) {
	res[0] = v1[1] * v2[2] - v1[2] * v2[1];
	res[1] = v1[2] * v2[0] - v1[0] * v2[2];
	res[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

inline float dot(const float* v1, const float* v2) {
	return v1[0] * v2[0] +
		v1[1] * v2[1] +
		v1[2] * v2[2];
}

inline float dot(const glm::vec3& v1, float* v2) {
	return v1[0] * v2[0] +
		v1[1] * v2[1] +
		v1[2] * v2[2];
}

PolyIntersectionPoint intersectPoly(const glm::vec3& vec, const glm::vec3& origin, const PolygonIO* poly) {
#ifdef DEBUG
	if (poly->numVertices != 3) {
		cerr << "ERROR: Polygon does not have 3 vertices" << endl;
		exit(1);
	}
#endif // DEBUG

	PolyIntersectionPoint point;

	float edge0[3] = {
		poly->vert[1].pos[0] - poly->vert[0].pos[0],
		poly->vert[1].pos[1] - poly->vert[0].pos[1],
		poly->vert[1].pos[2] - poly->vert[0].pos[2]
	};
	float edge1[3] = {
		poly->vert[2].pos[0] - poly->vert[0].pos[0],
		poly->vert[2].pos[1] - poly->vert[0].pos[1],
		poly->vert[2].pos[2] - poly->vert[0].pos[2]
	};
	float p[3];
	cross(vec, edge1, p);

	float determinant = dot(edge0, p);
	if (determinant > -EPSILON && determinant < EPSILON) return point;
	float invDeterminant = 1 / determinant;

	float vertToOrigin[3] = {
		origin[0] - poly->vert[0].pos[0],
		origin[1] - poly->vert[0].pos[1],
		origin[2] - poly->vert[0].pos[2]
	};

	float u = dot(vertToOrigin, p) * invDeterminant;
	if (u < 0 || u > 1) return point;

	float q[3];
	cross(vertToOrigin, edge0, q);
	float v = dot(vec, q) * invDeterminant;
	if (v < 0 || u + v > 1) return point;

	float t = dot(edge1, q) * invDeterminant;

	if (t > EPSILON) {
		point = { origin + t * vec, poly };
	}

	return point;
}

IntersectionPoint intersectPolySet(const glm::vec3& vec, const glm::vec3& origin, const ObjIO* shape) {
	IntersectionPoint finalPoint;

	PolySetIO * polySet = (PolySetIO *)shape->data;

#ifdef DEBUG
	if (polySet->type != POLYSET_TRI_MESH) {
		cerr << "ERROR: Polyset type is not POLYSET_TRI_MESH" << endl;
		exit(1);
	}
#endif // DEBUG

	PolygonIO* poly = polySet->poly;
	for (int i = 0; i < polySet->numPolys; i++, poly++) {
		PolyIntersectionPoint currPoint = intersectPoly(vec, origin, poly);
		if (currPoint.poly != NULL && (finalPoint.object == NULL ||
			glm::distance2(currPoint.position, origin) < glm::distance2(finalPoint.position, origin))) {
			finalPoint = IntersectionPoint(currPoint.position, shape, currPoint);
		}
	}

	return finalPoint;
}