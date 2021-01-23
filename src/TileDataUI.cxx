#include "TileDataUI.hxx"

#include <QtWidgets/QToolBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QAction>
#include <QtCore/QMap>
#include <QtGui/QIcon>
#include <QtCore/QSettings>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>
#include <QtWidgets/QHeaderView>
#include <QtCore/QSignalBlocker>


#include "Cytopia/src/engine/basics/tileData.hxx"

//--------------------------------------------------------------------------------

TileDataUI::TileDataUI()
{
  splitter = new QSplitter(this);

  tree = new QTreeWidget;
  tree->header()->hide();
  connect(tree, &QTreeWidget::currentItemChanged, this, &TileDataUI::itemSelected);
  splitter->addWidget(tree);

  QWidget* w = new QWidget;
  ui.setupUi(w);
  splitter->addWidget(w);

  ui.id->setMaxLength(TD_ID_MAX_CHARS);
  ui.category->setMaxLength(TD_CATEGORY_MAX_CHARS);
  ui.subCategory->setMaxLength(TD_SUBCATEGORY_MAX_CHARS);
  ui.title->setMaxLength(TD_TITLE_MAX_CHARS);
  ui.author->setMaxLength(TD_AUTHOR_MAX_CHARS);
  ui.buildCost->setRange(TD_PRICE_MIN, TD_PRICE_MAX);
  ui.upkeepCost->setRange(TD_UPKEEP_MIN, TD_UPKEEP_MAX);
  ui.powerProduction->setRange(TD_POWER_MIN, TD_POWER_MAX);
  ui.waterProduction->setRange(TD_WATER_MIN, TD_WATER_MAX);
  ui.requiredTilesHeight->setRange(TD_REQUIREDTILES_MIN, TD_REQUIREDTILES_MAX);
  ui.requiredTilesWidth->setRange(TD_REQUIREDTILES_MIN, TD_REQUIREDTILES_MAX);

  w = new QWidget;
  tilesSet.setupUi(w);
  setup(tilesSet);
  ui.tabWidget->addTab(w, tr("Tiles"));

  w = new QWidget;
  cornerSet.setupUi(w);
  setup(cornerSet);
  ui.tabWidget->addTab(w, tr("Corner"));

  w = new QWidget;
  slopeSet.setupUi(w);
  setup(slopeSet);
  ui.tabWidget->addTab(w, tr("Slope"));

  setCentralWidget(splitter);

  createActions();

  QSettings settings("JimmySnails", "Cytopia");
  restoreGeometry(settings.value("main/geometry").toByteArray());
  restoreState(settings.value("main/windowState").toByteArray());
  splitter->restoreState(settings.value("main/splitter").toByteArray());
}

//--------------------------------------------------------------------------------

void TileDataUI::createActions()
{
  QMenu* fileMenu = menuBar()->addMenu(tr("File"));
  QMenu* editMenu = menuBar()->addMenu(tr("Edit"));
  QMenu* helpMenu = menuBar()->addMenu(tr("Help"));

  QToolBar* toolBar = new QToolBar(this);
  toolBar->setObjectName("ToolBar");
  addToolBar(toolBar);

  // --- file menu ---
  QAction* action = new QAction(tr("Save"), this);
  action->setIcon(QIcon::fromTheme("document-save"));
  action->setShortcut(QKeySequence::Save);
  connect(action, &QAction::triggered, this, &TileDataUI::saveTileData);
  toolBar->addAction(action);
  fileMenu->addAction(action);

  action = new QAction(tr("Save and Quit"), this);
  action->setIcon(QIcon::fromTheme("application-exit"));
  action->setShortcut(QKeySequence::Quit);
  connect(action, &QAction::triggered, this, &TileDataUI::close);
  toolBar->addAction(action);
  fileMenu->addAction(action);

  // --- edit menu ---
  action = new QAction(tr("New Item"), this);
  action->setIcon(QIcon::fromTheme("document-new"));
  action->setShortcut(QKeySequence::New);
  connect(action, &QAction::triggered, this, &TileDataUI::newItem);
  toolBar->addAction(action);
  editMenu->addAction(action);

  action = new QAction(tr("Delete Item"), this);
  action->setIcon(QIcon::fromTheme("edit-delete"));
  action->setShortcut(QKeySequence::Cut);
  connect(action, &QAction::triggered, this, &TileDataUI::deleteItem);
  toolBar->addAction(action);
  editMenu->addAction(action);

  action = new QAction(tr("Duplicate Item"), this);
  action->setIcon(QIcon::fromTheme("edit-copy"));
  action->setShortcut(QKeySequence("CTRL+D"));
  connect(action, &QAction::triggered, this, &TileDataUI::duplicateItem);
  toolBar->addAction(action);
  editMenu->addAction(action);

  // --- help menu ---
  action = new QAction(tr("About Qt"), this);
  connect(action, &QAction::triggered, this, &QApplication::aboutQt);
  helpMenu->addAction(action);

  // --- create buttons ---
  createZoneButtons();
  createStyleButtons();
}

