#pragma once
#include <QString>
#include <QException>


//�Զ�������࣬�ο�cv::Exception
//���ڼ̳���QException�����Կ�����QtConcurrent�����׳�
class GraphError
	:QException
{
public:
	enum ErrorCode
	{
		AccessNull,//���ʿ�ֵ
		AssertFail,//����У�����
		KeyConflict,//��ֵ��ͻ
		NotExist,//������
		NotImplement,//����δ����
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
//�򵥵ĵ����������
//����GraphWarning�ڲ�����һ��������GraphWarning_StaticHandler
//�õ������ͨ����̬����connectXXX()���ӵ�ĳЩ�����棨�����������ڵ�statusBar()������ʾ��
class GraphWarning
{
public:
	QString msg;
	GraphWarning(QString msg) :msg(msg) {}

	template<typename Functor>
	static void connectInform(QObject*another, Functor func)//����֪ͨ�ź�
	{
		QtPrivate::GraphWarning_StaticHandler::connect(&handler, &QtPrivate::GraphWarning_StaticHandler::sig_Inform,
			another, func, Qt::QueuedConnection);//QueuedConnection�Ա�֤�̰߳�ȫ
	}
	template<typename Functor>
	static void connectWarning(QObject*another, Functor func)//���Ӿ����ź�
	{
		QtPrivate::GraphWarning_StaticHandler::connect(&handler, &QtPrivate::GraphWarning_StaticHandler::sig_Warning,
			another, func, Qt::QueuedConnection);//QueuedConnection�Ա�֤�̰߳�ȫ
	}
	void inform()//֪ͨ
	{
		handler.inform(msg);
	}
	void warning()//����
	{
		handler.warning(msg);
	}
#define GRAPH_INFORM(msg) GraphWarning(msg).inform();
#define GRAPH_WARNING(msg) GraphWarning(msg).warning();
private:
	static QtPrivate::GraphWarning_StaticHandler handler;//˽�е���
};


