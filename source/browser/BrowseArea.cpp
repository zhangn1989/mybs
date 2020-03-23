#include "BrowseArea.h"
#include <QPainter>

BrowseArea::BrowseArea(QWidget *parent)
	: QWidget(parent)
{
}

BrowseArea::~BrowseArea()
{
}

void BrowseArea::setText(const QString &text)
{
	m_text = text;
	update();
}

void BrowseArea::setPixmap(const QPixmap &pixmap)
{
	m_pixmap = pixmap;
	update();
}

void BrowseArea::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.drawText(0, 10, m_text);
	painter.drawPixmap(0, 50, m_pixmap);
}
