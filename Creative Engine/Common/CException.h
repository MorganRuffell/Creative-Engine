#pragma once
#include "pch.h"

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

class CConcurrencyException : CException
{
	const char* What() const throw ()
	{
		return "Creative Engine Concurrency Exception!";
	}
};