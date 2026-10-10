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
    void applyProfile(int index);
    void syncTrimFromSliders();
    void syncSlidersFromTrim();
    void openResult();
    void revealResult();

private:
    void setInputFile(const QString &path);
    void probeInput();
    void setRunning(bool running);
    QString findTool(const QString &name) const;
    QString formattedTime(double seconds) const;
    QStringList ffmpegArguments() const;

    QLineEdit *inputEdit_{};
    QLineEdit *outputEdit_{};
    QComboBox *formatCombo_{};
    QComboBox *profileCombo_{};
    QComboBox *widthCombo_{};
    QComboBox *fpsCombo_{};
    QComboBox *presetCombo_{};
    QDoubleSpinBox *startSpin_{};
    QDoubleSpinBox *durationSpin_{};
    QSlider *startRange_{};
    QSlider *endRange_{};
    QLabel *rangeLabel_{};
    QLabel *mediaInfoLabel_{};
    QSlider *crfSlider_{};
    QLabel *crfValue_{};
    QCheckBox *audioCheck_{};
    QProgressBar *progress_{};
    QLabel *statusLabel_{};
    QPushButton *startButton_{};
    QPushButton *cancelButton_{};
    QPushButton *openButton_{};
    QPushButton *revealButton_{};
    QProcess process_;
    QByteArray progressBuffer_;
    double sourceDuration_{};
    double targetDuration_{};
    qint64 sourceSize_{};
    bool syncingTrim_{};
};
