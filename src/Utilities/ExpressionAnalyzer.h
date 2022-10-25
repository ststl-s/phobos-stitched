#pragma once

#include <string>
#include <vector>
#include <stack>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Savegame.h>

class ExpressionAnalyzer
{
public:
	struct word
	{
		std::string Item;
		bool IsOperator;
		bool IsDecimal; // is it a direct value

		bool Load(PhobosStreamReader& stm, bool registerForChange);
		bool Save(PhobosStreamWriter& stm) const;
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
	static std::vector<word> InfixToPostfixWords(const std::string& expression, _value_converter converter)
	{
		std::vector<ExpressionAnalyzer::word> words;
		std::string expressionFix(expression);

		ExpressionAnalyzer::DeleteSpace(expressionFix);
		ExpressionAnalyzer::FixSignOperand(expressionFix);

		if (!ExpressionAnalyzer::IsValidInfixExpression(expressionFix))
			return words;

		std::stack<char> stackOperator;

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
					words.push_back({ operand, false, true });
				else
					words.push_back({ converter(operand),false,false });
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
						std::string _operator;
						_operator.push_back(stackOperator.top());
						words.push_back({ _operator, true, false });
						stackOperator.pop();
					}
					stackOperator.emplace(op);
				}
			}
		}

		while (!stackOperator.empty())
		{
			std::string _operator;
			_operator.push_back(stackOperator.top());
			words.push_back({ _operator, true, false });
			stackOperator.pop();
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

		std::string postfixExpr(std::move(ExpressionAnalyzer::InfixToPostfixWords(expression, [](const std::string& name) { return name; })));

		return ExpressionAnalyzer::CalculatePostfixExpression(postfixExpr, getter);
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
					stackOperand.emplace(operandB + operandA);
					break;
				case '-':
					stackOperand.emplace(operandB - operandA);
					break;
				case '*':
					stackOperand.emplace(operandB * operandA);
					break;
				case '/':
					stackOperand.emplace(operandB / operandA);
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
