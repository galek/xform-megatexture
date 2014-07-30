#include <xForm.h>

xBox box_zero(vec3_zero, vec3_zero, mat3_identity);

/*
            4---{4}---5
 +         /|        /|
 Z      {7} {8}   {5} |
 -     /    |    /    {9}
      7--{6}----6     |
      |     |   |     |
    {11}    0---|-{0}-1
      |    /    |    /       -
      | {3}  {10} {1}       Y
      |/        |/         +
      3---{2}---2

	    - X +

  plane bits:
  0 = min x
  1 = max x
  2 = min y
  3 = max y
  4 = min z
  5 = max z

*/

static int boxVertPlanes[8] = {
	((1<<0) | (1<<2) | (1<<4)),
	((1<<1) | (1<<2) | (1<<4)),
	((1<<1) | (1<<3) | (1<<4)),
	((1<<0) | (1<<3) | (1<<4)),
	((1<<0) | (1<<2) | (1<<5)),
	((1<<1) | (1<<2) | (1<<5)),
	((1<<1) | (1<<3) | (1<<5)),
	((1<<0) | (1<<3) | (1<<5))
};

static int boxVertEdges[8][3] = {
	// bottom
	{ 3, 0, 8 },
	{ 0, 1, 9 },
	{ 1, 2, 10 },
	{ 2, 3, 11 },
	// top
	{ 7, 4, 8 },
	{ 4, 5, 9 },
	{ 5, 6, 10 },
	{ 6, 7, 11 }
};

static int boxEdgePlanes[12][2] = {
	// bottom
	{ 4, 2 },
	{ 4, 1 },
	{ 4, 3 },
	{ 4, 0 },
	// top
	{ 5, 2 },
	{ 5, 1 },
	{ 5, 3 },
	{ 5, 0 },
	// sides
	{ 0, 2 },
	{ 2, 1 },
	{ 1, 3 },
	{ 3, 0 }
};

static int boxEdgeVerts[12][2] = {
	// bottom
	{ 0, 1 },
	{ 1, 2 },
	{ 2, 3 },
	{ 3, 0 },
	// top
	{ 4, 5 },
	{ 5, 6 },
	{ 6, 7 },
	{ 7, 4 },
	// sides
	{ 0, 4 },
	{ 1, 5 },
	{ 2, 6 },
	{ 3, 7 }
};

