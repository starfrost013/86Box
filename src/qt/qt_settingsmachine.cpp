/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Machine selection and configuration UI module.
 *
 *
 *
 * Authors: Joakim L. Gilje <jgilje@jgilje.net>
 *
 *          Copyright 2021 Joakim L. Gilje
 */
#include "qt_settingsmachine.hpp"
#include "ui_qt_settingsmachine.h"

#include <QDebug>
#include <QDialog>
#include <QFrame>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <algorithm>

extern "C" {
#include "../cpu/cpu.h"

#include <86box/86box.h>
#include <86box/config.h>
#include <86box/device.h>
#include <86box/machine.h>
#include <86box/nvr.h>
}

#include "qt_deviceconfig.hpp"
#include "qt_models_common.hpp"

SettingsMachine::SettingsMachine(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsMachine)
{
    ui->setupUi(this);

    switch (time_sync) {
        case TIME_SYNC_ENABLED:
            ui->radioButtonLocalTime->setChecked(true);
            break;
        case TIME_SYNC_ENABLED | TIME_SYNC_UTC:
            ui->radioButtonUTC->setChecked(true);
            break;
        case TIME_SYNC_DISABLED:
        default:
            ui->radioButtonDisabled->setChecked(true);
            break;
    }

    auto warning_icon = QIcon(":/misc/qt/icons/warning.ico");
    ui->softFloatWarningIcon->setPixmap(warning_icon.pixmap(warning_icon.actualSize(QSize(16, 16))));
    ui->softFloatWarningIcon->setVisible(false);
    ui->softFloatWarningText->setVisible(false);

    auto *waitStatesModel = ui->comboBoxWaitStates->model();
    waitStatesModel->insertRows(0, 9);
    auto idx = waitStatesModel->index(0, 0);
    waitStatesModel->setData(idx, tr("Default"), Qt::DisplayRole);
    waitStatesModel->setData(idx, 0, Qt::UserRole);
    for (int i = 0; i < 8; ++i) {
        idx = waitStatesModel->index(i + 1, 0);
        waitStatesModel->setData(idx, tr("%1 Wait state(s)").arg(i), Qt::DisplayRole);
        waitStatesModel->setData(idx, i + 1, Qt::UserRole);
    }

    auto *pitModeModel = ui->comboBoxPitMode->model();
    pitModeModel->insertRows(0, 3);
    idx = pitModeModel->index(0, 0);
    pitModeModel->setData(idx, tr("Auto"), Qt::DisplayRole);
    pitModeModel->setData(idx, -1, Qt::UserRole);
    idx = pitModeModel->index(1, 0);
    pitModeModel->setData(idx, tr("Slow"), Qt::DisplayRole);
    pitModeModel->setData(idx, 0, Qt::UserRole);
    idx = pitModeModel->index(2, 0);
    pitModeModel->setData(idx, tr("Fast"), Qt::DisplayRole);
    pitModeModel->setData(idx, 1, Qt::UserRole);

    ui->comboBoxPitMode->setCurrentIndex(-1);
    ui->comboBoxPitMode->setCurrentIndex(pit_mode + 1);

    int         selectedMachineType = 0;
    auto *      machineTypesModel   = ui->comboBoxMachineType->model();
    int         i                   = -1;
    int         j                   = 0;
    int         cur_j               = 0;
    const void *miname;
    do {
        miname = machine_get_internal_name_ex(j);

        if ((miname == nullptr) || (machine_get_type(j) != i)) {
            if ((i != -1) && (cur_j != 0)) {
                int row = Models::AddEntry(machineTypesModel, machine_types[i].name, machine_types[i].id);
                if (machine_types[i].id == machine_get_type(machine))
                    selectedMachineType = row;
            }

            i = machine_get_type(j);
            cur_j = 0;
        }

        if (machine_available(j))
            cur_j++;

        j++;
    } while (miname != nullptr);

    ui->comboBoxMachineType->setCurrentIndex(-1);
    ui->comboBoxMachineType->setCurrentIndex(selectedMachineType);

    ui->radioButtonLargerFrames->setChecked(force_10ms);
    ui->radioButtonSmallerFrames->setChecked(!force_10ms);

#ifndef USE_DYNAREC
    ui->checkBoxDynamicRecompiler->setEnabled(false);
    ui->checkBoxDynamicRecompiler->setVisible(false);
#endif
}

SettingsMachine::~SettingsMachine()
{
    delete ui;
}

