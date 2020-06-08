#pragma once

#include <QStyledItemDelegate>

class ParamDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ParamDelegate(QObject *parent);
	~ParamDelegate();

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;


	virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;


	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;


	virtual QString displayText(const QVariant &value, const QLocale &locale) const override;

};
