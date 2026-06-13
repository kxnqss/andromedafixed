#include <Windows.h>
#include <TlHelp32.h>
#include <shellapi.h>
#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include <ctime>

#pragma comment(lib, "shell32.lib")

static constexpr DWORD STEAM_APPID = 1422450;
static constexpr DWORD PROCESS_STALE_SECS = 10;

static constexpr const char* TARGET_PROCESS = "deadlock.exe";
static constexpr const char* WINDOW_TITLE = "Andromeda Injector";
static constexpr const char* LOG_FILE = "andromeda.logs";

static bool g_bDebug = false;
static FILE* g_pLogFile = nullptr;

#define DEBUG_LOG(fmt, ...) \
	do { if (g_bDebug) { \
		Console::SetColor(13); \
		printf("[DEBUG] " fmt "\n", ##__VA_ARGS__); \
		if (g_pLogFile) fprintf(g_pLogFile, "[DEBUG] " fmt "\n", ##__VA_ARGS__); \
		Console::SetColor(7); \
	} } while(0)

namespace Console
{
static void SetColor(WORD wColor)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
}

static void PrintBanner()
{
	SetColor(11);
	std::cout << "  ___            _                    _\n";
	std::cout << " / _\\  _ __   __| |_ __ ___  _ __ ___   __| | __ _\n";
	std::cout << "/ /_\\/| '_ \\ / _` | '__/ _ \\| '_ ` _ \\ / _` |/ _` |\n";
	std::cout << "/ /_\\\\ | | | | (_| | | | (_) | | | | | | (_| | (_| |\n";
	std::cout << "\\____/ |_| |_|\\__,_|_|  \\___/|_| |_| |_|\\__,_|\\__,_|\n";
	SetColor(8);
	std::cout << "                        Deadlock Injector\n\n";
	SetColor(7);
}

static void WriteToFile(const char* pPrefix, const char* pMsg)
{
	if (!g_pLogFile)
		return;

	char szTime[64] = {};
	time_t now = time(nullptr);
	struct tm timeinfo;
	if (localtime_s(&timeinfo, &now) == 0)
		strftime(szTime, sizeof(szTime), "%H:%M:%S", &timeinfo);

	fprintf(g_pLogFile, "[%s] [%s] %s\n", szTime, pPrefix, pMsg);
	fflush(g_pLogFile);
}

static void Log(const char* pPrefix, WORD wColor, const char* pMsg)
{
	SetColor(8);
	std::cout << "  [ ";
	SetColor(wColor);
	std::cout << pPrefix;
	SetColor(8);
	std::cout << " ] ";
	SetColor(7);
	std::cout << pMsg << "\n";

	WriteToFile(pPrefix, pMsg);
}

static void Debug(const char* pMsg)
{
	if (g_bDebug)
		Log("#", 13, pMsg);
}

static void Info(const char* pMsg)
{
	Log("*", 11, pMsg);
}

static void OK(const char* pMsg)
{
	Log("+", 10, pMsg);
}

static void Error(const char* pMsg)
{
	Log("!", 12, pMsg);
}

static void Warn(const char* pMsg)
{
	Log("~", 14, pMsg);
}

static bool InitLog()
{
	fopen_s(&g_pLogFile, LOG_FILE, "w");
	if (!g_pLogFile)
	{
		// Can't log to file, but we can still show on console
		SetColor(12);
		std::cout << "  [!] Warning: could not open " << LOG_FILE << " for writing.\n";
		SetColor(7);
		return false;
	}

	// Write header
	char szTime[64] = {};
	time_t now = time(nullptr);
	struct tm timeinfo;
	if (localtime_s(&timeinfo, &now) == 0)
		strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

	fprintf(g_pLogFile, "=== Andromeda Injector === started %s\n", szTime);
	fprintf(g_pLogFile, "Platform: x64  LogLevel: %s\n\n", g_bDebug ? "DEBUG" : "INFO");
	fflush(g_pLogFile);

	return true;
}

static void ShutdownLog()
{
	if (g_pLogFile)
	{
		char szTime[64] = {};
		time_t now = time(nullptr);
		struct tm timeinfo;
		if (localtime_s(&timeinfo, &now) == 0)
			strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

		fprintf(g_pLogFile, "\n=== Andromeda Injector === ended %s\n", szTime);
		fclose(g_pLogFile);
		g_pLogFile = nullptr;
	}
}
} // namespace Console

// ---------------------------------------------------------------------------
// Helper: format GetLastError() as a readable string with hex code
// ---------------------------------------------------------------------------
static std::string LastErrorStr(DWORD dwErr = 0)
{
	if (dwErr == 0)
		dwErr = GetLastError();

	char szBuf[256] = {};
	FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, dwErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		szBuf, sizeof(szBuf), nullptr
	);

	// Trim trailing \r\n
	std::string sMsg = szBuf;
	while (!sMsg.empty() && (sMsg.back() == '\r' || sMsg.back() == '\n' || sMsg.back() == ' '))
		sMsg.pop_back();

	std::ostringstream oss;
	oss << "0x" << std::hex << dwErr << " (" << sMsg << ")";
	return oss.str();
}

