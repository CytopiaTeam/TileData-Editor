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
#include <QPainter>
#include <QDialog>


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

  biomeSelector = new QWidget(this, Qt::Popup | Qt::Dialog);;

  itemSelection.setupUi(biomeSelector);
  setupNew(ui, itemSelection);

  groundDecorationSelector = new QWidget(this, Qt::Popup | Qt::Dialog);;

  itemSelection.setupUi(groundDecorationSelector);
  setupNew(ui, itemSelection);

  ui.id->setMaxLength(TD_ID_MAX_CHARS);
  ui.category->setMaxLength(TD_CATEGORY_MAX_CHARS);
  ui.subCategory->setMaxLength(TD_SUBCATEGORY_MAX_CHARS);
  ui.title->setMaxLength(TD_TITLE_MAX_CHARS);
  ui.author->setMaxLength(TD_AUTHOR_MAX_CHARS);
  ui.buildCost->setRange(TD_PRICE_MIN, TD_PRICE_MAX);
  ui.upkeepCost->setRange(TD_UPKEEP_MIN, TD_UPKEEP_MAX);
  ui.educationLevel->setRange(TD_EDUCATION_MIN, TD_EDUCATION_MAX);
  ui.fireHazardLevel->setRange(TD_FIREDANGER_MIN, TD_FIREDANGER_MAX);
  ui.crimeLevel->setRange(TD_CRIME_MIN, TD_CRIME_MAX);
  ui.pollutionLevel->setRange(TD_POLLUTION_MIN, TD_POLLUTION_MAX);
  ui.powerProduction->setRange(TD_POWER_MIN, TD_POWER_MAX);
  ui.waterProduction->setRange(TD_WATER_MIN, TD_WATER_MAX);
  ui.happiness->setRange(TD_HAPPINESS_MIN, TD_HAPPINESS_MAX);
  ui.requiredTilesHeight->setRange(TD_REQUIREDTILES_MIN, TD_REQUIREDTILES_MAX);
  ui.requiredTilesWidth->setRange(TD_REQUIREDTILES_MIN, TD_REQUIREDTILES_MAX);
  ui.inhabitants->setRange(TD_HABITANTS_MIN, TD_HABITANTS_MAX);

  w = new QWidget;
  tilesSet.setupUi(w);
  setup(tilesSet, ui, itemSelection);
  ui.tabWidget->addTab(w, tr("Tiles"));


  w = new QWidget;
  shoreTileSet.setupUi(w);
  setup(shoreTileSet, ui, itemSelection);
  ui.tabWidget->addTab(w, tr("ShoreLines"));

  w = new QWidget;
  slopeSet.setupUi(w);
  setup(slopeSet, ui, itemSelection);
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
  createWealthButtons();
  fillTileTypeDropdown();
}

//--------------------------------------------------------------------------------

void TileDataUI::closeEvent(QCloseEvent* event)
{
  saveTileData();

  QSettings settings("SimplyLiz", "Cytopia");
  settings.setValue("main/geometry", saveGeometry());
  settings.setValue("main/windowState", saveState());
  settings.setValue("main/splitter", splitter->saveState());

  QMainWindow::closeEvent(event);
}

//--------------------------------------------------------------------------------

