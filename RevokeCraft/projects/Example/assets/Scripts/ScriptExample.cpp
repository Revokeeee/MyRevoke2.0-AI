#include "ScriptExample.h"

#include "MyRevoke/Scene/Components.h"

ScriptEntity* Player()
{
	return new PlayerScript;
}

void PlayerScript::OnCreate()
{
	

}

void PlayerScript::OnDestroy()
{
}

void PlayerScript::OnUpdate(Timestep ts)
{
	if (HasComponent<TransformComponent>())
	{
		auto& comp = GetComponent<TransformComponent>();

		comp.Position.x -= 0.02f * ts;
	}
}
