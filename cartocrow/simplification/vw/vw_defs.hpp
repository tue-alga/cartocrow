namespace cartocrow::simplification::vw {

/* A vertex class with an additional handle */
template <class Gt, class Vb = CGAL::Triangulation_vertex_base_2<Gt>> class VWVertex : public Vb {
	typedef Vb Base;

  public:
	typedef typename Vb::Vertex_handle Vertex_handle;
	typedef typename Vb::Face_handle Face_handle;
	typedef typename Vb::Point Point;
	template <typename TDS2> struct Rebind_TDS {
		typedef typename Vb::template Rebind_TDS<TDS2>::Other Vb2;
		typedef VWVertex<Gt, Vb2> Other;
	};

	bool removable;
	Vertex_handle prev, next; // set only if removable
	Number<Exact> cost; // set only if removable
	bool blocked;
	Vertex_handle blocked_by; // set only if blocked

	VWVertex() : Base() {}
	VWVertex(const Point& p) : Base(p) {}
	VWVertex(const Point& p, Face_handle f) : Base(f, p) {}
	VWVertex(Face_handle f) : Base(f) {}
};

typedef VWVertex<Exact> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::No_constraint_intersection_tag ITag;

typedef CGAL::Constrained_Delaunay_triangulation_2<Exact, ITag, Tds> CDT;
typedef CDT::Vertex_handle Vertex_handle;

using Queue = std::priority_queue<Operation>;

} // namespace cartocrow::simplification::vw