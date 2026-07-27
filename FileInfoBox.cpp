#include "FileInfoBox.h"
#include "Constants.h"
#include "RippleButton.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QFileInfo>
#include <QProcess>

void FileInfoBox::showInfo(QWidget *parent, const QString &filePath) {
    QDialog dlg(parent);
    dlg.setWindowTitle("File Information");
    dlg.setMinimumSize(500, 400);

    QString qss = QStringLiteral(R"(
        QDialog { background-color: %1; }
        QLabel { color: %2; font-size: 14px; font-weight: bold; }
        QTextEdit {
            background-color: %3; color: %2; border-radius: 8px;
            padding: 10px; font-size: 13px; border: 1px solid %4;
        }
        QPushButton { 
            background-color: %3; color: %2; border-radius: 8px; 
            padding: 8px 24px; font-weight: bold; font-size: 14px;
        }
        QPushButton:hover { background-color: %5; }
    )").arg(Mocha::Base.name(), Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name(), Mocha::Accent.name());
    
    dlg.setStyleSheet(qss);

    auto *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QFileInfo fi(filePath);
    QString sizeStr = QString::number(fi.size() / (1024.0 * 1024.0), 'f', 2) + " MB";
    
    QString basicInfo = QString("File Name: %1\nSize: %2\nCreated: %3")
                            .arg(fi.fileName(), sizeStr, fi.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
    
    auto *lbl = new QLabel(basicInfo);
    layout->addWidget(lbl);

    QProcess ffprobe;
    ffprobe.start("ffprobe", {"-hide_banner", filePath});
    ffprobe.waitForFinished();
    QString probeInfo = ffprobe.readAllStandardError(); 

    auto *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setText(probeInfo.isEmpty() ? "No additional media info available." : probeInfo);
    layout->addWidget(textEdit);
    
    auto *btnOk = new RippleButton("Close");
    QObject::connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
    
    layout->addWidget(btnOk, 0, Qt::AlignCenter);
    
    dlg.exec();
}