#pragma once

#include "MyRevoke/Scripting/ScriptEntity.h"
#include "MyRevoke/Core/Time.h"


using namespace Revoke;

class PlayerScript : public ScriptEntity
{
public:
	PlayerScript() = default;

	void OnCreate();
	
	void OnDestroy();

	void OnUpdate(Timestep ts);

private:
	
}; 


extern "C" __declspec(dllexport) ScriptEntity * Player();