//--------------------------------------------------------------------------------

void TileDataUI::closeEvent(QCloseEvent* event)
{
  saveTileData();

  QSettings settings("JimmySnails", "Cytopia");
  settings.setValue("main/geometry", saveGeometry());
  settings.setValue("main/windowState", saveState());
  settings.setValue("main/splitter", splitter->saveState());

  QMainWindow::closeEvent(event);
}

//--------------------------------------------------------------------------------

void TileDataUI::setup(Ui_TileSetDataUi& ui)
{
  connect(ui.fileButton, &QPushButton::clicked, this, [ui]() {
    QString fileName = QFileDialog::getOpenFileName(ui.fileButton, tr("Select Image"), ui.fileName->text(), tr("Images (*.png)"));
    if (!fileName.isEmpty())
    {
      ui.fileName->setText(QDir::current().relativeFilePath(fileName));
      QPixmap pix(fileName);
      ui.origImage->setPixmap(pix);
      ui.image->setPixmap(pix);
      ui.width->setValue(pix.width());
      ui.height->setValue(pix.height());
      ui.imageSize->setText(tr("(%1 x %2)").arg(pix.width()).arg(pix.height()));
      ui.imageSize->show();
      ui.size1->setChecked(true);
      ui.deleteButton->setEnabled(true);

      // recalc width based on current count
      ui.width->setValue(ui.origImage->pixmap()->width() / ui.count->value());
    }
    });

  connect(ui.deleteButton, &QPushButton::clicked, this, [ui, this]() {
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Delete Image"), tr("Shall the image really be deleted?"));

    if (ret == QMessageBox::No)
      return;

    ui.fileName->setText(QString());
    QPixmap pix;
    ui.origImage->setPixmap(pix);
    ui.image->setPixmap(pix);
    ui.width->setValue(pix.width());
    ui.height->setValue(pix.height());
    ui.imageSize->hide();
    ui.size1->setChecked(true);
    ui.deleteButton->setEnabled(false);
    });

  connect(ui.count, QOverload<int>::of(&QSpinBox::valueChanged), this, [ui](int value) {
    if (!ui.origImage->pixmap() || (value == 0))
      return;

    ui.width->setValue(ui.origImage->pixmap()->width() / value);
    });

  ui.origImage->hide(); // a hidden storage for the original sized pixmap

  connect(ui.buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this,
    [ui](QAbstractButton* button) {
      if (!ui.origImage->pixmap())
        return;

      QPixmap pix = *(ui.origImage->pixmap());

      if (button == ui.size2)
        pix = pix.transformed(QTransform().scale(2, 2));
      else if (button == ui.size4)
        pix = pix.transformed(QTransform().scale(4, 4));

      ui.image->setPixmap(pix);
    });
}

//--------------------------------------------------------------------------------

