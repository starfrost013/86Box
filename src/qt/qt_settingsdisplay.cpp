/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Display settings UI module.
 *
 * Authors: Joakim L. Gilje <jgilje@jgilje.net>
 *
 *          Copyright 2021 Joakim L. Gilje
 */
#include "qt_settingsdisplay.hpp"
#include "ui_qt_settingsdisplay.h"

#include "qt_util.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QStringBuilder>

#include <cstdint>

extern "C" {
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/machine.h>
#ifndef USE_VIDEO2
#include <86box/video.h>
#include <86box/vid_8514a_device.h>
#include <86box/vid_xga_device.h>
#include <86box/vid_ps55da2.h>
#include <86box/vid_ddc.h>
#else
#include <86box/video2/video.h>
#include <86box/video2/video_devices.h>
#include <86box/video2/base/ddc.h>
#endif
}

#include "qt_deviceconfig.hpp"
#include "qt_models_common.hpp"

SettingsDisplay::SettingsDisplay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsDisplay)
{
    ui->setupUi(this);

    for (uint8_t i = 0; i < GFXCARD_MAX; i ++)
        videoCard[i] = gfxcard[i];

    ui->lineEdit->setFilter(tr("EDID") % util::DlgFilter({ "bin", "dat", "edid", "txt" }) % tr("All files") % util::DlgFilter({ "*" }, true));

    onCurrentMachineChanged(machine);
}

SettingsDisplay::~SettingsDisplay()
{
    delete ui;
}

void
SettingsDisplay::save()
{
    // TODO
#if 0
    for (uint8_t i = 0; i < GFXCARD_MAX; ++i) {
        QComboBox *cbox = findChild<QComboBox *>(QString("comboBoxVideo%1").arg(i + 1));
        gfxcard[i]      = cbox->currentData().toInt();
    }
#else
    video_engine.devices[0].device = video_devices[ui->comboBoxVideo->currentData().toInt()];
    for (uint8_t i = 1; i < GFXCARD_MAX; i ++)
        video_engine.devices[i].device = video_devices[ui->comboBoxVideoSecondary->currentData().toInt()];
#endif

    voodoo_enabled             = ui->checkBoxVoodoo->isChecked() ? 1 : 0;
    ibm8514_standalone_enabled = ui->checkBox8514->isChecked() ? 1 : 0;
    xga_standalone_enabled     = ui->checkBoxXga->isChecked() ? 1 : 0;
    da2_standalone_enabled     = ui->checkBoxDa2->isChecked() ? 1 : 0;
    monitor_edid               = ui->radioButtonCustom->isChecked() ? 1 : 0;

    strncpy(monitor_edid_path, ui->lineEdit->fileName().toUtf8().data(), sizeof(monitor_edid_path) - 1);
}

void
SettingsDisplay::onCurrentMachineChanged(int machineId)
{
    // win_settings_video_proc, WM_INITDIALOG
    this->machineId   = machineId;
    auto curVideoCard = videoCard[0];

    auto *model      = ui->comboBoxVideo->model();
    auto  removeRows = model->rowCount();

    int c           = 0;
    int selectedRow = 0;
    while (true) {
        /* Skip "internal" if machine doesn't have it. */
        if ((c == 1) && (machine_has_flags(machineId, MACHINE_VIDEO) == 0)) {
            c++;
            continue;
        }

        device_t *video_dev = video_engine.devices[c].device;
        QString         name      = DeviceConfig::DeviceName(video_dev, video_dev->internal_name, 1);
        if (name.isEmpty()) {
            break;
        }

        if (video_card_index_is_available(c) && device_is_valid(video_dev, machineId)) {
            int row = Models::AddEntry(model, name, c);
            if (c == curVideoCard) {
                selectedRow = row - removeRows;
            }
        }

        c++;
    }
    model->removeRows(0, removeRows);

    // TODO
    if (machine_has_flags(machineId, MACHINE_VIDEO_ONLY) > 0) {
        ui->comboBoxVideo->setEnabled(false);
        ui->comboBoxVideoSecondary->setEnabled(false);
        ui->pushButtonConfigureVideoSecondary->setEnabled(false);
        selectedRow = 1;
    } else {
        ui->comboBoxVideo->setEnabled(true);
        ui->comboBoxVideoSecondary->setEnabled(true);
        ui->pushButtonConfigureVideoSecondary->setEnabled(true);
    }
    ui->comboBoxVideo->setCurrentIndex(selectedRow);
    // TODO
    for (uint8_t i = 1; i < GFXCARD_MAX; i ++)
        if (gfxcard[i] == 0)
            ui->pushButtonConfigureVideoSecondary->setEnabled(false);

    ui->radioButtonDefault->setChecked(monitor_edid == 0);
    ui->radioButtonCustom->setChecked(monitor_edid == 1);
    ui->lineEdit->setFileName(monitor_edid_path);
    ui->lineEdit->setEnabled(monitor_edid == 1);
}