// ---------------------------------------------------------------------------
// Admin check
// ---------------------------------------------------------------------------
static bool IsRunningAsAdmin()
{
	BOOL bAdmin = FALSE;
	PSID pAdminGroup = nullptr;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminGroup))
	{
		if (!CheckTokenMembership(nullptr, pAdminGroup, &bAdmin))
			bAdmin = FALSE;
		FreeSid(pAdminGroup);
	}

	return bAdmin != FALSE;
}

// ---------------------------------------------------------------------------
// CInjector
// ---------------------------------------------------------------------------
class CInjector
{
public:
	CInjector(const std::string& szDllPath) : m_szDllPath(szDllPath) {}

	bool Inject()
	{
		Console::Debug("--- CInjector::Inject() ---");

		// 1) Admin check
		if (!IsRunningAsAdmin())
		{
			Console::Error("Not running as Administrator! The injector requires admin rights.");
			Console::Info("Right-click Injector.exe -> 'Run as administrator'");
			return false;
		}
		Console::OK("Running as Administrator.");

		// 2) Verify DLL exists
		std::error_code ec;
		if (!std::filesystem::exists(m_szDllPath, ec))
		{
			if (ec)
				Console::Error(("filesystem error: " + std::string(ec.message())).c_str());
			Console::Error(("DLL not found: " + m_szDllPath).c_str());

			// Show where the injector exe is located
			char szExePath[MAX_PATH]{};
			GetModuleFileNameA(nullptr, szExePath, MAX_PATH);
			Console::Info(("Injector location: " + std::string(szExePath)).c_str());
			return false;
		}

		// Show DLL size
		uintmax_t dllSize = std::filesystem::file_size(m_szDllPath, ec);
		Console::Info(("DLL: " + m_szDllPath + " (" + std::to_string(dllSize / 1024) + " KB)").c_str());

		// 3) Wait for target process
		const DWORD dwPID = WaitForProcess();
		if (!dwPID)
		{
			Console::Error("Failed to find target process.");
			return false;
		}

		Console::OK(("Target PID: " + std::to_string(dwPID)).c_str());

		// Brief pause to let the process initialise fully
		Console::Warn("Waiting 2 seconds for process initialisation...");
		Sleep(2000);

		// 4) Inject
		return InjectLoadLibrary(dwPID);
	}

private:
	// -----------------------------------------------------------------------
	// Find a process by name
	// -----------------------------------------------------------------------
	DWORD GetProcessID(const char* pszProcessName) const
	{
		DEBUG_LOG("GetProcessID(\"%s\")", pszProcessName);

		DWORD dwPID = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap == INVALID_HANDLE_VALUE)
		{
			Console::Error(("CreateToolhelp32Snapshot failed: " + LastErrorStr()).c_str());
			return 0;
		}

		PROCESSENTRY32 pe32{};
		pe32.dwSize = sizeof(PROCESSENTRY32);

		DWORD dwCount = 0;
		if (Process32First(hSnap, &pe32))
		{
			do
			{
				++dwCount;
				if (_stricmp(pe32.szExeFile, pszProcessName) == 0)
				{
					dwPID = pe32.th32ProcessID;
					DEBUG_LOG("  Found PID %lu (threads=%u, parent=%u)",
						dwPID, pe32.cntThreads, pe32.th32ParentProcessID);
					break;
				}
			} while (Process32Next(hSnap, &pe32));
		}
		else
		{
			Console::Error(("Process32First failed: " + LastErrorStr()).c_str());
		}

