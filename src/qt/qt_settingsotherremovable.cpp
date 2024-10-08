/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Other removable devices configuration UI module.
 *
 *
 *
 * Authors: Joakim L. Gilje <jgilje@jgilje.net>
 *          Cacodemon345
 *
 *          Copyright 2021-2022 Cacodemon345
 *          Copyright 2021 Joakim L. Gilje
 */
#include "qt_settingsotherremovable.hpp"
#include "ui_qt_settingsotherremovable.h"

extern "C" {
#include <86box/timer.h>
#include <86box/scsi_device.h>
#include <86box/zip.h>
}

#include <QStandardItemModel>

#include "qt_models_common.hpp"
#include "qt_harddrive_common.hpp"
#include "qt_settings_bus_tracking.hpp"
#include "qt_progsettings.hpp"


static void
setZIPBus(QAbstractItemModel *model, const QModelIndex &idx, uint8_t bus, uint8_t channel)
{
    QIcon icon;
    switch (bus) {
        case ZIP_BUS_DISABLED:
            icon = ProgSettings::loadIcon("/zip_disabled.ico");
            break;
        case ZIP_BUS_ATAPI:
        case ZIP_BUS_SCSI:
            icon = ProgSettings::loadIcon("/zip.ico");
            break;

        default:
            break;
    }

    auto i = idx.siblingAtColumn(0);
    model->setData(i, Harddrives::BusChannelName(bus, channel));
    model->setData(i, bus, Qt::UserRole);
    model->setData(i, channel, Qt::UserRole + 1);
    model->setData(i, icon, Qt::DecorationRole);
}

static void
setZIPType(QAbstractItemModel *model, const QModelIndex &idx, bool is250)
{
    auto i = idx.siblingAtColumn(1);
    model->setData(i, is250 ? "ZIP 250" : "ZIP 100");
    model->setData(i, is250, Qt::UserRole);
}

SettingsOtherRemovable::SettingsOtherRemovable(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsOtherRemovable)
{

    ui->setupUi(this);

    Harddrives::populateRemovableBuses(ui->comboBoxZIPBus->model());
    auto* model = new QStandardItemModel(0, 2, this);
    
    ui->tableViewZIP->setModel(model);
    model->setHeaderData(0, Qt::Horizontal, tr("Bus"));
    model->setHeaderData(1, Qt::Horizontal, tr("Type"));
    model->insertRows(0, ZIP_NUM);
    for (int i = 0; i < ZIP_NUM; i++) {
        auto idx = model->index(i, 0);
        setZIPBus(model, idx, zip_drives[i].bus_type, zip_drives[i].res);
        setZIPType(model, idx, zip_drives[i].is_250 > 0);
        Harddrives::busTrackClass->device_track(1, DEV_ZIP, zip_drives[i].bus_type, zip_drives[i].bus_type == ZIP_BUS_ATAPI ? zip_drives[i].ide_channel : zip_drives[i].scsi_device_id);
    }
    ui->tableViewZIP->resizeColumnsToContents();
    ui->tableViewZIP->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(ui->tableViewZIP->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SettingsOtherRemovable::onZIPRowChanged);
    ui->tableViewZIP->setCurrentIndex(model->index(0, 0));
}

SettingsOtherRemovable::~SettingsOtherRemovable()
{
    delete ui;
}

void
SettingsOtherRemovable::save()
{
    const auto *model = ui->tableViewZIP->model();

    for (uint8_t i = 0; i < ZIP_NUM; i++) {
        zip_drives[i].fp       = NULL;
        zip_drives[i].priv     = NULL;
        zip_drives[i].bus_type = model->index(i, 0).data(Qt::UserRole).toUInt();
        zip_drives[i].res      = model->index(i, 0).data(Qt::UserRole + 1).toUInt();
        zip_drives[i].is_250   = model->index(i, 1).data(Qt::UserRole).toBool() ? 1 : 0;
    }
}

