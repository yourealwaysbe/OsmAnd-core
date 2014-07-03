#ifndef _OSMAND_CORE_UTILITIES_H_
#define _OSMAND_CORE_UTILITIES_H_

#include <OsmAndCore/stdlib_common.h>
#include <cassert>
#include <climits>
#include <functional>

#include <OsmAndCore/QtExtensions.h>
#include <QtMath>
#include <QtNumeric>
#include <QString>
#include <QStringList>
#include <QList>
#include <QDir>
#include <QFile>
#include <QVector>
#include <QSet>

#include <OsmAndCore.h>
#include <OsmAndCore/CommonTypes.h>

namespace OsmAnd
{
    struct OSMAND_CORE_API Utilities Q_DECL_FINAL
    {
        inline static double toRadians(const double angle)
        {
            return angle / 180.0 * M_PI;
        }

        inline static int32_t get31TileNumberX(const double longitude)
        {
            const auto l = (1UL << 31);
            return static_cast<int32_t>((normalizeLongitude(longitude) + 180.0) / 360.0*l);
        }

        inline static int32_t get31TileNumberY(const double latitude)
        {
            const auto l = (1UL << 31);

            const auto latitude_ = normalizeLatitude(latitude);
            double eval = log(tan(toRadians(latitude_)) + 1.0 / cos(toRadians(latitude_)));
            if (eval > M_PI)
                eval = M_PI;
            return static_cast<int32_t>((1.0 - eval / M_PI) / 2.0*l);
        }

        inline static double get31LongitudeX(const double x)
        {
            return getLongitudeFromTile(21, x / 1024.);
        }

        inline static double get31LatitudeY(const double y)
        {
            return getLatitudeFromTile(21, y / 1024.);
        }

        inline static double getTileNumberX(const float zoom, const double longitude)
        {
            if (qAbs(longitude - 180.) < std::numeric_limits<double>::epsilon())
                return getPowZoom(zoom) - 1;

            return (normalizeLongitude(longitude) + 180.) / 360. * getPowZoom(zoom);
        }

        inline static double getTileNumberY(const float zoom, double latitude)
        {
            latitude = normalizeLatitude(latitude);
            double eval = log(tan(toRadians(latitude)) + 1 / cos(toRadians(latitude)));
            if (qIsInf(eval) || qIsNaN(eval))
            {
                latitude = latitude < 0 ? -89.9 : 89.9;
                eval = log(tan(toRadians(latitude)) + 1 / cos(toRadians(latitude)));
            }
            double result = (1 - eval / M_PI) / 2 * getPowZoom(zoom);
            return result;
        }

        inline static float convert31toFloat(const int32_t value, const ZoomLevel zoom)
        {
            /*
            const auto tileSize31 = (1u << (ZoomLevel::MaxZoomLevel - zoom));
            const auto fractionalMask = ?;
            const auto intPart = value >> (ZoomLevel::MaxZoomLevel - zoom);
            const auto intFractionalPart = tileSize31 / (value & fractionalMask);
            // res = intPart + 1.0f/intFractionalPart + float<remainder>/(value & fractionalMask)
            */
            return static_cast<float>((static_cast<double>(value) / (1u << (ZoomLevel::MaxZoomLevel - zoom))));
        }

        inline static PointF convert31toFloat(const PointI& p, const ZoomLevel zoom)
        {
            const auto tileSize31 = (1u << (ZoomLevel::MaxZoomLevel - zoom));
            return PointF(
                static_cast<float>((static_cast<double>(p.x) / tileSize31)),
                static_cast<float>((static_cast<double>(p.y) / tileSize31)));
        }

        inline static double normalizeLatitude(double latitude)
        {
            while (latitude < -90.0 || latitude > 90.0)
            {
                if (latitude < 0.0)
                    latitude += 180.0;
                else
                    latitude -= 180.0;
            }

            if (latitude < -85.0511)
                return -85.0511;
            else if (latitude > 85.0511)
                return 85.0511;

            return latitude;
        }

        inline static double normalizeLongitude(double longitude)
        {
            while (longitude < -180.0 || longitude >= 180.0)
            {
                if (longitude < 0.0)
                    longitude += 360.0;
                else
                    longitude -= 360.0;
            }
            return longitude;
        }

