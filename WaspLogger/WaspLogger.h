//**************************************************************************
//	Daniel Waseem, Copyright 2020
//
//************************************************************************
#pragma once

#include <string>

namespace WaspLogger
{
	void LogError(std::string message);
	void LogInformation(std::string message);
	void LogWarning(std::string message);
	void LogDebug(std::string message);
}
