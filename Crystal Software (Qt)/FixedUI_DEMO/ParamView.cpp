#include "stdafx.h"
#include "ParamView.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include "ParamDelegate.h"
#include "GraphError.h"
#include "CustomTypes.h"

ParamView::ParamView(QWidget *parent)
	: QTableView(parent),_role(PARAMETER),_model(this)
{
	_model.setColumnCount(5);
	_model.setHeaderData(COLUMN::STATUS, Qt::Orientation::Horizontal, "", Qt::ItemDataRole::DisplayRole);
	_model.setHeaderData(COLUMN::NAME, Qt::Orientation::Horizontal, "Name", Qt::ItemDataRole::DisplayRole);
	_model.setHeaderData(COLUMN::VALUE, Qt::Orientation::Horizontal, "Value", Qt::ItemDataRole::EditRole);
	_model.setHeaderData(COLUMN::TYPE, Qt::Orientation::Horizontal, "Type", Qt::ItemDataRole::DisplayRole);
	_model.setHeaderData(COLUMN::EXPLAINATION, Qt::Orientation::Horizontal, "Explanation", Qt::ItemDataRole::DisplayRole);
	setModel(&_model);

	setSelectionBehavior(QTreeView::SelectionBehavior::SelectRows);//一次选中一行
	//ui.treeView->setEditTriggers(QTreeView::EditTrigger::NoEditTriggers);//不可修改
	// 	ui.treeView->resizeColumnsToContents();//列宽随内容变化
	// 	ui.treeView->resizeRowsToContents();//行高随内容变化
	
	setItemDelegate(new ParamDelegate(this));
	setColumnWidth(COLUMN::STATUS, 40);
	setColumnWidth(COLUMN::NAME, 80);
	setColumnWidth(COLUMN::VALUE, 80);
	setColumnWidth(COLUMN::TYPE, 40);
	setColumnWidth(COLUMN::EXPLAINATION, 200);

	connect(&_model, &QStandardItemModel::itemChanged, this, &ParamView::sig_ParamChanged);
// 	connect(&_model, &QStandardItemModel::itemChanged, [this](QStandardItem*item)
// 	{
// 		qDebug() << item->row() << item->column() << item->data();
// 	});
}

ParamView::~ParamView()
{
}

void ParamView::AddParam(QString name, QVariant::Type type, QString explaination /*= ""*/, QVariant defaultValue /*= QVariant()*/)
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//原则上名称唯一
	GRAPH_ASSERT(defaultValue.isValid() == false || defaultValue.canConvert(type));//类型检查
	if (items.empty() == true)//没有，则添加
	{
		QList<QStandardItem*>list{ new QStandardItem(), new QStandardItem(),new QStandardItem(),new QStandardItem(),new QStandardItem() };

		list[COLUMN::STATUS]->setData("", Qt::ItemDataRole::DisplayRole);
		list[COLUMN::NAME]->setData(name, Qt::ItemDataRole::DisplayRole);
		list[COLUMN::VALUE]->setData(defaultValue, Qt::ItemDataRole::EditRole);
		list[COLUMN::TYPE]->setData((int)type, Qt::ItemDataRole::DisplayRole);
		list[COLUMN::EXPLAINATION]->setData(explaination, Qt::ItemDataRole::DisplayRole);
		_model.appendRow(list);
		
		resizeRowsToContents();
		//_slot_ItemChanged(list[E_ColumnHeader::VALUE]);
	}
	else//已有，则更换
	{
		for (auto item : items)
		{
			auto row = item->row();
			_model.setData(_model.index(row, COLUMN::STATUS), "", Qt::ItemDataRole::DisplayRole);
			//_model.setData(_model.index(row, COLUMN::VALUE), name, Qt::ItemDataRole::DisplayRole);
			_model.setData(_model.index(row, COLUMN::VALUE), defaultValue, Qt::ItemDataRole::EditRole);
			_model.setData(_model.index(row, COLUMN::TYPE), (int)type, Qt::ItemDataRole::DisplayRole);
			_model.setData(_model.index(row, COLUMN::EXPLAINATION), explaination, Qt::ItemDataRole::DisplayRole);
		}
	}
}

