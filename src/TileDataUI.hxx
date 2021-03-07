#ifndef _TileDataUI_H_
#define _TileDataUI_H_

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QSplitter>
#include <QtCore/QJsonObject>

#include "ui_TileDataUI.h"
#include "ui_TileSetDataUI.h"
#include "ui_ItemSelectionUI.h"

#include "helpers.hxx"

#include "TileDataContainer.hxx"


class TileDataUI : public QMainWindow
{
  Q_OBJECT

public:
  TileDataUI();

  bool loadFile(const QString& name);
  bool loadBiomeData(const QString& name);

protected:
  void closeEvent(QCloseEvent* event) override;

private slots:
  void itemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void saveTileData();
  void newItem();
  void deleteItem();
  void duplicateItem();

private: // methods
  void setup(Ui_TileSetDataUi& ui, Ui_TileDataUi& parentUI, Ui_ItemSelectionUI& itemSelectionDialog);
  void setupNew(Ui_TileDataUi& parentUI, Ui_ItemSelectionUI& itemSelectionDialog);
  void createActions();
  void createZoneButtons(); /// dynamically create Buttons for assignable zones from the Zones enum
  void createStyleButtons(); /// dynamically create Buttons for assignable styles from the Style enum
  void createWealthButtons(); /// dynamically create Buttons for assignable walth classes from the Wealth enum
  void fillTileTypeDropdown(); /// dynamically create Buttons forall available TileTypes
  void toggleActiveZoneButtons(const std::vector<Zones>& data); /// when an item is loaded, check all zones button that are assigned in the json
  void toggleActiveStyleButtons(const std::vector<Style>& data); /// when an item is loaded, check all style button that are assigned in the json
  void toggleActiveWealthButtons(const std::vector<Wealth>& data); /// when an item is loaded, check all wealth button that are assigned in the json
  QPixmap preparePixMap(const Ui_TileSetDataUi& ui);

  QTreeWidgetItem* newTreeRootItem(const TileData& tile);
  QTreeWidgetItem* newTreeItem(const TileData& tile);
  void addItem(const TileData& tile);
  void ensureUniqueId(TileData& tile);
  void writeToTileData(TileData& tile);
  void readFromTileData(const TileData& tile);
  std::vector<Zones> ZonesEnumVectorFromString(QString zones);
  std::vector<Zones> ZonesEnumVectorFromButtons();
  std::vector<Style> StyleEnumVectorFromButtons();
  std::vector<Wealth> WealthEnumVectorFromButtons();
  std::string ZonesEnumVectorToString(const std::vector<Zones>& data);
  void fillTileSetDataWidget(const Ui_TileSetDataUi& ui, const TileSetData& data);
  void readTileSetDataWidget(const Ui_TileSetDataUi& ui, TileSetData& data);
  QJsonObject tileSetDataToJson(const TileSetData& data);
  int zoomLevel = 1; // zoomLevel for the tile preview. 1x, 2x, 4x or 0 == auto
private: // members
  TileDataContainer tileContainer;
  QTreeWidget* tree;
  QSplitter* splitter;
  QWidget* biomeSelector;
  QWidget* groundDecorationSelector;
  Ui_TileSetDataUi tilesSet;
  Ui_TileSetDataUi shoreTileSet;
  Ui_TileSetDataUi slopeSet;
  Ui_TileDataUi ui;
  Ui_ItemSelectionUI itemSelection;
  

protected:
  virtual void keyPressEvent(QKeyEvent*);
};

#endif
