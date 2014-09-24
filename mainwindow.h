#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class QTableWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    enum Scope {
        UserScope,
        SystemScope
    };

private:
    void initConnection();
    void initData();
    QString getVarFromTable(QTableWidget *);
    QString getPath(Scope scope);
    void setPath(Scope scope, const QString &var);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
