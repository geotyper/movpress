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
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QStandardPaths>
#include <QSignalBlocker>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("MovPress");
    resize(700, 640);
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
    profileCombo_ = new QComboBox(central);
    profileCombo_->addItems({"Свои настройки", "YouTube", "Хабр GIF", "Telegram", "Максимальное сжатие"});
    form->addRow("Профиль", profileCombo_);

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

    auto *rangeBox = new QVBoxLayout;
    startRange_ = new QSlider(Qt::Horizontal, central);
    endRange_ = new QSlider(Qt::Horizontal, central);
    rangeLabel_ = new QLabel("Выберите видео для настройки диапазона", central);
    startRange_->setEnabled(false);
    endRange_->setEnabled(false);
    rangeBox->addWidget(new QLabel("Начало", central));
    rangeBox->addWidget(startRange_);
    rangeBox->addWidget(new QLabel("Конец", central));
    rangeBox->addWidget(endRange_);
    rangeBox->addWidget(rangeLabel_);
    form->addRow("Диапазон", rangeBox);

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

    mediaInfoLabel_ = new QLabel("", central);
    layout->addWidget(mediaInfoLabel_);

    progress_ = new QProgressBar(central);
    progress_->setRange(0, 0);
    progress_->setVisible(false);
    statusLabel_ = new QLabel("Готово к работе", central);
    layout->addWidget(progress_);
    layout->addWidget(statusLabel_);

    auto *actions = new QHBoxLayout;
    actions->addStretch();
    cancelButton_ = new QPushButton("Отмена", central);
    openButton_ = new QPushButton("Открыть файл", central);
    revealButton_ = new QPushButton("Показать в Finder", central);
    startButton_ = new QPushButton("Сжать видео", central);
    cancelButton_->setEnabled(false);
    openButton_->setEnabled(false);
    revealButton_->setEnabled(false);
    startButton_->setDefault(true);
    actions->addWidget(cancelButton_);
    actions->addWidget(openButton_);
    actions->addWidget(revealButton_);
    actions->addWidget(startButton_);
    layout->addLayout(actions);
    setCentralWidget(central);

    connect(inputButton, &QPushButton::clicked, this, &MainWindow::chooseInput);
    connect(outputButton, &QPushButton::clicked, this, &MainWindow::chooseOutput);
    connect(formatCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::updateOutputForFormat);
    connect(profileCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::applyProfile);
    connect(crfSlider_, &QSlider::valueChanged, crfValue_, [this](int value) { crfValue_->setText(QString::number(value)); });
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::startConversion);
    connect(cancelButton_, &QPushButton::clicked, this, &MainWindow::cancelConversion);
    connect(openButton_, &QPushButton::clicked, this, &MainWindow::openResult);
    connect(revealButton_, &QPushButton::clicked, this, &MainWindow::revealResult);
    connect(startRange_, &QSlider::valueChanged, this, &MainWindow::syncTrimFromSliders);
    connect(endRange_, &QSlider::valueChanged, this, &MainWindow::syncTrimFromSliders);
    connect(startSpin_, &QDoubleSpinBox::valueChanged, this, &MainWindow::syncSlidersFromTrim);
    connect(durationSpin_, &QDoubleSpinBox::valueChanged, this, &MainWindow::syncSlidersFromTrim);
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
    probeInput();
}

QString MainWindow::findTool(const QString &name) const
{
    QString path = QStandardPaths::findExecutable(name);
    for (const QString &prefix : {QString("/opt/homebrew/bin/"), QString("/usr/local/bin/")})
        if (path.isEmpty() && QFileInfo::exists(prefix + name)) path = prefix + name;
    return path;
}

QString MainWindow::formattedTime(double seconds) const
{
    const int total = qMax(0, qRound(seconds));
    return QString("%1:%2").arg(total / 60).arg(total % 60, 2, 10, QLatin1Char('0'));
}

