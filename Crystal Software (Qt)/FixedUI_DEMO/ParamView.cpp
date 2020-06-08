#include "stdafx.h"
#include "ParamView.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include "ParamDelegate.h"
#include "GraphError.h"

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
		list[COLUMN::TYPE]->setData(QVariant::typeToName(type), Qt::ItemDataRole::DisplayRole);
		list[COLUMN::EXPLAINATION]->setData(explaination, Qt::ItemDataRole::DisplayRole);
		_model.appendRow(list);
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
			_model.setData(_model.index(row, COLUMN::TYPE), QVariant::typeToName(type), Qt::ItemDataRole::DisplayRole);
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
		GRAPH_ASSERT(value.isValid() == false
			|| value.canConvert(_model.item(item->row(), COLUMN::TYPE)->data().type()));//类型检查
		_model.item(item->row(), COLUMN::VALUE)->setData(value);
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
