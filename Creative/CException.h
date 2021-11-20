#pragma once

#include <exception>


class CException : public std::exception
{
	const char* What() const throw ()
	{
		return "Creative Engine Exception!"; 
	}

};

class CMathException : CException
{
	const char* What() const throw ()
	{
		return "Creative Engine Math Exception!";
	}

};

class CGraphicsException : CException
{
	const char* What() const throw ()
	{
		return "Creative Engine Graphics Exception!";
	}
};