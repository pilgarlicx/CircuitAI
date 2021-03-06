/*
 * MetalData.h
 *
 *  Created on: Aug 11, 2014
 *      Author: rlcevg
 */

#ifndef SRC_CIRCUIT_STATIC_METALDATA_H_
#define SRC_CIRCUIT_STATIC_METALDATA_H_

#include "AIFloat3.h"

#include "kdtree/nanoflann.hpp"
#include "lemon/smart_graph.h"

#include <vector>
#include <atomic>
#include <memory>
#include <set>

namespace circuit {

class CRagMatrix;

class CMetalData {
public:
	using Graph = lemon::SmartGraph;
	using WeightMap = Graph::EdgeMap<float>;
	using CenterMap = Graph::EdgeMap<springai::AIFloat3>;

	struct SMetal {
		float income;
		springai::AIFloat3 position;
	};
	using Metals = std::vector<SMetal>;
	template <class Derived>
	struct SPointAdaptor {
		const Derived& pts;
		SPointAdaptor(const Derived& v) : pts(v) {}
		/*
		 * KDTree adapter interface
		 */
		// Must return the number of data points
		inline size_t kdtree_get_point_count() const { return pts.size(); }
		// Returns the dim'th component of the idx'th point in the class:
			// Since this is inlined and the "dim" argument is typically an immediate value, the
			//  "if/else's" are actually solved at compile time.
		inline float kdtree_get_pt(const size_t idx, const size_t dim) const {
			return (dim == 0) ? pts[idx].position.x : pts[idx].position.z;
		}
		// Optional bounding-box computation: return false to default to a standard bbox computation loop.
			//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
			//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
		template <class BBOX>
		bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
	};
	using PointPredicate = nanoflann::KNNCondResultSet<float, int>::Predicate;
	using MetalIndices = std::vector<int>;
	struct SCluster {
		MetalIndices idxSpots;
		springai::AIFloat3 position;  // geoCentr
		springai::AIFloat3 weightCentr;
		float income;
	};
	using Clusters = std::vector<SCluster>;

public:
	CMetalData();
	virtual ~CMetalData();
	void Init(const Metals& spots);

	bool IsInitialized() const { return isInitialized; }
	bool IsEmpty() const { return spots.empty(); }

	bool IsClusterizing() const { return isClusterizing.load(); }
	void SetClusterizing(bool value) { isClusterizing = value; }

	float GetMinIncome() const { return minIncome; }
	float GetAvgIncome() const { return avgIncome; }
	float GetMaxIncome() const { return maxIncome; }

	const Metals& GetSpots() const { return spots; }
	const int FindNearestSpot(const springai::AIFloat3& pos) const;
	const int FindNearestSpot(const springai::AIFloat3& pos, PointPredicate& predicate) const;

	const int FindNearestCluster(const springai::AIFloat3& pos) const;
	const int FindNearestCluster(const springai::AIFloat3& pos, PointPredicate& predicate) const;

	const Clusters& GetClusters() const { return clusters; }
	const Graph& GetGraph() const { return clusterGraph; }
	const WeightMap& GetWeights() const { return weights; }
	const CenterMap& GetCenters() const { return centers; }

	/*
	 * Hierarchical clusterization. Not reusable. Metric: complete link. Thread-unsafe
	 */
	void Clusterize(float maxDistance, std::shared_ptr<CRagMatrix> distmatrix);

	// debug, could be used for defence perimeter calculation
//	void DrawConvexHulls(springai::Drawer* drawer);
//	void DrawCentroids(springai::Drawer* drawer);
//	void ClearMetalClusters(springai::Drawer* drawer);

	const SMetal& operator[](int idx) const { return spots[idx]; }

	static void TriangulateGraph(const std::vector<double>& coords,
			std::function<float (std::size_t A, std::size_t B)> distance,
			std::function<void (std::size_t A, std::size_t B)> addEdge);
private:
	void BuildClusterGraph();

	bool isInitialized;
	Metals spots;
	SPointAdaptor<Metals> spotsAdaptor;
	using MetalTree = nanoflann::KDTreeSingleIndexAdaptor<
			nanoflann::L2_Simple_Adaptor<float, SPointAdaptor<Metals> >,
			SPointAdaptor<Metals>,
			2 /* dim */, int>;
	MetalTree metalTree;
	float minIncome;
	float avgIncome;
	float maxIncome;

	Clusters clusters;
	SPointAdaptor<Clusters> clustersAdaptor;
	using ClusterTree = nanoflann::KDTreeSingleIndexAdaptor<
			nanoflann::L2_Simple_Adaptor<float, SPointAdaptor<Clusters> >,
			SPointAdaptor<Clusters>,
			2 /* dim */, int>;
	ClusterTree clusterTree;

	Graph clusterGraph;
	WeightMap weights;
	CenterMap centers;

	std::atomic<bool> isClusterizing;
};

} // namespace circuit

#endif // SRC_CIRCUIT_STATIC_METALDATA_H_