bool TileDataUI::loadFile(const QString& fileName)
{
  QString error = tileContainer.loadFile(fileName);
  if (!error.isEmpty())
  {
    QMessageBox::critical(this, tr("Load Error"), tr("Could not load %1. %2").arg(fileName).arg(error));
    return false;
  }

  QMap<QString, QTreeWidgetItem*> categories;
  for (const TileData& tile : tileContainer)
  {
    QString category(QString::fromStdString(tile.category));

    if (!categories.contains(category))
    {
      QTreeWidgetItem* item = newTreeRootItem(tile);
      categories.insert(category, item);
    }
  }

  for (const TileData& tile : tileContainer)
  {
    QString category(QString::fromStdString(tile.category));
    QTreeWidgetItem* root = categories.value(category);

    root->addChild(newTreeItem(tile));
  }

  tree->resizeColumnToContents(0);
  tree->resize(tree->columnWidth(0), tree->height());

  return true;
}

//--------------------------------------------------------------------------------

void TileDataUI::itemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  // store current form values into previous item
  if (previous && previous->data(0, Qt::UserRole).isValid())
  {
    QString id = previous->data(0, Qt::UserRole).toString();

    if (tileContainer.hasTileData(id))
    {
      TileData tile = tileContainer.getTileData(id);
      tileContainer.removeTileData(id);
      writeToTileData(tile);
      tileContainer.addTileData(tile);

      // could be different from what is seen in ui widgets since writeToTileData() adjusts
      QString tileCategory = QString::fromStdString(tile.category);
      QString tileId = QString::fromStdString(tile.id);

      // also update tree
      previous->setText(0, ui.title->text());
      previous->setData(0, Qt::UserRole, tileId);

      // if category changed, move item
      QTreeWidgetItem* root = previous->parent();
      if (tileCategory != root->text(0))
      {
        QSignalBlocker blocker(tree); // avoid recursive call of itemSelected

        // different from current when called from saveTileData()
        QTreeWidgetItem* currentItem = tree->currentItem();

        int idx = root->indexOfChild(previous);
        QTreeWidgetItem* item = root->takeChild(idx);
        if (root->childCount() == 0)
        {
          if (current == root)
            current = nullptr;

          delete root;
        }

        // find new category
        for (int i = 0; i < tree->topLevelItemCount(); i++)
        {
          if (tree->topLevelItem(i)->text(0) == tileCategory) // new parent found
          {
            tree->topLevelItem(i)->addChild(item);
            item = nullptr;
            break;
          }
        }

        if (item) // only if no toplevel category node found - create a new one
        {
          QTreeWidgetItem* root = newTreeRootItem(tile);
          root->addChild(item);
        }

        tree->setCurrentItem(currentItem);
      }
    }
  }

  if (!current || !current->data(0, Qt::UserRole).isValid())
    return;

  // show current item values
  TileData tile = tileContainer.getTileData(current->data(0, Qt::UserRole).toString());
  readFromTileData(tile);
}

//--------------------------------------------------------------------------------

void TileDataUI::ensureUniqueId(TileData& tile)
{
  if (tile.category.empty())
    tile.category = "unknown category";

  if (tile.id.empty())
    tile.id = tile.category + "1";

  QString id = QString::fromStdString(tile.id);

  if (tileContainer.hasTileData(id)) // non-unique; crete a new one
  {
    // find trailing number
    int i;
    for (i = id.length() - 1; id[i].isNumber() && (i > 0); i--)
      ;

    int num = id.mid(i + 1).toInt();
    QString newId;
    do
    {
      num++;
      newId = id.left(i + 1) + QString::number(num);
    } while (tileContainer.hasTileData(newId));

    tile.id = newId.toStdString();
  }
}

//--------------------------------------------------------------------------------

