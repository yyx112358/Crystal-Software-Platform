#pragma once
#include "qgraphicsitem.h"
class GuiVertex :
	public QGraphicsObject
{
public:
	GuiVertex();
	virtual ~GuiVertex();

	static size_t GetAmount() { return _amount; }
protected:

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

