#include "mainwindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("MovPress");
    resize(650, 470);
    setAcceptDrops(true);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);

    auto *title = new QLabel("MovPress", central);
    QFont titleFont = title->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);
    layout->addWidget(new QLabel("Перетащите видео сюда или выберите файл", central));

    auto fileRow = [](QLineEdit *&edit, const QString &buttonText, QWidget *parent) {
        auto *row = new QHBoxLayout;
        edit = new QLineEdit(parent);
        auto *button = new QPushButton(buttonText, parent);
        row->addWidget(edit, 1);
        row->addWidget(button);
        return std::pair{row, button};
    };

    auto [inputRow, inputButton] = fileRow(inputEdit_, "Выбрать…", central);
    layout->addLayout(inputRow);

    auto *form = new QFormLayout;
    formatCombo_ = new QComboBox(central);
    formatCombo_->addItems({"MP4 (H.264)", "GIF"});
    form->addRow("Формат", formatCombo_);

    auto *trimRow = new QHBoxLayout;
    startSpin_ = new QDoubleSpinBox(central);
    durationSpin_ = new QDoubleSpinBox(central);
    for (auto *spin : {startSpin_, durationSpin_}) {
        spin->setRange(0.0, 86400.0);
        spin->setDecimals(1);
        spin->setSuffix(" с");
    }
    durationSpin_->setSpecialValueText("до конца");
    trimRow->addWidget(new QLabel("Начало", central));
    trimRow->addWidget(startSpin_);
    trimRow->addSpacing(16);
    trimRow->addWidget(new QLabel("Длительность", central));
    trimRow->addWidget(durationSpin_);
    form->addRow("Фрагмент", trimRow);

    widthCombo_ = new QComboBox(central);
    widthCombo_->addItem("Исходная", 0);
    for (int width : {1920, 1280, 960, 640})
        widthCombo_->addItem(QString::number(width) + " px", width);
    widthCombo_->setCurrentIndex(2);
    form->addRow("Ширина", widthCombo_);

    fpsCombo_ = new QComboBox(central);
    fpsCombo_->addItem("Исходная", 0);
    for (int fps : {60, 30, 24, 12})
        fpsCombo_->addItem(QString::number(fps), fps);
    fpsCombo_->setCurrentIndex(2);
    form->addRow("Кадров в секунду", fpsCombo_);

    auto *crfRow = new QHBoxLayout;
    crfSlider_ = new QSlider(Qt::Horizontal, central);
    crfSlider_->setRange(18, 32);
    crfSlider_->setValue(26);
    crfValue_ = new QLabel("26", central);
    crfRow->addWidget(crfSlider_);
    crfRow->addWidget(crfValue_);
    form->addRow("Качество CRF", crfRow);

    presetCombo_ = new QComboBox(central);
    presetCombo_->addItems({"medium", "slow", "slower", "fast", "veryfast"});
    presetCombo_->setCurrentText("slow");
    form->addRow("Скорость кодирования", presetCombo_);

    audioCheck_ = new QCheckBox("Оставить звук", central);
    form->addRow("Звук", audioCheck_);
    layout->addLayout(form);

    auto [outputRow, outputButton] = fileRow(outputEdit_, "Куда сохранить…", central);
    layout->addLayout(outputRow);

    progress_ = new QProgressBar(central);
    progress_->setRange(0, 0);
    progress_->setVisible(false);
    statusLabel_ = new QLabel("Готово к работе", central);
    layout->addWidget(progress_);
    layout->addWidget(statusLabel_);

    auto *actions = new QHBoxLayout;
    actions->addStretch();
    cancelButton_ = new QPushButton("Отмена", central);
    startButton_ = new QPushButton("Сжать видео", central);
    cancelButton_->setEnabled(false);
    startButton_->setDefault(true);
    actions->addWidget(cancelButton_);
    actions->addWidget(startButton_);
    layout->addLayout(actions);
    setCentralWidget(central);

    connect(inputButton, &QPushButton::clicked, this, &MainWindow::chooseInput);
    connect(outputButton, &QPushButton::clicked, this, &MainWindow::chooseOutput);
    connect(formatCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::updateOutputForFormat);
    connect(crfSlider_, &QSlider::valueChanged, crfValue_, [this](int value) { crfValue_->setText(QString::number(value)); });
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::startConversion);
    connect(cancelButton_, &QPushButton::clicked, this, &MainWindow::cancelConversion);
    connect(&process_, &QProcess::readyReadStandardOutput, this, &MainWindow::readProgress);
    connect(&process_, &QProcess::finished, this, &MainWindow::conversionFinished);
}

void MainWindow::chooseInput()
{
    const QString path = QFileDialog::getOpenFileName(this, "Выберите видео", {}, "Видео (*.mov *.mp4 *.mkv *.avi *.webm);;Все файлы (*)");
    if (!path.isEmpty()) setInputFile(path);
}

