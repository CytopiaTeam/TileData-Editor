#include "TileDataContainer.hxx"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>

//--------------------------------------------------------------------------------

bool TileDataContainer::hasTileData(const QString& id) const { return tileData.contains(id); }

//--------------------------------------------------------------------------------

TileData TileDataContainer::getTileData(const QString& id) const
{
  if (!tileData.contains(id))
    return TileData();

  return tileData[id];
}

//--------------------------------------------------------------------------------

std::vector<QString> TileDataContainer::getAllGroundDecorationIDs()
{
  std::vector<QString> foundIDs;

  for (const TileData& it : tileData)
  {
    if (it.tileType == +TileType::GROUNDDECORATION)
    {
      foundIDs.push_back(QString::fromStdString( it.id));
    }
  }
   
  return foundIDs;
}
//--------------------------------------------------------------------------------


QString TileDataContainer::loadFile(const QString& theFileName)
{
  fileName = theFileName;
  tileData.clear();

  QFile file(fileName);
  if (!file.exists())
    return QString();

  if (!file.open(QIODevice::ReadOnly))
    return file.errorString();

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isNull() || !doc.isArray())
    return tr("Illegal file content");

  for (const QJsonValue& value : doc.array())
  {
    QJsonObject obj = value.toObject();

    TileData tile;
    QString id = obj.value("id").toString();
    tile.id = id.toStdString();
    tile.category = obj.value("category").toString().toStdString();
    tile.subCategory = obj.value("subCategory").toString().toStdString();
    tile.title = obj.value("title").toString().toStdString();
    tile.description = obj.value("description").toString().toStdString();
    tile.author = obj.value("author").toString().toStdString();
    tile.price = obj.value("price").toInt();
    tile.upkeepCost = obj.value("upkeep cost").toInt();
    tile.power = obj.value("power").toInt();
    tile.water = obj.value("water").toInt();
    tile.inhabitants = obj.value("inhabitants").toInt();
    tile.pollutionLevel = obj.value("pollutionLevel").toInt();
    tile.happiness = obj.value("happiness").toInt();
    tile.fireHazardLevel = obj.value("fireHazardLevel").toInt();
    tile.educationLevel = obj.value("educationLevel").toInt();
    tile.crimeLevel = obj.value("crimeLevel").toInt();
    tile.isOverPlacable = obj.value("isOverPlacable").toBool();
    tile.placeOnWater = obj.value("placeOnWater").toBool();

    // Place on ground should default to true, if not explictly set to false
    if (obj.value("placeOnGround").isUndefined())
    {
      tile.placeOnGround = true;
    }
    else
    {
      tile.placeOnGround = obj.value("placeOnGround").toBool();
    }

    tileTypeFromJson(tile.tileType, obj.value("tileType"));
    requiredTilesFromJson(tile.RequiredTiles, obj.value("RequiredTiles"));

    stringArrayFromJson(tile.tags, obj.value("tags"));
    stringArrayFromJson(tile.biomes, obj.value("biomes"));
    stringArrayFromJson(tile.groundDecoration, obj.value("groundDecoration"));
    zonesFromJson(tile.zones, obj.value("zones"));
    stylesFromJson(tile, obj.value("style"));
    wealthFromJson(tile, obj.value("wealth"));

    tileSetDataFromJson(tile.tiles, obj.value("tiles"));
    tileSetDataFromJson(tile.shoreTiles, obj.value("shoreLine"));
    tileSetDataFromJson(tile.slopeTiles, obj.value("slopeTiles"));

    tileData.insert(id, tile);
  }

  return QString();
}

//--------------------------------------------------------------------------------

void TileDataContainer::tileSetDataFromJson(TileSetData& data, const QJsonValue& value)
{
  QJsonObject obj = value.toObject();

  data.fileName = obj.value("fileName").toString().toStdString();
  data.count = obj.value("count").toInt();
  data.clippingWidth = obj.value("clip_width").toInt();
  data.clippingHeight = obj.value("clip_height").toInt();
  data.offset = obj.value("offset").toInt();
  if (obj.value("pickRandomTile").isUndefined())
  {
    if (data.count > 1)
    {
      data.pickRandomTile = true;
    }
    else
    {
      data.pickRandomTile = false;
    }
  }
  else
  {
    data.pickRandomTile = obj.value("pickRandomTile").toBool();
  }
}

void TileDataContainer::requiredTilesFromJson(RequiredTilesData& data, const QJsonValue& value)
{
  QJsonObject obj = value.toObject();

  data.width = static_cast<unsigned int>(obj.value("width").toInt());
  data.height = static_cast<unsigned int>(obj.value("height").toInt());
}

void TileDataContainer::stringArrayFromJson(std::vector<std::string>& data, const QJsonValue& value)
{
  for (const QJsonValue& tag : value.toArray())
  {
    data.push_back(tag.toString().toStdString());
  }
}


void TileDataContainer::zonesFromJson(std::vector<Zones>& data, const QJsonValue& value)
{
  for (const QJsonValue& zone : value.toArray())
  {
    data.push_back(Zones::_from_string_nocase(zone.toString().toStdString().c_str()));
  }
}

void TileDataContainer::stylesFromJson(TileData& data, const QJsonValue& value)
{
  // if no values are supplied, we add all styles
  if (value.toArray().empty() && data.tileType == +TileType::RCI)
  {
    for (Style style : Style::_values())
    {
      data.style.push_back(style);
    }
  }
  else
  {
    for (const QJsonValue& style : value.toArray())
    {
      data.style.push_back(Style::_from_string_nocase(style.toString().toStdString().c_str()));
    }
  }
}