		CloseHandle(hSnap);
		DEBUG_LOG("  Scanned %lu processes, result PID=%lu", dwCount, dwPID);
		return dwPID;
	}

	// -----------------------------------------------------------------------
	// Returns process uptime in seconds. 0 on failure.
	// -----------------------------------------------------------------------
	static DWORD GetProcessUptimeSecs(DWORD dwPID)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPID);
		if (!hProcess)
		{
			DEBUG_LOG("GetProcessUptimeSecs: OpenProcess(%lu) failed: %s", dwPID, LastErrorStr().c_str());
			return 0;
		}

		FILETIME ftCreation{}, ftExit{}, ftKernel{}, ftUser{};
		DWORD dwUptime = 0;

		if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser))
		{
			FILETIME ftNow{};
			GetSystemTimeAsFileTime(&ftNow);

			ULARGE_INTEGER uliCreate, uliNow;
			uliCreate.LowPart = ftCreation.dwLowDateTime;
			uliCreate.HighPart = ftCreation.dwHighDateTime;
			uliNow.LowPart = ftNow.dwLowDateTime;
			uliNow.HighPart = ftNow.dwHighDateTime;

			const ULONGLONG ullElapsed = uliNow.QuadPart - uliCreate.QuadPart;
			dwUptime = static_cast<DWORD>(ullElapsed / 10'000'000ULL);
			DEBUG_LOG("GetProcessUptimeSecs(%lu): creation=%llu now=%llu elapsed=%llu -> %lu s",
				dwPID, uliCreate.QuadPart, uliNow.QuadPart, ullElapsed, dwUptime);
		}
		else
		{
			DEBUG_LOG("GetProcessUptimeSecs: GetProcessTimes failed: %s", LastErrorStr().c_str());
		}

		CloseHandle(hProcess);
		return dwUptime;
	}

	// -----------------------------------------------------------------------
	static bool KillProcess(DWORD dwPID)
	{
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwPID);
		if (!hProcess)
		{
			Console::Error(("KillProcess: OpenProcess failed: " + LastErrorStr()).c_str());
			return false;
		}

		const bool bOK = TerminateProcess(hProcess, 0) != FALSE;
		if (!bOK)
			Console::Error(("TerminateProcess failed: " + LastErrorStr()).c_str());

		CloseHandle(hProcess);
		return bOK;
	}

	// -----------------------------------------------------------------------
	static void LaunchViaSteam()
	{
		Console::Info("Launching Deadlock via Steam URL...");

		char szURL[64];
		wsprintfA(szURL, "steam://rungameid/%lu", STEAM_APPID);

		Console::Debug(("  URL: " + std::string(szURL)).c_str());

		HINSTANCE hResult = ShellExecuteA(nullptr, "open", szURL, nullptr, nullptr, SW_SHOWNORMAL);
		// ShellExecute returns a value > 32 on success
		if ((INT_PTR)hResult <= 32)
		{
			Console::Warn(("ShellExecute returned " + std::to_string((INT_PTR)hResult) + " — Steam may not be installed or the URL handler failed.").c_str());
		}
	}

	// -----------------------------------------------------------------------
	// Wait for a fresh target process (or launch it)
	// -----------------------------------------------------------------------
	DWORD WaitForProcess() const
	{
		Console::Warn(("Waiting for " + std::string(TARGET_PROCESS) + "...").c_str());

		DWORD dwNotFoundTime = 0;
		bool bLaunchAttempted = false;

		while (true)
		{
			const DWORD dwPID = GetProcessID(TARGET_PROCESS);

			if (!dwPID)
			{
				if (!bLaunchAttempted)
				{
					dwNotFoundTime += 1000;
					if (dwNotFoundTime >= 2000)
					{
						bLaunchAttempted = true;
						LaunchViaSteam();
					}
				}

				Sleep(1000);
				continue;
			}

			const DWORD dwUptime = GetProcessUptimeSecs(dwPID);

			// If uptime is 0 and the process exists, it likely means we couldn't
			// query times (access denied, OS race, etc.). Treat it as fresh.
			if (dwUptime == 0)
			{
				Console::Warn(("Found " + std::string(TARGET_PROCESS) + " PID " + std::to_string(dwPID) + " (uptime unknown, treating as fresh)").c_str());
				return dwPID;
			}

			if (dwUptime > PROCESS_STALE_SECS)
			{
				char szMsg[128];
				wsprintfA(szMsg, "Process PID %lu is stale (%lu s). Killing and relaunching...", dwPID, dwUptime);
				Console::Warn(szMsg);

				if (!KillProcess(dwPID))
					Console::Warn("  TerminateProcess failed — trying to continue anyway.");

				Sleep(1500);
				LaunchViaSteam();

				Console::Warn(("Waiting for fresh " + std::string(TARGET_PROCESS) + "...").c_str());

				while (true)
				{
					const DWORD dwNewPID = GetProcessID(TARGET_PROCESS);
					const DWORD dwNewUptime = GetProcessUptimeSecs(dwNewPID);

					// Accept if fresh or if uptime is unknown (0)
					if (dwNewPID && (dwNewUptime <= PROCESS_STALE_SECS || dwNewUptime == 0))
					{
						Console::OK(("Fresh " + std::string(TARGET_PROCESS) + " found (PID: " + std::to_string(dwNewPID) + ")").c_str());
						return dwNewPID;
					}
					Sleep(1000);
				}
			}

			Console::OK(("Found " + std::string(TARGET_PROCESS) + " (PID: " + std::to_string(dwPID) + ", uptime: " + std::to_string(dwUptime) + "s)").c_str());
			return dwPID;
		}
	}

	// -----------------------------------------------------------------------
	// Inject DLL via CreateRemoteThread + LoadLibraryA
	// -----------------------------------------------------------------------
	bool InjectLoadLibrary(DWORD dwPID) const
	{
		Console::Info("--- InjectLoadLibrary ---");

		// 1) OpenProcess — use minimal rights
		const DWORD dwDesiredAccess =
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION |
			PROCESS_VM_WRITE |
			PROCESS_VM_READ |
			PROCESS_QUERY_INFORMATION;

		Console::Debug(("  Opening PID " + std::to_string(dwPID) + " with access " + std::to_string(dwDesiredAccess)).c_str());

		HANDLE hProcess = OpenProcess(dwDesiredAccess, FALSE, dwPID);
		if (!hProcess)
		{
			Console::Error(("OpenProcess failed: " + LastErrorStr()).c_str());

			// Diagnostic: try ALL_ACCESS to see if that's the issue
			Console::Debug("  Retrying with PROCESS_ALL_ACCESS for diagnostics...");
			if (OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID))
			{
				Console::Debug("  PROCESS_ALL_ACCESS succeeded — the restricted flag set was too restrictive.");
			}
			else
			{
				Console::Debug(("  PROCESS_ALL_ACCESS also failed: " + LastErrorStr()).c_str());
			}
			return false;
		}

		Console::OK("  OpenProcess succeeded.");

		// 2) VirtualAllocEx
		const std::size_t nPathLen = m_szDllPath.length() + 1;
		Console::Debug(("  Allocating " + std::to_string(nPathLen) + " bytes in target...").c_str());

		LPVOID pRemoteMem = VirtualAllocEx(hProcess, nullptr, nPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!pRemoteMem)
		{
			Console::Error(("VirtualAllocEx failed: " + LastErrorStr()).c_str());
			CloseHandle(hProcess);
			return false;
		}

		Console::OK(("  VirtualAllocEx -> 0x" + std::to_string((uintptr_t)pRemoteMem)).c_str());

		// 3) WriteProcessMemory
		if (!WriteProcessMemory(hProcess, pRemoteMem, m_szDllPath.c_str(), nPathLen, nullptr))
		{
			Console::Error(("WriteProcessMemory failed: " + LastErrorStr()).c_str());
			VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}
		Console::OK("  WriteProcessMemory succeeded.");

		// 4) Get LoadLibraryA address
		HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
		if (!hKernel32)
		{
			Console::Error(("GetModuleHandleA(kernel32) failed: " + LastErrorStr()).c_str());
			VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}

		LPTHREAD_START_ROUTINE pLoadLibrary =
			reinterpret_cast<LPTHREAD_START_ROUTINE>(
				GetProcAddress(hKernel32, "LoadLibraryA"));

		if (!pLoadLibrary)
		{
			Console::Error(("GetProcAddress(LoadLibraryA) failed: " + LastErrorStr()).c_str());
			VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}

		Console::OK(("  LoadLibraryA at 0x" + std::to_string((uintptr_t)pLoadLibrary)).c_str());

		// 5) CreateRemoteThread
		Console::Debug("  Creating remote thread...");
		HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, pLoadLibrary, pRemoteMem, 0, nullptr);
		if (!hThread)
		{
			Console::Error(("CreateRemoteThread failed: " + LastErrorStr()).c_str());

			// Extra diagnostics
			DWORD dwLastErr = GetLastError();
			if (dwLastErr == ERROR_NOT_SUPPORTED)
				Console::Error("  ERROR_NOT_SUPPORTED — possible anti-cheat (e.g. AC disable CreateRemoteThread).");
			else if (dwLastErr == ERROR_ACCESS_DENIED)
				Console::Error("  ERROR_ACCESS_DENIED — try running as admin or disabling anti-cheat.");

			VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}

		Console::OK("  CreateRemoteThread succeeded.");

		// 6) Wait for thread completion
		Console::Debug("  Waiting for remote thread to finish...");
		DWORD dwWaitResult = WaitForSingleObject(hThread, 30000); // 30s timeout
		if (dwWaitResult == WAIT_TIMEOUT)
		{
			Console::Error("  Remote thread timed out after 30 s.");
			TerminateThread(hThread, 0);
		}
		else if (dwWaitResult == WAIT_FAILED)
		{
			Console::Error(("  WaitForSingleObject failed: " + LastErrorStr()).c_str());
		}
		else
		{
			Console::OK("  Remote thread finished.");
		}

		// 7) Check exit code
		DWORD dwExitCode = 0;
		GetExitCodeThread(hThread, &dwExitCode);

		Console::Debug(("  Thread exit code: 0x" + std::to_string(dwExitCode) + " (" + std::to_string(dwExitCode) + ")").c_str());

		CloseHandle(hThread);
		VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
		CloseHandle(hProcess);

		if (!dwExitCode)
		{
			// LoadLibrary returned NULL
			DWORD dwLastError = GetLastError();
			Console::Error("LoadLibrary returned NULL in target process.");
			Console::Error(("  LastError (captured before clean-up): " + LastErrorStr(dwLastError)).c_str());
			Console::Warn("  Possible causes:");
			Console::Warn("    1. The DLL has unresolved dependencies (missing protobuf, etc.)");
			Console::Warn("    2. The DLL was compiled for a different architecture (x64 vs x86)");
			Console::Warn("    3. Anti-cheat / antivirus blocked the DLL load");
			Console::Warn("    4. The DLL path in the remote process is invalid");
			return false;
		}

		Console::OK("Injection successful!");
		return true;
	}

