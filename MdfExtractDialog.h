#ifndef MDFEXTRACTDIALOG_H
#define MDFEXTRACTDIALOG_H

#include <QDialog>

namespace Ui {
class MdfExtractDialog;
}

class MdfExtractDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MdfExtractDialog(QWidget *parent = nullptr);
    ~MdfExtractDialog();

    QString inputFile() const;
    QString outputFile() const;
    double startTime() const;
    double endTime() const;
    bool isAllData() const;

private slots:
    void on_selectInputFileButton_clicked();
    void on_selectOutputFileButton_clicked();
    void on_checkAllData_toggled(bool checked);

private:
    Ui::MdfExtractDialog *ui;
};

#endif // MDFEXTRACTDIALOG_H
