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
    void mapsHardwareQuality();
    void buildsEncoderArguments();
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

void MediaUtilsTest::mapsHardwareQuality()
{
    QCOMPARE(MediaUtils::videoToolboxQuality(18), 84);
    QCOMPARE(MediaUtils::videoToolboxQuality(26), 68);
    QCOMPARE(MediaUtils::videoToolboxQuality(32), 56);
    QCOMPARE(MediaUtils::vaapiUploadFilters(), QStringList({"format=nv12", "hwupload"}));
}

void MediaUtilsTest::buildsEncoderArguments()
{
    const auto software = MediaUtils::encoderArguments("libx264", 23, "slow");
    QCOMPARE(software, QStringList({"-c:v", "libx264", "-crf", "23", "-preset", "slow", "-pix_fmt", "yuv420p"}));

    const auto hardware = MediaUtils::encoderArguments("h264_videotoolbox", 26, "slow");
    QCOMPARE(hardware.at(1), "h264_videotoolbox");
    QCOMPARE(hardware.at(3), QString::number(MediaUtils::videoToolboxQuality(26)));
    QVERIFY(!hardware.contains("slow"));

    const auto fallback = MediaUtils::encoderArguments("missing", 23, "medium");
    QCOMPARE(fallback.at(1), "libx264");
}

QTEST_APPLESS_MAIN(MediaUtilsTest)
#include "test_mediautils.moc"
