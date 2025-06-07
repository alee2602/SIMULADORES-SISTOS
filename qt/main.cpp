#include <QApplication>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include "processsimulator.h"
#include "synchronizationsimulator.h"

QPushButton* createMenuButton(const QString &text, const QString &color, const QString &description) {
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(900, 140);
    btn->setMaximumSize(1200, 140);
    btn->setFont(QFont("Arial", 28, QFont::Bold)); 
    btn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: black; font-size: 28px; border-radius: 20px; padding: 20px; }"
        "QPushButton:hover { background-color: #e3f0fc; }"
    ).arg(color));
    btn->setToolTip(description);
    return btn;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QStackedWidget *mainStack = new QStackedWidget;

    // Menú principal
    QWidget *menu = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(menu);
    layout->setSpacing(20);
    layout->setContentsMargins(250, 120, 50, 30);

    QLabel *title = new QLabel("PROCESS SIMULATOR");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Arial", 32, QFont::Bold));
    title->setStyleSheet("color: black; background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 rgb(168, 220, 230), stop: 1 rgb(184, 231, 255)); border-radius: 20px; padding: 30px; margin: 20px;");

    QLabel *subtitle = new QLabel("Choose Simulation Type");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setFont(QFont("Arial", 18));
    subtitle->setStyleSheet("color: #6c757d; margin: 10px;");

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(70); 
    buttonLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QPushButton *btnSched = createMenuButton(
        "Simulador de Algoritmos de Calendarización",
        "#c4dafa",
        "Simulate process scheduling algorithms like FIFO, SJF, Round Robin, and Priority"
    );
    QPushButton *btnSync = createMenuButton(
        "Simulador de Mecanismos de Sincronización",
        "#c4e5fb",
        "Simulate synchronization mechanisms like Mutex Locks and Semaphores"
    );

    buttonLayout->addWidget(btnSched);
    buttonLayout->addWidget(btnSync);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addStretch(1);
    layout->addLayout(buttonLayout);
    layout->addStretch(2);

    mainStack->addWidget(menu);
    ProcessSimulator *schedSim = new ProcessSimulator(mainStack, menu, nullptr);
    SynchronizationSimulatorWidget *syncSim = new SynchronizationSimulatorWidget(mainStack, menu, nullptr);

    QObject::connect(schedSim, &ProcessSimulator::returnToMenuRequested, [mainStack, menu]() {
        mainStack->setCurrentWidget(menu);
    });

    mainStack->addWidget(schedSim);
    mainStack->addWidget(syncSim);

    QObject::connect(syncSim, &SynchronizationSimulatorWidget::backToMenuRequested, [mainStack, menu]() {
        mainStack->setCurrentWidget(menu);
    });

    QObject::connect(btnSched, &QPushButton::clicked, [mainStack, schedSim]() {
        mainStack->setCurrentWidget(schedSim);
    });
    QObject::connect(btnSync, &QPushButton::clicked, [mainStack, syncSim]() {
        mainStack->setCurrentWidget(syncSim);
    });

    menu->setFixedSize(1200, 800);
    mainStack->setFixedSize(1400, 1000);
    mainStack->show();

    return app.exec();
}