void
SettingsDisplay::on_pushButtonConfigureVideo_clicked()
{
    // todo: this is a bit messy.
    video_engine.devices[0].device = video_devices[ui->comboBoxVideo->currentData().toInt()];
    video_engine.devices[0].device = video_engine.device_current; 
    
    DeviceConfig::ConfigureDevice(video_engine.devices[0].device);
}

void
SettingsDisplay::on_pushButtonConfigureVoodoo_clicked()
{
    #ifndef USE_VIDEO2
    DeviceConfig::ConfigureDevice(&voodoo_device);
    #else
    warning("Video System 2.0: This feature isn't implemented yet, tell starfrost");
    #endif
}

void
SettingsDisplay::on_pushButtonConfigure8514_clicked()
{
    #ifndef USE_VIDEO2
    if (machine_has_bus(machineId, MACHINE_BUS_MCA) > 0) {
        DeviceConfig::ConfigureDevice(&ibm8514_mca_device);
    } else {
        DeviceConfig::ConfigureDevice(&gen8514_isa_device);
    }
    #else
    warning("Video System 2.0: This feature isn't implemented yet, tell starfrost");

    #endif
}

void
SettingsDisplay::on_pushButtonConfigureXga_clicked()
{
    #ifndef USE_VIDEO2
    if (machine_has_bus(machineId, MACHINE_BUS_MCA) > 0)
        DeviceConfig::ConfigureDevice(&xga_device);
    #else
    warning("Video System 2.0: This feature isn't implemented yet, tell starfrost");
    #endif
}

void
SettingsDisplay::on_pushButtonConfigureDa2_clicked()
{
    #ifndef USE_VIDEO2
    DeviceConfig::ConfigureDevice(&ps55da2_device);
    #else
    warning("Video System 2.0: This feature isn't implemented yet, tell starfrost");
    #endif

}