static int boxPlaneBitsSilVerts[64][7] = {
	{ 0, 0, 0, 0, 0, 0, 0 }, // 000000 = 0
	{ 4, 7, 4, 0, 3, 0, 0 }, // 000001 = 1
	{ 4, 5, 6, 2, 1, 0, 0 }, // 000010 = 2
	{ 0, 0, 0, 0, 0, 0, 0 }, // 000011 = 3
	{ 4, 4, 5, 1, 0, 0, 0 }, // 000100 = 4
	{ 6, 3, 7, 4, 5, 1, 0 }, // 000101 = 5
	{ 6, 4, 5, 6, 2, 1, 0 }, // 000110 = 6
	{ 0, 0, 0, 0, 0, 0, 0 }, // 000111 = 7
	{ 4, 6, 7, 3, 2, 0, 0 }, // 001000 = 8
	{ 6, 6, 7, 4, 0, 3, 2 }, // 001001 = 9
	{ 6, 5, 6, 7, 3, 2, 1 }, // 001010 = 10
	{ 0, 0, 0, 0, 0, 0, 0 }, // 001011 = 11
	{ 0, 0, 0, 0, 0, 0, 0 }, // 001100 = 12
	{ 0, 0, 0, 0, 0, 0, 0 }, // 001101 = 13
	{ 0, 0, 0, 0, 0, 0, 0 }, // 001110 = 14
	{ 0, 0, 0, 0, 0, 0, 0 }, // 001111 = 15
	{ 4, 0, 1, 2, 3, 0, 0 }, // 010000 = 16
	{ 6, 0, 1, 2, 3, 7, 4 }, // 010001 = 17
	{ 6, 3, 2, 6, 5, 1, 0 }, // 010010 = 18
	{ 0, 0, 0, 0, 0, 0, 0 }, // 010011 = 19
	{ 6, 1, 2, 3, 0, 4, 5 }, // 010100 = 20
	{ 6, 1, 2, 3, 7, 4, 5 }, // 010101 = 21
	{ 6, 2, 3, 0, 4, 5, 6 }, // 010110 = 22
	{ 0, 0, 0, 0, 0, 0, 0 }, // 010111 = 23
	{ 6, 0, 1, 2, 6, 7, 3 }, // 011000 = 24
	{ 6, 0, 1, 2, 6, 7, 4 }, // 011001 = 25
	{ 6, 0, 1, 5, 6, 7, 3 }, // 011010 = 26
	{ 0, 0, 0, 0, 0, 0, 0 }, // 011011 = 27
	{ 0, 0, 0, 0, 0, 0, 0 }, // 011100 = 28
	{ 0, 0, 0, 0, 0, 0, 0 }, // 011101 = 29
	{ 0, 0, 0, 0, 0, 0, 0 }, // 011110 = 30
	{ 0, 0, 0, 0, 0, 0, 0 }, // 011111 = 31
	{ 4, 7, 6, 5, 4, 0, 0 }, // 100000 = 32
	{ 6, 7, 6, 5, 4, 0, 3 }, // 100001 = 33
	{ 6, 5, 4, 7, 6, 2, 1 }, // 100010 = 34
	{ 0, 0, 0, 0, 0, 0, 0 }, // 100011 = 35
	{ 6, 4, 7, 6, 5, 1, 0 }, // 100100 = 36
	{ 6, 3, 7, 6, 5, 1, 0 }, // 100101 = 37
	{ 6, 4, 7, 6, 2, 1, 0 }, // 100110 = 38
	{ 0, 0, 0, 0, 0, 0, 0 }, // 100111 = 39
	{ 6, 6, 5, 4, 7, 3, 2 }, // 101000 = 40
	{ 6, 6, 5, 4, 0, 3, 2 }, // 101001 = 41
	{ 6, 5, 4, 7, 3, 2, 1 }, // 101010 = 42
	{ 0, 0, 0, 0, 0, 0, 0 }, // 101011 = 43
	{ 0, 0, 0, 0, 0, 0, 0 }, // 101100 = 44
	{ 0, 0, 0, 0, 0, 0, 0 }, // 101101 = 45
	{ 0, 0, 0, 0, 0, 0, 0 }, // 101110 = 46
	{ 0, 0, 0, 0, 0, 0, 0 }, // 101111 = 47
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110000 = 48
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110001 = 49
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110010 = 50
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110011 = 51
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110100 = 52
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110101 = 53
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110110 = 54
	{ 0, 0, 0, 0, 0, 0, 0 }, // 110111 = 55
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111000 = 56
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111001 = 57
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111010 = 58
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111011 = 59
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111100 = 60
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111101 = 61
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111110 = 62
	{ 0, 0, 0, 0, 0, 0, 0 }, // 111111 = 63
};


/*
============
xBox::Add
============
*/
bool xBox::Add(const xVec3 &v) {
	xMat3 axis2;
	xBounds bounds1, bounds2;

	if (extents[0] < 0.0f) {
		extents.Zero();
		center = v;
		axis.Identity();
		return true;
	}

	bounds1[0][0] = bounds1[1][0] = center * axis[0];
	bounds1[0][1] = bounds1[1][1] = center * axis[1];
	bounds1[0][2] = bounds1[1][2] = center * axis[2];
	bounds1[0] -= extents;
	bounds1[1] += extents;
	if (!bounds1.Add(xVec3(v * axis[0], v * axis[1], v * axis[2]))) {
		// point is contained in the box
		return false;
	}

	axis2[0] = v - center;
	axis2[0].Normalize();
	axis2[1] = axis[ Min3Index(axis2[0] * axis[0], axis2[0] * axis[1], axis2[0] * axis[2]) ];
	axis2[1] = axis2[1] - (axis2[1] * axis2[0]) * axis2[0];
	axis2[1].Normalize();
	axis2[2].Cross(axis2[0], axis2[1]);

	AxisProjection(axis2, bounds2);
	bounds2.Add(xVec3(v * axis2[0], v * axis2[1], v * axis2[2]));

	// create new box based on the smallest bounds
	if (bounds1.Volume() < bounds2.Volume()) {
		center = (bounds1[0] + bounds1[1]) * 0.5f;
		extents = bounds1[1] - center;
		center *= axis;
	}
	else {
		center = (bounds2[0] + bounds2[1]) * 0.5f;
		extents = bounds2[1] - center;
		center *= axis2;
		axis = axis2;
	}
	return true;
}

