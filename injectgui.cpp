#include "injectgui.h"
#include "ui_injectgui.h"

InjectGui::InjectGui(QString cpath, QFileInfo file, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InjectGui)
{
    current_path = QFileInfo(cpath).path() + "/";
    codec = QTextCodec::codecForName("cp1251");
    ui->setupUi(this);
    ui->statusBar->showMessage("Injector by SR_team (c)prime-hack.net");
    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.suffix().toLower() == "dll")
            ui->disabled->addItem(fileInfo.fileName());

    }
    QSettings ext("HKEY_CURRENT_USER\\Software\\Classes\\.inj",
                        QSettings::NativeFormat);
    ext.setValue(".", "inj_auto_file");
    QSettings open ("HKEY_CURRENT_USER\\Software\\Classes\\inj_auto_file\\shell\\open\\command",
                        QSettings::NativeFormat);
    open.setValue(".", QString("\"" + codec->fromUnicode(cpath) + "\" %1"));

    if (file.isFile()){
        fileInj = new QFile(file.fileName());
        fileInj->open(QIODevice::ReadOnly);
        if (!fileInj->isReadable())
            return;
        fileInj->reset();

        loadInj();

        fileInj->close();
        ui->statusBar->showMessage("Load from " + file.fileName());
    }
    else fileInj = nullptr;
}

InjectGui::~InjectGui()
{
    delete ui;
}

void InjectGui::closeEvent(QCloseEvent *e)
{
    if (ui->exe->text().isEmpty())
        return;

    if (fileInj == nullptr){
        QString name = ui->exe->text() + ".inj";
        fileInj = new QFile(name);
    }

    if (fileInj->exists())
        fileInj->remove();
    fileInj->open(QIODevice::WriteOnly);
    if (!fileInj->isWritable())
        return;
    fileInj->reset();

    pushInj(ui->exe->text());
    pushInj(ui->args->text());

    pushInj(ui->disabled->count());
    foreach (auto lib, libs(ui->disabled)) {
        pushInj(lib);
    }

    pushInj(ui->enabled->count());
    foreach (auto lib, libs(ui->enabled)) {
        pushInj(lib);
    }

    fileInj->close();
}

void InjectGui::on_toolButton_clicked()
{
    QString exe = QFileDialog::getOpenFileName(this, tr("Select executable file"),
                                              "" , tr("*.exe"));
    if (QFile::exists(exe + ".inj")){
        if (fileInj == nullptr){
            fileInj = new QFile(exe + ".inj");
            fileInj->open(QIODevice::ReadOnly);
            if (!fileInj->isReadable())
                return;
            fileInj->reset();

            loadInj();

            fileInj->close();
            ui->statusBar->showMessage("Load from " + exe + ".inj");
        }
    }
    ui->exe->setText(exe);
}

void InjectGui::on_disabled_itemDoubleClicked(QListWidgetItem *item)
{
    ui->enabled->addItem(item->text());
    ui->disabled->removeItemWidget(item);
    delete item;
}

void InjectGui::on_enabled_itemDoubleClicked(QListWidgetItem *item)
{
    ui->disabled->addItem(item->text());
    ui->enabled->removeItemWidget(item);
    delete item;
}

void InjectGui::on_add_clicked()
{
    QString new_lib = QFileDialog::getOpenFileName(this, tr("Select library file"),
                                              "" , tr("*.dll"));
    if (isLibraryExist(new_lib)){
        ui->statusBar->showMessage("This library is already defined!");
        return;
    }
    ui->disabled->addItem(new_lib);
}

BOOL InjectGui::inject( DWORD pId, QString dll )
{
    HANDLE h = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pId );
    if ( h )
    {
        LPSTR dllName = (char*)codec->fromUnicode(dll).toStdString().c_str();
        LPVOID LoadLibAddr = (LPVOID)GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "LoadLibraryA" );
        LPVOID dereercomp = VirtualAllocEx( h, NULL, strlen( dllName ), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
        WriteProcessMemory( h, dereercomp, dllName, strlen( dllName ), NULL );
        HANDLE asdc = CreateRemoteThread( h, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, dereercomp, 0, NULL );
        WaitForSingleObject( asdc, INFINITE );
        VirtualFreeEx( h, dereercomp, strlen( dllName ), MEM_RELEASE );
        CloseHandle( asdc );
        CloseHandle( h );
        return TRUE;
    }
    return FALSE;
}

PROCESS_INFORMATION InjectGui::run()
{
    STARTUPINFOA cif;
    ZeroMemory( &cif, sizeof( STARTUPINFOA ) );
    PROCESS_INFORMATION pi;
    if(!CreateProcessA(codec->fromUnicode(ui->exe->text()).toStdString().c_str(),
                       (char*)codec->fromUnicode(ui->args->text()).toStdString().c_str(),
                       NULL,
                       NULL,
                       FALSE,
                       DETACHED_PROCESS | CREATE_SUSPENDED,
                       NULL,
                       NULL,
                       &cif,
                       &pi))
        ui->statusBar->showMessage("Failed to Create Process!");
    return pi;
}

void InjectGui::pushInj(QString data)
{
    uint len = data.length();
    fileInj->write((const char*)&len, 4);
    fileInj->write(codec->fromUnicode(data).toStdString().c_str(), len);
}

QString InjectGui::popInj()
{
    uint len;
    fileInj->read((char*)&len, 4);
    char *value = new char[len + 1];
    fileInj->read(value, len);
    value[len] = 0;
    QString ret = codec->toUnicode(value);
    delete[] value;
    return ret;
}

QList<QString> InjectGui::libs(QListWidget *list)
{
    QList<QString> libs;
    for (int i = 0; i < list->count() ; ++i){
        QModelIndex *model_index = new QModelIndex(list->model()->index(i,0) );
        QString lib = model_index->data(Qt::DisplayRole).toString();
        libs.push_back(lib);
    }
    return libs;
}

bool InjectGui::isLibraryExist(QString lib)
{
    foreach (auto library, libs(ui->disabled))
        if (library == lib)
            return true;
    foreach (auto library, libs(ui->enabled))
        if (library == lib)
            return true;
    return false;
}

void InjectGui::loadInj()
{
    ui->exe->setText(popInj());
    ui->args->setText(popInj());
    int libs = popInj<int>();
    for (int i = 0; i < libs; ++i)
        ui->disabled->addItem(popInj());
    libs = popInj<int>();
    for (int i = 0; i < libs; ++i)
        ui->enabled->addItem(popInj());
}

template<typename T>
void InjectGui::pushInj(T data)
{
    uint len = sizeof(data);
    fileInj->write((const char*)&len, 4);
    fileInj->write((const char*)&data, len);
}

template<typename T>
T InjectGui::popInj()
{
    uint len;
    fileInj->read((char*)&len, 4);
    T value;
    fileInj->read((char*)&value, len);
    return value;
}

void InjectGui::on_run_clicked()
{
    PROCESS_INFORMATION pi = run();
    if(pi.hProcess!=NULL){
        foreach (auto lib, libs(ui->enabled)) {
            if(!inject(pi.dwProcessId, lib)){
                TerminateProcess(pi.hProcess, 0);
                ui->statusBar->showMessage("Can't inject " + lib);
                return;
            }
        }
        ui->statusBar->showMessage("Process is runned!");
        ResumeThread(pi.hThread);
    }
}