void
SettingsDisplay::on_comboBoxVideo_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    static QRegularExpression voodooRegex("3dfx|voodoo|banshee|raven", QRegularExpression::CaseInsensitiveOption);

    // may have to add one
    video_engine.devices[0].device = video_devices[ui->comboBoxVideo->currentData().toInt()];
    video_engine.devices[1].device = video_devices[ui->comboBoxVideo->currentData().toInt()];

    video_engine.device_current = video_engine.devices[0].device; // this is dumb 

    // VIDEO2 TODO ui->pushButtonConfigureVideo->setEnabled(video_card_has_config(videoCard[0]) > 0);
    ui->pushButtonConfigureVideo->setEnabled(true);

    bool machineHasPci = machine_has_bus(machineId, MACHINE_BUS_PCI) > 0;
    ui->pushButtonConfigureVoodoo->setEnabled(machineHasPci && ui->checkBoxVoodoo->isChecked());

    bool machineHasIsa16 = machine_has_bus(machineId, MACHINE_BUS_ISA16) > 0;
    bool machineHasMca   = machine_has_bus(machineId, MACHINE_BUS_MCA) > 0;

    // *** These are deprecated ***
    ui->checkBox8514->setEnabled(false);
    ui->checkBoxDa2->setEnabled(false);
    ui->checkBoxXga->setEnabled(false);
    ui->pushButtonConfigure8514->setEnabled(false);
    ui->pushButtonConfigureXga->setEnabled(false);
    ui->pushButtonConfigureDa2->setEnabled(false);

    ui->comboBoxVideoSecondary->clear();
    ui->comboBoxVideoSecondary->addItem(QObject::tr("None"), 0);

    ui->comboBoxVideoSecondary->setCurrentIndex(0);
    // TODO: Implement support for selecting non-MDA secondary cards properly when MDA cards are the primary ones.

    if (video_engine.devices[0].flags & VIDEO_CARD_FLAG_MDA) {
        ui->comboBoxVideoSecondary->setCurrentIndex(0);
        return;
    }

    for (uint32_t i = 0; i < VIDEO_MAX_DEVICES; i++)
    {
        device_t *video_dev = video_engine.devices[i].device;

        QString         name      = DeviceConfig::DeviceName(video_dev, video_devices[i]->internal_name, 1);
        if (name.isEmpty()) {
            break;
        }

        int32_t primaryFlags   = video_engine.devices[0].flags;
        int32_t secondaryFlags = video_engine.devices[1].flags;

        if (video_card_index_is_available(i))
            ui->comboBoxVideoSecondary->setCurrentIndex(ui->comboBoxVideoSecondary->count() - 1);
    }

    if ((videoCard[1] == 0) || (machine_has_flags(machineId, MACHINE_VIDEO_ONLY) > 0)) {
        ui->comboBoxVideoSecondary->setCurrentIndex(0);
        ui->pushButtonConfigureVideoSecondary->setEnabled(false);
    }

    // Is the currently selected video card a voodoo?
    if (ui->comboBoxVideo->currentText().contains(voodooRegex)) {
        // Get the name of the video card currently in use
        const device_t *video_dev        = video_engine.devices[0].device;
        const QString   currentVideoName = DeviceConfig::DeviceName(video_dev, video_engine.devices[0].device->internal_name, 1);
        // Is it a voodoo?
        const bool currentCardIsVoodoo = currentVideoName.contains(voodooRegex);
        // Don't uncheck if
        // * Current card is voodoo
        // * Add-on voodoo was manually overridden in config
        if (ui->checkBoxVoodoo->isChecked() && !currentCardIsVoodoo) {
            // Otherwise, uncheck the add-on voodoo when a main voodoo is selected
            ui->checkBoxVoodoo->setCheckState(Qt::Unchecked);
        }
        ui->checkBoxVoodoo->setDisabled(true);
    } else {
        ui->checkBoxVoodoo->setEnabled(machineHasPci);
        if (machineHasPci) {
            ui->checkBoxVoodoo->setChecked(voodoo_enabled);
        }
    }
}

void
SettingsDisplay::on_checkBoxVoodoo_stateChanged(int state)
{
    ui->pushButtonConfigureVoodoo->setEnabled(state == Qt::Checked);
}

void
SettingsDisplay::on_checkBox8514_stateChanged(int state)
{
    ui->pushButtonConfigure8514->setEnabled(state == Qt::Checked);
}

void
SettingsDisplay::on_checkBoxXga_stateChanged(int state)
{
    ui->pushButtonConfigureXga->setEnabled(state == Qt::Checked);
}

void
SettingsDisplay::on_checkBoxDa2_stateChanged(int state)
{
    ui->pushButtonConfigureDa2->setEnabled(state == Qt::Checked);
}

void
SettingsDisplay::on_comboBoxVideoSecondary_currentIndexChanged(int index)
{
    if (index < 0) {
        ui->pushButtonConfigureVideoSecondary->setEnabled(false);
        return;
    }
    videoCard[1] = ui->comboBoxVideoSecondary->currentData().toInt();
    ui->pushButtonConfigureVideoSecondary->setEnabled(index != 0 && device_has_config(video_engine.devices[1].device) > 0);
}

void
SettingsDisplay::on_pushButtonConfigureVideoSecondary_clicked()
{
    auto *device = video_engine.devices[1].device;
    DeviceConfig::ConfigureDevice(device);
}

void SettingsDisplay::on_radioButtonDefault_clicked()
{
    ui->radioButtonDefault->setChecked(true);
    ui->radioButtonCustom->setChecked(false);
    ui->lineEdit->setEnabled(false);
}


void SettingsDisplay::on_radioButtonCustom_clicked()
{
    ui->radioButtonDefault->setChecked(false);
    ui->radioButtonCustom->setChecked(true);
    ui->lineEdit->setEnabled(true);
}

void SettingsDisplay::on_pushButtonExportDefault_clicked()
{
    auto str = QFileDialog::getSaveFileName(this, tr("Export EDID"));
    if (!str.isEmpty()) {
        QFile file(str);
        if (file.open(QFile::WriteOnly)) {
            uint8_t *bytes = nullptr;
            auto size = ddc_create_default_edid(&bytes);
            file.write((char*)bytes, size);
            file.close();
        }
    }
}