/*
============
xBox::Add
============
*/
bool xBox::Add(const xBox &a) {
	int i, besti;
	float v, bestv;
	xVec3 dir;
	xMat3 ax[4];
	xBounds bounds[4], b;

	if (a.extents[0] < 0.0f) {
		return false;
	}

	if (extents[0] < 0.0f) {
		center = a.center;
		extents = a.extents;
		axis = a.axis;
		return true;
	}

	// test axis of this box
	ax[0] = axis;
	bounds[0][0][0] = bounds[0][1][0] = center * ax[0][0];
	bounds[0][0][1] = bounds[0][1][1] = center * ax[0][1];
	bounds[0][0][2] = bounds[0][1][2] = center * ax[0][2];
	bounds[0][0] -= extents;
	bounds[0][1] += extents;
	a.AxisProjection(ax[0], b);
	if (!bounds[0].Add(b)) {
		// the other box is contained in this box
		return false;
	}

	// test axis of other box
	ax[1] = a.axis;
	bounds[1][0][0] = bounds[1][1][0] = a.center * ax[1][0];
	bounds[1][0][1] = bounds[1][1][1] = a.center * ax[1][1];
	bounds[1][0][2] = bounds[1][1][2] = a.center * ax[1][2];
	bounds[1][0] -= a.extents;
	bounds[1][1] += a.extents;
	AxisProjection(ax[1], b);
	if (!bounds[1].Add(b)) {
		// this box is contained in the other box
		center = a.center;
		extents = a.extents;
		axis = a.axis;
		return true;
	}

	// test axes aligned with the vector between the box centers and one of the box axis
	dir = a.center - center;
	dir.Normalize();
	for (i = 2; i < 4; i++) {
		ax[i][0] = dir;
		ax[i][1] = ax[i-2][ Min3Index(dir * ax[i-2][0], dir * ax[i-2][1], dir * ax[i-2][2]) ];
		ax[i][1] = ax[i][1] - (ax[i][1] * dir) * dir;
		ax[i][1].Normalize();
		ax[i][2].Cross(dir, ax[i][1]);

		AxisProjection(ax[i], bounds[i]);
		a.AxisProjection(ax[i], b);
		bounds[i].Add(b);
	}

	// get the bounds with the smallest volume
	bestv = xMath::WORLD_INFINITY;
	besti = 0;
	for (i = 0; i < 4; i++) {
		v = bounds[i].Volume();
		if (v < bestv) {
			bestv = v;
			besti = i;
		}
	}

	// create a box from the smallest bounds axis pair
	center = (bounds[besti][0] + bounds[besti][1]) * 0.5f;
	extents = bounds[besti][1] - center;
	center *= ax[besti];
	axis = ax[besti];

	return false;
}

/*
================
xBox::PlaneDistance
================
*/
float xBox::PlaneDistance(const xPlane &plane) const {
	float d1, d2;

	d1 = plane.Distance(center);
	d2 = xMath::Fabs(extents[0] * plane.Normal()[0]) +
			xMath::Fabs(extents[1] * plane.Normal()[1]) +
				xMath::Fabs(extents[2] * plane.Normal()[2]);

	if (d1 - d2 > 0.0f) {
		return d1 - d2;
	}
	if (d1 + d2 < 0.0f) {
		return d1 + d2;
	}
	return 0.0f;
}

