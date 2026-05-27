#ifndef FRAMERANGEDIALOG_H
#define FRAMERANGEDIALOG_H

#include <QDialog>

class QSpinBox;
class QLineEdit;

class FrameRangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FrameRangeDialog(int maxFrame, QWidget *parent = nullptr);

    int startFrame() const;
    int endFrame() const;
    QString saveFilePath() const;

private slots:
    void chooseFile();

private:
    QSpinBox *spinStart;
    QSpinBox *spinEnd;
    QLineEdit *lineEditFile;
};

#endif // FRAMERANGEDIALOG_H
