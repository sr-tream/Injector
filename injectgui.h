#ifndef INJECTGUI_H
#define INJECTGUI_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QTextCodec>
#include <QSettings>
#include <QDir>
#include <windows.h>
#include <tlhelp32.h>

namespace Ui {
class InjectGui;
}

typedef unsigned char byte;

template <typename T>
union byteValue{
    T value;
    byte bytes[sizeof( T )];
};

class InjectGui : public QMainWindow
{
    Q_OBJECT

public:
    explicit InjectGui(QString cpath, QFileInfo file, QWidget *parent = 0);
    ~InjectGui();

protected:
    void closeEvent(QCloseEvent * e);

    BOOL inject( DWORD pId, QString dll );
    PROCESS_INFORMATION run();

    template<typename T>
    void pushInj(T data);
    void pushInj(QString data);

    template<typename T>
    T popInj();
    QString popInj();

    QList<QString> libs(QListWidget *list);
    bool isLibraryExist(QString lib);

    QFile* fileInj;
    void loadInj();

private slots:
    void on_toolButton_clicked();

    void on_disabled_itemDoubleClicked(QListWidgetItem *item);

    void on_enabled_itemDoubleClicked(QListWidgetItem *item);

    void on_add_clicked();

    void on_run_clicked();

private:
    Ui::InjectGui *ui;

    QString current_path;
    QTextCodec *codec;
};

#endif // INJECTGUI_H