void TileDataUI::writeToTileData(TileData& tile)
{
  tile.id = ui.id->text().toStdString();
  tile.category = ui.category->text().toStdString();
  tile.subCategory = ui.subCategory->text().toStdString();
  tile.title = ui.title->text().toStdString();
  tile.description = ui.description->toPlainText().toStdString();
  tile.author = ui.author->text().toStdString();
  tile.power = ui.powerProduction->value();
  tile.water = ui.waterProduction->value();
  tile.upkeepCost = ui.upkeepCost->value();
  tile.price = ui.buildCost->value();
  tile.inhabitants = ui.inhabitants->value();
  tile.pollutionLevel = ui.pollutionLevel->value();
  tile.fireHazardLevel = ui.fireHazardLevel->value();
  tile.educationLevel = ui.educationLevel->value();
  tile.crimeLevel = ui.crimeLevel->value();
  tile.happyness = ui.happyness->value();
  tile.isOverPlacable = ui.isOverPlacable->checkState();
  tile.placeOnGround = ui.placeOnGround->checkState();
  tile.placeOnWater = ui.placeOnWater->checkState();
  tile.RequiredTiles.height = static_cast<unsigned int>(ui.requiredTilesHeight->value());
  tile.RequiredTiles.width = static_cast<unsigned int>(ui.requiredTilesWidth->value());

  tile.zones = ZonesEnumVectorFromButtons();
  tile.style = StyleEnumVectorFromButtons();

  commaSeperatedStringToVector(ui.tags->text().toStdString(), tile.tags, ",");

  readTileSetDataWidget(tilesSet, tile.tiles);
  readTileSetDataWidget(cornerSet, tile.shoreTiles);
  readTileSetDataWidget(slopeSet, tile.slopeTiles);

  ensureUniqueId(tile);
  readFromTileData(tile); // might have been changed in ensureUniqueId()
}

//--------------------------------------------------------------------------------

void TileDataUI::readFromTileData(const TileData& tile)
{
  ui.id->setText(QString::fromStdString(tile.id));
  ui.category->setText(QString::fromStdString(tile.category));
  ui.subCategory->setText(QString::fromStdString(tile.subCategory));
  ui.title->setText(QString::fromStdString(tile.title));
  ui.description->setPlainText(QString::fromStdString(tile.description));
  ui.author->setText(QString::fromStdString(tile.author));

  ui.powerProduction->setValue(tile.power);
  ui.waterProduction->setValue(tile.water);
  ui.upkeepCost->setValue(tile.upkeepCost);
  ui.buildCost->setValue(tile.price);
  ui.inhabitants->setValue(tile.inhabitants);
  ui.pollutionLevel->setValue(tile.pollutionLevel);
  ui.fireHazardLevel->setValue(tile.fireHazardLevel);
  ui.educationLevel->setValue(tile.educationLevel);
  ui.crimeLevel->setValue(tile.crimeLevel);
  ui.happyness->setValue(tile.happyness);
  ui.isOverPlacable->setChecked(tile.isOverPlacable);
  ui.placeOnWater->setChecked(tile.placeOnWater);
  ui.placeOnGround->setChecked(tile.placeOnGround);
  ui.requiredTilesHeight->setValue(tile.RequiredTiles.height);
  ui.requiredTilesWidth->setValue(tile.RequiredTiles.width);

  // present tags as a comma seperated string
  ui.tags->setText(QString::fromStdString(commaSeperateVector(tile.tags, ",")));

  fillTileSetDataWidget(tilesSet, tile.tiles);
  fillTileSetDataWidget(cornerSet, tile.shoreTiles);
  fillTileSetDataWidget(slopeSet, tile.slopeTiles);

  toggleActiveZoneButtons(tile.zones);
  toggleActiveStyleButtons(tile.style);

}

//------------------------ Zones -------------------------------------------------

std::string TileDataUI::ZonesEnumVectorToString(const std::vector<Zones>& data)
{
  std::vector<std::string> zones;

  for (const Zones zone : data)
  {
    zones.push_back(zone._to_string());
  }
  return commaSeperateVector(zones);
}

std::vector<Zones> TileDataUI::ZonesEnumVectorFromString(QString zones)
{
  std::vector<std::string> zoneNames;
  commaSeperatedStringToVector(zones.toStdString(), zoneNames);
  std::vector<Zones> result;
  for (const std::string& zoneName : zoneNames)
  {
    result.push_back(Zones::_from_string_nocase(zoneName.c_str()));
  }
  return result;
}

