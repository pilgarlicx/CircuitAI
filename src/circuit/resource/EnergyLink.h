/*
 * EnergyLink.h
 *
 *  Created on: Apr 19, 2015
 *      Author: rlcevg
 */

#ifndef SRC_CIRCUIT_RESOURCE_ENERGYLINK_H_
#define SRC_CIRCUIT_RESOURCE_ENERGYLINK_H_

#include "static/MetalData.h"
#include "unit/CircuitUnit.h"
#include "unit/CircuitDef.h"

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>

namespace circuit {

class CCircuitAI;

class CEnergyLink {
public:
	CEnergyLink(CCircuitAI* circuit);
	virtual ~CEnergyLink();

	void AddMex(const springai::AIFloat3& pos);
	void AddMex(const springai::AIFloat3& pos, int index);
	void RemoveMex(const springai::AIFloat3& pos);
	void RemoveMex(const springai::AIFloat3& pos, int index);
	void RebuildTree();
	void MarkAllyPylons();

private:
	void Init();
	CCircuitAI* circuit;

	int markFrame;
	struct Structure {
		CCircuitDef::Id cdefId;
		springai::AIFloat3 pos;
	};
	using Structures = std::map<CCircuitUnit::Id, Structure>;
	Structures markedAllies;
	std::unordered_map<CCircuitDef::Id, float> pylonRanges;

	std::unordered_set<int> linkedMexes;
	struct LinkVertex {
		int mexCount;
		bool isConnected;
	};
	std::unordered_map<int, LinkVertex> linkedClusters;
	struct Link {
		Link() : isBeingBuilt(false) {}
		std::set<CCircuitUnit::Id> pylons;
		bool isBeingBuilt;
	};
	std::vector<Link> links;  // Graph's exterior property
	typedef boost::property_map<CMetalData::Graph, int CMetalData::Edge::*>::const_type EdgeIndexMap;
	boost::iterator_property_map<Link*, EdgeIndexMap, Link, Link&> linkIt;  // Alternative: links[clusterGraph[*linkEdgeIt].index]

	void MarkPylon(CCircuitUnit::Id unitId, const Structure& building, bool alive);

	std::list<int> linkClusters, unlinkClusters;  // - must not contain same clusters
	std::vector<CMetalData::EdgeDesc> spanningTree;
	CMetalData::Graph spanningGraph;
};

} // namespace circuit

#endif // SRC_CIRCUIT_RESOURCE_ENERGYLINK_H_
