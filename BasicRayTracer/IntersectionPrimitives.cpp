#include "IntersectionPrimitives.h"

glm::vec3 getBarycentricWeights(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& point) {
	glm::vec3 res;
	float sideLengths[3] = { glm::distance(p1, p2),
		glm::distance(p2, p3),
		glm::distance(p3, p1) };
	const glm::vec3* verts[4] = { &p1, &p2, &p3, &p1 };

	for (int i = 0; i < 3; i++) {
		float a = sideLengths[i];
		float b = glm::distance(*verts[i], point);
		float c = glm::distance(*verts[i + 1], point);
		float p = (a + b + c) / 2.0f;
		res[i] = max(sqrt(p * (p - a) * (p - b) * (p - c)), 0.0f);
	}
	float sum = res.x + res.y + res.z;
	res *= 1.0f / sum;
	return res;
}

glm::vec3 interpolateVecs(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& inter1, const glm::vec3& inter2, const glm::vec3& inter3,
	const glm::vec3& point) {
	glm::vec3 weights = getBarycentricWeights(p1, p2, p3, point);
	return inter3 * weights.x + inter1 * weights.y + inter2 * weights.z;
}

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

void calcUVSphere(const IntersectionPoint& iPoint, float& u, float& v) {
	SphereIO* data = (SphereIO*)iPoint.object->data;
	glm::vec3 n = glm::normalize(iPoint.position -
		glm::vec3(data->origin[0], data->origin[1], data->origin[2]));
	u = abs((float)atan2(n.x, n.z) / (2 * (float)M_PI) + 0.5f);
	v = n.y * 0.5f + 0.5f;

	u = max(min(u, 0.9999f), 0.0f);
	v = max(min(v, 0.9999f), 0.0f);
	if (!isfinite(u)) u = 0;
	if (!isfinite(v)) v = 0;
}

void calcUVPoly(const PolyIntersectionPoint& iPoint, float& u, float& v) {
	glm::vec3 corner1(iPoint.poly->vert[0].pos[0],
		iPoint.poly->vert[0].pos[1],
		iPoint.poly->vert[0].pos[2]);
	glm::vec3 corner2(iPoint.poly->vert[1].pos[0],
		iPoint.poly->vert[1].pos[1],
		iPoint.poly->vert[1].pos[2]);
	glm::vec3 corner3(iPoint.poly->vert[2].pos[0],
		iPoint.poly->vert[2].pos[1],
		iPoint.poly->vert[2].pos[2]);

	float dist12 = glm::distance2(corner1, corner2);
	float dist23 = glm::distance2(corner3, corner2);
	float dist13 = glm::distance2(corner1, corner3);

	glm::vec3 *corner, *o1, *o2;

	if (dist12 > dist23 && dist12 > dist13) {
		corner = &corner3;
		o1 = &corner1;
		o2 = &corner2;
	}
	else if (dist23 > dist12 && dist23 > dist13) {
		corner = &corner1;
		o1 = &corner2;
		o2 = &corner3;
	}
	else if (dist13 > dist23 && dist13 > dist12) {
		corner = &corner2;
		o1 = &corner1;
		o2 = &corner3;
	}

	glm::vec3 inter = interpolateVecs(*corner, *o1, *o2,
		glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), iPoint.position);
	u = inter.x;
	v = inter.y;

	u = max(min(u, 0.9999f), 0.0f);
	v = max(min(v, 0.9999f), 0.0f);
	if (!isfinite(u)) u = 0;
	if (!isfinite(v)) v = 0;
}

bool intersectionShader(const PolyIntersectionPoint& iPoint) {
	return true;
}

bool intersectionShader(const IntersectionPoint& iPoint) {
	if (iPoint.object->type == SPHERE_OBJ) {
		float u, v;
		calcUVSphere(iPoint, u, v);

		v = 1 - v;
		if (v >= 1) v = 0.999f;

		if (sphereMap.count(iPoint.object) > 0) {
			float sphereInd = sphereMap.at(iPoint.object);
			float rotate = sphereInd / sphereMap.size();
			if (u >= 1.0f - rotate) u += -1 + rotate;
			else u += rotate;
		}

		COLORREF color = texture1.GetPixel((int)(u * texture1.GetWidth()), (int)(v * texture1.GetHeight()));
		if (color == CLR_INVALID) {
			cerr << "Could not load color value at " <<
				(int)(u * texture1.GetWidth()) << ", " <<
				(int)(v * texture1.GetHeight()) << endl;
			cerr << "u: " << u << endl;
			cerr << "v: " << v << endl;
			exit(1);
		}

		glm::vec3 vecColor(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
		if (sphereMap.count(iPoint.object) > 0) {
			switch (sphereMap.at(iPoint.object)) {
			case 2:
			case 4:
				if (glm::normalize(vecColor).b > 0.5f) return false;
				break;
			default:

				break;
			}
		}
		return true;
	}
	else {
		return true;
	}
}

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

	if (complexIntersectShaders && !intersectionShader(point)) {
		point = IntersectionPoint();
	}

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

	if (point.poly != NULL && complexIntersectShaders && !intersectionShader(point)) {
		point = PolyIntersectionPoint();
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