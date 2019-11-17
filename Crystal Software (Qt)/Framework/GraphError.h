#pragma once
#include <QString>
#include <QException>


//自定义错误类，参考cv::Exception
//由于继承了QException，所以可以在QtConcurrent当中抛出
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
namespace QtPrivate
{
#include <QObject>
	class GraphWarning_StaticHandler
		:public QObject
	{
		Q_OBJECT
	public:
		void inform(QString msg) { emit sig_Inform(msg); }
		void warning(QString msg) { emit sig_Warning(msg); }
	signals:
		void sig_Inform(QString msg);
		void sig_Warning(QString msg);
	public:
	};
}
//简单的单例警告输出
//所有GraphWarning内部共用一个单例类GraphWarning_StaticHandler
//该单例类可通过静态函数connectXXX()连接到某些槽上面（例如在主窗口的statusBar()上面显示）
class GraphWarning
{
public:
	QString msg;
	GraphWarning(QString msg) :msg(msg) {}

	template<typename Functor>
	static void connectInform(QObject*another, Functor func)//连接通知信号
	{
		QtPrivate::GraphWarning_StaticHandler::connect(&handler, &QtPrivate::GraphWarning_StaticHandler::sig_Inform,
			another, func, Qt::QueuedConnection);//QueuedConnection以保证线程安全
	}
	template<typename Functor>
	static void connectWarning(QObject*another, Functor func)//连接警告信号
	{
		QtPrivate::GraphWarning_StaticHandler::connect(&handler, &QtPrivate::GraphWarning_StaticHandler::sig_Warning,
			another, func, Qt::QueuedConnection);//QueuedConnection以保证线程安全
	}
	void inform()//通知
	{
		handler.inform(msg);
	}
	void warning()//警告
	{
		handler.warning(msg);
	}
#define GRAPH_INFORM(msg) GraphWarning(msg).inform();
#define GRAPH_WARNING(msg) GraphWarning(msg).warning();
private:
	static QtPrivate::GraphWarning_StaticHandler handler;//私有单例
};


