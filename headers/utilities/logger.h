#pragma once

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string>
#include <type_traits>
#include "core/filepathHolder.h"
#include <format>

enum LogOption { consoleAndLogFile, consoleOnly, logFileOnly };

class Logger {

public:
	struct LogMsg {
		std::string message;
		std::source_location loc;

		template <typename T>
		LogMsg(const T& msg, std::source_location loc = std::source_location::current()) : loc(loc) {
			std::ostringstream ss;
			ss << msg;
			message = ss.str();
		}
	};

private:
	enum LogType { log = 0, warning = 1, error = 2 };

	inline static std::string logString = "";

	static constexpr const char* colors[3] = {"\033[0m", "\033[33m", "\033[31m"};
	static constexpr const char* logTexts[3] = {"", "[WARNING]", "[ERROR] "};

	static void PrintToFile(std::string info) {
		std::ofstream file(FilepathHolder::GetExeDirectory() / ".." / ".." / "log.txt", std::ios::app);
		file << info << "\n";
		file.close();
	}

	template <typename T, typename... Types>
	static void createLog(std::string logOutput, T info, Types... infos) {
		std::ostringstream stringToReturn;
		stringToReturn << info;
		logOutput.append(stringToReturn.str());
		createLog(logOutput, infos...);
	}

	static void createLog(std::string logOutput) {
		logString.append(logOutput + "\n");

		std::cout << logOutput << "\033[0m" << std::endl;
		PrintToFile(logOutput);
	}

public:
	template <typename... Types>
	static void Warn(LogMsg firstArg, Types... args) {
		std::string prefix = std::string(logTexts[LogType::warning]) + "[" +
							 std::filesystem::path(firstArg.loc.file_name()).filename().string() + ":" +
							 std::to_string(firstArg.loc.line()) + "] ";

		createLog(prefix, firstArg.message, args...);
	}

	template <typename T, typename... Types>
	static void Log(T info, Types... infos) {
		createLog(logTexts[LogType::log], info, infos...);
	}

	template <typename... Types>
	static void Error(LogMsg firstArg, Types... args) {
		std::string prefix = std::string(logTexts[LogType::error]) + "[" +
							 std::filesystem::path(firstArg.loc.file_name()).filename().string() + ":" +
							 std::to_string(firstArg.loc.line()) + "] ";

		createLog(prefix, firstArg.message, args...);
	}

	static std::string_view getLogStringRef() { return logString; }
};