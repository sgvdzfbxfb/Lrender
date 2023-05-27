/********************************************************************************
** Form generated from reading UI file 'LRenderWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LRENDERWIDGET_H
#define UI_LRENDERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LRenderWidget
{
public:
    QLabel *FPSLabel;

    void setupUi(QWidget *LRenderWidget)
    {
        if (LRenderWidget->objectName().isEmpty())
            LRenderWidget->setObjectName(QString::fromUtf8("LRenderWidget"));
        LRenderWidget->resize(800, 600);
        LRenderWidget->setStyleSheet(QString::fromUtf8(""));
        FPSLabel = new QLabel(LRenderWidget);
        FPSLabel->setObjectName(QString::fromUtf8("FPSLabel"));
        FPSLabel->setGeometry(QRect(20, 25, 150, 20));
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        FPSLabel->setFont(font);

        retranslateUi(LRenderWidget);

        QMetaObject::connectSlotsByName(LRenderWidget);
    } // setupUi

    void retranslateUi(QWidget *LRenderWidget)
    {
        LRenderWidget->setWindowTitle(QCoreApplication::translate("LRenderWidget", "LRenderWidget", nullptr));
        FPSLabel->setText(QCoreApplication::translate("LRenderWidget", "FPS : ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LRenderWidget: public Ui_LRenderWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LRENDERWIDGET_H
