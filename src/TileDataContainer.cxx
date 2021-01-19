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
		tile.happyness = obj.value("happyness").toInt();
		tile.fireHazardLevel = obj.value("fireHazardLevel").toInt();
		tile.educationLevel = obj.value("educationLevel").toInt();
		tile.crimeLevel = obj.value("crimeLevel").toInt();
		tile.isOverPlacable = obj.value("isOverPlacable").toBool();
		tile.placeOnGround = obj.value("placeOnGround").toBool();
		tile.placeOnWater = obj.value("placeOnWater").toBool();

		requiredTilesFromJson(tile.RequiredTiles, obj.value("RequiredTiles"));

		tileSetDataFromJson(tile.tiles, obj.value("tiles"));
		tileSetDataFromJson(tile.shoreTiles, obj.value("shoreTiles"));
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
}

void TileDataContainer::requiredTilesFromJson(RequiredTilesData& data, const QJsonValue& value)
{
	QJsonObject obj = value.toObject();

	data.width = static_cast<unsigned int>(obj.value("width").toInt());
	data.height= static_cast<unsigned int>(obj.value("height").toInt());
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
		obj.insert("happyness", tile.happyness);
		obj.insert("fireHazardLevel", tile.fireHazardLevel);
		obj.insert("educationLevel", tile.educationLevel);
		obj.insert("crimeLevel", tile.crimeLevel);
		obj.insert("isOverPlacable", tile.isOverPlacable);
		obj.insert("placeOnGround", tile.placeOnGround);
		obj.insert("placeOnWater", tile.placeOnWater);

		obj.insert("RequiredTiles",  requiredTilesToJson(tile.RequiredTiles));

		if (!tile.tiles.fileName.empty())
			obj.insert("tiles", tileSetDataToJson(tile.tiles));

		if (!tile.shoreTiles.fileName.empty())
			obj.insert("shoreTiles", tileSetDataToJson(tile.shoreTiles));

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

	return obj;
}

QJsonObject TileDataContainer::requiredTilesToJson(const RequiredTilesData& data)
{
	QJsonObject obj;
	obj.insert("width", static_cast<int>(data.width));
	obj.insert("height", static_cast<int>(data.height));

	return obj;
}

//--------------------------------------------------------------------------------

void TileDataContainer::removeTileData(const QString& id) { tileData.remove(id); }

//--------------------------------------------------------------------------------

void TileDataContainer::addTileData(const TileData& tile) { tileData.insert(QString::fromStdString(tile.id), tile); }

//--------------------------------------------------------------------------------
