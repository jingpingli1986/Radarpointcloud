#include "FrameRangeDialog.h"
#include <QSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLineEdit>

FrameRangeDialog::FrameRangeDialog(int maxFrame, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("选择帧范围和保存文件");

    // 起始帧
    QLabel *labelStart = new QLabel("起始帧:", this);
    spinStart = new QSpinBox(this);
    spinStart->setRange(0, maxFrame);
    spinStart->setValue(0);

    setMinimumSize(500, 200);
    // 结束帧
    QLabel *labelEnd = new QLabel("结束帧:", this);
    spinEnd = new QSpinBox(this);
    spinEnd->setRange(0, maxFrame);
    spinEnd->setValue(maxFrame);

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addWidget(labelStart);
    hlayout1->addWidget(spinStart);

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(labelEnd);
    hlayout2->addWidget(spinEnd);

    // 文件选择
    QLabel *labelFile = new QLabel("保存路径:", this);
    lineEditFile = new QLineEdit(this);
    lineEditFile->setReadOnly(true);
    QPushButton *btnChoose = new QPushButton("选择文件...", this);
    connect(btnChoose, &QPushButton::clicked, this, &FrameRangeDialog::chooseFile);

    QHBoxLayout *hlayoutFile = new QHBoxLayout;
    hlayoutFile->addWidget(labelFile);
    hlayoutFile->addWidget(lineEditFile);
    hlayoutFile->addWidget(btnChoose);

    // 按钮
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (spinStart->value() <= spinEnd->value() && !lineEditFile->text().isEmpty()) {
            accept();
        } else {
            spinStart->setStyleSheet("background-color: #ffdddd;");
            spinEnd->setStyleSheet("background-color: #ffdddd;");
            if (lineEditFile->text().isEmpty()) {
                lineEditFile->setStyleSheet("background-color: #ffdddd;");
            }
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(hlayout1);
    mainLayout->addLayout(hlayout2);
    mainLayout->addLayout(hlayoutFile);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void FrameRangeDialog::chooseFile()
{
    QString file = QFileDialog::getSaveFileName(this, "选择保存路径", "", "Bin Files (*.bin)");
    if (!file.isEmpty()) {
        lineEditFile->setText(file);
        lineEditFile->setStyleSheet(""); // 重置高亮
    }
}

int FrameRangeDialog::startFrame() const
{
    return spinStart->value();
}

int FrameRangeDialog::endFrame() const
{
    return spinEnd->value();
}

QString FrameRangeDialog::saveFilePath() const
{
    return lineEditFile->text();
}
