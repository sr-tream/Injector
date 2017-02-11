#include "injectgui.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFileInfo file;
    for (int i = 1; i < argc; ++i){
        QTextCodec *codec = QTextCodec::codecForName("cp1251");
        file = QFileInfo(codec->toUnicode(argv[i]));
        if (file.isFile())
            if (file.suffix() == "inj")
                break;
    }
    InjectGui w(argv[0], file);
    w.show();

    return a.exec();
}