void ParamView::RemoveParam(QString name)
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//原则上名称唯一
	if(items.empty() == false)//找到了
		_model.removeRow(items[0]->row());
}

void ParamView::SetParam(QString name, QVariant value)
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//原则上名称唯一
	for (auto &item : items)
	{
		auto t = _model.data(_model.index(item->row(), COLUMN::TYPE)).toInt();
		GRAPH_ASSERT(value.isValid() == false
			|| value.canConvert(t) == true);//类型检查
		_model.item(item->row(), COLUMN::VALUE)->setData(value,Qt::ItemDataRole::EditRole);
		resizeRowsToContents();
	}
}

QVariant ParamView::GetParam(QString name) const
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//原则上名称唯一。因此只取第一个
	if (items.empty() == false)
		return _model.data(_model.index(items[0]->row(), COLUMN::VALUE));
	else
		return QVariant();
}

QString ParamView::GetRowName(QStandardItem*item) const
{
	if (item == nullptr || item->model() != &_model)
		return QString();
	int row = item->row();
	return _model.item(row, ParamView::NAME)->data(Qt::DisplayRole).toString();
}

int ParamView::GetRow(QString name) const
{
	auto items = _model.findItems(name, Qt::MatchFlag::MatchExactly, COLUMN::NAME);
	GRAPH_ASSERT(items.size() <= 1);//原则上名称唯一。因此只取第一个
	if (items.size() == 0)
		return -1;
	else
		return items[0]->row();
}

QVariant::Type ParamView::GetType(QString name) const
{
	int row = GetRow(name);
	if (row < 0)return QVariant::Invalid;
	return static_cast<QVariant::Type>(_model.item(row, TYPE)->data(Qt::ItemDataRole::DisplayRole).toInt());
}

QString ParamView::GetExplaination(QString name) const
{
	int row = GetRow(name);
	if (row < 0)return QString();
	return _model.item(row, EXPLAINATION)->data(Qt::ItemDataRole::DisplayRole).toString();
}

QStandardItem* ParamView::GetValueItem(QString name)
{
	int row = GetRow(name);
	if (row < 0)return nullptr;
	return _model.item(row, VALUE);
}

void ParamView::contextMenuEvent(QContextMenuEvent *event)
{
	QModelIndex modelIndex = currentIndex();
	if (modelIndex.isValid() == true)
	{
		int row = modelIndex.row(), col = modelIndex.column();
		int type = _model.data(_model.index(row, COLUMN::TYPE)).toInt() ;
		QVariantList paramInfo({ QVariant(),QVariant() ,QVariant() ,QVariant() ,QVariant() });
		
		paramInfo[STATUS] = _model.data(_model.index(row, COLUMN::STATUS));
		paramInfo[NAME] = _model.data(_model.index(row, COLUMN::NAME));
		paramInfo[VALUE] = _model.data(_model.index(row, COLUMN::VALUE));
		paramInfo[TYPE] = _model.data(_model.index(row, COLUMN::TYPE));
		paramInfo[EXPLAINATION] = _model.data(_model.index(row, COLUMN::EXPLAINATION));

		QMenu menu(this);
		QAction*action = nullptr;
		if (type == MatTypeId()&&_role==INPUT)
		{
			if(paramInfo[VALUE].isValid() == false)
				menu.addAction(QStringLiteral("连接输入源"));
			else
				menu.addAction(QStringLiteral("断开输入源"));
		}
		action = menu.addAction(QStringLiteral("监视"));
		action->setCheckable(true);
		
		
		QAction*result = menu.exec(cursor().pos());
		if (result != nullptr)
			emit sig_ActionTriggered(result->text(), modelIndex, paramInfo, result->isChecked());
	}
	return QTableView::contextMenuEvent(event);
}