        inline static double getPowZoom(const float zoom)
        {
            if (zoom >= 0.0f && qFuzzyCompare(zoom, static_cast<uint8_t>(zoom)))
                return 1 << static_cast<uint8_t>(zoom);

            return qPow(2, zoom);
        }

        inline static double getLongitudeFromTile(const float zoom, const double x)
        {
            return x / getPowZoom(zoom) * 360.0 - 180.0;
        }

        inline static double getLatitudeFromTile(const float zoom, const double y)
        {
            int sign = y < 0 ? -1 : 1;
            double result = atan(sign * sinh(M_PI * (1 - 2 * y / getPowZoom(zoom)))) * 180. / M_PI;
            return result;
        }

        inline static double x31toMeters(const int32_t x31)
        {
            return static_cast<double>(x31)* 0.011;
        }

        inline static double y31toMeters(const int32_t y31)
        {
            return static_cast<double>(y31)* 0.01863;
        }

        inline static double squareDistance31(const int32_t x31a, const int32_t y31a, const int32_t x31b, const int32_t y31b)
        {
            const auto dx = Utilities::x31toMeters(x31a - x31b);
            const auto dy = Utilities::y31toMeters(y31a - y31b);
            return dx * dx + dy * dy;
        }

        inline static double distance31(const int32_t x31a, const int32_t y31a, const int32_t x31b, const int32_t y31b)
        {
            return qSqrt(squareDistance31(x31a, y31a, x31b, y31b));
        }

        inline static double squareDistance31(const PointI& a, const PointI& b)
        {
            const auto dx = Utilities::x31toMeters(a.x - b.x);
            const auto dy = Utilities::y31toMeters(a.y - b.y);
            return dx * dx + dy * dy;
        }

        inline static double distance31(const PointI& a, const PointI& b)
        {
            return qSqrt(squareDistance31(a, b));
        }

        inline static double distance(const double xLonA, const double yLatA, const double xLonB, const double yLatB)
        {
            double R = 6371; // km
            double dLat = toRadians(yLatB - yLatA);
            double dLon = toRadians(xLonB - xLonA);
            double a =
                qSin(dLat / 2.0) * qSin(dLat / 2.0) +
                qCos(toRadians(yLatA)) * qCos(toRadians(yLatB)) *
                qSin(dLon / 2.0) * qSin(dLon / 2.0);
            double c = 2.0 * qAtan2(qSqrt(a), qSqrt(1.0 - a));
            return R * c * 1000.0;
        }

        inline static double projection31(const int32_t x31a, const int32_t y31a, const int32_t x31b, const int32_t y31b, const int32_t x31c, const int32_t y31c)
        {
            // Scalar multiplication between (AB, AC)
            auto p =
                Utilities::x31toMeters(x31b - x31a) * Utilities::x31toMeters(x31c - x31a) +
                Utilities::y31toMeters(y31b - y31a) * Utilities::y31toMeters(y31c - y31a);
            return p;
        }

        inline static double normalizedAngleRadians(double angle)
        {
            while (angle > M_PI)
                angle -= 2.0 * M_PI;
            while (angle <= -M_PI)
                angle += 2.0 * M_PI;
            return angle;
        }

        inline static double normalizedAngleDegrees(double angle)
        {
            while (angle > 180.0)
                angle -= 360.0;
            while (angle <= -180.0)
                angle += 360.;
            return angle;
        }

        inline static double polygonArea(const QVector<PointI>& points)
        {
            double area = 0.0;

            assert(points.first() == points.last());

            auto p0 = points.constData();
            auto p1 = p0 + 1;
            for (auto idx = 1, count = points.size(); idx < count; idx++, p0++, p1++)
            {
                area += static_cast<double>(p0->x) * static_cast<double>(p1->y) - static_cast<double>(p1->x) * static_cast<double>(p0->y);
            }
            area = qAbs(area) * 0.5;

            return area;
        }