std::vector<Zones> TileDataUI::ZonesEnumVectorFromButtons()
{
  std::vector<Zones> result;

  for (int i = 0; i < ui.zoneButtonsHorizontalLayout->count(); i++)
  {
    QPushButton* myButton = dynamic_cast<QPushButton*>(ui.zoneButtonsHorizontalLayout->itemAt(i)->widget());

    if (myButton->isChecked())
    {
      result.push_back(Zones::_from_string_nocase(myButton->objectName().toStdString().c_str()));
    }
  }
  return result;
}

void TileDataUI::createZoneButtons()
{
  // Iterate over all Enum values as strings
  for (const auto zone : Zones::_names())
  {
    QPushButton* button = new QPushButton(QString::fromStdString(zone));
    button->setCheckable(true);
    button->setObjectName(QString::fromStdString(zone));
    ui.zoneButtonsHorizontalLayout->addWidget(button);
  }
}

void TileDataUI::toggleActiveZoneButtons(const std::vector<Zones>& data)
{
  // iterate over all buttons
  for (int i = 0; i < ui.zoneButtonsHorizontalLayout->count(); i++)
  {
    // grab buttons from the horizontal layout they are located at.
    QPushButton* myButton = dynamic_cast<QPushButton*>(ui.zoneButtonsHorizontalLayout->itemAt(i)->widget());
    myButton->setChecked(false); // reset button
    // iterate over all active zones for this tile
    for (const Zones& zone : data)
    {
      if (myButton->objectName().toStdString().find(zone._to_string()) != std::string::npos)
      {
        // if it matches, check the button
        myButton->setChecked(true);
      }
    }
  }
}

//-------------------------- Styles ---------------------------------------------
void TileDataUI::createStyleButtons()
{
  for (const auto style : Style::_names())
  {
    QPushButton* button = new QPushButton(QString::fromStdString(style));
    button->setCheckable(true);
    button->setObjectName(QString::fromStdString(style));
    ui.stylesButtonsHorizontalLayout->addWidget(button);
  }
}

void TileDataUI::toggleActiveStyleButtons(const std::vector<Style>& data)
{
  // iterate over all buttons
  for (int i = 0; i < ui.stylesButtonsHorizontalLayout->count(); i++)
  {
    // grab buttons from the horizontal layout they are located at.
    QPushButton* myButton = dynamic_cast<QPushButton*>(ui.stylesButtonsHorizontalLayout->itemAt(i)->widget());
    myButton->setChecked(false); // reset button
    // iterate over all active styles for this tile
    for (const Style& style : data)
    {
      if (myButton->objectName().toStdString().find(style._to_string()) != std::string::npos)
      {
        // if it matches, check the button
        myButton->setChecked(true);
      }
    }
  }
}

std::vector<Style> TileDataUI::StyleEnumVectorFromButtons()
{
  std::vector<Style> result;

  for (int i = 0; i < ui.stylesButtonsHorizontalLayout->count(); i++)
  {
    QPushButton* myButton = dynamic_cast<QPushButton*>(ui.stylesButtonsHorizontalLayout->itemAt(i)->widget());

    if (myButton->isChecked())
    {
      result.push_back(Style::_from_string_nocase(myButton->objectName().toStdString().c_str()));
    }
  }
  return result;
}

//--------------------------------------------------------------------------------

void TileDataUI::fillTileSetDataWidget(const Ui_TileSetDataUi& ui, const TileSetData& data)
{
  ui.fileName->setText(QString::fromStdString(data.fileName));
  QPixmap pix(QString::fromStdString(data.fileName));
  ui.image->setPixmap(pix);
  ui.origImage->setPixmap(pix);
  ui.imageSize->setText(tr("(%1 x %2)").arg(pix.width()).arg(pix.height()));
  ui.imageSize->setVisible(!pix.isNull());
  ui.size1->setChecked(true);
  ui.width->setValue(data.clippingWidth);
  ui.height->setValue(data.clippingHeight);
  ui.count->setValue(data.count);

  ui.deleteButton->setEnabled(!pix.isNull());
}