/*
================
xBox::PlaneSide
================
*/
int xBox::PlaneSide(const xPlane &plane, const float epsilon) const {
	float d1, d2;

	d1 = plane.Distance(center);
	d2 = xMath::Fabs(extents[0] * plane.Normal()[0]) +
			xMath::Fabs(extents[1] * plane.Normal()[1]) +
				xMath::Fabs(extents[2] * plane.Normal()[2]);

	if (d1 - d2 > epsilon) {
		return SIDE_FRONT;
	}
	if (d1 + d2 < -epsilon) {
		return SIDE_BACK;
	}
	return SIDE_CROSS;
}

/*
============
xBox::IntersectsBox
============
*/
bool xBox::IntersectsBox(const xBox &a) const {
    xVec3 dir;			// vector between centers
    float c[3][3];		// matrix c = axis.Transpose() * a.axis
    float ac[3][3];		// absolute values of c
    float axisdir[3];	// axis[i] * dir
    float d, e0, e1;	// distance between centers and projected extents

	dir = a.center - center;
    
    // axis C0 + t * A0
    c[0][0] = axis[0] * a.axis[0];
    c[0][1] = axis[0] * a.axis[1];
    c[0][2] = axis[0] * a.axis[2];
    axisdir[0] = axis[0] * dir;
    ac[0][0] = xMath::Fabs(c[0][0]);
    ac[0][1] = xMath::Fabs(c[0][1]);
    ac[0][2] = xMath::Fabs(c[0][2]);

    d = xMath::Fabs(axisdir[0]);
	e0 = extents[0];
    e1 = a.extents[0] * ac[0][0] + a.extents[1] * ac[0][1] + a.extents[2] * ac[0][2];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A1
    c[1][0] = axis[1] * a.axis[0];
    c[1][1] = axis[1] * a.axis[1];
    c[1][2] = axis[1] * a.axis[2];
    axisdir[1] = axis[1] * dir;
    ac[1][0] = xMath::Fabs(c[1][0]);
    ac[1][1] = xMath::Fabs(c[1][1]);
    ac[1][2] = xMath::Fabs(c[1][2]);

    d = xMath::Fabs(axisdir[1]);
	e0 = extents[1];
    e1 = a.extents[0] * ac[1][0] + a.extents[1] * ac[1][1] + a.extents[2] * ac[1][2];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A2
    c[2][0] = axis[2] * a.axis[0];
    c[2][1] = axis[2] * a.axis[1];
    c[2][2] = axis[2] * a.axis[2];
    axisdir[2] = axis[2] * dir;
    ac[2][0] = xMath::Fabs(c[2][0]);
    ac[2][1] = xMath::Fabs(c[2][1]);
    ac[2][2] = xMath::Fabs(c[2][2]);

    d = xMath::Fabs(axisdir[2]);
	e0 = extents[2];
    e1 = a.extents[0] * ac[2][0] + a.extents[1] * ac[2][1] + a.extents[2] * ac[2][2];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * B0
    d = xMath::Fabs(a.axis[0] * dir);
    e0 = extents[0] * ac[0][0] + extents[1] * ac[1][0] + extents[2] * ac[2][0];
	e1 = a.extents[0];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * B1
    d = xMath::Fabs(a.axis[1] * dir);
    e0 = extents[0] * ac[0][1] + extents[1] * ac[1][1] + extents[2] * ac[2][1];
	e1 = a.extents[1];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * B2
    d = xMath::Fabs(a.axis[2] * dir);
    e0 = extents[0] * ac[0][2] + extents[1] * ac[1][2] + extents[2] * ac[2][2];
	e1 = a.extents[2];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A0xB0
    d = xMath::Fabs(axisdir[2] * c[1][0] - axisdir[1] * c[2][0]);
    e0 = extents[1] * ac[2][0] + extents[2] * ac[1][0];
    e1 = a.extents[1] * ac[0][2] + a.extents[2] * ac[0][1];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A0xB1
    d = xMath::Fabs(axisdir[2] * c[1][1] - axisdir[1] * c[2][1]);
    e0 = extents[1] * ac[2][1] + extents[2] * ac[1][1];
    e1 = a.extents[0] * ac[0][2] + a.extents[2] * ac[0][0];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A0xB2
    d = xMath::Fabs(axisdir[2] * c[1][2] - axisdir[1] * c[2][2]);
    e0 = extents[1] * ac[2][2] + extents[2] * ac[1][2];
    e1 = a.extents[0] * ac[0][1] + a.extents[1] * ac[0][0];
    if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A1xB0
    d = xMath::Fabs(axisdir[0] * c[2][0] - axisdir[2] * c[0][0]);
    e0 = extents[0] * ac[2][0] + extents[2] * ac[0][0];
    e1 = a.extents[1] * ac[1][2] + a.extents[2] * ac[1][1];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A1xB1
    d = xMath::Fabs(axisdir[0] * c[2][1] - axisdir[2] * c[0][1]);
    e0 = extents[0] * ac[2][1] + extents[2] * ac[0][1];
    e1 = a.extents[0] * ac[1][2] + a.extents[2] * ac[1][0];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A1xB2
    d = xMath::Fabs(axisdir[0] * c[2][2] - axisdir[2] * c[0][2]);
    e0 = extents[0] * ac[2][2] + extents[2] * ac[0][2];
    e1 = a.extents[0] * ac[1][1] + a.extents[1] * ac[1][0];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A2xB0
    d = xMath::Fabs(axisdir[1] * c[0][0] - axisdir[0] * c[1][0]);
    e0 = extents[0] * ac[1][0] + extents[1] * ac[0][0];
    e1 = a.extents[1] * ac[2][2] + a.extents[2] * ac[2][1];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A2xB1
    d = xMath::Fabs(axisdir[1] * c[0][1] - axisdir[0] * c[1][1]);
    e0 = extents[0] * ac[1][1] + extents[1] * ac[0][1];
    e1 = a.extents[0] * ac[2][2] + a.extents[2] * ac[2][0];
	if (d > e0 + e1) {
        return false;
	}

    // axis C0 + t * A2xB2
    d = xMath::Fabs(axisdir[1] * c[0][2] - axisdir[0] * c[1][2]);
    e0 = extents[0] * ac[1][2] + extents[1] * ac[0][2];
    e1 = a.extents[0] * ac[2][1] + a.extents[1] * ac[2][0];
	if (d > e0 + e1) {
        return false;
	}
    return true;
}