void TileDataUI::setupNew(Ui_TileDataUi& parentUI, Ui_ItemSelectionUI& itemSelectionDialog)
{
  connect(parentUI.BiomeButton, QOverload<bool>::of(&QPushButton::clicked), this, [parentUI, itemSelectionDialog, this]() {

    itemSelectionDialog.availableItems->clear();
    itemSelectionDialog.usedItems->clear();

    std::vector<std::string>biomesUsed;
    commaSeperatedStringToVector(parentUI.biomes->text().toStdString(), biomesUsed);

    for (const QString& biome : tileContainer.getBiomes())
    {
      bool found = false;
      for (const std::string& usedBiome : biomesUsed)
      {
        if (usedBiome == biome.toStdString())
        {
          itemSelectionDialog.usedItems->addItem(biome);
          found = true;
        }
      }
      if (!found)
      {
        itemSelectionDialog.availableItems->addItem(biome);
      }
    }

    itemSelectionDialog.listOfLabel->setText("Available Biomes");
    itemSelectionDialog.usedLabel->setText("Used Biomes");
    biomeSelector->setWindowTitle("Select Biome");
    biomeSelector->setWindowModality(Qt::WindowModal);
    biomeSelector->show();

    });

  connect(parentUI.groundDecorationButton, QOverload<bool>::of(&QPushButton::clicked), this, [parentUI, itemSelectionDialog, this]() {

    itemSelectionDialog.availableItems->clear();
    itemSelectionDialog.usedItems->clear();

    std::vector<std::string>groundDecorationUsed;
    commaSeperatedStringToVector(parentUI.groundDecoration->text().toStdString(), groundDecorationUsed);

    for (const QString& groundDecoration : tileContainer.getAllGroundDecorationIDs())
    {
      bool found = false;
      for (const auto& usedDecoration : groundDecorationUsed)
      {
        if (usedDecoration == groundDecoration.toStdString())
        {
          itemSelectionDialog.usedItems->addItem(groundDecoration);
          found = true;
        }
      }
      if (!found)
      {
        itemSelectionDialog.availableItems->addItem(groundDecoration);
      }
    }

    groundDecorationSelector->setWindowTitle("Select GroundDecoration");
    itemSelectionDialog.listOfLabel->setText("Available GroundDecoration");
    itemSelectionDialog.usedLabel->setText("Used GroundDecoration");
    groundDecorationSelector->show();

    });

  connect(itemSelectionDialog.useItemButton, QOverload<bool>::of(&QPushButton::clicked), this, [parentUI, itemSelectionDialog, this]() {
    if (!itemSelectionDialog.availableItems->currentItem())
      return;

    QListWidgetItem* widget = itemSelectionDialog.availableItems->currentItem()->clone();
    itemSelectionDialog.usedItems->addItem(widget);
    itemSelectionDialog.usedItems->setCurrentItem(widget);
    delete itemSelectionDialog.availableItems->currentItem();
    });

  connect(itemSelectionDialog.clearButton, QOverload<bool>::of(&QPushButton::clicked), this, [itemSelectionDialog, this]() {
    // store count as it will change when we manipulate the list
    int count = itemSelectionDialog.usedItems->count() - 1;
    // iterate over all items, starting backwards to avoid index out of range
    for (int i = count; i >= 0; --i)
    {
      // we have to clone the widget, moving is not possible
      QListWidgetItem* widget = itemSelectionDialog.usedItems->item(i)->clone();
      if (widget) // better be safe than sorry
      {
        itemSelectionDialog.availableItems->addItem(widget); // add the cloned item to the list
        itemSelectionDialog.usedItems->takeItem(i); // remove the old item we found and cloned
      }
    }
    });

  connect(itemSelectionDialog.addAllButton, QOverload<bool>::of(&QPushButton::clicked), this, [itemSelectionDialog, this]() {
    // store count as it will change when we manipulate the list
    int count = itemSelectionDialog.availableItems->count() - 1;
    // iterate over all items, starting backwards to avoid index out of range
    for (int i = count; i >= 0; --i)
    {
      // we have to clone the widget, moving is not possible
      QListWidgetItem* widget = itemSelectionDialog.availableItems->item(i)->clone();
      if (widget) // better be safe than sorry
      {
        itemSelectionDialog.usedItems->addItem(widget); // add the cloned item to the list
        itemSelectionDialog.availableItems->takeItem(i); // remove the old item we found and cloned
      }
    }
    });

  connect(itemSelectionDialog.okButton, QOverload<bool>::of(&QPushButton::clicked), this, [parentUI, itemSelectionDialog, this]() {
    QString items;
    if (itemSelectionDialog.usedItems->count() > 0)
    {
      for (int i = 0; i < itemSelectionDialog.usedItems->count(); i++)
      {
        items += itemSelectionDialog.usedItems->item(i)->text();
        if (itemSelectionDialog.usedItems->count() - i > 1)
        {
          items += ",";
        }
      }
    }

    if (biomeSelector->isVisible())
    {
      biomeSelector->hide();
      parentUI.biomes->setText(items);
    }
    else if (groundDecorationSelector->isVisible())
    {
      groundDecorationSelector->hide();
      parentUI.groundDecoration->setText(items);
    }
    else
    {
      qInfo() << "Error! Clicked OK on an unimplemented DIalog!";
    }
    });

  connect(itemSelectionDialog.cancelButton, QOverload<bool>::of(&QPushButton::clicked), this, [parentUI, itemSelectionDialog, this]() {
    biomeSelector->hide();
    groundDecorationSelector->hide();
    });

  connect(itemSelectionDialog.removeItemButton, QOverload<bool>::of(&QPushButton::clicked), this, [parentUI, itemSelectionDialog, this]() {
    if (!itemSelectionDialog.usedItems->currentItem())
      return;

    QListWidgetItem* widget = itemSelectionDialog.usedItems->currentItem()->clone();
    itemSelectionDialog.availableItems->addItem(widget);
    itemSelectionDialog.availableItems->setCurrentItem(widget);
    delete itemSelectionDialog.usedItems->currentItem();
    });
}

