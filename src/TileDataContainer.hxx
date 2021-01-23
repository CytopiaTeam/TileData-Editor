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
  QString loadFile(const QString &fileName);
  bool saveFile();

  bool hasTileData(const QString &id) const;
  TileData getTileData(const QString &id) const;

  void removeTileData(const QString &id);
  void addTileData(const TileData &tile);

  using Map = QMap<QString, TileData>;
  Map::iterator begin() { return tileData.begin(); }
  Map::iterator end() { return tileData.end(); }

private:
  void tagsFromJson(TileData& data, const QJsonValue& value);
  void tileSetDataFromJson(TileSetData &data, const QJsonValue &value);
  void requiredTilesFromJson(RequiredTilesData& data, const QJsonValue& value);
  void zonesFromJson(std::vector<Zones> &data, const QJsonValue& value);
  void stylesFromJson(std::vector<Style>& data, const QJsonValue& value);
  QJsonObject tileSetDataToJson(const TileSetData& data);
  QJsonObject requiredTilesToJson(const RequiredTilesData& data);
  QJsonArray tagsToJson(const std::vector<std::string>& data);
  QJsonArray zonesToJson(const std::vector<Zones>& data);
  QJsonArray stylesToJson(const std::vector<Style>& data);

private:
  QString fileName;
  QMap<QString, TileData> tileData;
};

#endif
