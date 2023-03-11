#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <shared_mutex>

const uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));

namespace addresses {
	const uintptr_t getscheduler = base + 0x732250;
	const uintptr_t task_defer = base + 0x732250; // This could be any function that pops a function from the Luau stack and calls it. Defer is one of them. Also note that task.defer silently logs suspicious calls
	const uintptr_t luavm_load = base + 0x732250;
	const uintptr_t luavm_load_bytecode_hook = base + 0x732250;
	const uintptr_t luavm_load_hashcheck_hook = base + 0x732250;
}

namespace offsets {
	namespace scriptcontext {
		uintptr_t get_scriptstate(uintptr_t scriptcontext) {
			return scriptcontext + 0xEC + *(uintptr_t *)(scriptcontext + 0xEC); // Every encryption changes every week
		}
	}
	namespace scriptcontext {

	}
}

// Might as well paste from V2 here
namespace objects {
	typedef struct instance instance;
	typedef struct job job;
	struct property_descriptor {
		uintptr_t* vftable;
		std::string* name;
	};
	struct class_descriptor {
		uintptr_t* vftable;
		std::string* class_name;
		char padding1[0x10];
		std::vector<property_descriptor*> properties;
	};
	struct instance {
		uintptr_t* vftable;
		std::shared_ptr<instance> self;
		class_descriptor* class_descriptor;
		char padding1[0x18];
		std::string* name;
		std::vector<std::shared_ptr<instance>>* children;
		char padding2[0x4];
		instance* parent;
	};
	struct job {
		uintptr_t* vftable;
		job* self;
		char padding1[0x8];
		std::string name;
		uintptr_t datamodel_minus_4; // add 4 then cast to rbx::objects::instance* to get datamodel
	};
	constexpr int x = offsetof(instance, name);
	struct waiting_hybrid_scripts_job {
		char padding1[0x130];
		instance* script_context;
	};
	struct task_scheduler {
		char padding1[0x118];
		double fps;
		char padding2[0x14];
		std::vector<std::shared_ptr<job>> jobs;
	};
}

namespace functions {
	namespace types {
		typedef objects::task_scheduler*(__cdecl* getscheduler)();
		typedef bool(__fastcall* luavm_load)(IN uintptr_t state, IN std::string* compressed_bytecode, IN const char* chunkname, IN OPTIONAL int env);
		typedef int(__cdecl* task_defer)(IN uintptr_t state);
	}
	types::getscheduler getscheduler = reinterpret_cast<types::getscheduler>(addresses::getscheduler);
	types::luavm_load luavm_load = reinterpret_cast<types::luavm_load>(addresses::luavm_load);
	types::task_defer task_defer = reinterpret_cast<types::task_defer>(addresses::task_defer);
}