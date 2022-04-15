#ifndef _TILE_DATA_H_
#define _TILE_DATA_H_

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QJsonObject>

#include "Cytopia/src/engine/basics/tileData.hxx"
#include "helpers.hxx"

class TileDataContainer : public QObject
{
public:
  QString loadFile(const QString& fileName);
  QString getBiomeDataFromFile(const QString& fileName);
  bool saveFile();

  bool hasTileData(const QString& id) const;
  TileData getTileData(const QString& id) const;

  std::vector<QString> getAllGroundDecorationIDs();
  std::vector<QString> getBiomes() { return biomes; };

  void removeTileData(const QString& id);
  void addTileData(const TileData& tile);

  using Map = QMap<QString, TileData>;
  Map::iterator begin() { return tileData.begin(); }
  Map::iterator end() { return tileData.end(); }

private:
  void stringArrayFromJson(std::vector<std::string>& data, const QJsonValue& value);
  void biomesFromJson(TileData& data, const QJsonValue& value);
  void groundDecorationFromJson(TileData& data, const QJsonValue& value);
  void tileSetDataFromJson(TileSetData& data, const QJsonValue& value);
  void requiredTilesFromJson(TileSize& data, const QJsonValue& value);
  void zoneTypesFromJson(std::vector<ZoneType>& data, const QJsonValue& value);
  void stylesFromJson(TileData& data, const QJsonValue& value);
  void zoneDensityFromJson(TileData& data, const QJsonValue& value);
  void tileTypeFromJson(TileType& tileType, const QJsonValue& value);
  QJsonObject tileSetDataToJson(const TileSetData& data);
  QJsonObject requiredTilesToJson(const TileSize& data);
  QJsonArray stringArrayToJson(const std::vector<std::string>& data);
  QJsonArray zoneTypeToJson(const std::vector<ZoneType>& data);
  QJsonArray stylesToJson(const std::vector<Style>& data);
  QJsonArray zoneDensityToJson(const std::vector<ZoneDensity>& data);

private:
  QString fileName;
  QMap<QString, TileData> tileData;
  std::vector<QString> biomes;
};

#endif
