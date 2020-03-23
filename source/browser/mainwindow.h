#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
	void onPushButtonClicked(bool checked = false);
	void onReadyRead();
	void onDisconnected();

private:
    Ui::MainWindow *ui;
	char *m_tempBuff;
	int m_cursor;
	int m_surplus;
	const int m_maxRead = 65536;
};

#endif // MAINWINDOW_H