/*
============
xBox::LineIntersection

  Returns true if the line intersects the box between the start and end point.
============
*/
bool xBox::LineIntersection(const xVec3 &start, const xVec3 &end) const {
    float ld[3];
    xVec3 lineDir = 0.5f * (end - start);
    xVec3 lineCenter = start + lineDir;
    xVec3 dir = lineCenter - center;

    ld[0] = xMath::Fabs(lineDir * axis[0]);
	if (xMath::Fabs(dir * axis[0]) > extents[0] + ld[0]) {
        return false;
	}

    ld[1] = xMath::Fabs(lineDir * axis[1]);
	if (xMath::Fabs(dir * axis[1]) > extents[1] + ld[1]) {
        return false;
	}

    ld[2] = xMath::Fabs(lineDir * axis[2]);
	if (xMath::Fabs(dir * axis[2]) > extents[2] + ld[2]) {
        return false;
	}

    xVec3 cross = lineDir.Cross(dir);

	if (xMath::Fabs(cross * axis[0]) > extents[1] * ld[2] + extents[2] * ld[1]) {
        return false;
	}

	if (xMath::Fabs(cross * axis[1]) > extents[0] * ld[2] + extents[2] * ld[0]) {
        return false;
	}

	if (xMath::Fabs(cross * axis[2]) > extents[0] * ld[1] + extents[1] * ld[0]) {
        return false;
	}

    return true;
}

