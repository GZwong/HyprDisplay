#include "views/MainWindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_viewModel = new DisplayViewModel(this);

    buildUi();

    connect(m_viewModel, &DisplayViewModel::monitorsChanged,
            this, &MainWindow::onMonitorsChanged);
    connect(m_viewModel, &DisplayViewModel::errorOccurred,
            this, &MainWindow::showError);
    connect(m_viewModel, &DisplayViewModel::applySucceeded,
            this, &MainWindow::showSuccess);
    connect(m_viewModel, &DisplayViewModel::applyFailed,
            this, &MainWindow::showError);

    connect(m_arrangeWidget, &DisplayArrangeWidget::monitorMoved,
            this, &MainWindow::onMonitorMoved);
    connect(m_applyButton,   &QPushButton::clicked,
            this, &MainWindow::onApplyClicked);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);

    m_viewModel->refresh();
}

// ---------------------------------------------------------------------------
// UI construction (no .ui file — everything built in code)
// ---------------------------------------------------------------------------

void MainWindow::buildUi()
{
    setWindowTitle("HyprDisplay");
    setMinimumSize(700, 520);
    resize(900, 620);

    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 12);
    mainLayout->setSpacing(10);
    setCentralWidget(central);

    // ── Title ────────────────────────────────────────────────────────────────
    auto *title = new QLabel("Display Settings", central);
    QFont titleFont;
    titleFont.setPointSize(15);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // ── Drag canvas ──────────────────────────────────────────────────────────
    m_arrangeWidget = new DisplayArrangeWidget(central);
    mainLayout->addWidget(m_arrangeWidget, 2); // stretch factor 2

    // ── Separator ────────────────────────────────────────────────────────────
    auto *sep = new QFrame(central);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    // ── Monitor rows (scrollable) ────────────────────────────────────────────
    auto *scroll = new QScrollArea(central);
    scroll->setWidgetResizable(true);
    scroll->setMaximumHeight(200);
    scroll->setFrameShape(QFrame::NoFrame);

    m_rowsContainer = new QWidget();
    m_rowsLayout    = new QVBoxLayout(m_rowsContainer);
    m_rowsLayout->setContentsMargins(0, 0, 0, 0);
    m_rowsLayout->setSpacing(4);
    m_rowsLayout->addStretch();
    scroll->setWidget(m_rowsContainer);
    mainLayout->addWidget(scroll, 1); // stretch factor 1

    // ── Bottom bar ───────────────────────────────────────────────────────────
    auto *bottomBar    = new QWidget(central);
    auto *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 4, 0, 0);

    m_statusLabel = new QLabel(bottomBar);
    bottomLayout->addWidget(m_statusLabel, 1);

    m_refreshButton = new QPushButton("Refresh", bottomBar);
    m_applyButton   = new QPushButton("Apply", bottomBar);
    m_applyButton->setDefault(true);
    bottomLayout->addWidget(m_refreshButton);
    bottomLayout->addWidget(m_applyButton);

    mainLayout->addWidget(bottomBar);
}

// ---------------------------------------------------------------------------
// Slot implementations
// ---------------------------------------------------------------------------

void MainWindow::onMonitorsChanged()
{
    m_arrangeWidget->setMonitors(m_viewModel->monitors());
    rebuildMonitorRows();
}

void MainWindow::rebuildMonitorRows()
{
    // Remove all existing rows from the layout, destroying their widgets
    while (m_rowsLayout->count() > 1) { // keep the trailing stretch
        QLayoutItem *item = m_rowsLayout->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QList<Monitor> &monitors = m_viewModel->monitors();
    for (const Monitor &m : monitors) {
        auto *row       = new QWidget(m_rowsContainer);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(6, 2, 6, 2);

        auto *enableCheck = new QCheckBox(m.name, row);
        enableCheck->setChecked(m.enabled);

        const QString detail = QString("%1 × %2  @  %3 Hz    pos (%4, %5)")
                                   .arg(m.width)
                                   .arg(m.height)
                                   .arg(m.refreshRate, 0, 'f', 1)
                                   .arg(m.x)
                                   .arg(m.y);
        auto *infoLabel = new QLabel(detail, row);
        infoLabel->setStyleSheet("color: #888;");

        rowLayout->addWidget(enableCheck);
        rowLayout->addWidget(infoLabel, 1);

        const int monitorId = m.id;
        connect(enableCheck, &QCheckBox::toggled, this,
                [this, monitorId](bool checked) {
                    m_viewModel->setMonitorEnabled(monitorId, checked);
                });

        // Insert before the trailing stretch
        m_rowsLayout->insertWidget(m_rowsLayout->count() - 1, row);
    }
}

void MainWindow::onApplyClicked()
{
    m_statusLabel->setText("Applying…");
    m_statusLabel->setStyleSheet("color: #aaa;");
    QApplication::processEvents(); // show "Applying…" before blocking on hyprctl
    m_viewModel->applyChanges();
}

void MainWindow::onRefreshClicked()
{
    m_viewModel->refresh();
    m_statusLabel->clear();
}

void MainWindow::onMonitorMoved(int id, int x, int y)
{
    m_viewModel->setMonitorPosition(id, x, y);
}

void MainWindow::showError(const QString &message)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet("color: #ef5350;");
}

void MainWindow::showSuccess()
{
    m_statusLabel->setText("Configuration applied successfully.");
    m_statusLabel->setStyleSheet("color: #66bb6a;");
}
