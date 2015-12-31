/*
 * RecruitTask.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: rlcevg
 */

#include "task/static/RecruitTask.h"
#include "task/TaskManager.h"
#include "unit/CircuitUnit.h"
#include "unit/CircuitDef.h"
#include "util/utils.h"
#include "CircuitAI.h"

#include "Command.h"
#include "AISCommands.h"
#include "Sim/Units/CommandAI/Command.h"

namespace circuit {

using namespace springai;

CRecruitTask::CRecruitTask(ITaskManager* mgr, Priority priority,
		CCircuitDef* buildDef, const AIFloat3& position,
		RecruitType type, float radius)
				: IBuilderTask(mgr, priority, buildDef, position, .0f, -1)
				, recruitType(type)
				, sqradius(radius * radius)
{
}

CRecruitTask::~CRecruitTask()
{
	PRINT_DEBUG("Execute: %s\n", __PRETTY_FUNCTION__);
}

bool CRecruitTask::CanAssignTo(CCircuitUnit* unit)
{
	return (target == nullptr) && unit->GetCircuitDef()->CanBuild(buildDef) &&
		   (position.SqDistance2D(unit->GetPos(manager->GetCircuit()->GetLastFrame())) <= sqradius);
}

void CRecruitTask::Execute(CCircuitUnit* unit)
{
	const AIFloat3& buildPos = unit->GetPos(manager->GetCircuit()->GetLastFrame());
	unit->GetUnit()->Build(buildDef->GetUnitDef(), buildPos, UNIT_COMMAND_BUILD_NO_FACING);
}

void CRecruitTask::Update()
{
	// TODO: Analyze nearby situation, enemies
}

void CRecruitTask::Finish()
{
	Cancel();
}

void CRecruitTask::Cancel()
{
	for (CCircuitUnit* unit : units) {
		// Clear build-queue
		auto commands = std::move(unit->GetUnit()->GetCurrentCommands());
		std::vector<float> params;
		params.reserve(commands.size());
		for (springai::Command* cmd : commands) {
			int cmdId = cmd->GetId();
			if (cmdId < 0) {
				params.push_back(cmdId);
			}
			delete cmd;
		}
		int frame = manager->GetCircuit()->GetLastFrame() + FRAMES_PER_SEC * 60;
		unit->GetUnit()->ExecuteCustomCommand(CMD_REMOVE, params, UNIT_COMMAND_OPTION_ALT_KEY|UNIT_COMMAND_OPTION_CONTROL_KEY, frame);
	}
}

void CRecruitTask::OnUnitIdle(CCircuitUnit* unit)
{
	Execute(unit);
}

void CRecruitTask::OnUnitDamaged(CCircuitUnit* unit, CEnemyUnit* attacker)
{
	// TODO: React: analyze, abort, create appropriate task
}

void CRecruitTask::OnUnitDestroyed(CCircuitUnit* unit, CEnemyUnit* attacker)
{
	RemoveAssignee(unit);
}

} // namespace circuit