/*
============
BoxPlaneClip
============
*/
static bool BoxPlaneClip(const float denom, const float numer, float &scale0, float &scale1) {
	if (denom > 0.0f) {
		if (numer > denom * scale1) {
			return false;
		}
		if (numer > denom * scale0) {
			scale0 = numer / denom;
		}
		return true;
	}
	else if (denom < 0.0f) {
		if (numer > denom * scale0) {
			return false;
		}
		if (numer > denom * scale1) {
			scale1 = numer / denom;
		}
		return true;
	}
	else {
		return (numer <= 0.0f);
	}
}

/*
============
xBox::RayIntersection

  Returns true if the ray intersects the box.
  The ray can intersect the box in both directions from the start point.
  If start is inside the box then scale1 < 0 and scale2 > 0.
============
*/
bool xBox::RayIntersection(const xVec3 &start, const xVec3 &dir, float &scale1, float &scale2) const {
	xVec3 localStart, localDir;

	localStart = (start - center) * axis.Transpose();
	localDir = dir * axis.Transpose();

	scale1 = -xMath::WORLD_INFINITY;
	scale2 = xMath::WORLD_INFINITY;
    return	BoxPlaneClip(localDir.x, -localStart.x - extents[0], scale1, scale2) &&
			BoxPlaneClip(-localDir.x,  localStart.x - extents[0], scale1, scale2) &&
			BoxPlaneClip(localDir.y, -localStart.y - extents[1], scale1, scale2) &&
			BoxPlaneClip(-localDir.y,  localStart.y - extents[1], scale1, scale2) &&
			BoxPlaneClip(localDir.z, -localStart.z - extents[2], scale1, scale2) &&
			BoxPlaneClip(-localDir.z,  localStart.z - extents[2], scale1, scale2);
}

/*
============
xBox::FromPointTranslation

  Most tight box for the translational movement of the given point.
============
*/
void xBox::FromPointTranslation(const xVec3 &point, const xVec3 &translation) {
	// FIXME: implement
}

/*
============
xBox::FromBoxTranslation

  Most tight box for the translational movement of the given box.
============
*/
void xBox::FromBoxTranslation(const xBox &box, const xVec3 &translation) {
	// FIXME: implement
}

/*
============
xBox::FromPointRotation

  Most tight bounds for the rotational movement of the given point.
============
*/
void xBox::FromPointRotation(const xVec3 &point, const xRotation &rotation) {
	// FIXME: implement
}

/*
============
xBox::FromBoxRotation

  Most tight box for the rotational movement of the given box.
============
*/
void xBox::FromBoxRotation(const xBox &box, const xRotation &rotation) {
	// FIXME: implement
}

/*
============
xBox::ToPoints
============
*/
void xBox::ToPoints(xVec3 points[8]) const {
	xMat3 ax;
	xVec3 temp[4];

	ax[0] = extents[0] * axis[0];
	ax[1] = extents[1] * axis[1];
	ax[2] = extents[2] * axis[2];
	temp[0] = center - ax[0];
	temp[1] = center + ax[0];
	temp[2] = ax[1] - ax[2];
	temp[3] = ax[1] + ax[2];
	points[0] = temp[0] - temp[3];
	points[1] = temp[1] - temp[3];
	points[2] = temp[1] + temp[2];
	points[3] = temp[0] + temp[2];
	points[4] = temp[0] - temp[2];
	points[5] = temp[1] - temp[2];
	points[6] = temp[1] + temp[3];
	points[7] = temp[0] + temp[3];
}

