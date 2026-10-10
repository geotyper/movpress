#pragma once

#include <QStringList>

namespace MediaUtils {

struct TrimRange {
    double start{};
    double end{};
};

struct CropRect {
    int x{};
    int y{};
    int width{};
    int height{};
};

TrimRange normalizedTrim(double start, double end, double totalDuration, double minimumDuration = 0.1);
CropRect normalizedCrop(int sourceWidth, int sourceHeight, CropRect requested);
QStringList videoFilters(int fps, int width, bool cropEnabled, CropRect crop);
double parseFrameRate(const QString &value);
TrimRange trimFromFrames(int startFrame, int endFrame, double fps, double totalDuration);
QString frameTrimFilter(int startFrame, int endFrame);

}