//--------------------------------------------------------------------------------

void TileDataUI::readTileSetDataWidget(const Ui_TileSetDataUi& ui, TileSetData& data)
{
  data.fileName = ui.fileName->text().toStdString();
  data.clippingWidth = ui.width->value();
  data.clippingHeight = ui.height->value();
  data.count = ui.count->value();
}

//--------------------------------------------------------------------------------

void TileDataUI::saveTileData()
{
  if (tree->currentItem())
    itemSelected(nullptr, tree->currentItem()); // ensure current widget content is stored into TileData

  tileContainer.saveFile();
}

//--------------------------------------------------------------------------------

QTreeWidgetItem* TileDataUI::newTreeRootItem(const TileData& tile)
{
  QTreeWidgetItem* root = new QTreeWidgetItem(tree);
  root->setIcon(0, QIcon::fromTheme("folder"));
  root->setText(0, QString::fromStdString(tile.category));
  root->setExpanded(true);

  return root;
}

//--------------------------------------------------------------------------------

QTreeWidgetItem* TileDataUI::newTreeItem(const TileData& tile)
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setIcon(0, QIcon::fromTheme("text-x-generic"));
  item->setText(0, QString::fromStdString(tile.title));
  item->setData(0, Qt::UserRole, QString::fromStdString(tile.id));

  return item;
}

//--------------------------------------------------------------------------------

void TileDataUI::newItem()
{
  TileData tile;

  if (tree->currentItem()) // ensure current widget content is stored into TileData
  {
    if (tree->currentItem()->data(0, Qt::UserRole).isValid()) // item, not root
    {
      QString id = tree->currentItem()->data(0, Qt::UserRole).toString();
      tile.category = tileContainer.getTileData(id).category;
    }
    else
      tile.category = tree->currentItem()->text(0).toStdString(); // root

    tree->setCurrentItem(nullptr); // and select nothing (this calls itemSelected)
  }

  // get new unique id
  ensureUniqueId(tile);

  if (tile.title.empty())
    tile.title = "new " + tile.category;

  addItem(tile);
}

//--------------------------------------------------------------------------------

void TileDataUI::addItem(const TileData& tile)
{
  tileContainer.addTileData(tile);

  // show in tree
  QTreeWidgetItem* root = nullptr;
  for (int i = 0; i < tree->topLevelItemCount(); i++)
  {
    if (tree->topLevelItem(i)->text(0) == QString::fromStdString(tile.category)) // new parent found
    {
      root = tree->topLevelItem(i);
      break;
    }
  }

  if (!root)
    root = newTreeRootItem(tile);

  QTreeWidgetItem* item = newTreeItem(tile);
  root->addChild(item);
  tree->setCurrentItem(item);
}

//--------------------------------------------------------------------------------

void TileDataUI::deleteItem()
{
  if (!tree->currentItem() || !tree->currentItem()->data(0, Qt::UserRole).isValid())
    return;

  QString id = tree->currentItem()->data(0, Qt::UserRole).toString();
  TileData tile = tileContainer.getTileData(id);

  QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Delete Item"),
    tr("Shall the item '%1/%2' be deleted?")
    .arg(QString::fromStdString(tile.category))
    .arg(QString::fromStdString(tile.title)));

  if (ret == QMessageBox::No)
    return;

  tileContainer.removeTileData(id);
  QTreeWidgetItem* root = tree->currentItem()->parent();
  delete tree->currentItem();
  tree->setCurrentItem(nullptr); // and select nothing

  if (root->childCount() == 0)
    delete root;
}

//--------------------------------------------------------------------------------

void TileDataUI::duplicateItem()
{
  if (!tree->currentItem() || !tree->currentItem()->data(0, Qt::UserRole).isValid())
    return;

  QString id = tree->currentItem()->data(0, Qt::UserRole).toString();
  TileData tile = tileContainer.getTileData(id);

  // get new unique id
  ensureUniqueId(tile);

  addItem(tile);
}

//--------------------------------------------------------------------------------