private:
	std::string m_szDllPath;
};

// ---------------------------------------------------------------------------
// Default DLL path: same directory as the injector .exe
// ---------------------------------------------------------------------------
static std::string GetDefaultDllPath()
{
	char szExePath[MAX_PATH]{};
	GetModuleFileNameA(nullptr, szExePath, MAX_PATH);

	std::string sDir = szExePath;
	auto pos = sDir.find_last_of('\\');
	if (pos != std::string::npos)
		sDir.resize(pos + 1);

	return sDir + "Andromeda-DeadLock-Base.dll";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	SetConsoleTitleA(WINDOW_TITLE);

	// Parse args
	std::string szDllPath;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--debug" || arg == "-d")
			g_bDebug = true;
		else if (szDllPath.empty())
			szDllPath = arg;
	}

	if (szDllPath.empty())
		szDllPath = GetDefaultDllPath();

	// Initialise log file (always — in injector directory)
	Console::InitLog();

	Console::PrintBanner();

	if (g_bDebug)
	{
		Console::SetColor(13);
		std::cout << "  [DEBUG] mode enabled — use --debug / -d\n";
		Console::SetColor(7);

		char szExePath[MAX_PATH]{};
		GetModuleFileNameA(nullptr, szExePath, MAX_PATH);
		std::cout << "  [DEBUG] Exe path: " << szExePath << "\n";
		std::cout << "  [DEBUG] DLL path: " << szDllPath << "\n";
	}

	CInjector injector(szDllPath);

	if (!injector.Inject())
	{
		Console::Error("Injection failed.");
		Console::SetColor(7);
		std::cout << "\n  Press any key to exit...\n";
		std::cin.get();
		Console::ShutdownLog();
		return 1;
	}

	Console::OK("Injection completed successfully.\n");
	Console::ShutdownLog();
	return 0;
}