void TileDataContainer::wealthFromJson(TileData& data, const QJsonValue& value)
{
  if (value.toArray().empty() && data.tileType == +TileType::RCI)
  {
    for (Wealth wealth : Wealth::_values())
    {
      data.wealth.push_back(wealth);
    }
  }
  else
  {
    for (const QJsonValue& wealth : value.toArray())
    {
      data.wealth.push_back(Wealth::_from_string_nocase(wealth.toString().toStdString().c_str()));
    }
  }
}

void TileDataContainer::tileTypeFromJson(TileType& tileType, const QJsonValue& value)
{
  if (!value.toString().isEmpty())
  {
    tileType = TileType::_from_string_nocase(value.toString().toStdString().c_str());
  }
  else
  {
    tileType = TileType::DEFAULT;
  }
}

//--------------------------------------------------------------------------------

bool TileDataContainer::saveFile()
{
  QJsonDocument doc;
  QJsonArray array;

  for (const TileData& tile : tileData)
  {
    QJsonObject obj;
    obj.insert("id", QString::fromStdString(tile.id));
    obj.insert("category", QString::fromStdString(tile.category));
    obj.insert("subCategory", QString::fromStdString(tile.subCategory));
    obj.insert("title", QString::fromStdString(tile.title));
    obj.insert("description", QString::fromStdString(tile.description));
    obj.insert("author", QString::fromStdString(tile.author));
    obj.insert("price", tile.price);
    obj.insert("upkeep cost", tile.upkeepCost);
    obj.insert("power", tile.power);
    obj.insert("water", tile.water);
    obj.insert("inhabitants", tile.inhabitants);
    obj.insert("pollutionLevel", tile.pollutionLevel);
    obj.insert("happiness", tile.happiness);
    obj.insert("fireHazardLevel", tile.fireHazardLevel);
    obj.insert("educationLevel", tile.educationLevel);
    obj.insert("crimeLevel", tile.crimeLevel);
    obj.insert("isOverPlacable", tile.isOverPlacable);
    obj.insert("placeOnGround", tile.placeOnGround);
    obj.insert("placeOnWater", tile.placeOnWater);

    obj.insert("RequiredTiles", requiredTilesToJson(tile.RequiredTiles));

    if (!tile.groundDecoration.empty()) { obj.insert("groundDecoration", stringArrayToJson(tile.groundDecoration)); }
    if (!tile.tags.empty()) { obj.insert("tags", stringArrayToJson(tile.tags)); }
    if (!tile.biomes.empty()) { obj.insert("biomes", stringArrayToJson(tile.biomes)); }
    if (!tile.style.empty()) { obj.insert("style", stylesToJson(tile.style)); }
    if (!tile.wealth.empty()) { obj.insert("wealth", wealthToJson(tile.wealth)); }
    if (!tile.zones.empty()) { obj.insert("zones", zonesToJson(tile.zones)); }

    obj.insert("tileType", QString::fromUtf8(tile.tileType._to_string()));

    if (!tile.tiles.fileName.empty())
      obj.insert("tiles", tileSetDataToJson(tile.tiles));

    if (!tile.shoreTiles.fileName.empty())
      obj.insert("shoreLine", tileSetDataToJson(tile.shoreTiles));

    if (!tile.slopeTiles.fileName.empty())
      obj.insert("slopeTiles", tileSetDataToJson(tile.slopeTiles));

    array.append(obj);
  }

  doc.setArray(array);

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
  {
    // error handling
    return false;
  }
  file.write(doc.toJson());
  return true;
}

//--------------------------------------------------------------------------------

QJsonObject TileDataContainer::tileSetDataToJson(const TileSetData& data)
{
  QJsonObject obj;

  obj.insert("fileName", QString::fromStdString(data.fileName));
  obj.insert("count", data.count);
  obj.insert("clip_width", data.clippingWidth);
  obj.insert("clip_height", data.clippingHeight);
  obj.insert("offset", data.offset);
  obj.insert("pickRandomTile", data.pickRandomTile);

  return obj;
}

QJsonObject TileDataContainer::requiredTilesToJson(const RequiredTilesData& data)
{
  QJsonObject obj;
  obj.insert("width", static_cast<int>(data.width));
  obj.insert("height", static_cast<int>(data.height));

  return obj;
}

QJsonArray TileDataContainer::stringArrayToJson(const std::vector<std::string>& data)
{
  QJsonArray result;

  for (const std::string value : data)
  {
    result.append(QString::fromStdString(value));
  }

  return result;
}

QJsonArray TileDataContainer::zonesToJson(const std::vector<Zones>& data)
{
  QJsonArray result;

  for (const Zones zone : data)
  {
    result.append(QString::fromUtf8(zone._to_string()));
  }

  return result;
}

QJsonArray TileDataContainer::stylesToJson(const std::vector<Style>& data)
{
  QJsonArray result;

  for (const Style style : data)
  {
    result.append(QString::fromUtf8(style._to_string()));
  }

  return result;
}

QJsonArray TileDataContainer::wealthToJson(const std::vector<Wealth>& data)
{
  QJsonArray result;

  for (const Wealth wealth : data)
  {
    result.append(QString::fromUtf8(wealth._to_string()));
  }

  return result;
}


//--------------------------------------------------------------------------------

void TileDataContainer::removeTileData(const QString& id) { tileData.remove(id); }

//--------------------------------------------------------------------------------

void TileDataContainer::addTileData(const TileData& tile) { tileData.insert(QString::fromStdString(tile.id), tile); }

//--------------------------------------------------------------------------------
