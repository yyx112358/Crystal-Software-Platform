#pragma once
#include <QString>
#include <QException>

class GraphError
	:QException
{
public:
	enum ErrorCode
	{
		AccessNull,//访问空值
		AssertFail,//参数校验错误
		KeyConflict,//键值冲突
		NotExist,//不存在
		NotImplement,//函数未定义
	}code;
	QString	err;
	QString file;
	QString	func;
	int	line;
	QString msg;

	GraphError(ErrorCode code, QString err, QString file, QString func, int line)
		:code(code), err(err), file(file), func(func), line(line)
	{
		msg = QString("[GraphError %1]\n").arg(code)
			+ err + '\n'
			+ "File:" + file + '\n'
			+ "Function:" + func + '\n'
			+ "Line:" + QString::number(line) + '\n';
	}
	virtual QException * clone() const override { return new GraphError(*this); }
	virtual void raise()const { throw *this; }
	virtual char const* what() const override { return msg.toStdString().data(); }
#define GRAPH_ASSERT(expr)	do { if (!!(expr)); else throw GraphError(GraphError::AssertFail, #expr, __FUNCTION__, __FILE__, __LINE__); } while (0)
#define GRAPH_NOT_IMPLEMENT throw GraphError(GraphError::NotImplement, "Not Implement", __FUNCTION__, __FILE__, __LINE__);
#define GRAPH_NOT_EXIST(tbl,key) throw GraphError(GraphError::NotExist,"["#key"] Not Exist in ["#tbl"]", __FUNCTION__, __FILE__, __LINE__);
#define GRAPH_ACCESS_NULL(value) throw GraphError(GraphError::NotExist,"["#value "] is null", __FUNCTION__, __FILE__, __LINE__);
};