void MainWindow::chooseOutput()
{
    const QString extension = formatCombo_->currentIndex() == 0 ? "mp4" : "gif";
    const QString path = QFileDialog::getSaveFileName(this, "Сохранить результат", outputEdit_->text(),
                                                      extension.toUpper() + " (*." + extension + ")");
    if (!path.isEmpty()) outputEdit_->setText(path);
}

void MainWindow::setInputFile(const QString &path)
{
    inputEdit_->setText(path);
    updateOutputForFormat();
}

void MainWindow::updateOutputForFormat()
{
    if (inputEdit_->text().isEmpty()) return;
    QFileInfo info(inputEdit_->text());
    const QString extension = formatCombo_->currentIndex() == 0 ? "mp4" : "gif";
    outputEdit_->setText(info.dir().filePath(info.completeBaseName() + "-compressed." + extension));
    const bool mp4 = formatCombo_->currentIndex() == 0;
    crfSlider_->setEnabled(mp4);
    presetCombo_->setEnabled(mp4);
    audioCheck_->setEnabled(mp4);
}

QStringList MainWindow::ffmpegArguments() const
{
    QStringList args{"-hide_banner", "-y"};
    if (startSpin_->value() > 0) args << "-ss" << QString::number(startSpin_->value());
    if (durationSpin_->value() > 0) args << "-t" << QString::number(durationSpin_->value());
    args << "-i" << inputEdit_->text();

    const int width = widthCombo_->currentData().toInt();
    const int fps = fpsCombo_->currentData().toInt();
    QStringList filters;
    if (fps > 0) filters << "fps=" + QString::number(fps);
    if (width > 0) filters << "scale=" + QString::number(width) + ":-2:flags=lanczos";

    if (formatCombo_->currentIndex() == 0) {
        if (!filters.isEmpty()) args << "-vf" << filters.join(',');
        args << "-c:v" << "libx264" << "-crf" << QString::number(crfSlider_->value())
             << "-preset" << presetCombo_->currentText() << "-pix_fmt" << "yuv420p";
        if (!audioCheck_->isChecked()) args << "-an";
        else args << "-c:a" << "aac" << "-b:a" << "128k";
        args << "-movflags" << "+faststart";
    } else {
        const QString base = filters.isEmpty() ? "null" : filters.join(',');
        args << "-filter_complex" << base + ",split[a][b];[a]palettegen[p];[b][p]paletteuse" << "-an";
    }
    args << "-progress" << "pipe:1" << "-nostats" << outputEdit_->text();
    return args;
}

void MainWindow::startConversion()
{
    if (!QFileInfo::exists(inputEdit_->text())) {
        QMessageBox::warning(this, "MovPress", "Выберите существующий входной файл.");
        return;
    }
    if (outputEdit_->text().isEmpty() || QFileInfo(outputEdit_->text()).absoluteFilePath() == QFileInfo(inputEdit_->text()).absoluteFilePath()) {
        QMessageBox::warning(this, "MovPress", "Укажите другой путь для результата.");
        return;
    }
    QString ffmpeg = QStandardPaths::findExecutable("ffmpeg");
    if (ffmpeg.isEmpty() && QFileInfo::exists("/opt/homebrew/bin/ffmpeg"))
        ffmpeg = "/opt/homebrew/bin/ffmpeg";
    if (ffmpeg.isEmpty() && QFileInfo::exists("/usr/local/bin/ffmpeg"))
        ffmpeg = "/usr/local/bin/ffmpeg";
    if (ffmpeg.isEmpty()) {
        QMessageBox::critical(this, "MovPress", "ffmpeg не найден в PATH.");
        return;
    }
    setRunning(true);
    statusLabel_->setText("Конвертация…");
    process_.start(ffmpeg, ffmpegArguments());
}

void MainWindow::cancelConversion()
{
    process_.terminate();
    if (!process_.waitForFinished(1500)) process_.kill();
}

void MainWindow::readProgress()
{
    const QString data = QString::fromUtf8(process_.readAllStandardOutput());
    if (data.contains("progress=end")) statusLabel_->setText("Завершение…");
}

void MainWindow::conversionFinished(int exitCode, QProcess::ExitStatus status)
{
    setRunning(false);
    if (status == QProcess::NormalExit && exitCode == 0) {
        const auto size = QFileInfo(outputEdit_->text()).size();
        statusLabel_->setText(QString("Готово — %1 МБ").arg(size / 1048576.0, 0, 'f', 1));
    } else {
        statusLabel_->setText("Ошибка конвертации");
        QMessageBox::critical(this, "MovPress", QString::fromUtf8(process_.readAllStandardError()).right(3000));
    }
}

void MainWindow::setRunning(bool running)
{
    startButton_->setEnabled(!running);
    cancelButton_->setEnabled(running);
    progress_->setVisible(running);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty() && urls.first().isLocalFile()) setInputFile(urls.first().toLocalFile());
}
