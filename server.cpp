#include "httplib.h"

#include <Windows.h>

#include "json.hpp"

#include <vector>
#include <string>
#include <sstream>

using namespace httplib;

using json = nlohmann::json;

inline std::vector<uint8_t> bytesFromString(std::string const& s)
{
	static std::string const DICT("0123456789ABCDEF");

	std::vector<uint8_t> bytes;
	std::stringstream ss(s);
	std::string buffer;

	while (std::getline(ss, buffer, ' '))
	{
		uint8_t b;

		b = DICT.find(buffer[0]) << 4;
		b |= DICT.find(buffer[1]);

		bytes.push_back(b);
	}

	return bytes;
}

inline uintptr_t getBase(std::string const& mod)
{
	return reinterpret_cast<uintptr_t>(
		GetModuleHandleA(mod.c_str()));
}

inline bool writeMemory(
	uintptr_t const address,
	std::vector<uint8_t> const& bytes)
{
	return WriteProcessMemory(
		GetCurrentProcess(),
		reinterpret_cast<LPVOID>(address),
		bytes.data(),
		bytes.size(),
		NULL);
}

void writeCallback(
	Request const& req,
	Response& res)
{
	if (req.has_header("Content-Type") &&
		req.get_header_value("Content-Type") == "application/json")
	{
		auto json = json::parse(req.body);

		auto base = getBase(json["module"]);
		auto address = base + json["address"];
		auto bytes = bytesFromString(json["bytes"]);

		writeMemory(address, bytes);

		res.set_header("Access-Control-Allow-Origin", "*"); //CORS

		res.status = 204;
	}
	else res.status = 400;
}

DWORD WINAPI MainThread(LPVOID)
{
	Server server;

	server.Options("/write", [](Request const&, Response& res) //CORS
		{
			res.set_header("Connection", "keep-alive");
			res.set_header("Access-Control-Allow-Origin", "*");
			res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
			res.set_header("Access-Control-Allow-Headers", "Content-Type");

			res.status = 204;
		});

	server.Post("/write", writeCallback);

	server.listen("localhost", 1337);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, &MainThread, 0, 0, 0);

	return TRUE;
}