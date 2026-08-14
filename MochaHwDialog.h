#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QMap>
#include "Constants.h"
#include "RippleButton.h"

class MochaHwDialog {
public:
    // دالة لاستخراج اسم المعالج الحقيقي من النظام
    static QString getCpuName() {
        QFile file("/proc/cpuinfo");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("model name")) {
                    return line.split(":").last().trimmed();
                }
            }
        }
        return "Generic CPU";
    }

    struct GpuInfo { 
        QString displayName; 
        QString apiName; 
    };

    // دالة لاستخراج أسماء كروت الشاشة الحقيقية المتصلة بالجهاز
    static QList<GpuInfo> getGpus() {
        QList<GpuInfo> gpus;
        QProcess p;
        // البحث عن الكروت الرسومية المتصلة بمنافذ PCI
        p.start("sh", {"-c", "lspci | grep -E 'VGA|3D'"});
        p.waitForFinished();
        QString output = p.readAllStandardOutput();
        
        for (QString line : output.split('\n', Qt::SkipEmptyParts)) {
            int idx = line.indexOf(": ");
            if (idx != -1) {
                QString fullName = line.mid(idx + 2).trimmed();
                // تنظيف الاسم من الأجزاء غير الهامة مثل (rev a2)
                int revIdx = fullName.lastIndexOf(" (rev");
                if (revIdx != -1) fullName = fullName.left(revIdx);

                // التحديد التلقائي لواجهة FFmpeg المناسبة بناءً على مصنع الكارت
                QString api = "vaapi"; // الافتراضي لـ Intel و AMD على لينكس
                if (fullName.contains("NVIDIA", Qt::CaseInsensitive)) {
                    api = "cuda";
                }
                
                gpus.append({"GPU: " + fullName, api});
            }
        }
        return gpus;
    }

    static QString getHwDevice(QWidget *parent = nullptr) {
        QDialog dlg(parent);
        dlg.setWindowTitle("Hardware Acceleration");
        dlg.setMinimumSize(550, 200);

        // تصميم البطاقات (Radio Buttons) لتكون أنيقة ومريحة للعين
        QString qss = QStringLiteral(R"(
            QDialog { background-color: %1; }
            QLabel#Title { color: %2; font-size: 16px; font-weight: bold; margin-bottom: 10px; }
            QRadioButton {
                background-color: %3; color: %2; border: 1px solid %4;
                border-radius: 8px; padding: 15px; font-size: 14px; font-weight: bold;
            }
            QRadioButton:hover { background-color: %4; }
            QRadioButton::indicator { width: 0px; height: 0px; } /* إخفاء الدائرة الافتراضية */
            QRadioButton:checked {
                background-color: %5; color: %3; border: 1px solid %5;
            }
            QPushButton { 
                background-color: %4; color: %2; border-radius: 8px; 
                padding: 10px 24px; font-weight: bold; font-size: 14px;
            }
            QPushButton:hover { background-color: %5; color: %3; }
        )").arg(Mocha::Base.name(), Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name(), Mocha::Accent.name());
        
        dlg.setStyleSheet(qss);

        auto *layout = new QVBoxLayout(&dlg);
        layout->setContentsMargins(25, 25, 25, 25);
        layout->setSpacing(15);
        
        auto *titleLbl = new QLabel("Select Hardware Accelerator:");
        titleLbl->setObjectName("Title");
        titleLbl->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLbl);

        QButtonGroup *btnGroup = new QButtonGroup(&dlg);
        QMap<int, QString> apiMap;
        int btnId = 0;

        // 1. إضافة المعالج (CPU) كخيار أول وبطاقة افتراضية
        QRadioButton *cpuRadio = new QRadioButton("CPU: " + getCpuName());
        cpuRadio->setChecked(true); // جعله الخيار الافتراضي
        btnGroup->addButton(cpuRadio, btnId);
        apiMap[btnId] = "CPU";
        layout->addWidget(cpuRadio);
        btnId++;

        // 2. إضافة كروت الشاشة (GPUs) ديناميكياً
        QList<GpuInfo> gpus = getGpus();
        for (const GpuInfo &gpu : gpus) {
            QRadioButton *gpuRadio = new QRadioButton(gpu.displayName);
            gpuRadio->setToolTip(QString("Uses FFmpeg API: %1").arg(gpu.apiName)); // Tooltip لمن يريد معرفة الـ API المستخدم
            btnGroup->addButton(gpuRadio, btnId);
            apiMap[btnId] = gpu.apiName;
            layout->addWidget(gpuRadio);
            btnId++;
        }
        
        layout->addStretch();
        
        auto *btnLayout = new QHBoxLayout();
        auto *btnCancel = new RippleButton("Cancel");
        auto *btnOk = new RippleButton("Start");
        
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnOk);
        layout->addLayout(btnLayout);

        QString selectedHw = "";
        
        QObject::connect(btnOk, &QPushButton::clicked, [&]() {
            selectedHw = apiMap[btnGroup->checkedId()]; // إرسال الـ API المناسب للبرنامج بناءً على التحديد
            dlg.accept();
        });
        
        QObject::connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);
        
        if (dlg.exec() == QDialog::Accepted) {
            return selectedHw;
        }
        
        return QString(""); 
    }
};