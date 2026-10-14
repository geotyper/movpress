#pragma once

#include "mediautils.h"

#include <QMainWindow>
#include <QProcess>
#include <QRect>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSlider;
class QSpinBox;
class QCloseEvent;
class QAudioOutput;
class QGraphicsScene;
class QGraphicsVideoItem;
class QMediaPlayer;
class QStackedWidget;
class CropOverlayItem;
class PreviewView;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void chooseInput();
    void chooseOutput();
    void updateOutputForFormat();
    void startConversion();
    void cancelConversion();
    void readProgress();
    void conversionFinished(int exitCode, QProcess::ExitStatus status);
    void conversionError(QProcess::ProcessError error);
    void probeFinished(int exitCode, QProcess::ExitStatus status);
    void applyProfile(int index);
    void syncTrimFromSliders();
    void syncSlidersFromTrim();
    void syncTrimFromFrames();
    void updateRangeMode();
    void openResult();
    void revealResult();
    void togglePlayback();
    void seekPreview(int value);
    void updatePlaybackPosition(qint64 position);
    void updatePlaybackDuration(qint64 duration);
    void updateCropControls();
    void normalizeCropControls();
    void applyCropFromOverlay(const QRect &rect);
    void applyTheme(int index);
    void changeLanguage(int index);
    void encoderProbeFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setInputFile(const QString &path);
    void probeInput();
    void setRunning(bool running);
    QString findTool(const QString &name) const;
    QString formattedTime(double seconds) const;
    MediaUtils::TrimRange selectedTrimRange() const;
    QStringList ffmpegArguments() const;
    bool publishConvertedFile();
    void loadSettings();
    void saveSettings() const;
    void updatePreviewRange(bool seekToStart = true);
    void updateEncoderControls();
    void detectEncoders();
    void selectEncoder(const QString &id);
    void fitPreview();
    void syncCropOverlay();
    QRect mapCropToSource(const QRect &rect) const;
    QRect mapCropToPreview(const QRect &rect) const;
    QString uiText(const char *english, const char *russian) const;

    QLineEdit *inputEdit_{};
    QLineEdit *outputEdit_{};
    QComboBox *formatCombo_{};
    QComboBox *profileCombo_{};
    QComboBox *themeCombo_{};
    QComboBox *languageCombo_{};
    QComboBox *widthCombo_{};
    QComboBox *fpsCombo_{};
    QComboBox *encoderCombo_{};
    QComboBox *presetCombo_{};
    QDoubleSpinBox *startSpin_{};
    QDoubleSpinBox *durationSpin_{};
    QComboBox *rangeModeCombo_{};
    QStackedWidget *trimStack_{};
    QSpinBox *startFrameSpin_{};
    QSpinBox *endFrameSpin_{};
    QSlider *startRange_{};
    QSlider *endRange_{};
    QLabel *rangeLabel_{};
    QLabel *mediaInfoLabel_{};
    QLabel *playbackTimeLabel_{};
    QPushButton *playButton_{};
    QSlider *playbackSlider_{};
    QMediaPlayer *mediaPlayer_{};
    QAudioOutput *audioOutput_{};
    PreviewView *videoView_{};
    QGraphicsScene *videoScene_{};
    QGraphicsVideoItem *videoItem_{};
    CropOverlayItem *cropItem_{};
    QComboBox *cropCombo_{};
    QSpinBox *cropX_{};
    QSpinBox *cropY_{};
    QSpinBox *cropWidth_{};
    QSpinBox *cropHeight_{};
    QSlider *crfSlider_{};
    QLabel *crfValue_{};
    QCheckBox *audioCheck_{};
    QProgressBar *progress_{};
    QLabel *statusLabel_{};
    QPushButton *startButton_{};
    QPushButton *inputButton_{};
    QPushButton *outputButton_{};
    QPushButton *cancelButton_{};
    QPushButton *openButton_{};
    QPushButton *revealButton_{};
    QWidget *settingsPane_{};
    QProcess process_;
    QProcess probeProcess_;
    QProcess encoderProbe_;
    QString preferredEncoder_{"libx264"};
    QByteArray progressBuffer_;
    double sourceDuration_{};
    double targetDuration_{};
    qint64 sourceSize_{};
    bool syncingTrim_{};
    bool syncingCrop_{};
    bool previewingOutput_{};
    bool resettingPlayback_{};
    int sourceWidth_{};
    int sourceHeight_{};
    double sourceFps_{};
    int totalFrames_{};
    QString probedPath_;
    QString activeOutputPath_;
    QString activeTempPath_;
    qint64 activeSourceSize_{};
    bool conversionCancelled_{};
    bool closeAfterProcess_{};
    bool russian_{};
};