        inline static bool rayIntersectX(const PointF& v0_, const PointF& v1_, float mY, float& mX)
        {
            // prev node above line
            // x,y node below line

            const auto& v0 = (v0_.y > v1_.y) ? v1_ : v0_;
            const auto& v1 = (v0_.y > v1_.y) ? v0_ : v1_;

            if (qFuzzyCompare(v1.y, mY) || qFuzzyCompare(v0.y, mY))
                mY -= 1.0f;

            if (v0.y > mY || v1.y < mY)
                return false;

            if (v1 == v0)
            {
                // the node on the boundary !!!
                mX = v1.x;
                return true;
            }

            // that tested on all cases (left/right)
            mX = v1.x + (mY - v1.y) * (v1.x - v0.x) / (v1.y - v0.y);
            return true;
        }

        inline static bool rayIntersect(const PointF& v0, const PointF& v1, const PointF& v)
        {
            float t;
            if (!rayIntersectX(v0, v1, v.y, t))
                return false;

            if (t < v.x)
                return true;

            return false;
        }

        inline static bool rayIntersectX(const PointI& v0_, const PointI& v1_, int32_t mY, int32_t& mX)
        {
            // prev node above line
            // x,y node below line

            const auto& v0 = (v0_.y > v1_.y) ? v1_ : v0_;
            const auto& v1 = (v0_.y > v1_.y) ? v0_ : v1_;

            if (v1.y == mY || v0.y == mY)
                mY -= 1;

            if (v0.y > mY || v1.y < mY)
                return false;

            if (v1 == v0)
            {
                // the node on the boundary !!!
                mX = v1.x;
                return true;
            }

            // that tested on all cases (left/right)
            mX = static_cast<int32_t>(v1.x + static_cast<double>(mY - v1.y) * static_cast<double>(v1.x - v0.x) / static_cast<double>(v1.y - v0.y));
            return true;
        }

        inline static bool rayIntersect(const PointI& v0, const PointI& v1, const PointI& v)
        {
            int32_t t;
            if (!rayIntersectX(v0, v1, v.y, t))
                return false;

            if (t < v.x)
                return true;

            return false;
        }

        inline static double degreesDiff(const double a1, const double a2)
        {
            auto diff = a1 - a2;
            while (diff > 180.0)
                diff -= 360.0;
            while (diff <= -180.0)
                diff += 360.0;
            return diff;
        }

        inline static AreaI tileBoundingBox31(const TileId tileId, const ZoomLevel zoom)
        {
            AreaI output;

            const auto zoomShift = ZoomLevel31 - zoom;

            output.top = tileId.y << zoomShift;
            output.left = tileId.x << zoomShift;
            output.bottom = ((tileId.y + 1) << zoomShift) - 1;
            output.right = ((tileId.x + 1) << zoomShift) - 1;

            assert(output.top >= 0 && output.top <= std::numeric_limits<int32_t>::max());
            assert(output.left >= 0 && output.left <= std::numeric_limits<int32_t>::max());
            assert(output.bottom >= 0 && output.bottom <= std::numeric_limits<int32_t>::max());
            assert(output.right >= 0 && output.right <= std::numeric_limits<int32_t>::max());
            assert(output.right >= output.left);
            assert(output.bottom >= output.top);

            return output;
        }

        inline static AreaI areaRightShift(const AreaI& input, const uint32_t shift)
        {
            AreaI output;
            uint32_t tail;

            output.top = input.top >> shift;
            output.left = input.left >> shift;

            tail = input.bottom & ((1 << shift) - 1);
            output.bottom = (input.bottom >> shift) + (tail ? 1 : 0);
            tail = input.right & ((1 << shift) - 1);
            output.right = (input.right >> shift) + (tail ? 1 : 0);

            assert(output.top >= 0 && output.top <= std::numeric_limits<int32_t>::max());
            assert(output.left >= 0 && output.left <= std::numeric_limits<int32_t>::max());
            assert(output.bottom >= 0 && output.bottom <= std::numeric_limits<int32_t>::max());
            assert(output.right >= 0 && output.right <= std::numeric_limits<int32_t>::max());
            assert(output.right >= output.left);
            assert(output.bottom >= output.top);

            return output;
        }

