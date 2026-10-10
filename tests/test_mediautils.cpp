#include "mediautils.h"

#include <QtTest>

class MediaUtilsTest final : public QObject
{
    Q_OBJECT

private slots:
    void keepsValidTrim();
    void preventsEmptyTrim();
    void clampsTrimAtEnd();
    void clampsAndAlignsCrop();
    void alignsCropForOddSourceSize();
    void buildsFiltersInOrder();
};

void MediaUtilsTest::keepsValidTrim()
{
    const auto range = MediaUtils::normalizedTrim(3.5, 8.5, 10.0);
    QCOMPARE(range.start, 3.5);
    QCOMPARE(range.end, 8.5);
}

void MediaUtilsTest::preventsEmptyTrim()
{
    const auto range = MediaUtils::normalizedTrim(4.0, 4.0, 10.0, 1.0);
    QCOMPARE(range.start, 4.0);
    QCOMPARE(range.end, 5.0);
}

void MediaUtilsTest::clampsTrimAtEnd()
{
    const auto range = MediaUtils::normalizedTrim(10.0, 10.0, 10.0, 1.0);
    QCOMPARE(range.start, 9.0);
    QCOMPARE(range.end, 10.0);
}

void MediaUtilsTest::clampsAndAlignsCrop()
{
    const auto crop = MediaUtils::normalizedCrop(778, 774, {777, 773, 500, 500});
    QCOMPARE(crop.x, 776);
    QCOMPARE(crop.y, 772);
    QCOMPARE(crop.width, 2);
    QCOMPARE(crop.height, 2);
}

void MediaUtilsTest::alignsCropForOddSourceSize()
{
    const auto crop = MediaUtils::normalizedCrop(779, 775, {900, 900, 500, 500});
    QCOMPARE(crop.x, 776);
    QCOMPARE(crop.y, 772);
    QCOMPARE(crop.width, 2);
    QCOMPARE(crop.height, 2);
}

void MediaUtilsTest::buildsFiltersInOrder()
{
    const auto filters = MediaUtils::videoFilters(30, 1280, true, {2, 4, 640, 480});
    QCOMPARE(filters, QStringList({"crop=640:480:2:4", "fps=30", "scale=1280:-2:flags=lanczos"}));
}

QTEST_APPLESS_MAIN(MediaUtilsTest)
#include "test_mediautils.moc"
