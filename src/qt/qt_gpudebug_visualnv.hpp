/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          GPU Debugging Tools - VRAM Viewer headers
 *
 *
 *
 * Authors: starfrost
 *
 *          Copyright 2025 starfrost
 */


#pragma once

#include <QDialog>

namespace Ui
{
    class VisualNVDialog;
}

class VisualNVDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit VisualNVDialog(QWidget *parent = nullptr);
        ~VisualNVDialog();

        void on_btnLoadSavestate_clicked();
        void on_fbStartAddress_changed();

    protected:
    private:
        Ui::VisualNVDialog* ui; 


};