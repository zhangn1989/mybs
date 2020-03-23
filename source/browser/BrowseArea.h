#pragma once

#include <QWidget>

class BrowseArea : public QWidget
{
	Q_OBJECT

public:
	BrowseArea(QWidget *parent = nullptr);
	~BrowseArea();

	void setText(const QString &text);
	void setPixmap(const QPixmap &pixmap);

protected:
	void paintEvent(QPaintEvent * event);

private:
	QString m_text;
	QPixmap m_pixmap;
};
