// Welcome to TaaprWare V3

//#define USE_PIPE
#define USE_CONSOLE
#define RAISE_IDENTITY 8 // Change the number or comment it out

#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include <Luau\BytecodeBuilder.h>
#include <Luau\Compiler.h>

#include "roblox.h"
#include "luavm_load_hook.h"

uintptr_t state;

void refresh_state() {
	printf("Getting lua state\n");
	printf("Getting TaskScheduler...\n");
	Sleep(100);
	objects::task_scheduler* scheduler = functions::getscheduler();
	printf("Iterating through jobs...\n");
	Sleep(100);
	objects::instance* datamodel = new objects::instance;
	// Simplicity, skids
	for (std::shared_ptr<objects::job> job : scheduler->jobs) {
		printf("Job %p: ", job.get()); // In the case that printing the job name crashes Roblox we can see which job was responsible
		printf("%s\n", job->name.c_str());
		if (job->name == "WaitingHybridScriptsJob") {
			printf("Found WaitingHybridScriptsJob\n");
			datamodel = reinterpret_cast<objects::instance*>(job->datamodel_minus_4 + 4);
		} 
	}
	printf("DataModel: %p\n", datamodel);
	printf("Getting ScriptContext...\n");
	Sleep(100);
	objects::instance* scriptcontext = new objects::instance;
	for (std::shared_ptr<objects::instance> instance : *(datamodel->children)) {
		// Always check ClassName, skids
		if (*(instance->class_descriptor->class_name) == "ScriptContext") {
			scriptcontext = instance.get();
		}
	}
	printf("ScriptContext: %p\n", scriptcontext);
	Sleep(100);
	// This is how scripts start, skids
	state = offsets::scriptcontext::get_scriptstate(reinterpret_cast<uintptr_t>(scriptcontext));
	printf("Lua state: %X\n", state);
	Sleep(100);
#ifdef RAISE_IDENTITY
	// This navigates to L->userdata aka extra space
	// Then navigates to userdata->identity
	// Userdata/extra space is every Roblox-related attribute to lua states
	*reinterpret_cast<int*>(*reinterpret_cast<uintptr_t*>(state + 0x48) + 0x1C) = RAISE_IDENTITY;
	printf("Raised identity to %d\n", RAISE_IDENTITY);
#endif
	Sleep(100);
}

class : public Luau::BytecodeEncoder {
	std::uint8_t encodeOp(const std::uint8_t opcode) {
		return opcode * 227;
	}
} encoder{};
void execute(std::string source) {
	Luau::CompileOptions options{};
	options.coverageLevel = 0;
	options.debugLevel = 1;
	options.optimizationLevel = 1;
	std::string compiled = Luau::compile("spawn(function() " + source + " end)", options, {}, &encoder);
	bool success = decompressed_luavm_load(state, compiled);
	if (success) {
		printf("Running function\n");
		// If you're a beginner, luavm_load pushes a function onto the lua state's stack
		// task.defer (or any other function like it) takes a function from the stack and runs it
		int retnum = functions::task_defer(state);
		// All C functions return the number of returns on the Luau side
		// Based on the number of returns, we clean the stack using our only Luau offset
		*reinterpret_cast<uintptr_t*>(state + offsets::state::top) -= retnum * 16; // 16 is sizeof(TValue)
		printf("Execution success\n");
	} else {
		printf("Not running function because luavm_load failed\n");
		printf("Execution failed\n");
	}
}

int main() {
	printf("TaaprWare V3 has begun!\n");
	Sleep(100);
	refresh_state();
#ifdef USE_PIPE
	char* buffer = new char[0xFFFFFF];
	std::string source = "";
	HANDLE pipe = CreateNamedPipeA(
		"\\\\.\\pipe\\Hydra",
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_WAIT,
		1,
		0xFFFFFF,
		0xFFFFFF,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL
	);
	while (pipe != INVALID_HANDLE_VALUE) {
		source = "";
		if (ConnectNamedPipe(pipe, NULL) != FALSE) {
			DWORD read_size;
			while (ReadFile(pipe, buffer, sizeof(buffer) - 1, &read_size, NULL) != FALSE) {
				buffer[read_size] = '\0';
				source += buffer;
				if (read_size < sizeof(buffer) - 1) {
					break;
				}
			}
			PurgeComm(pipe, PURGE_RXCLEAR | PURGE_TXCLEAR);
			execute(source);
		}
	}
#else
	while (true) {
		printf("Not using pipe, enter script below (no newlines):\n");
		std::string source;
		std::getline(std::cin, source);
		execute(source);
	}
#endif
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		// The classic detected console bypass that all skids use, disable it if you want
#ifdef USE_CONSOLE
		DWORD original_protection;
		VirtualProtect(&FreeConsole, sizeof(uint8_t), PAGE_EXECUTE_READWRITE, &original_protection);
		*(uint8_t*)(&FreeConsole) = 0xC3;
		VirtualProtect(&FreeConsole, sizeof(uint8_t), original_protection, NULL);
		AllocConsole();
		FILE* stream;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w", stdout);
#endif
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(main), NULL, 0, NULL);
    }
    return TRUE;
}