void
SettingsOtherRemovable::onZIPRowChanged(const QModelIndex &current)
{
    uint8_t bus     = current.siblingAtColumn(0).data(Qt::UserRole).toUInt();
    uint8_t channel = current.siblingAtColumn(0).data(Qt::UserRole + 1).toUInt();
    bool    is250   = current.siblingAtColumn(1).data(Qt::UserRole).toBool();

    ui->comboBoxZIPBus->setCurrentIndex(-1);
    const auto *model = ui->comboBoxZIPBus->model();
    auto        match = model->match(model->index(0, 0), Qt::UserRole, bus);
    if (!match.isEmpty())
        ui->comboBoxZIPBus->setCurrentIndex(match.first().row());

    model = ui->comboBoxZIPChannel->model();
    match = model->match(model->index(0, 0), Qt::UserRole, channel);
    if (!match.isEmpty())
        ui->comboBoxZIPChannel->setCurrentIndex(match.first().row());
    ui->checkBoxZIP250->setChecked(is250);
    enableCurrentlySelectedChannel_ZIP();
}


void
SettingsOtherRemovable::reloadBusChannels_ZIP() {
    auto selected = ui->comboBoxZIPChannel->currentIndex();
    Harddrives::populateBusChannels(ui->comboBoxZIPChannel->model(),
                                    ui->comboBoxZIPBus->currentData().toInt(), Harddrives::busTrackClass);
    ui->comboBoxZIPChannel->setCurrentIndex(selected);
    enableCurrentlySelectedChannel_ZIP();
}

void
SettingsOtherRemovable::on_comboBoxZIPBus_currentIndexChanged(int index)
{
    if (index >= 0) {
        int  bus     = ui->comboBoxZIPBus->currentData().toInt();
        bool enabled = (bus != ZIP_BUS_DISABLED);
        ui->comboBoxZIPChannel->setEnabled(enabled);
        ui->checkBoxZIP250->setEnabled(enabled);
        Harddrives::populateBusChannels(ui->comboBoxZIPChannel->model(), bus, Harddrives::busTrackClass);
    }
}

void
SettingsOtherRemovable::on_comboBoxZIPBus_activated(int)
{
    auto i = ui->tableViewZIP->selectionModel()->currentIndex().siblingAtColumn(0);
    Harddrives::busTrackClass->device_track(0, DEV_ZIP, ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole).toInt(), ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole + 1).toInt());
    ui->comboBoxZIPChannel->setCurrentIndex(ui->comboBoxZIPBus->currentData().toUInt() == ZIP_BUS_ATAPI ?
                                            Harddrives::busTrackClass->next_free_ide_channel() :
                                            Harddrives::busTrackClass->next_free_scsi_id());
    setZIPBus(ui->tableViewZIP->model(),
              ui->tableViewZIP->selectionModel()->currentIndex(),
              ui->comboBoxZIPBus->currentData().toUInt(),
              ui->comboBoxZIPChannel->currentData().toUInt());
    Harddrives::busTrackClass->device_track(1, DEV_ZIP, ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole).toInt(), ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole + 1).toInt());
    emit zipChannelChanged();
}

void
SettingsOtherRemovable::enableCurrentlySelectedChannel_ZIP()
{
    const auto *item_model = qobject_cast<QStandardItemModel*>(ui->comboBoxZIPChannel->model());
    const auto index = ui->comboBoxZIPChannel->currentIndex();
    auto *item = item_model->item(index);
    if (item)
        item->setEnabled(true);
}

void
SettingsOtherRemovable::on_comboBoxZIPChannel_activated(int)
{
    auto i = ui->tableViewZIP->selectionModel()->currentIndex().siblingAtColumn(0);
    Harddrives::busTrackClass->device_track(0, DEV_ZIP, ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole).toInt(), ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole + 1).toInt());
    setZIPBus(ui->tableViewZIP->model(),
              ui->tableViewZIP->selectionModel()->currentIndex(),
              ui->comboBoxZIPBus->currentData().toUInt(),
              ui->comboBoxZIPChannel->currentData().toUInt());
    Harddrives::busTrackClass->device_track(1, DEV_ZIP, ui->tableViewZIP->model()->data(i,
                                            Qt::UserRole).toInt(),
                                            ui->tableViewZIP->model()->data(i, Qt::UserRole + 1).toInt());
    emit zipChannelChanged();
}

void
SettingsOtherRemovable::on_checkBoxZIP250_stateChanged(int state)
{
    setZIPType(ui->tableViewZIP->model(),
               ui->tableViewZIP->selectionModel()->currentIndex(),
               state == Qt::Checked);
}
