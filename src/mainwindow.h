#pragma once

#include <QMainWindow>
#include <QProcess>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSlider;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void chooseInput();
    void chooseOutput();
    void updateOutputForFormat();
    void startConversion();
    void cancelConversion();
    void readProgress();
    void conversionFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setInputFile(const QString &path);
    void setRunning(bool running);
    QStringList ffmpegArguments() const;

    QLineEdit *inputEdit_{};
    QLineEdit *outputEdit_{};
    QComboBox *formatCombo_{};
    QComboBox *widthCombo_{};
    QComboBox *fpsCombo_{};
    QComboBox *presetCombo_{};
    QDoubleSpinBox *startSpin_{};
    QDoubleSpinBox *durationSpin_{};
    QSlider *crfSlider_{};
    QLabel *crfValue_{};
    QCheckBox *audioCheck_{};
    QProgressBar *progress_{};
    QLabel *statusLabel_{};
    QPushButton *startButton_{};
    QPushButton *cancelButton_{};
    QProcess process_;
};