/*
============
xBox::ToPlanes
============
*/
void xBox::ToPlanes(xPlane planes[6]) const
{
  xVec3 ex[3];
	ex[0] = extents[0] * axis[0];
	ex[1] = extents[1] * axis[1];
	ex[2] = extents[2] * axis[2];

  planes[0].SetNormal(axis[0]);
  planes[0].FitThroughPoint(center + ex[0]);

  planes[1].SetNormal(axis[1]);
  planes[1].FitThroughPoint(center + ex[1]);

  planes[2].SetNormal(axis[2]);
  planes[2].FitThroughPoint(center + ex[2]);

  planes[3].SetNormal(-axis[0]);
  planes[3].FitThroughPoint(center - ex[0]);

  planes[4].SetNormal(-axis[1]);
  planes[4].FitThroughPoint(center - ex[1]);

  planes[5].SetNormal(-axis[2]);
  planes[5].FitThroughPoint(center - ex[2]);
}

/*
============
xBox::ToPlanes
============
*/
void xBox::ToPlanes(xPlaneExact planes[6]) const
{
  xVec3d axis[] = {this->axis[0].ToVec3d(), this->axis[1].ToVec3d(), this->axis[2].ToVec3d()};  
  xVec3d ex[3] = {extents[0] * axis[0], extents[1] * axis[1], extents[2] * axis[2]};
  xVec3d center = this->center.ToVec3d();

  planes[0].SetNormal(axis[0]);
  planes[0].FitThroughPoint(center + ex[0]);

  planes[1].SetNormal(axis[1]);
  planes[1].FitThroughPoint(center + ex[1]);

  planes[2].SetNormal(axis[2]);
  planes[2].FitThroughPoint(center + ex[2]);

  planes[3].SetNormal(-axis[0]);
  planes[3].FitThroughPoint(center - ex[0]);

  planes[4].SetNormal(-axis[1]);
  planes[4].FitThroughPoint(center - ex[1]);

  planes[5].SetNormal(-axis[2]);
  planes[5].FitThroughPoint(center - ex[2]);
}

/*
============
xBox::GetProjectionSilhouetteVerts
============
*/
int xBox::GetProjectionSilhouetteVerts(const xVec3 &projectionOrigin, xVec3 silVerts[6]) const {
	float f;
	int i, planeBits, *index;
	xVec3 points[8], dir1, dir2;

	ToPoints(points);

	dir1 = points[0] - projectionOrigin;
	dir2 = points[6] - projectionOrigin;
	f = dir1 * axis[0];
	planeBits = FLOATSIGNBITNOTSET(f);
	f = dir2 * axis[0];
	planeBits |= FLOATSIGNBITSET(f) << 1;
	f = dir1 * axis[1];
	planeBits |= FLOATSIGNBITNOTSET(f) << 2;
	f = dir2 * axis[1];
	planeBits |= FLOATSIGNBITSET(f) << 3;
	f = dir1 * axis[2];
	planeBits |= FLOATSIGNBITNOTSET(f) << 4;
	f = dir2 * axis[2];
	planeBits |= FLOATSIGNBITSET(f) << 5;

	index = boxPlaneBitsSilVerts[planeBits];
	for (i = 0; i < index[0]; i++) {
		silVerts[i] = points[index[i+1]];
	}

	return index[0];
}

/*
============
xBox::GetParallelProjectionSilhouetteVerts
============
*/
int xBox::GetParallelProjectionSilhouetteVerts(const xVec3 &projectionDir, xVec3 silVerts[6]) const {
	float f;
	int i, planeBits, *index;
	xVec3 points[8];

	ToPoints(points);

	planeBits = 0;
	f = projectionDir * axis[0];
	if (FLOATNOTZERO(f)) {
		planeBits = 1 << FLOATSIGNBITSET(f);
	}
	f = projectionDir * axis[1];
	if (FLOATNOTZERO(f)) {
		planeBits |= 4 << FLOATSIGNBITSET(f);
	}
	f = projectionDir * axis[2];
	if (FLOATNOTZERO(f)) {
		planeBits |= 16 << FLOATSIGNBITSET(f);
	}

	index = boxPlaneBitsSilVerts[planeBits];
	for (i = 0; i < index[0]; i++) {
		silVerts[i] = points[index[i+1]];
	}

	return index[0];
}
