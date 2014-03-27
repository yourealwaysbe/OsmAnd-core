#include "OfflineMapRasterTileProvider_GPU_P.h"
#include "OfflineMapRasterTileProvider_GPU.h"

#if !defined(OSMAND_PERFORMANCE_METRICS)
#   define OSMAND_PERFORMANCE_METRICS 0
#endif

#include <cassert>
#if OSMAND_PERFORMANCE_METRICS
#   include <chrono>
#endif // OSMAND_PERFORMANCE_METRICS

#include <SkStream.h>
#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkBitmapDevice.h>
#include <SkImageDecoder.h>
#include <SkImageEncoder.h>

#include "OfflineMapDataProvider.h"
#include "OfflineMapDataTile.h"
#include "ObfsCollection.h"
#include "ObfDataInterface.h"
#include "Rasterizer.h"
#include "RasterizerContext.h"
#include "RasterizerEnvironment.h"
#include "Utilities.h"
#include "Logging.h"

OsmAnd::OfflineMapRasterTileProvider_GPU_P::OfflineMapRasterTileProvider_GPU_P( OfflineMapRasterTileProvider_GPU* owner_, const uint32_t outputTileSize_, const float density_ )
    : owner(owner_)
    , outputTileSize(outputTileSize_)
    , density(density_)
{
}

OsmAnd::OfflineMapRasterTileProvider_GPU_P::~OfflineMapRasterTileProvider_GPU_P()
{
}

bool OsmAnd::OfflineMapRasterTileProvider_GPU_P::obtainTile(const TileId tileId, const ZoomLevel zoom, std::shared_ptr<const MapTile>& outTile, const IQueryController* const queryController)
{
    // Obtain offline map data tile
    std::shared_ptr< const OfflineMapDataTile > dataTile;
    owner->dataProvider->obtainTile(tileId, zoom, dataTile);
    if(!dataTile)
    {
        outTile.reset();
        return true;
    }

#if OSMAND_PERFORMANCE_METRICS
    const auto dataRasterization_Begin = std::chrono::high_resolution_clock::now();
#endif // OSMAND_PERFORMANCE_METRICS

    // Allocate rasterization target
    auto rasterizationSurface = new SkBitmap();
    rasterizationSurface->setConfig(SkBitmap::kARGB_8888_Config, outputTileSize, outputTileSize);
    if(!rasterizationSurface->allocPixels())
    {
        delete rasterizationSurface;

        LogPrintf(LogSeverityLevel::Error, "Failed to allocate buffer for ARGB8888 rasterization surface %dx%d", outputTileSize, outputTileSize);
        return false;
    }
    //TODO: SkGpuDevice
    SkBitmapDevice rasterizationTarget(*rasterizationSurface);

    // Create rasterization canvas
    SkCanvas canvas(&rasterizationTarget);

    // Perform actual rendering
    if(!dataTile->nothingToRasterize)
    {
        Rasterizer rasterizer(dataTile->rasterizerContext);
        rasterizer.rasterizeMap(canvas, queryController);
    }

#if OSMAND_PERFORMANCE_METRICS
    const auto dataRasterization_End = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> dataRasterization_Elapsed = dataRasterization_End - dataRasterization_Begin;
    if(!dataTile->nothingToRasterize)
    {
        LogPrintf(LogSeverityLevel::Info,
            "%d map objects in %dx%d@%d: rasterization %fs",
            dataTile->mapObjects.count(), tileId.x, tileId.y, zoom, dataRasterization_Elapsed.count());
    }
    else
    {
        LogPrintf(LogSeverityLevel::Info,
            "%d map objects in %dx%d@%d: nothing to rasterize (%fs)",
            dataTile->mapObjects.count(), tileId.x, tileId.y, zoom, dataRasterization_Elapsed.count());
    }
#endif // OSMAND_PERFORMANCE_METRICS

    // If there is no data to rasterize, tell that this tile is not available
    if(dataTile->nothingToRasterize)
    {
        delete rasterizationSurface;

        outTile.reset();
        return true;
    }

    // Or supply newly rasterized tile
    auto tile = new Tile(rasterizationSurface, dataTile);
    outTile.reset(tile);
    return true;
}

OsmAnd::OfflineMapRasterTileProvider_GPU_P::Tile::Tile( SkBitmap* bitmap, const std::shared_ptr<const OfflineMapDataTile>& dataTile_ )
    : MapBitmapTile(bitmap, AlphaChannelData::NotPresent)
    , _dataTile(dataTile_)
    , dataTile(_dataTile)
{
}

OsmAnd::OfflineMapRasterTileProvider_GPU_P::Tile::~Tile()
{
}

void OsmAnd::OfflineMapRasterTileProvider_GPU_P::Tile::releaseNonRetainedData()
{
    _bitmap.reset();
}
