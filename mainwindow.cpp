#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <windows.h>

#define MAX_VALUE 16383
#define SYSPATH L"System\\CurrentControlSet\\Control\\Session Manager\\Environment"
#define USRPATH L"Environment"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initData();
    initConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initConnection()
{
    connect(ui->btnAdd, &QPushButton::clicked, this, [&]{
        int index = ui->dataUser->rowCount();
        ui->dataUser->setRowCount(index+1);
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->dataUser->setItem(index, 0, item);
    });

    connect(ui->btnDel, &QPushButton::clicked, this, [&]{
        int index = ui->dataUser->currentRow();
        if (index == -1)
            return;
        ui->dataUser->removeRow(index);
        setPath(UserScope, getVarFromTable(ui->dataUser));
    });

    connect(ui->btnUp, &QPushButton::clicked, this, [&]{
        int index = ui->dataUser->currentRow();
        if (index == -1 || index == 0)
            return;
        QString temp = ui->dataUser->item(index, 0)->text();
        ui->dataUser->item(index, 0)->setText(ui->dataUser->item(index-1, 0)->text());
        ui->dataUser->item(index-1, 0)->setText(temp);
        ui->dataUser->setCurrentCell(index-1, 0);
    });

    connect(ui->btnDown, &QPushButton::clicked, this, [&]{
        int index = ui->dataUser->currentRow();
        if (index == -1 || index == ui->dataUser->rowCount()-1)
            return;
        QString temp = ui->dataUser->item(index, 0)->text();
        ui->dataUser->item(index, 0)->setText(ui->dataUser->item(index+1, 0)->text());
        ui->dataUser->item(index+1, 0)->setText(temp);
        ui->dataUser->setCurrentCell(index+1, 0);
    });

    connect(ui->dataUser, &QTableWidget::itemChanged, this, [&](QTableWidgetItem *){
        setPath(UserScope, getVarFromTable(ui->dataUser));
    });

    connect(ui->btnAdd2, &QPushButton::clicked, this, [&]{
        int index = ui->dataGlobal->rowCount();
        ui->dataGlobal->setRowCount(index+1);
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->dataGlobal->setItem(index, 0, item);
    });

    connect(ui->btnDel2, &QPushButton::clicked, this, [&]{
        int index = ui->dataGlobal->currentRow();
        if (index == -1)
            return;
        ui->dataGlobal->removeRow(index);
        setPath(SystemScope, getVarFromTable(ui->dataGlobal));
    });

    connect(ui->btnUp2, &QPushButton::clicked, this, [&]{
        int index = ui->dataGlobal->currentRow();
        if (index == -1 || index == 0)
            return;
        QString temp = ui->dataGlobal->item(index, 0)->text();
        ui->dataGlobal->item(index, 0)->setText(ui->dataGlobal->item(index-1, 0)->text());
        ui->dataGlobal->item(index-1, 0)->setText(temp);
        ui->dataGlobal->setCurrentCell(index-1, 0);
    });

    connect(ui->btnDown2, &QPushButton::clicked, this, [&]{
        int index = ui->dataUser->currentRow();
        if (index == -1 || index == ui->dataGlobal->rowCount()-1)
            return;
        QString temp = ui->dataGlobal->item(index, 0)->text();
        ui->dataGlobal->item(index, 0)->setText(ui->dataGlobal->item(index+1, 0)->text());
        ui->dataGlobal->item(index+1, 0)->setText(temp);
        ui->dataGlobal->setCurrentCell(index+1, 0);
    });

    connect(ui->dataGlobal, &QTableWidget::itemChanged, this, [&](QTableWidgetItem *){
        setPath(SystemScope, getVarFromTable(ui->dataGlobal));
    });
}

void MainWindow::initData()
{
    QString userPath = getPath(UserScope);
    QStringList items = userPath.split(";", QString::SkipEmptyParts);
    ui->dataUser->setRowCount(items.count());
    for (int i = 0; i < items.count(); i++) {
        QTableWidgetItem *item = new QTableWidgetItem(items[i]);
        ui->dataUser->setItem(i, 0, item);
    }

    QString systemPath = getPath(SystemScope);
    items = systemPath.split(";", QString::SkipEmptyParts);
    ui->dataGlobal->setRowCount(items.count());
    for (int i = 0; i < items.count(); i++) {
        QTableWidgetItem *item = new QTableWidgetItem(items[i]);
        ui->dataGlobal->setItem(i, 0, item);
    }
}

QString MainWindow::getVarFromTable(QTableWidget *table)
{
    int row = table->rowCount();
    QString result;
    for(int i = 0; i < row; i++) {
        result += table->item(i, 0)->text();
        result += ";";
    }
    return result;
}

QString MainWindow::getPath(Scope scope)
{
    HKEY hKey;
    LSTATUS lResult;
    if (scope == SystemScope) {
        lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SYSPATH, 0, KEY_READ, &hKey);
    } else {
        lResult = RegOpenKeyExW(HKEY_CURRENT_USER, USRPATH, 0, KEY_READ, &hKey);
    }
    if (lResult != ERROR_SUCCESS) {
        return QString();
    }

    DWORD type;
    DWORD cbData = MAX_VALUE;
    BYTE  data[MAX_VALUE];
    lResult = RegGetValueW(hKey, NULL, L"Path", RRF_RT_ANY, &type, data, &cbData);
    if (lResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return QString();
    }

    RegCloseKey(hKey);
    return QString::fromWCharArray((const wchar_t *)data).trimmed();
}

void MainWindow::setPath(Scope scope, const QString &var)
{
    HKEY hKey;
    LSTATUS lResult;
    if (scope == SystemScope) {
        lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SYSPATH, 0, KEY_WRITE, &hKey);
    } else {
        lResult = RegOpenKeyExW(HKEY_CURRENT_USER, USRPATH, 0, KEY_WRITE, &hKey);
    }
    if (lResult != ERROR_SUCCESS)
        return;

    BYTE *data = (BYTE*)calloc(var.length() * 2 + 2, 1);
    memcpy(data, var.utf16(), var.length() * 2);
    RegSetValueExW(hKey, L"Path", NULL, REG_EXPAND_SZ, data, var.length() * 2 + 2);
    RegCloseKey(hKey);
    free(data);
}