void
SettingsMachine::save()
{
    machine         = ui->comboBoxMachine->currentData().toInt();
    cpu_f           = const_cast<cpu_family_t *>(&cpu_families[ui->comboBoxCPU->currentData().toInt()]);
    cpu             = ui->comboBoxSpeed->currentData().toInt();
    fpu_type        = ui->comboBoxFPU->currentData().toInt();
    cpu_use_dynarec = ui->checkBoxDynamicRecompiler->isChecked() ? 1 : 0;
    fpu_softfloat   = ui->checkBoxFPUSoftfloat->isChecked() ? 1 : 0;
    force_10ms      = ui->radioButtonLargerFrames->isChecked() ? 1 : 0;

    int64_t temp_mem_size;
    if (machine_get_ram_granularity(machine) < 1024)
        temp_mem_size = ui->spinBoxRAM->value();
    else
        temp_mem_size = ui->spinBoxRAM->value() * 1024;

    temp_mem_size &= ~(machine_get_ram_granularity(machine) - 1);
    if (temp_mem_size < machine_get_min_ram(machine))
        temp_mem_size = machine_get_min_ram(machine);
    else if (temp_mem_size > machine_get_max_ram(machine))
        temp_mem_size = machine_get_max_ram(machine);
    mem_size = static_cast<uint32_t>(temp_mem_size);

    if (ui->comboBoxWaitStates->isEnabled())
        cpu_waitstates = ui->comboBoxWaitStates->currentData().toInt();
    else
        cpu_waitstates = 0;

    pit_mode = ui->comboBoxPitMode->currentData().toInt();

    time_sync = 0;
    if (ui->radioButtonLocalTime->isChecked())
        time_sync = TIME_SYNC_ENABLED;
    if (ui->radioButtonUTC->isChecked())
        time_sync = TIME_SYNC_ENABLED | TIME_SYNC_UTC;
}

void
SettingsMachine::on_comboBoxMachineType_currentIndexChanged(int index)
{
    if (index >= 0) {
        auto *model      = ui->comboBoxMachine->model();
        int   removeRows = model->rowCount();

        int selectedMachineRow = 0;
        for (int i = 0; i < machine_count(); ++i) {
            if ((machine_get_type(i) == ui->comboBoxMachineType->currentData().toInt()) &&
                machine_available(i)) {
                int row = Models::AddEntry(model, machines[i].name, i);
                if (i == machine)
                    selectedMachineRow = row - removeRows;
            }
        }
        model->removeRows(0, removeRows);

        ui->comboBoxMachine->setCurrentIndex(-1);
        ui->comboBoxMachine->setCurrentIndex(selectedMachineRow);
    }
}

void
SettingsMachine::on_comboBoxMachine_currentIndexChanged(int index)
{
    // win_settings_machine_recalc_machine
    if (index >= 0) {
        int         machineId = ui->comboBoxMachine->currentData().toInt();
        const auto *device    = machine_get_device(machineId);
        ui->pushButtonConfigure->setEnabled((device != nullptr) && (device->config != nullptr));

        auto *modelCpu   = ui->comboBoxCPU->model();
        int   removeRows = modelCpu->rowCount();

        int i                    = 0;
        int eligibleRows         = 0;
        int selectedCpuFamilyRow = 0;
        while (cpu_families[i].package != 0) {
            if (cpu_family_is_eligible(&cpu_families[i], machineId)) {
                Models::AddEntry(modelCpu, QString("%1 %2").arg(cpu_families[i].manufacturer,
                                 cpu_families[i].name), i);
                if (&cpu_families[i] == cpu_f)
                    selectedCpuFamilyRow = eligibleRows;
                ++eligibleRows;
            }
            ++i;
        }
        modelCpu->removeRows(0, removeRows);
        ui->comboBoxCPU->setEnabled(eligibleRows > 1);
        ui->comboBoxCPU->setCurrentIndex(-1);
        ui->comboBoxCPU->setCurrentIndex(selectedCpuFamilyRow);

        int divisor;
        if (machine_get_ram_granularity(machineId) < 1024) {
            divisor = 1;
            ui->spinBoxRAM->setSuffix(QCoreApplication::translate("", "KB").prepend(' '));
        } else {
            divisor = 1024;
            ui->spinBoxRAM->setSuffix(QCoreApplication::translate("", "MB").prepend(' '));
        }
        ui->spinBoxRAM->setMinimum(machine_get_min_ram(machineId) / divisor);
        ui->spinBoxRAM->setMaximum(machine_get_max_ram(machineId) / divisor);
        ui->spinBoxRAM->setSingleStep(machine_get_ram_granularity(machineId) / divisor);
        ui->spinBoxRAM->setValue(mem_size / divisor);
        ui->spinBoxRAM->setEnabled(machine_get_min_ram(machineId) != machine_get_max_ram(machineId));

        emit currentMachineChanged(machineId);
    }
}

void
SettingsMachine::on_comboBoxCPU_currentIndexChanged(int index)
{
    if (index >= 0) {
        int         machineId   = ui->comboBoxMachine->currentData().toInt();
        int         cpuFamilyId = ui->comboBoxCPU->currentData().toInt();
        const auto *cpuFamily   = &cpu_families[cpuFamilyId];

        auto *modelSpeed = ui->comboBoxSpeed->model();
        int   removeRows = modelSpeed->rowCount();

        // win_settings_machine_recalc_cpu_m
        int i                = 0;
        int eligibleRows     = 0;
        int selectedSpeedRow = 0;
        while (cpuFamily->cpus[i].cpu_type != 0) {
            if (cpu_is_eligible(cpuFamily, i, machineId)) {
                Models::AddEntry(modelSpeed, QString("%1").arg(cpuFamily->cpus[i].name), i);
                if (cpu == i)
                    selectedSpeedRow = eligibleRows;
                ++eligibleRows;
            }
            ++i;
        }
        modelSpeed->removeRows(0, removeRows);
        ui->comboBoxSpeed->setEnabled(eligibleRows > 1);
        ui->comboBoxSpeed->setCurrentIndex(-1);
        ui->comboBoxSpeed->setCurrentIndex(selectedSpeedRow);
    }
}

