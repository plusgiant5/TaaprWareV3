// Welcome to TaaprWare V3

//#define USE_PIPE
#define USE_CONSOLE
#define RAISE_IDENTITY 8 // Change the number or comment it out

#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include "roblox.h"

uintptr_t state;

void refresh_state() {
	printf("Getting lua state\n");
	printf("Getting TaskScheduler...\n");
	objects::task_scheduler* scheduler = functions::getscheduler();
	printf("Iterating through jobs...\n");
	objects::instance* datamodel = NULL;
	for (std::shared_ptr<objects::job> job : scheduler->jobs) {
		printf("Job %p: ", job.get());
		printf("%s\n", job->name.c_str());
		if (job->name == "WaitingHybridScriptsJob") {
			printf("Found WaitingHybridScriptsJob\n");
			datamodel = reinterpret_cast<objects::instance*>(job->datamodel_minus_4 + 4);
			break;
		}
	}
	printf("DataModel: %p\n", datamodel);
	printf("Getting ScriptContext...\n");
	objects::instance* scriptcontext = NULL;
	for (std::shared_ptr<objects::instance> instance : *(datamodel->children)) {
		if (*(instance->class_descriptor->class_name) == "ScriptContext") {
			scriptcontext = instance.get();
		}
	}
	printf("ScriptContext: %p\n", scriptcontext);
	state = offsets::scriptcontext::get_scriptstate(reinterpret_cast<uintptr_t>(scriptcontext));
	printf("Lua state: %X\n", state);
}

void execute(std::string bytecode) {
	
}

int main() {
	printf("TaaprWare V3 has begun!\n");
	refresh_state();
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

		}
	}
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

