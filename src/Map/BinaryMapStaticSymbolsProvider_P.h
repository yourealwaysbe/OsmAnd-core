#ifndef _OSMAND_CORE_BINARY_MAP_STATIC_SYMBOLS_PROVIDER_P_H_
#define _OSMAND_CORE_BINARY_MAP_STATIC_SYMBOLS_PROVIDER_P_H_

#include "stdlib_common.h"
#include <array>
#include <functional>

#include "QtExtensions.h"
#include <QReadWriteLock>
#include <QList>

#include "OsmAndCore.h"
#include "CommonTypes.h"
#include "PrivateImplementation.h"
#include "IMapTiledSymbolsProvider.h"
#include "BinaryMapObjectSymbolsGroup.h"
#include "BinaryMapObject.h"
#include "BinaryMapStaticSymbolsProvider.h"

namespace OsmAnd
{
    class BinaryMapPrimitivesTile;
    class MapSymbolsGroup;
    class TiledMapSymbolsData;

    class BinaryMapStaticSymbolsProvider;
    class BinaryMapStaticSymbolsProvider_P Q_DECL_FINAL
    {
    public:
        typedef BinaryMapStaticSymbolsProvider::FilterCallback FilterCallback;

    private:
        struct SymbolForPinPointsComputation
        {
            float leftPaddingInPixels;
            float widthInPixels;
            float rightPaddingInPixels;
        };
        struct ComputedPinPoint
        {
            PointI point;

            unsigned int basePathPointIndex;
            double offsetFromBasePathPoint;
            float normalizedOffsetFromBasePathPoint;
        };
        static QVector<ComputedPinPoint> computePinPoints(
            const QVector<PointI>& path31,
            const float globalLeftPaddingInPixels,
            const float globalRightPaddingInPixels,
            const QVector<SymbolForPinPointsComputation>& symbolsForPinPointsComputation,
            const ZoomLevel minZoom,
            const ZoomLevel maxZoom);
    protected:
        BinaryMapStaticSymbolsProvider_P(BinaryMapStaticSymbolsProvider* owner);

        ImplementationInterface<BinaryMapStaticSymbolsProvider> owner;
    public:
        virtual ~BinaryMapStaticSymbolsProvider_P();

        bool obtainData(
            const TileId tileId, const ZoomLevel zoom,
            std::shared_ptr<MapTiledData>& outTiledData,
            const FilterCallback filterCallback,
            const IQueryController* const queryController);

    friend class OsmAnd::BinaryMapStaticSymbolsProvider;
    };
}

#endif // !defined(_OSMAND_CORE_BINARY_MAP_STATIC_SYMBOLS_PROVIDER_P_H_)
