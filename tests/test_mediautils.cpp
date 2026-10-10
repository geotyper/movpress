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
    void parsesFrameRates();
    void convertsInclusiveFrameRange();
    void buildsInclusiveFrameFilter();
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

void MediaUtilsTest::parsesFrameRates()
{
    QCOMPARE(MediaUtils::parseFrameRate("30000/1001"), 30000.0 / 1001.0);
    QCOMPARE(MediaUtils::parseFrameRate("60"), 60.0);
    QCOMPARE(MediaUtils::parseFrameRate("0/0"), 0.0);
    QCOMPARE(MediaUtils::parseFrameRate("invalid"), 0.0);
}

void MediaUtilsTest::convertsInclusiveFrameRange()
{
    const auto range = MediaUtils::trimFromFrames(30, 59, 30.0, 10.0);
    QCOMPARE(range.start, 1.0);
    QCOMPARE(range.end, 2.0);

    const auto clamped = MediaUtils::trimFromFrames(299, 400, 30.0, 10.0);
    QVERIFY(qAbs(clamped.start - 299.0 / 30.0) < 0.000001);
    QCOMPARE(clamped.end, 10.0);
}

void MediaUtilsTest::buildsInclusiveFrameFilter()
{
    QCOMPARE(MediaUtils::frameTrimFilter(30, 59),
             QString("trim=start_frame=30:end_frame=60,setpts=PTS-STARTPTS"));
}

QTEST_APPLESS_MAIN(MediaUtilsTest)
#include "test_mediautils.moc"
