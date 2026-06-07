#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "views/DisplayArrangeWidget.h"
#include "viewmodels/DisplayViewModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onMonitorsChanged();
    void onApplyClicked();
    void onRefreshClicked();
    void onMonitorMoved(int id, int x, int y);
    void showError(const QString &message);
    void showSuccess();

private:
    void buildUi();
    void rebuildMonitorRows();

    DisplayArrangeWidget *m_arrangeWidget   = nullptr;
    QWidget              *m_rowsContainer   = nullptr;
    QVBoxLayout          *m_rowsLayout      = nullptr;
    QPushButton          *m_applyButton     = nullptr;
    QPushButton          *m_refreshButton   = nullptr;
    QLabel               *m_statusLabel     = nullptr;

    DisplayViewModel     *m_viewModel       = nullptr;
};
