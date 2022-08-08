#pragma once

#include <string>
#include <deque>
#include <vector>

class ExpressionAnalyzer
{
public:
	struct word
	{
		std::string Item;
		bool IsOperator;
		bool IsDecimal; // is it a direct value or a value obtained by getter
	};

	static bool IsDecimal(const std::string& operand);
	static bool IsValidInfixExpression(const std::string& expression);

	//converter: return std::string, only has one paramter with type const std::string&
	template <class _value_converter>
	static std::string InfixToPostfixExpression(const std::string& expression,_value_converter converter);

	//converter: return std::string, only has one paramter with type const std::string&
	template <class _value_converter>
	static std::deque<word> InfixToPostfixWords(const std::string& expression, _value_converter converter);

	static void FixSignOperand(std::string& expression);
	static void DeleteSpace(std::string& expression);

	//getter: return double, only has one paramter with type const std::string&
	//delete all sapce and convert +x, -x to (0+x), (0-x) 
	template <class _value_getter>
	static double CalculateInfixExpression(std::string& expression, _value_getter getter);

	//getter: return double, only has one paramter with type const std::string&
	template <class _value_getter>
	static double CalculatePostfixExpression(const std::string& expression, _value_getter getter);

	//getter: return double, only has one paramter with type const std::string&
	template <class _value_getter>
	static double CalculatePostfixExpression(const std::deque<word>& expression, _value_getter getter);

private:

	static bool IsSign(const std::string& expression, size_t idx);
	static std::deque<word> Words;
	static size_t NowIdx;
	static bool Error;
	static std::deque<word> Word_Analysis(const std::string& expression);
	static void E();
	static void T();
	static void K();
	static void F();
};
