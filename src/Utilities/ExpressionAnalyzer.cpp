#include "ExpressionAnalyzer.h"

#include <stack>

#include <Utilities/GeneralUtils.h>

std::deque<ExpressionAnalyzer::word> ExpressionAnalyzer::Words;
bool ExpressionAnalyzer::Error = false;
size_t ExpressionAnalyzer::NowIdx = 0;

bool ExpressionAnalyzer::IsDecimal(const std::string& operand)
{
	bool point = false;
	bool scientific = false;

	for (int i = 0; i < operand.size(); i++)
	{
		char c = operand[i];

		if (isdigit(c))
			continue;

		if (c == '.')
		{
			if (point)
				return false;

			point = true;
		}
		else if (c == 'e')
		{
			if (scientific)
				return false;

			scientific = true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

/*
E -> E + T |  E - T | T
T -> T * K | T / K | K
K -> + F | - F | F
F -> ( E ) | d

E -> T { + T | - T } }
T -> K { * K | / K } }
K -> F | + F | - F
F -> ( E ) | d
*/

bool ExpressionAnalyzer::IsValidInfixExpression(const std::string& expression)
{
	Words = std::move(Word_Analysis(expression));
	NowIdx = 0;
	Error = false;
	E();
	return !Error && NowIdx >= Words.size();
}

std::deque<ExpressionAnalyzer::word> ExpressionAnalyzer::Word_Analysis(const std::string& expression)
{
	std::deque<word> words;
	size_t idx = 0;

	for (;idx < expression.size();++idx)
	{
		if (!GeneralUtils::IsOperator(expression[idx]))
		{
			std::string operand;

			for (; idx < expression.size() && !GeneralUtils::IsOperator(expression[idx]); ++idx)
			{
				operand.push_back(expression[idx]);
			}

			if (IsDecimal(operand))
				words.push_back({ operand, false, true });
			else
				words.push_back({ operand, false, false });
		}
		else
		{
			std::string _operator;
			_operator.push_back(expression[idx]);
			words.push_back({ _operator, true, false });
		}
	}

	return words;
}

void ExpressionAnalyzer::E()
{
	T();

	while (NowIdx < Words.size() && (Words[NowIdx].Item == "+" || Words[NowIdx].Item == "-"))
	{
		++NowIdx;

		T();
	}
}

void ExpressionAnalyzer::T()
{
	K();

	while (NowIdx < Words.size() && (Words[NowIdx].Item == "*" || Words[NowIdx].Item == "/"))
	{
		++NowIdx;

		K();
	}
}

void ExpressionAnalyzer::K()
{
	while (NowIdx < Words.size() && (Words[NowIdx].Item == "+" || Words[NowIdx].Item == "-"))
	{
		++NowIdx;
	}

	F();
}

void ExpressionAnalyzer::F()
{
	if (NowIdx < Words.size())
	{
		if (!Words[NowIdx].IsOperator)
		{
			++NowIdx;
		}
		else if (Words[NowIdx].Item == "(")
		{
			++NowIdx;
			E();

			if (NowIdx < Words.size() && Words[NowIdx].Item == ")")
			{
				++NowIdx;
			}
			else
			{
				Error = true;
				NowIdx = Words.size();
			}
		}
		else
		{
			Error = true;
			NowIdx = Words.size();
		}
	}
	else
	{
		Error = true;
		NowIdx = Words.size();
	}
}

template <class _value_converter>
std::string ExpressionAnalyzer::InfixToPostfixExpression(const std::string& expression, _value_converter converter)
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
		for (; i < expressionFix.length() && !GeneralUtils::IsOperator(expressionFix[i]); ++i)
		{
			std::string operand;
			operand.push_back(expressionFix[i]);

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
				stackOperator.emplace(expressionFix[i]);
			}
			else
			{
				while (!stackOperator.empty() && !GeneralUtils::OperatorPriorityGreaterThan(expressionFix[i], stackOperator.top()))
				{
					postfixExpr.push_back(stackOperator.top());
					stackOperator.pop();
				}
				stackOperator.emplace(expressionFix[i]);
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

template <class _value_converter>
static std::deque<ExpressionAnalyzer::word> InfixToPostfixWords(const std::string& expression, _value_converter converter)
{
	std::deque<ExpressionAnalyzer::word> words;
	std::string expressionFix(expression);

	ExpressionAnalyzer::DeleteSpace(expressionFix);
	ExpressionAnalyzer::FixSignOperand(expressionFix);

	if (!ExpressionAnalyzer::IsValidInfixExpression(expressionFix))
		return words;

	words = std::move(ExpressionAnalyzer::Word_Analysis(ExpressionAnalyzer::InfixToPostfixExpression(expressionFix)));

	for (auto& word : words)
	{
		if (!word.IsOperator && !word.IsDecimal)
			word.Item = converter(word.Item);
	}

	return words;
}

bool ExpressionAnalyzer::IsSign(const std::string& expression, size_t idx)
{
	return idx == 0 || GeneralUtils::IsOperator(expression[idx - 1]) && expression[idx - 1] != ')';
}

void ExpressionAnalyzer::FixSignOperand(std::string& expression)
{
	for (size_t i = 0; i < expression.length(); i++)
	{
		if (IsSign(expression, i))
		{
			expression.insert(expression.begin() + i, '0');
			expression.insert(expression.begin() + i, '(');
			i += 2;

			while (!GeneralUtils::IsOperator(expression[i]))
			{
				++i;
			}

			expression.insert(expression.begin() + i, ')');
		}
	}
}

void ExpressionAnalyzer::DeleteSpace(std::string& expression)
{
	expression.erase
	(
		std::remove_if(expression.begin(), expression.end(), isspace),
		expression.end()
	);
}

template <class _value_getter>
double ExpressionAnalyzer::CalculateInfixExpression(std::string& expression, _value_getter getter)
{
	DeleteSpace(expression);
	FixSignOperand(expression);
	
	std::string postfixExpr(std::move(InfixToPostfixExpression(expression)));
	return ExpressionAnalyzer::CalculatePostfixExpression(postfixExpr, getter);
}

template <class _value_getter>
double ExpressionAnalyzer::CalculatePostfixExpression(const std::string& expression, _value_getter getter)
{
	std::deque<word> words(std::move(Word_Analysis(expression)));

	return ExpressionAnalyzer::CalculatePostfixExpression(words, getter);
}

template <class _value_getter>
double ExpressionAnalyzer::CalculatePostfixExpression(const std::deque<word>& expression, _value_getter getter)
{
	std::stack<double> stackOperand;

	for (const auto& word : expression)
	{
		if (word.IsOperator)
		{
			if (stackOperand.size() < 2)
				return NAN;

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
				return NAN;
				break;
			}
		}
		else
		{
			if (word.IsDecimal)
				stackOperand.emplace(atof(word.Item.c_str()));
			else
				stackOperand.emplace(getter(word.Item));
		}
	}

	if (stackOperand.size() != 1U)
		return NAN;

	return stackOperand.top();
}
