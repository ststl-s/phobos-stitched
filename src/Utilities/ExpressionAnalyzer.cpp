#include "ExpressionAnalyzer.h"

std::vector<ExpressionAnalyzer::word> ExpressionAnalyzer::Words;
bool ExpressionAnalyzer::Error = false;
size_t ExpressionAnalyzer::NowIdx = 0;

bool ExpressionAnalyzer::word::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return stm
		.Process(this->Item)
		.Process(this->IsOperator)
		.Process(this->IsDecimal)
		.Success()
		;
}

bool ExpressionAnalyzer::word::Save(PhobosStreamWriter& stm) const
{
	return stm
		.Process(this->Item)
		.Process(this->IsOperator)
		.Process(this->IsDecimal)
		.Success()
		;
}

bool ExpressionAnalyzer::IsDecimal(const std::string& operand)
{
	bool point = false;
	bool scientific = false;

	for (size_t i = 0; i < operand.size(); i++)
	{
		char c = operand[i];

		if (isdigit(c))
			continue;

		if (c == '.')
		{
			if (point || scientific)
				return false;

			point = true;
		}
		else if (c == 'e' || c == 'E')
		{
			if (scientific)
				return false;

			scientific = true;
		}
		else if (c == '%')
		{
			if (i + 1 != operand.size() || i == 0)
				return false;
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

std::vector<ExpressionAnalyzer::word> ExpressionAnalyzer::Word_Analysis(const std::string& expression)
{
	std::vector<word> words;
	size_t idx = 0;

	for (; idx < expression.length(); ++idx)
	{
		if (!GeneralUtils::IsOperator(expression[idx]))
		{
			std::string operand;

			for (; idx < expression.length() && !GeneralUtils::IsOperator(expression[idx]); ++idx)
			{
				operand.push_back(expression[idx]);
			}

			if (IsDecimal(operand))
			{
				if (operand.back() == '%')
				{
					operand = std::to_string(atof(operand.substr(0, operand.length() - 1).c_str()) * 0.01);
				}

				words.push_back({ operand, false, true });
			}
			else
			{
				words.push_back({ operand, false, false });
			}
		}

		if (idx < expression.length())
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

bool ExpressionAnalyzer::IsSign(const std::string& expression, size_t idx)
{
	return (expression[idx] == '+' || expression[idx] == '-') &&
		(idx == 0 || (GeneralUtils::IsOperator(expression[idx - 1]) && expression[idx - 1] != ')'));
}

void ExpressionAnalyzer::FixSignOperand(std::string& expression)
{
	for (size_t i = 0; i < expression.length(); ++i)
	{
		if (IsSign(expression, i))
		{
			expression.insert(expression.begin() + i, '0');
			expression.insert(expression.begin() + i, '(');
			i += 3;

			while (i < expression.length() && !GeneralUtils::IsOperator(expression[i]))
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
