#pragma once
#include "qgraphicsitem.h"
class GuiConnection :
	public QGraphicsObject
{
public:
	GuiConnection();
	virtual ~GuiConnection();

	static size_t GetAmount() { return _amount; }
protected:

private:
	static std::atomic_uint64_t _amount;//类实例总数
};

