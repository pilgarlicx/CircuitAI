/*
 * FactoryTask.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: rlcevg
 */

#include "FactoryTask.h"
#include "CircuitUnit.h"
#include "utils.h"

#include "Unit.h"

namespace circuit {

using namespace springai;

CFactoryTask::CFactoryTask(Priority priority,
		AIFloat3& position, std::list<IConstructTask*>& owner,
		TaskType type, int quantity, float radius) :
				IConstructTask(priority, position, owner, ConstructType::FACTORY),
				type(type),
				quantity(quantity),
				sqradius(radius * radius)
{
}

CFactoryTask::~CFactoryTask()
{
	PRINT_DEBUG("Execute: %s\n", __PRETTY_FUNCTION__);
}

bool CFactoryTask::CanAssignTo(CCircuitUnit* unit)
{
	Unit* u = unit->GetUnit();
	AIFloat3 pos = u->GetPos();
	return IsDistanceOk(pos);
}

CFactoryTask::TaskType CFactoryTask::GetType()
{
	return type;
}

void CFactoryTask::Progress()
{
	quantity--;
}

void CFactoryTask::Regress()
{
	quantity++;
}

bool CFactoryTask::IsDone()
{
	return quantity <= 0;
}

bool CFactoryTask::IsDistanceOk(AIFloat3& pos)
{
	float dx = pos.x - position.x;
	float dz = pos.z - position.z;
	float sqdistance = dx * dx + dz * dz;
	return sqdistance <= sqradius;
}

} // namespace circuit
