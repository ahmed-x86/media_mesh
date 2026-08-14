#include <QApplication>
#include <QFileInfo>
#include "HomeWindow.h"
#include "ModernConverterWindow.h"
#include "MochaMsgBox.h"
#include "MochaHwDialog.h"
#include "FileInfoBox.h"

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORMTHEME", "xdgdesktopportal");

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, true);

    QString profile = "";
    QString inputFile = "";

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg.startsWith("-")) {
            profile = arg.mid(1).toLower();
        } else {
            inputFile = arg;
        }
    }

    if (!inputFile.isEmpty() && !profile.isEmpty()) {
        if (!QFileInfo::exists(inputFile)) {
            MochaMsgBox::showMsg(nullptr, "Error", "The provided input file does not exist.\n\n" + inputFile, true);
            return 1;
        }
        
        if (profile == "info") {
            FileInfoBox::showInfo(nullptr, inputFile);
            return 0;
        }

        // استدعاء نافذة اختيار العتاد قبل التحويل
        QString selectedHw = MochaHwDialog::getHwDevice();
        
        // إذا قام المستخدم بإلغاء النافذة يتم إغلاق البرنامج
        if (selectedHw.isEmpty()) {
            return 0;
        }

        // تمرير العتاد الذي اختاره المستخدم إلى نافذة التحويل
        ModernConverterWindow window(profile, inputFile, selectedHw);
        window.show();
        return app.exec();
    }

    HomeWindow home;
    home.show();

    return app.exec();
}