        inline static AreaI areaLeftShift(const AreaI& input, const uint32_t shift)
        {
            AreaI output;

            output.top = input.top << shift;
            output.left = input.left << shift;
            output.bottom = input.bottom << shift;
            output.right = input.right << shift;

            assert(output.top >= 0 && output.top <= std::numeric_limits<int32_t>::max());
            assert(output.left >= 0 && output.left <= std::numeric_limits<int32_t>::max());
            assert(output.bottom >= 0 && output.bottom <= std::numeric_limits<int32_t>::max());
            assert(output.right >= 0 && output.right <= std::numeric_limits<int32_t>::max());
            assert(output.right >= output.left);
            assert(output.bottom >= output.top);

            return output;
        }

        inline static uint32_t getNextPowerOfTwo(const uint32_t value)
        {
            if (value == 0)
                return 0;

            auto n = value;

            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n++;

            return n;
        }

        inline static double getMetersPerTileUnit(const float zoom, const double yTile, const double unitsPerTile)
        {
            // Equatorial circumference of the Earth in meters
            const static double C = 40075017.0;

            const auto powZoom = getPowZoom(zoom);
            const auto sinhValue = sinh((2.0 * M_PI * yTile) / powZoom - M_PI);
            auto res = C / (powZoom * unitsPerTile * qSqrt(sinhValue*sinhValue + 1.0));

            return res;
        }

        inline static TileId normalizeTileId(const TileId input, const ZoomLevel zoom)
        {
            TileId output = input;

            const auto tilesCount = static_cast<int32_t>(1u << zoom);

            while (output.x < 0)
                output.x += tilesCount;
            while (output.y < 0)
                output.y += tilesCount;

            // Max zoom level (31) is skipped, since value stored in int31 can not be more than tilesCount(31)
            if (zoom < ZoomLevel31)
            {
                while (output.x >= tilesCount)
                    output.x -= tilesCount;
                while (output.y >= tilesCount)
                    output.y -= tilesCount;
            }

            assert(output.x >= 0 && ((zoom < ZoomLevel31 && output.x < tilesCount) || (zoom == ZoomLevel31)));
            assert(output.x >= 0 && ((zoom < ZoomLevel31 && output.y < tilesCount) || (zoom == ZoomLevel31)));

            return output;
        }

        inline static PointI normalizeCoordinates(const PointI& input, const ZoomLevel zoom)
        {
            PointI output = input;

            const auto tilesCount = static_cast<int32_t>(1u << zoom);

            while (output.x < 0)
                output.x += tilesCount;
            while (output.y < 0)
                output.y += tilesCount;

            // Max zoom level (31) is skipped, since value stored in int31 can not be more than tilesCount(31)
            if (zoom < ZoomLevel31)
            {
                while (output.x >= tilesCount)
                    output.x -= tilesCount;
                while (output.y >= tilesCount)
                    output.y -= tilesCount;
            }

            assert(output.x >= 0 && ((zoom < ZoomLevel31 && output.x < tilesCount) || (zoom == ZoomLevel31)));
            assert(output.x >= 0 && ((zoom < ZoomLevel31 && output.y < tilesCount) || (zoom == ZoomLevel31)));

            return output;
        }

        inline static PointI normalizeCoordinates(const PointI64& input, const ZoomLevel zoom)
        {
            PointI64 output = input;

            const auto tilesCount = static_cast<int64_t>(1ull << zoom);

            while (output.x < 0)
                output.x += tilesCount;
            while (output.y < 0)
                output.y += tilesCount;

            while (output.x >= tilesCount)
                output.x -= tilesCount;
            while (output.y >= tilesCount)
                output.y -= tilesCount;

            assert(output.x >= 0 && output.x < tilesCount);
            assert(output.y >= 0 && output.y < tilesCount);

            return PointI(static_cast<int32_t>(output.x), static_cast<int32_t>(output.y));
        }

        enum class CHCode : uint8_t
        {
            Inside = 0, // 0000
            Left = 1,   // 0001
            Right = 2,  // 0010
            Bottom = 4, // 0100
            Top = 8,    // 1000
        };

