#include "MdfExtractDialog.h"
#include "ui_MdfExtractDialog.h"
#include <QFileDialog>

MdfExtractDialog::MdfExtractDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MdfExtractDialog)
{
    ui->setupUi(this);

    // 默认勾选“全部转换”，禁用时间选择
    ui->checkAllData->setChecked(true);
    ui->labelStartTime->setEnabled(false);
    ui->labelEndTime->setEnabled(false);
}

MdfExtractDialog::~MdfExtractDialog()
{
    delete ui;
}

QString MdfExtractDialog::inputFile() const
{
    return ui->inputFileEdit->text();
}

QString MdfExtractDialog::outputFile() const
{
    return ui->outputFileEdit->text();
}

double MdfExtractDialog::startTime() const
{
    return ui->startTimeSpin->value();
}

double MdfExtractDialog::endTime() const
{
    return ui->endTimeSpin->value();
}

bool MdfExtractDialog::isAllData() const
{
    return ui->checkAllData->isChecked();
}

void MdfExtractDialog::on_selectInputFileButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("选择 MF4 文件"),
        QString(),
        tr("MF4 文件 (*.mf4)")
        );
    if (!file.isEmpty()) {
        ui->inputFileEdit->setText(file);
    }
}

void MdfExtractDialog::on_selectOutputFileButton_clicked()
{
    QString file = QFileDialog::getSaveFileName(
        this,
        tr("保存输出 bin 文件"),
        QString(),
        tr("BIN 文件 (*.bin)")
        );
    if (!file.isEmpty()) {
        ui->outputFileEdit->setText(file);
    }
}

void MdfExtractDialog::on_checkAllData_toggled(bool checked)
{
    // 如果勾选了“全部转换”，禁用时间选择
    ui->startTimeSpin->setEnabled(!checked);
    ui->endTimeSpin->setEnabled(!checked);
}
