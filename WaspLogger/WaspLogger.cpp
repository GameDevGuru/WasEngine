//**************************************************************************
//	Daniel Waseem, Copyright 2020
//
//************************************************************************
#include "WaspLogger.h"

#include <iostream>

namespace WaspLogger
{
	namespace
	{
		const std::string RESET_COLOR = "\033[0m";
	}

	void LogError(std::string message)
	{
		const std::string BOLD_RED = "\033[1;31m";
		std::cout << BOLD_RED << "[ERROR] " << message << RESET_COLOR << std::endl;
	}

	void LogInformation(std::string message)
	{
		const std::string BOLD_GREEN = "\033[1;32m";
		std::cout << BOLD_GREEN << "[INFO] " << message << RESET_COLOR << std::endl;
	}

	void LogWarning(std::string message)
	{
		const std::string YELLOW = "\033[33m";
		std::cout << YELLOW << "[WARNING] " << message << RESET_COLOR << std::endl;
	}

	void LogDebug(std::string message)
	{
		const std::string CYAN = "\033[36m";
		std::cout << CYAN << "[DEBUG] " << message << RESET_COLOR << std::endl;
	}
}