//--------------------------------------------------------------------------------

void TileDataUI::setup(Ui_TileSetDataUi& ui, Ui_TileDataUi& parentUI, Ui_ItemSelectionUI& itemSelectionDialog)
{
  connect(ui.fileButton, &QPushButton::clicked, this, [ui, this]() {
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
      int spriteSheetLength = ui.origImage->pixmap()->width();
      int numOffset = abs(ui.offset->value());
      int numCount = ui.count->value();
      int singleTile_width = 0;

      // calculate length of one single tile
      if (numOffset > 0)
      {
        singleTile_width = (spriteSheetLength / (numOffset + numCount));
      }
      else
      {
        singleTile_width = spriteSheetLength / numCount;
      }
      ui.width->setValue(singleTile_width);
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

  connect(parentUI.TileTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [ui, parentUI, this](int value) {
    int numCount = ui.count->value();
    // get parent ui to access tile type combobox

    TileType tileType = TileType::_from_index(parentUI.TileTypeComboBox->currentIndex());

    if (numCount > 1 && (tileType != +TileType::AUTOTILE && tileType != +TileType::ROAD && tileType != +TileType::UNDERGROUND))
    {
      ui.pickRandomTile->setChecked(true);
    }
    else
    {
      ui.pickRandomTile->setChecked(false);
    }
    // disable buttons if necessary
    if (TileType::_from_integral(tileType == +TileType::RCI))
    {
      for (int i = 0; i < parentUI.zoneButtonsHorizontalLayout->count(); ++i)
      {
        dynamic_cast<QPushButton*>(parentUI.zoneButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(true);
      }
      for (int i = 0; i < parentUI.wealthButtonsHorizontalLayout->count(); ++i)
      {
        dynamic_cast<QPushButton*>(parentUI.wealthButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(true);
      }
      for (int i = 0; i < parentUI.stylesButtonsHorizontalLayout->count(); ++i)
      {
        dynamic_cast<QPushButton*>(parentUI.stylesButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(true);
      }
    }
    else
    {
      for (int i = 0; i < parentUI.zoneButtonsHorizontalLayout->count(); ++i)
      {
        dynamic_cast<QPushButton*>(parentUI.zoneButtonsHorizontalLayout->itemAt(i)->widget())->setChecked(false);
        dynamic_cast<QPushButton*>(parentUI.zoneButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(false);
      }
      for (int i = 0; i < parentUI.wealthButtonsHorizontalLayout->count(); ++i)
      {
        dynamic_cast<QPushButton*>(parentUI.wealthButtonsHorizontalLayout->itemAt(i)->widget())->setChecked(false);
        dynamic_cast<QPushButton*>(parentUI.wealthButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(false);
      }
      for (int i = 0; i < parentUI.stylesButtonsHorizontalLayout->count(); ++i)
      {
        dynamic_cast<QPushButton*>(parentUI.stylesButtonsHorizontalLayout->itemAt(i)->widget())->setChecked(false);
        dynamic_cast<QPushButton*>(parentUI.stylesButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(false);
      }
    }
    });

  connect(ui.count, QOverload<int>::of(&QSpinBox::valueChanged), this, [ui, parentUI, this](int value) {
    if (!ui.origImage->pixmap() || !ui.image->pixmap())
      return;

    TileType tileType = TileType::_from_index(parentUI.TileTypeComboBox->currentIndex());

    int spriteSheetLength = ui.origImage->pixmap()->width();
    int numOffset = abs(ui.offset->value());
    int numCount = ui.count->value();
    int singleTile_width = 0;

    // check/uncheck pickRandomTile checkbox
    if (numCount > 1 && (tileType != +TileType::AUTOTILE && tileType != +TileType::ROAD && tileType != +TileType::UNDERGROUND))
    {
      ui.pickRandomTile->setChecked(true);
    }
    else
    {
      ui.pickRandomTile->setChecked(false);
    }

    // calculate length of one single tile
    if (numOffset > 0)
    {
      singleTile_width = (spriteSheetLength / (numOffset + numCount));
    }
    else
    {
      singleTile_width = spriteSheetLength / numCount;
    }
    ui.width->setValue(singleTile_width);

    // set tileSetPreview selection boxes
    ui.image->setPixmap(preparePixMap(ui));
    });

  connect(ui.offset, QOverload<int>::of(&QSpinBox::valueChanged), this, [ui, this](int value) {
    if (!ui.origImage->pixmap() || !ui.image->pixmap())
      return;

    int spriteSheetLength = ui.origImage->pixmap()->width();
    int numOffset = abs(ui.offset->value());
    int numCount = ui.count->value();
    int singleTile_width = 0;

    // calculate length of one single tile
    if (numOffset > 0)
    {
      singleTile_width = (spriteSheetLength / (numOffset + numCount));
    }
    else
    {
      singleTile_width = spriteSheetLength / numCount;
    }
    ui.width->setValue(singleTile_width);

    // set tileSetPreview selection boxes
    ui.image->setPixmap(preparePixMap(ui));
    });

  ui.origImage->hide(); // a hidden storage for the original sized pixmap

  // Scale buttons
  connect(ui.buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this,
    [this, ui](QAbstractButton* button) {
      if (!ui.origImage->pixmap() || !ui.image->pixmap())
        return;

      // Set the zoomlevel property if a button has changed
      if (ui.buttonGroup->checkedButton() == ui.size2)
        zoomLevel = 2;
      else if (ui.buttonGroup->checkedButton() == ui.size4)
        zoomLevel = 4;
      else if (ui.buttonGroup->checkedButton() == ui.sizeAuto)
        zoomLevel = 0;
      else
        zoomLevel = 1;

      // set tileSetPreview selection boxes
      ui.image->setPixmap(preparePixMap(ui));

    });
}

//--------------------------------------------------------------------------------

bool TileDataUI::loadBiomeData(const QString& fileName)
{
  QString error = tileContainer.getBiomeDataFromFile(fileName);
  return true;
}

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

  // disable buttons if necessary
  if (TileType::_from_integral(ui.TileTypeComboBox->currentIndex()) == +TileType::RCI)
  {
    for (int i = 0; i < ui.zoneButtonsHorizontalLayout->count(); ++i)
    {
      dynamic_cast<QPushButton*>(ui.zoneButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(true);
    }
    for (int i = 0; i < ui.wealthButtonsHorizontalLayout->count(); ++i)
    {
      dynamic_cast<QPushButton*>(ui.wealthButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(true);
    }
    for (int i = 0; i < ui.stylesButtonsHorizontalLayout->count(); ++i)
    {
      dynamic_cast<QPushButton*>(ui.stylesButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(true);
    }
  }
  else
  {
    for (int i = 0; i < ui.zoneButtonsHorizontalLayout->count(); ++i)
    {
      dynamic_cast<QPushButton*>(ui.zoneButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(false);
    }
    for (int i = 0; i < ui.wealthButtonsHorizontalLayout->count(); ++i)
    {
      dynamic_cast<QPushButton*>(ui.wealthButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(false);
    }
    for (int i = 0; i < ui.stylesButtonsHorizontalLayout->count(); ++i)
    {
      dynamic_cast<QPushButton*>(ui.stylesButtonsHorizontalLayout->itemAt(i)->widget())->setEnabled(false);
    }
  }
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
  tile.happiness = ui.happiness->value();
  tile.isOverPlacable = ui.isOverPlacable->checkState();
  tile.placeOnGround = ui.placeOnGround->checkState();
  tile.placeOnWater = ui.placeOnWater->checkState();
  tile.RequiredTiles.height = static_cast<unsigned int>(ui.requiredTilesHeight->value());
  tile.RequiredTiles.width = static_cast<unsigned int>(ui.requiredTilesWidth->value());

  tile.zones = ZonesEnumVectorFromButtons();
  tile.style = StyleEnumVectorFromButtons();
  tile.wealth = WealthEnumVectorFromButtons();
  tile.tileType = TileType::_from_index(ui.TileTypeComboBox->currentIndex());

  commaSeperatedStringToVector(ui.tags->text().toStdString(), tile.tags, ",");
  commaSeperatedStringToVector(ui.biomes->text().toStdString(), tile.biomes, ",");
  commaSeperatedStringToVector(ui.groundDecoration->text().toStdString(), tile.groundDecoration, ",");

  readTileSetDataWidget(tilesSet, tile.tiles);
  readTileSetDataWidget(shoreTileSet, tile.shoreTiles);
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
  ui.happiness->setValue(tile.happiness);
  ui.isOverPlacable->setChecked(tile.isOverPlacable);
  ui.placeOnWater->setChecked(tile.placeOnWater);
  ui.placeOnGround->setChecked(tile.placeOnGround);
  ui.requiredTilesHeight->setValue(tile.RequiredTiles.height);
  ui.requiredTilesWidth->setValue(tile.RequiredTiles.width);

  // present tags as a comma seperated string
  ui.tags->setText(QString::fromStdString(commaSeperateVector(tile.tags, ",")));
  ui.biomes->setText(QString::fromStdString(commaSeperateVector(tile.biomes, ",")));
  ui.groundDecoration->setText(QString::fromStdString(commaSeperateVector(tile.groundDecoration, ",")));

  fillTileSetDataWidget(tilesSet, tile.tiles);
  fillTileSetDataWidget(shoreTileSet, tile.shoreTiles);
  fillTileSetDataWidget(slopeSet, tile.slopeTiles);

  toggleActiveZoneButtons(tile.zones);
  toggleActiveStyleButtons(tile.style);
  toggleActiveWealthButtons(tile.wealth);
  ui.TileTypeComboBox->setCurrentIndex(tile.tileType._to_index());

  // add little preview image
  QPixmap pix(QString::fromStdString(tile.tiles.fileName));
  int w = ui.tilePreview->width();
  int h = ui.tilePreview->height();

  ui.tilePreview->setPixmap(pix.scaled(w, h, Qt::KeepAspectRatio));
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

void TileDataUI::createWealthButtons()
{
  // Iterate over all Enum values as strings
  for (const auto wealth : Wealth::_names())
  {
    QPushButton* button = new QPushButton(QString::fromStdString(wealth));
    button->setCheckable(true);
    button->setObjectName(QString::fromStdString(wealth));
    ui.wealthButtonsHorizontalLayout->addWidget(button);
  }
}

void TileDataUI::fillTileTypeDropdown()
{
  // Iterate over all Enum values as strings
  for (const auto tileType : TileType::_names())
  {
    ui.TileTypeComboBox->addItem(QString::fromStdString(tileType));
  }
}

void TileDataUI::toggleActiveWealthButtons(const std::vector<Wealth>& data)
{
  // iterate over all buttons
  for (int i = 0; i < ui.wealthButtonsHorizontalLayout->count(); i++)
  {
    // grab buttons from the horizontal layout they are located at.
    QPushButton* myButton = dynamic_cast<QPushButton*>(ui.wealthButtonsHorizontalLayout->itemAt(i)->widget());
    myButton->setChecked(false); // reset button
    // iterate over all active wealth for this tile
    for (const Wealth& wealth : data)
    {
      if (myButton->objectName().toStdString().find(wealth._to_string()) != std::string::npos)
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

std::vector<Wealth> TileDataUI::WealthEnumVectorFromButtons()
{
  std::vector<Wealth> result;

  for (int i = 0; i < ui.wealthButtonsHorizontalLayout->count(); i++)
  {
    QPushButton* myButton = dynamic_cast<QPushButton*>(ui.wealthButtonsHorizontalLayout->itemAt(i)->widget());

    if (myButton->isChecked())
    {
      result.push_back(Wealth::_from_string_nocase(myButton->objectName().toStdString().c_str()));
    }
  }
  return result;
}

//--------------------------------------------------------------------------------

void TileDataUI::fillTileSetDataWidget(const Ui_TileSetDataUi& ui, const TileSetData& data)
{
  ui.fileName->setText(QString::fromStdString(data.fileName));
  QPixmap pix(QString::fromStdString(data.fileName));
  ui.origImage->setPixmap(pix);
  ui.image->setPixmap(pix);
  ui.imageSize->setText(tr("(%1 x %2)").arg(pix.width()).arg(pix.height()));
  ui.imageSize->setVisible(!pix.isNull());
  ui.size1->setChecked(true);
  ui.width->setValue(data.clippingWidth);
  ui.height->setValue(data.clippingHeight);
  ui.count->setValue(data.count);
  ui.offset->setValue(data.offset);
  ui.pickRandomTile->setChecked(data.pickRandomTile);

  ui.deleteButton->setEnabled(!pix.isNull());

  switch (zoomLevel)
  {
  case 0: // auto
    ui.sizeAuto->setChecked(true);
    break;
  case 1:
    ui.size1->setChecked(true);
    break;
  case 2:
    ui.size2->setChecked(true);
    break;
  case 4:
    ui.size4->setChecked(true);
    break;
  default:
    break;
  }

  ui.image->setPixmap(preparePixMap(ui)); // set tileSetPreview selection boxes
}

//--------------------------------------------------------------------------------

void TileDataUI::readTileSetDataWidget(const Ui_TileSetDataUi& ui, TileSetData& data)
{
  data.fileName = ui.fileName->text().toStdString();
  data.clippingWidth = ui.width->value();
  data.clippingHeight = ui.height->value();
  data.count = ui.count->value();
  data.offset = ui.offset->value();
  data.pickRandomTile = ui.pickRandomTile->isChecked();
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


QPixmap TileDataUI::preparePixMap(const Ui_TileSetDataUi& ui)
{
  if (ui.origImage->pixmap()->isNull())
  {
    QPixmap emptyPixMap;
    return emptyPixMap;
  }

  int numCount = ui.count->value();
  int numOffset = ui.offset->value();
  // tiledata preview

  QPixmap pix = *(ui.origImage->pixmap());
  QPainter* paint = new QPainter(&pix);

  paint->setPen(QColor(255, 34, 255, 255));
  int width = ui.width->value();
  int height = ui.height->value() - 1;
  int offsetY = pix.height() - ui.height->value(); // in case our height is smaller then the total height, start drawing from bottom up
  int offsetX = 0;

  if (numCount > 1 || numOffset >= 1)
  {
    for (int i = 0; i < numCount; i++)
    {
      if (numOffset >= 0)
      {
        offsetX = (width * numOffset) + (width * i);
      }
      else
      {
        offsetX = (width * i);
      }
      paint->drawRect(offsetX, offsetY, width - 1, height);
    }
  }
  delete paint;

  // values for auto scaling - subtract a few pixels to prevent scrollarea
  int windowWidth = ui.scrollArea->width() - 20;
  int windowHeight = ui.scrollArea->height() - 20;
 
  double XScalefactor = (double)windowWidth / (double)pix.width();
  double YScalefactor = (double)windowHeight / (double)pix.height();

  switch (zoomLevel)
  {
  case 0: //auto 7
    // check if we need to scale the image in the width or height
    if (pix.height() * XScalefactor < windowHeight && pix.width() * YScalefactor > windowWidth) // scale the width to max
    {
      // calculate Height, width = max
      pix = pix.transformed(QTransform().scale(XScalefactor, XScalefactor));
    }
    else // scale the height to max
    {
      pix = pix.transformed(QTransform().scale(YScalefactor, YScalefactor));
    }
    break;
  case 1:
    break; // do nothing if it's not zoomed
  case 2:
    pix = pix.transformed(QTransform().scale(2, 2));
    break;
  case 4:
    pix = pix.transformed(QTransform().scale(4, 4));
    break;
  default:
    break;
  }

  // return the modified image
  return pix;
}

//--------------------------------------------------------------------------------

void TileDataUI::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Delete)
  {
    deleteItem();
  }
}