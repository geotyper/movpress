#include "mediautils.h"

#include <QtGlobal>

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

}