        inline static uint8_t computeCohenSutherlandCode(const PointI& p, const AreaI& box)
        {
            uint8_t res = static_cast<uint8_t>(CHCode::Inside);

            if (p.x < box.left)           // to the left of clip box
                res |= static_cast<uint8_t>(CHCode::Left);
            else if (p.x > box.right)     // to the right of clip box
                res |= static_cast<uint8_t>(CHCode::Right);
            if (p.y < box.top)            // above the clip box
                res |= static_cast<uint8_t>(CHCode::Bottom);
            else if (p.y > box.bottom)    // below the clip box
                res |= static_cast<uint8_t>(CHCode::Top);

            return res;
        }

        static bool extractFirstNumberPosition(const QString& value, int& first, int& last, bool allowSigned, bool allowDot);
        static double parseSpeed(const QString& value, const double defValue, bool* wasParsed = nullptr);
        static double parseLength(const QString& value, const double defValue, bool* wasParsed = nullptr);
        static double parseWeight(const QString& value, const double defValue, bool* wasParsed = nullptr);
        static ColorARGB parseColor(const QString& value, const ColorARGB defValue, bool* wasParsed = nullptr);
        static int parseArbitraryInt(const QString& value, const int defValue, bool* wasParsed = nullptr);
        static long parseArbitraryLong(const QString& value, const long defValue, bool* wasParsed = nullptr);
        static unsigned int parseArbitraryUInt(const QString& value, const unsigned int defValue, bool* wasParsed = nullptr);
        static unsigned long parseArbitraryULong(const QString& value, const unsigned long defValue, bool* wasParsed = nullptr);
        static float parseArbitraryFloat(const QString& value, const float defValue, bool* wasParsed = nullptr);
        static bool parseArbitraryBool(const QString& value, const bool defValue, bool* wasParsed = nullptr);

        static int javaDoubleCompare(const double l, const double r);
        static void findFiles(const QDir& origin, const QStringList& masks, QFileInfoList& files, const bool recursively = true);
        static void findDirectories(const QDir& origin, const QStringList& masks, QFileInfoList& directories, const bool recursively = true);

        static void scanlineFillPolygon(const unsigned int verticesCount, const PointF* const vertices, std::function<void(const PointI&)> fillPoint);

        inline static QSet<ZoomLevel> enumerateZoomLevels(const ZoomLevel from, const ZoomLevel to)
        {
            QSet<ZoomLevel> result;
            result.reserve(to - from + 1);
            for (int level = from; level <= to; level++)
                result.insert(static_cast<ZoomLevel>(level));

            return result;
        }

        inline static QString stringifyZoomLevels(const QSet<ZoomLevel>& zoomLevels)
        {
            QString result;

            auto sortedZoomLevels = zoomLevels.values();
            qSort(sortedZoomLevels.begin(), sortedZoomLevels.end());
            bool previousCaptured = false;
            ZoomLevel previousZoomLevel = sortedZoomLevels.first();
            bool range = false;
            ZoomLevel rangeStart;
            for (const auto zoomLevel : sortedZoomLevels)
            {
                if (previousCaptured && static_cast<int>(zoomLevel) == static_cast<int>(previousZoomLevel)+1)
                {
                    if (!range)
                        rangeStart = previousZoomLevel;
                    range = true;
                    previousZoomLevel = zoomLevel;
                    previousCaptured = true;
                }
                else if (range)
                {
                    range = false;
                    previousZoomLevel = zoomLevel;
                    previousCaptured = true;

                    result += QString::fromLatin1("%1-%2, %3, ").arg(rangeStart).arg(previousZoomLevel).arg(zoomLevel);
                }
                else
                {
                    previousZoomLevel = zoomLevel;
                    previousCaptured = true;

                    result += QString::fromLatin1("%1, ").arg(zoomLevel);
                }
            }

            // Process last range
            if (range)
                result += QString::fromLatin1("%1-%2, ").arg(rangeStart).arg(sortedZoomLevels.last());

            if (result.length() > 2)
                result.truncate(result.length() - 2);

            return result;
        }

    private:
        Utilities()
        {
        }

        ~Utilities()
        {
        }
    };
}

#endif // !defined(_OSMAND_CORE_UTILITIES_H_)