void MainWindow::probeInput()
{
    sourceDuration_ = 0;
    sourceSize_ = QFileInfo(inputEdit_->text()).size();
    const QString ffprobe = findTool("ffprobe");
    if (ffprobe.isEmpty()) {
        mediaInfoLabel_->setText("Не удалось найти ffprobe");
        return;
    }

    QProcess probe;
    probe.start(ffprobe, {"-v", "quiet", "-print_format", "json", "-show_format", inputEdit_->text()});
    if (!probe.waitForFinished(5000)) {
        probe.kill();
        mediaInfoLabel_->setText("Не удалось прочитать параметры видео");
        return;
    }
    const auto document = QJsonDocument::fromJson(probe.readAllStandardOutput());
    sourceDuration_ = document.object().value("format").toObject().value("duration").toString().toDouble();
    if (sourceDuration_ <= 0) {
        mediaInfoLabel_->setText("Длительность видео не определена");
        return;
    }
    const int maximum = qMax(1, qCeil(sourceDuration_));
    for (auto *slider : {startRange_, endRange_}) {
        slider->setRange(0, maximum);
        slider->setEnabled(true);
    }
    startRange_->setValue(0);
    endRange_->setValue(maximum);
    startSpin_->setMaximum(sourceDuration_);
    durationSpin_->setMaximum(sourceDuration_);
    startSpin_->setValue(0);
    durationSpin_->setValue(0);
    mediaInfoLabel_->setText(QString("Исходник: %1 · %2 МБ")
                                 .arg(formattedTime(sourceDuration_))
                                 .arg(sourceSize_ / 1048576.0, 0, 'f', 1));
    syncTrimFromSliders();
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

void MainWindow::applyProfile(int index)
{
    if (index == 0) return;
    const QSignalBlocker formatBlock(formatCombo_);
    if (index == 1) { // YouTube
        formatCombo_->setCurrentIndex(0);
        widthCombo_->setCurrentIndex(widthCombo_->findData(1920));
        fpsCombo_->setCurrentIndex(fpsCombo_->findData(30));
        crfSlider_->setValue(23);
        presetCombo_->setCurrentText("slow");
        audioCheck_->setChecked(true);
    } else if (index == 2) { // Habr GIF
        formatCombo_->setCurrentIndex(1);
        widthCombo_->setCurrentIndex(widthCombo_->findData(640));
        fpsCombo_->setCurrentIndex(fpsCombo_->findData(12));
        audioCheck_->setChecked(false);
    } else if (index == 3) { // Telegram
        formatCombo_->setCurrentIndex(0);
        widthCombo_->setCurrentIndex(widthCombo_->findData(1280));
        fpsCombo_->setCurrentIndex(fpsCombo_->findData(30));
        crfSlider_->setValue(26);
        presetCombo_->setCurrentText("medium");
        audioCheck_->setChecked(true);
    } else { // Maximum compression
        formatCombo_->setCurrentIndex(0);
        widthCombo_->setCurrentIndex(widthCombo_->findData(960));
        fpsCombo_->setCurrentIndex(fpsCombo_->findData(24));
        crfSlider_->setValue(30);
        presetCombo_->setCurrentText("slow");
        audioCheck_->setChecked(false);
    }
    updateOutputForFormat();
}

void MainWindow::syncTrimFromSliders()
{
    if (syncingTrim_ || sourceDuration_ <= 0) return;
    syncingTrim_ = true;
    if (startRange_->value() > endRange_->value())
        endRange_->setValue(startRange_->value());
    const double start = startRange_->value();
    const double end = qMin<double>(endRange_->value(), sourceDuration_);
    startSpin_->setValue(start);
    durationSpin_->setValue(end >= sourceDuration_ - 0.5 ? 0 : qMax(0.0, end - start));
    rangeLabel_->setText(QString("%1 — %2 · фрагмент %3")
                             .arg(formattedTime(start), formattedTime(end), formattedTime(end - start)));
    syncingTrim_ = false;
}

void MainWindow::syncSlidersFromTrim()
{
    if (syncingTrim_ || sourceDuration_ <= 0) return;
    syncingTrim_ = true;
    const double start = qMin(startSpin_->value(), sourceDuration_);
    const double end = durationSpin_->value() <= 0
                           ? sourceDuration_
                           : qMin(sourceDuration_, start + durationSpin_->value());
    startRange_->setValue(qRound(start));
    endRange_->setValue(qRound(end));
    rangeLabel_->setText(QString("%1 — %2 · фрагмент %3")
                             .arg(formattedTime(start), formattedTime(end), formattedTime(end - start)));
    syncingTrim_ = false;
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
    const QString ffmpeg = findTool("ffmpeg");
    if (ffmpeg.isEmpty()) {
        QMessageBox::critical(this, "MovPress", "ffmpeg не найден в PATH.");
        return;
    }
    setRunning(true);
    progressBuffer_.clear();
    targetDuration_ = durationSpin_->value() > 0
                          ? durationSpin_->value()
                          : qMax(0.0, sourceDuration_ - startSpin_->value());
    progress_->setRange(targetDuration_ > 0 ? 0 : 0, targetDuration_ > 0 ? 1000 : 0);
    progress_->setValue(0);
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
    progressBuffer_ += process_.readAllStandardOutput();
    const QList<QByteArray> lines = progressBuffer_.split('\n');
    progressBuffer_ = lines.constLast();
    for (const QByteArray &line : lines) {
        if (line.startsWith("out_time_us=") && targetDuration_ > 0) {
            const double seconds = line.mid(12).toDouble() / 1000000.0;
            const int value = qBound(0, qRound(seconds / targetDuration_ * 1000.0), 1000);
            progress_->setValue(value);
            statusLabel_->setText(QString("Конвертация — %1%").arg(value / 10));
        } else if (line == "progress=end") {
            statusLabel_->setText("Завершение…");
        }
    }
}

void MainWindow::conversionFinished(int exitCode, QProcess::ExitStatus status)
{
    setRunning(false);
    if (status == QProcess::NormalExit && exitCode == 0) {
        const auto size = QFileInfo(outputEdit_->text()).size();
        const double saved = sourceSize_ > 0 ? (1.0 - static_cast<double>(size) / sourceSize_) * 100.0 : 0.0;
        statusLabel_->setText(QString("Готово: %1 МБ → %2 МБ · экономия %3%")
                                  .arg(sourceSize_ / 1048576.0, 0, 'f', 1)
                                  .arg(size / 1048576.0, 0, 'f', 1)
                                  .arg(saved, 0, 'f', 0));
        openButton_->setEnabled(true);
        revealButton_->setEnabled(true);
    } else {
        statusLabel_->setText("Ошибка конвертации");
        QMessageBox::critical(this, "MovPress", QString::fromUtf8(process_.readAllStandardError()).right(3000));
    }
}

void MainWindow::openResult()
{
    if (QFileInfo::exists(outputEdit_->text()))
        QDesktopServices::openUrl(QUrl::fromLocalFile(outputEdit_->text()));
}

void MainWindow::revealResult()
{
    if (QFileInfo::exists(outputEdit_->text()))
        QProcess::startDetached("open", {"-R", outputEdit_->text()});
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
