#ifndef QT_SETTINGSOTHERREMOVABLE_HPP
#define QT_SETTINGSOTHERREMOVABLE_HPP

#include <QWidget>

namespace Ui {
class SettingsOtherRemovable;
}

class SettingsOtherRemovable : public QWidget {
    Q_OBJECT

public:
    explicit SettingsOtherRemovable(QWidget *parent = nullptr);
    ~SettingsOtherRemovable();
    void reloadBusChannels_ZIP();

    void save();

signals:
    void zipChannelChanged();
private slots:
    void on_checkBoxZIP250_stateChanged(int arg1);

private slots:
    void on_comboBoxZIPChannel_activated(int index);

private slots:
    void on_comboBoxZIPBus_activated(int index);

private slots:
    void on_comboBoxZIPBus_currentIndexChanged(int index);

private slots:
    void onZIPRowChanged(const QModelIndex &current);

private:
    Ui::SettingsOtherRemovable *ui;
    void enableCurrentlySelectedChannel_ZIP();
};

#endif // QT_SETTINGSOTHERREMOVABLE_HPP
