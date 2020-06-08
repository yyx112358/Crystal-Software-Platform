#include "stdafx.h"
#include "ParamView.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include "ParamDelegate.h"
#include "GraphError.h"
#include "CustomTypes.h"

ParamView::ParamView(QWidget *parent, ROLE role)
	: QTableView(parent),_role(role),_model(this)
{
	_model.setColumnCount(5);
	_model.setHeaderData(COLUMN::STATUS, Qt::Orientation::Horizontal, "", Qt::ItemDataRole::DisplayRole);
	_model.setHeaderData(COLUMN::NAME, Qt::Orientation::Horizontal, "Name", Qt::ItemDataRole::DisplayRole);
	_model.setHeaderData(COLUMN::VALUE, Qt::Orientation::Horizontal, "Value", Qt::ItemDataRole::EditRole);
	_model.setHeaderData(COLUMN::TYPE, Qt::Orientation::Horizontal, "Type", Qt::ItemDataRole::DisplayRole);
	_model.setHeaderData(COLUMN::EXPLAINATION, Qt::Orientation::Horizontal, "Explanation", Qt::ItemDataRole::DisplayRole);
	setModel(&_model);

	setSelectionBehavior(QTreeView::SelectionBehavior::SelectRows);//һ��ѡ��һ��
	//ui.treeView->setEditTriggers(QTreeView::EditTrigger::NoEditTriggers);//�����޸�
	// 	ui.treeView->resizeColumnsToContents();//�п������ݱ仯
	// 	ui.treeView->resizeRowsToContents();//�и������ݱ仯
	
	setItemDelegate(new ParamDelegate(this));
	setColumnWidth(COLUMN::STATUS, 40);
	setColumnWidth(COLUMN::NAME, 80);
	setColumnWidth(COLUMN::VALUE, 80);
	setColumnWidth(COLUMN::TYPE, 40);
	setColumnWidth(COLUMN::EXPLAINATION, 200);
}

ParamView::~ParamView()
{
}

void ParamView::AddParam(QString name, QVariant::Type type, QString explaination /*= ""*/, QVariant defaultValue /*= QVariant()*/)
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//ԭ��������Ψһ
	GRAPH_ASSERT(defaultValue.isValid() == false || defaultValue.canConvert(type));//���ͼ��
	if (items.empty() == true)//û�У������
	{
		QList<QStandardItem*>list{ new QStandardItem(), new QStandardItem(),new QStandardItem(),new QStandardItem(),new QStandardItem() };

		list[COLUMN::STATUS]->setData("", Qt::ItemDataRole::DisplayRole);
		list[COLUMN::NAME]->setData(name, Qt::ItemDataRole::DisplayRole);
		list[COLUMN::VALUE]->setData(defaultValue, Qt::ItemDataRole::EditRole);
		list[COLUMN::TYPE]->setData(QVariant::typeToName(type), Qt::ItemDataRole::DisplayRole);
		list[COLUMN::EXPLAINATION]->setData(explaination, Qt::ItemDataRole::DisplayRole);
		_model.appendRow(list);

		resizeRowsToContents();
		//_slot_ItemChanged(list[E_ColumnHeader::VALUE]);
	}
	else//���У������
	{
		for (auto item : items)
		{
			auto row = item->row();
			_model.setData(_model.index(row, COLUMN::STATUS), "", Qt::ItemDataRole::DisplayRole);
			//_model.setData(_model.index(row, COLUMN::VALUE), name, Qt::ItemDataRole::DisplayRole);
			_model.setData(_model.index(row, COLUMN::VALUE), defaultValue, Qt::ItemDataRole::EditRole);
			_model.setData(_model.index(row, COLUMN::TYPE), QVariant::typeToName(type), Qt::ItemDataRole::DisplayRole);
			_model.setData(_model.index(row, COLUMN::EXPLAINATION), explaination, Qt::ItemDataRole::DisplayRole);
		}
	}
}

void ParamView::RemoveParam(QString name)
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//ԭ��������Ψһ
	if(items.empty() == false)//�ҵ���
		_model.removeRow(items[0]->row());
}

void ParamView::SetParam(QString name, QVariant value)
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//ԭ��������Ψһ
	for (auto &item : items)
	{
		GRAPH_ASSERT(value.isValid() == false
			|| value.canConvert(_model.item(item->row(), COLUMN::TYPE)->data().userType()));//���ͼ��
		_model.item(item->row(), COLUMN::VALUE)->setData(value);
	}
}

QVariant ParamView::GetParam(QString name) const
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//ԭ��������Ψһ�����ֻȡ��һ��
	if (items.empty() == false)
		return _model.data(_model.index(items[0]->row(), COLUMN::VALUE));
	else
		return QVariant();
}

void ParamView::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex modelIndex = currentIndex();
	if (modelIndex.isValid() == true)
	{
		int row = modelIndex.row(),col=modelIndex.column();
		int type = _model.item(row, TYPE)->data().toInt();
		QVariantList paramInfo({ QVariant(),QVariant() ,QVariant() ,QVariant() ,QVariant() });
		paramInfo[STATUS] = _model.item(row, STATUS)->data();
		paramInfo[NAME] = _model.item(row, NAME)->data();
		paramInfo[VALUE] = _model.item(row, VALUE)->data();
		paramInfo[TYPE] = _model.item(row, TYPE)->data();
		paramInfo[EXPLAINATION] = _model.item(row, EXPLAINATION)->data();

		QMenu menu(this);
		QAction*actionShow = menu.addAction(QString("Show"));
		QAction*actionMonitor = menu.addAction(QStringLiteral("����"));
		
		
		QAction*result = menu.exec(cursor().pos());
		if (result != nullptr)
			emit sig_ActionTriggered(result->text(), _role, paramInfo, result->isChecked());
	}
	return QTableView::contextMenuEvent(event);
}
