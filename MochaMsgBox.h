#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include "Constants.h"
#include "RippleButton.h"

class MochaMsgBox {
public:
    static void showMsg(QWidget *parent, const QString &title, const QString &text, bool isError = false) {
        QDialog dlg(parent);
        dlg.setWindowTitle(title);
        dlg.setMinimumSize(400, 150);
        
        QString qss = QStringLiteral(R"(
            QDialog { background-color: %1; }
            QLabel { color: %2; font-size: 14px; font-weight: bold; }
            QPushButton { 
                background-color: %3; color: %2; border-radius: 8px; 
                padding: 8px 24px; font-weight: bold; font-size: 14px;
            }
            QPushButton:hover { background-color: %4; }
        )").arg(Mocha::Base.name(), Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name());
        
        dlg.setStyleSheet(qss);

        auto *layout = new QVBoxLayout(&dlg);
        layout->setContentsMargins(20, 20, 20, 20);
        
        auto *lbl = new QLabel(text);
        lbl->setWordWrap(true);
        lbl->setAlignment(Qt::AlignCenter);
        
        if (isError) {
            lbl->setStyleSheet(QString("color: %1;").arg(Mocha::Red.name()));
        } else {
            lbl->setStyleSheet(QString("color: %1;").arg(Mocha::Green.name()));
        }
        
        auto *btnOk = new RippleButton("OK");
        QObject::connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
        
        layout->addWidget(lbl);
        layout->addStretch();
        layout->addWidget(btnOk, 0, Qt::AlignCenter);
        
        dlg.exec();
    }
};