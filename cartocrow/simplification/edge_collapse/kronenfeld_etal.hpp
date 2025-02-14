namespace cartocrow::simplification {

template <class FaceData>
void KSBBTraits<FaceData>::ecSetEdgeMark(Map::Halfedge_handle e, ECEdgeMark m) {
	e->data().mark = m;
}
template <class FaceData>
ECEdgeMark KSBBTraits<FaceData>::ecGetEdgeMark(Map::Halfedge_handle e) {
	return e->data().mark;
}

template <class FaceData>
Collapse KSBBTraits<FaceData>::ecComputeCollapse(Map::Halfedge_handle e) {
	Collapse col;

	Point<Exact> a = e->prev()->source()->point();
	Point<Exact> b = e->source()->point();
	Point<Exact> c = e->target()->point();
	Point<Exact> d = e->next()->target()->point();

	bool abc = CGAL::collinear(a, b, c);
	bool bcd = CGAL::collinear(b, c, d);
	if (abc && bcd) {
		col.erase_both = true;
		return col;
	} else if (abc) {
		col.erase_both = false;
		// TODO: optimize, to not even shift?
		col.point = c;
		return col;
	} else if (bcd) {
		col.erase_both = false;
		// TODO: optimize, to not even shift?
		col.point = b;
		return col;
	}

	// else, no consecutive collinear edges

	Polygon<Exact> P;
	P.push_back(a);
	P.push_back(b);
	P.push_back(c);
	P.push_back(d);

	Line<Exact> ad(a, d);
	Line<Exact> ab(a, b);
	Line<Exact> bc(b, c);
	Line<Exact> cd(c, d);

	// area = base * height / 2
	// height = 2*area / base
	// so, we're going to rotate the vector d-a, such that we get a normal of length |d-a| = base.
	// To get a vector of length height, we then multiply this vector with height_times_base / base^2.
	// This normalized the vector and makes it length height! (without squareroots...)
	Number<Exact> height_times_base = 2 * P.area();

	Vector<Exact> perpv = (d - a).perpendicular(CGAL::CLOCKWISE);

	Exact::Aff_transformation_2 s(CGAL::SCALING, height_times_base / perpv.squared_length());
	perpv = perpv.transform(s);

	Exact::Aff_transformation_2 t(CGAL::TRANSLATION, perpv);
	Line<Exact> arealine = ad.transform(t);

	if (ad.has_on_boundary(arealine.point())) {

		// these should be caught already by the collinearity checks earlier
		assert(!ad.has_on_boundary(b));
		assert(!ad.has_on_boundary(c));
		
			col.erase_both = true;

			// implies that neither b nor c is on ad
			auto intersection = CGAL::intersection(bc, ad);
			col.point = boost::get<Point<Exact>>(intersection.get());

			Polygon<Exact> first_triangle;
			first_triangle.push_back(a);
			first_triangle.push_back(b);
			first_triangle.push_back(col.point);

			Polygon<Exact> second_triangle;
			second_triangle.push_back(c);
			second_triangle.push_back(d);
			second_triangle.push_back(col.point);

			if (CGAL::left_turn(a, b, c)) {
				col.this_face_polygons.push_back(first_triangle);
				col.twin_face_polygons.push_back(second_triangle);
			} else {
				col.this_face_polygons.push_back(second_triangle);
				col.twin_face_polygons.push_back(first_triangle);
			}
	} else {
		col.erase_both = false;

		bool ab_determines_shape;
		// determine type
		if (ad.has_on_positive_side(b) == ad.has_on_positive_side(c)) {
			// same side of ab, so further point determines
			ab_determines_shape = CGAL::squared_distance(b, ad) > CGAL::squared_distance(c, ad);
		} else {
			// opposite sides of ad, so the one that is on same side as area line
			ab_determines_shape =
			    ad.has_on_positive_side(b) == ad.has_on_positive_side(arealine.point());
		}

		// configure type
		if (ab_determines_shape) {

			auto intersection = CGAL::intersection(arealine, ab);
			col.point = boost::get<Point<Exact>>(intersection.get());

			Segment<Exact> ns = Segment<Exact>(col.point, d);
			auto intersection2 = CGAL::intersection(bc, ns);
			Point<Exact> is = boost::get<Point<Exact>>(intersection2.get());

			Polygon<Exact> first_triangle;
			first_triangle.push_back(b);
			first_triangle.push_back(is);
			first_triangle.push_back(col.point);

			Polygon<Exact> second_triangle;
			second_triangle.push_back(c);
			second_triangle.push_back(d);
			second_triangle.push_back(is);

			if (first_triangle.is_clockwise_oriented()) {
				col.this_face_polygons.push_back(second_triangle);
				col.twin_face_polygons.push_back(first_triangle);
			} else {
				col.this_face_polygons.push_back(first_triangle);
				col.twin_face_polygons.push_back(second_triangle);
			}

		} else {
			auto intersection = CGAL::intersection(arealine, cd);
			col.point = boost::get<Point<Exact>>(intersection.get());

			Segment<Exact> ns = Segment<Exact>(col.point, a);
			auto intersection2 = CGAL::intersection(bc, ns);
			Point<Exact> is = boost::get<Point<Exact>>(intersection2.get());

			Polygon<Exact> first_triangle;
			first_triangle.push_back(a);
			first_triangle.push_back(b);
			first_triangle.push_back(is);

			Polygon<Exact> second_triangle;
			second_triangle.push_back(c);
			second_triangle.push_back(is);
			second_triangle.push_back(col.point);

			if (first_triangle.is_clockwise_oriented()) {
				col.this_face_polygons.push_back(second_triangle);
				col.twin_face_polygons.push_back(first_triangle);
			} else {
				col.this_face_polygons.push_back(first_triangle);
				col.twin_face_polygons.push_back(second_triangle);
			}
		}
	}

	return col;
}

template <class FaceData>
void KSBBTraits<FaceData>::ecSetCollapse(Map::Halfedge_handle e, Collapse collapse) {
	e->data().collapse = collapse;
}

template <class FaceData>
Collapse KSBBTraits<FaceData>::ecGetCollapse(Map::Halfedge_handle e) {
	return e->data().collapse;
}

template <class FaceData>
void KSBBTraits<FaceData>::ecSetCost(Map::Halfedge_handle e) {

	const Collapse col = e->data().collapse;
	Number<Exact> cost = 0;
	for (Polygon<Exact> p : col.this_face_polygons) {
		cost += CGAL::abs(p.area());
	}
	// since it is an area preserving method, the other face adds up to the same area...
	e->data().cost = 2*cost;
}
template <class FaceData>
Number<Exact> KSBBTraits<FaceData>::ecGetCost(Map::Halfedge_handle e) {
	return e->data().cost;
}

template <class FaceData>
void KSBBTraits<FaceData>::ecSetBlockingNumber(Map::Halfedge_handle e, int b) {
	e->data().block = b;
}
template <class FaceData>
int KSBBTraits<FaceData>::ecGetBlockingNumber(Map::Halfedge_handle e) {
	return e->data().block;
}

template <class FaceData>
void KSBBTraits<FaceData>::histSetData(Map::Halfedge_handle e, HalfedgeOperation<KSBBTraits<FaceData>>* data) {
	e->data().hist = data;
}
template <class FaceData>
HalfedgeOperation<KSBBTraits<FaceData>>* KSBBTraits<FaceData>::histGetData(Map::Halfedge_handle e) {
	return e->data().hist;
}

} // namespace cartocrow::simplification