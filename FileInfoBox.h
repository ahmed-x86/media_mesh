#pragma once
#include <QDialog>

class FileInfoBox : public QDialog {
public: 
    static void showInfo(QWidget *parent, const QString &filePath);
};