void
SettingsMachine::on_comboBoxSpeed_currentIndexChanged(int index)
{
    if (index >= 0) {
        // win_settings_machine_recalc_cpu
        int         cpuFamilyId = ui->comboBoxCPU->currentData().toInt();
        const auto *cpuFamily   = &cpu_families[cpuFamilyId];
        int         cpuId       = ui->comboBoxSpeed->currentData().toInt();
        uint        cpuType     = cpuFamily->cpus[cpuId].cpu_type;

        if ((cpuType >= CPU_286) && (cpuType <= CPU_386DX)) {
            ui->comboBoxWaitStates->setEnabled(true);
            ui->comboBoxWaitStates->setCurrentIndex(cpu_waitstates);
        } else {
            ui->comboBoxWaitStates->setCurrentIndex(0);
            ui->comboBoxWaitStates->setEnabled(false);
        }

#ifdef USE_DYNAREC
        uint8_t flags = cpuFamily->cpus[cpuId].cpu_flags;
        if (!(flags & CPU_SUPPORTS_DYNAREC)) {
            ui->checkBoxDynamicRecompiler->setChecked(false);
            ui->checkBoxDynamicRecompiler->setEnabled(false);
        } else if ((flags & CPU_REQUIRES_DYNAREC) && !cpu_override) {
            ui->checkBoxDynamicRecompiler->setChecked(true);
            ui->checkBoxDynamicRecompiler->setEnabled(false);
        } else {
            ui->checkBoxDynamicRecompiler->setChecked(cpu_use_dynarec);
            ui->checkBoxDynamicRecompiler->setEnabled(true);
        }
#endif

        // win_settings_machine_recalc_fpu
        auto *modelFpu   = ui->comboBoxFPU->model();
        int   removeRows = modelFpu->rowCount();

        int i              = 0;
        int selectedFpuRow = 0;
        for (const char *fpuName = fpu_get_name_from_index(cpuFamily, cpuId, i);
             fpuName != nullptr; fpuName = fpu_get_name_from_index(cpuFamily, cpuId, ++i)) {
            auto fpuType = fpu_get_type_from_index(cpuFamily, cpuId, i);
            Models::AddEntry(modelFpu, tr(QString("%1").arg(fpuName).toUtf8().data()), fpuType);
            if (fpu_type == fpuType)
                selectedFpuRow = i;
        }

        modelFpu->removeRows(0, removeRows);
        ui->comboBoxFPU->setEnabled(modelFpu->rowCount() > 1);
        ui->comboBoxFPU->setCurrentIndex(-1);
        ui->comboBoxFPU->setCurrentIndex(selectedFpuRow);
    }
}

void
SettingsMachine::on_comboBoxFPU_currentIndexChanged(int index)
{
    if (index >= 0) {
        int         cpuFamilyId = ui->comboBoxCPU->currentData().toInt();
        const auto *cpuFamily   = &cpu_families[cpuFamilyId];
        int         cpuId       = ui->comboBoxSpeed->currentData().toInt();
        int         machineId   = ui->comboBoxMachine->currentData().toInt();

        if (fpu_get_type_from_index(cpuFamily, cpuId, index) == FPU_NONE) {
            ui->checkBoxFPUSoftfloat->setChecked(false);
            ui->checkBoxFPUSoftfloat->setEnabled(false);
        } else {
            ui->checkBoxFPUSoftfloat->setChecked(machine_has_flags(machineId, MACHINE_SOFTFLOAT_ONLY) ?
                                                 true : fpu_softfloat);
            ui->checkBoxFPUSoftfloat->setEnabled(machine_has_flags(machineId, MACHINE_SOFTFLOAT_ONLY) ?
                                                 false : true);
        }
    }
}

void
SettingsMachine::on_pushButtonConfigure_clicked()
{
    // deviceconfig_inst_open
    int         machineId = ui->comboBoxMachine->currentData().toInt();
    const auto *device    = machine_get_device(machineId);
    DeviceConfig::ConfigureDevice(device);
}

void SettingsMachine::on_checkBoxFPUSoftfloat_stateChanged(int state) {
    if(state == Qt::Checked) {
        ui->softFloatWarningIcon->setVisible(true);
        ui->softFloatWarningText->setVisible(true);
    } else {
        ui->softFloatWarningIcon->setVisible(false);
        ui->softFloatWarningText->setVisible(false);
    }
}

void SettingsMachine::on_radioButtonSmallerFrames_clicked()
{
    ui->radioButtonLargerFrames->setChecked(false);
}


void SettingsMachine::on_radioButtonLargerFrames_clicked()
{
    ui->radioButtonSmallerFrames->setChecked(false);
}

