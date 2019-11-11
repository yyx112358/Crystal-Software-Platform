#pragma once
#include <QString>
#include <QException>

class GraphError
	:QException
{
public:
	enum ErrorCode
	{
		AssertFail,
		NotImplement,
	}code;
	QString	err;
	QString file;
	QString	func;
	int	line;
	QString msg;

	GraphError(ErrorCode code, QString err, QString file, QString func, int line)
		:code(code), err(err), file(file), func(func), line(line)
	{
		msg = QString("[GraphError %1]\n").arg(code) + err + '\n' + file + '\n' + func + '\n' + QString::number(line);
	}
	virtual QException * clone() const override { return new GraphError(*this); }
	virtual void raise()const { throw *this; }
	virtual char const* what() const override { return msg.toStdString().data(); }
#define GRAPH_ASSERT(expr)	do { if (!!(expr)); else GraphError(GraphError::AssertFail, #expr, __FUNCTION__, __FILE__, __LINE__).raise(); } while (0)
#define GRAPH_NOT_IMPLEMENT GraphError(GraphError::NotImplement, "Not Implement", __FUNCTION__, __FILE__, __LINE__).raise();
};