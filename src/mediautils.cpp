#include "mediautils.h"

#include <QtGlobal>

#include <cmath>

namespace MediaUtils {

TrimRange normalizedTrim(double start, double end, double totalDuration, double minimumDuration)
{
    if (totalDuration <= 0) return {};
    start = qBound(0.0, start, totalDuration);
    end = qBound(0.0, end, totalDuration);
    minimumDuration = qBound(0.0, minimumDuration, totalDuration);
    if (end - start < minimumDuration) {
        end = qMin(totalDuration, start + minimumDuration);
        if (end - start < minimumDuration) start = qMax(0.0, end - minimumDuration);
    }
    return {start, end};
}

CropRect normalizedCrop(int sourceWidth, int sourceHeight, CropRect requested)
{
    if (sourceWidth < 2 || sourceHeight < 2) return {};
    const int maximumX = qMax(0, (sourceWidth - 2) & ~1);
    const int maximumY = qMax(0, (sourceHeight - 2) & ~1);
    requested.x = qBound(0, requested.x & ~1, maximumX);
    requested.y = qBound(0, requested.y & ~1, maximumY);
    requested.width = qBound(2, requested.width & ~1, (sourceWidth - requested.x) & ~1);
    requested.height = qBound(2, requested.height & ~1, (sourceHeight - requested.y) & ~1);
    return requested;
}

QStringList videoFilters(int fps, int width, bool cropEnabled, CropRect crop)
{
    QStringList filters;
    if (cropEnabled && crop.width > 0 && crop.height > 0)
        filters << QString("crop=%1:%2:%3:%4").arg(crop.width).arg(crop.height).arg(crop.x).arg(crop.y);
    if (fps > 0) filters << "fps=" + QString::number(fps);
    if (width > 0) filters << "scale=" + QString::number(width) + ":-2:flags=lanczos";
    return filters;
}

double parseFrameRate(const QString &value)
{
    const QStringList parts = value.split('/');
    bool numeratorOk = false;
    bool denominatorOk = false;
    const double numerator = parts.value(0).toDouble(&numeratorOk);
    const double denominator = parts.size() > 1 ? parts.value(1).toDouble(&denominatorOk) : 1.0;
    if (parts.size() == 1) denominatorOk = true;
    if (!numeratorOk || !denominatorOk || numerator <= 0 || denominator <= 0) return 0.0;
    return numerator / denominator;
}

TrimRange trimFromFrames(int startFrame, int endFrame, double fps, double totalDuration)
{
    if (fps <= 0 || totalDuration <= 0) return {};
    const int totalFrames = qMax(1, static_cast<int>(std::floor(totalDuration * fps)));
    startFrame = qBound(0, startFrame, totalFrames - 1);
    endFrame = qBound(startFrame, endFrame, totalFrames - 1);
    return {startFrame / fps, qMin(totalDuration, (endFrame + 1) / fps)};
}

QString frameTrimFilter(int startFrame, int endFrame)
{
    startFrame = qMax(0, startFrame);
    endFrame = qMax(startFrame, endFrame);
    return QString("trim=start_frame=%1:end_frame=%2,setpts=PTS-STARTPTS")
        .arg(startFrame).arg(endFrame + 1);
}

int videoToolboxQuality(int crf)
{
    const int clamped = qBound(18, crf, 32);
    return qBound(1, 120 - clamped * 2, 100);
}

QStringList encoderArguments(const QString &encoderId, int crf, const QString &preset)
{
    const int quality = qBound(0, crf, 51);
    const QString id = encoderId.isEmpty() ? QString("libx264") : encoderId;
    if (id == "h264_videotoolbox") {
        return {"-c:v", "h264_videotoolbox", "-q:v", QString::number(videoToolboxQuality(quality)),
                "-profile:v", "high", "-pix_fmt", "yuv420p"};
    }
    if (id == "h264_nvenc") {
        return {"-c:v", "h264_nvenc", "-preset", "p4", "-rc", "vbr", "-cq", QString::number(quality),
                "-b:v", "0", "-pix_fmt", "yuv420p"};
    }
    if (id == "h264_qsv") {
        return {"-c:v", "h264_qsv", "-global_quality", QString::number(quality), "-pix_fmt", "nv12"};
    }
    if (id == "h264_amf") {
        return {"-c:v", "h264_amf", "-quality", "balanced", "-rc", "cqp",
                "-qp_i", QString::number(quality), "-qp_p", QString::number(quality)};
    }
    if (id == "h264_vaapi")
        return {"-c:v", "h264_vaapi", "-qp", QString::number(quality)};

    const QString speed = preset.isEmpty() ? QString("medium") : preset;
    return {"-c:v", "libx264", "-crf", QString::number(quality), "-preset", speed, "-pix_fmt", "yuv420p"};
}

QStringList vaapiUploadFilters()
{
    return {"format=nv12", "hwupload"};
}

}
