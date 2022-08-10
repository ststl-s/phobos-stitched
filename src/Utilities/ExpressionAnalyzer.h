#pragma once

#include <string>
#include <vector>
#include <stack>

#include <Utilities/GeneralUtils.h>

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

	static void FixSignOperand(std::string& expression);
	static void DeleteSpace(std::string& expression);

private:

	static bool IsSign(const std::string& expression, size_t idx);
	static std::vector<word> Words;
	static size_t NowIdx;
	static bool Error;
	static std::vector<word> Word_Analysis(const std::string& expression);
	static void E();
	static void T();
	static void K();
	static void F();

public:

	//converter: return std::string, only has one paramter with type const std::string&
	template <class _value_converter>
	static std::string InfixToPostfixExpression(const std::string& expression, _value_converter converter)
	{
		std::string expressionFix(expression);
		std::stack<char> stackOperator;
		std::string postfixExpr;

		ExpressionAnalyzer::DeleteSpace(expressionFix);
		ExpressionAnalyzer::FixSignOperand(expressionFix);

		if (!ExpressionAnalyzer::IsValidInfixExpression(expressionFix))
			return "";

		for (size_t i = 0; i < expressionFix.length(); i++)
		{
			std::string operand;

			for (; i < expressionFix.length() && !GeneralUtils::IsOperator(expressionFix[i]); ++i)
			{
				operand.push_back(expressionFix[i]);
			}

			if (!operand.empty())
			{
				if (ExpressionAnalyzer::IsDecimal(operand))
					postfixExpr += operand;
				else
					postfixExpr += converter(operand);
			}

			if (i < expressionFix.length() && GeneralUtils::IsOperator(expressionFix[i]))
			{
				char op = expressionFix[i];

				if (stackOperator.empty())
				{
					stackOperator.emplace(op);
				}
				else
				{
					while (!stackOperator.empty() && !GeneralUtils::OperatorPriorityGreaterThan(op, stackOperator.top()))
					{
						postfixExpr.push_back(stackOperator.top());
						stackOperator.pop();
					}
					stackOperator.emplace(op);
				}
			}
		}

		while (!stackOperator.empty())
		{
			postfixExpr.push_back(stackOperator.top());
			stackOperator.pop();
		}

		return postfixExpr;
	}

	//converter: return std::string, only has one paramter with type const std::string&
	template <class _value_converter>
	static std::vector<word> InfixToPostfixWords(const std::string& expression, _value_converter converter)
	{
		std::vector<ExpressionAnalyzer::word> words;
		std::string expressionFix(expression);

		ExpressionAnalyzer::DeleteSpace(expressionFix);
		ExpressionAnalyzer::FixSignOperand(expressionFix);

		if (!ExpressionAnalyzer::IsValidInfixExpression(expressionFix))
			return words;

		std::string postfixExpr = std::move
		(
			ExpressionAnalyzer::InfixToPostfixExpression(expressionFix, [](const std::string& name)
				{
					return name;
				})
		);
		words = std::move(ExpressionAnalyzer::Word_Analysis(postfixExpr));

		for (auto& word : words)
		{
			if (!word.IsOperator && !word.IsDecimal)
				word.Item = converter(word.Item);
		}

		return words;
	}

	//getter: return double, only has one paramter with type const std::string&
	//delete all sapce and convert +x, -x to (0+x), (0-x) 
	template <class _value_getter>
	static double CalculateInfixExpression(std::string& expression, _value_getter getter)
	{
		DeleteSpace(expression);
		FixSignOperand(expression);

		std::string postfixExpr(std::move(InfixToPostfixExpression(expression)));
		return ExpressionAnalyzer::CalculatePostfixExpression(postfixExpr, getter);
	}

	//getter: return double, only has one paramter with type const std::string&
	template <class _value_getter>
	static double CalculatePostfixExpression(const std::string& expression, _value_getter getter)
	{
		std::vector<word> words(std::move(Word_Analysis(expression)));

		return ExpressionAnalyzer::CalculatePostfixExpression(words, getter);
	}

	//getter: return double, only has one paramter with type const std::string&
	template <class _value_getter>
	static double CalculatePostfixExpression(const std::vector<word>& expression, _value_getter getter)
	{
		std::stack<double> stackOperand;

		for (const auto& word : expression)
		{
			if (word.IsOperator)
			{
				if (stackOperand.size() < 2)
					return 0.0;

				double operandA = stackOperand.top();
				stackOperand.pop();
				double operandB = stackOperand.top();
				stackOperand.pop();

				switch (word.Item[0])
				{
				case '+':
					stackOperand.emplace(operandA + operandB);
					break;
				case '-':
					stackOperand.emplace(operandA - operandB);
					break;
				case '*':
					stackOperand.emplace(operandA * operandB);
					break;
				case '/':
					stackOperand.emplace(operandA / operandB);
					break;
				default:
					return 0.0;
					break;
				}
			}
			else
			{
				if (word.IsDecimal)
				{
					stackOperand.emplace(atof(word.Item.c_str()));
				}
				else
				{
					stackOperand.emplace(getter(word.Item));
				}
			}
		}

		if (stackOperand.size() != 1U)
		{
			return 0.0;
		}

		return stackOperand.top();
	}
};
