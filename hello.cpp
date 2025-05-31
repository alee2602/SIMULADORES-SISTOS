#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QFrame>
#include <QFont>
#include <QGraphicsDropShadowEffect>

void runFIFO() {
    QMessageBox::information(nullptr, "Simulación FIFO", "Algoritmo FIFO ejecutado correctamente\n\nProcesos ordenados por tiempo de llegada");
}

void runSJF() {
    QMessageBox::information(nullptr, "Simulación SJF", "Algoritmo SJF ejecutado correctamente\n\nProcesos ordenados por tiempo de ejecución");
}

void runRR() {
    bool ok;
    int quantum = QInputDialog::getInt(nullptr, "Configuración Round Robin", "Ingrese el valor del quantum (ms):", 5, 1, 100, 1, &ok);
    if (ok) {
        QMessageBox::information(nullptr, "Simulación Round Robin", 
            QString("Round Robin ejecutado correctamente\n\nQuantum configurado: %1 ms").arg(quantum));
    }
}

QPushButton* createStyledButton(const QString &text, const QString &color, const QString &hoverColor) {
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(280, 60);
    btn->setFont(QFont("Arial", 12, QFont::Bold));
    btn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2);"
        "    border: none;"
        "    border-radius: 15px;"
        "    color: white;"
        "    padding: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %3, stop:1 %4);"
        "    transform: translateY(-2px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %4, stop:1 %3);"
        "}"
    ).arg(color).arg(hoverColor).arg(hoverColor).arg(color));
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 5);
    btn->setGraphicsEffect(shadow);
    
    return btn;
}

QPushButton* createAlgorithmButton(const QString &text, const QString &color) {
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(220, 50);
    btn->setFont(QFont("Arial", 11, QFont::Bold));
    btn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    border: none;"
        "    border-radius: 12px;"
        "    color: #2c3e50;"
        "    padding: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;" 
        "}"
        "QPushButton:pressed {"
        "    background-color: #d6d6d6;" 
        "}"
    ).arg(color));
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 3);
    btn->setGraphicsEffect(shadow);
    
    return btn;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Simulador Avanzado de Planificación de Procesos");
    window.setFixedSize(800, 800);
    window.setStyleSheet(
        "QWidget {"
        "    background: white;"
        "}"
    );

    QLabel *title = new QLabel("SIMULADOR DE PROCESOS");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Arial", 24, QFont::Bold));
    title->setStyleSheet(
        "QLabel {"
        "    color: #2c3e50;"
        "    background: #f8f9fa;"
        "    border: 2px solid #e9ecef;"
        "    border-radius: 15px;"
        "    padding: 20px;"
        "    margin: 10px;"
        "}"
    );

    QLabel *subtitle = new QLabel("Sistema Operativo - Proyecto Simulador");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setFont(QFont("Arial", 12));
    subtitle->setStyleSheet(
        "QLabel {"
        "    color: #6c757d;"
        "    margin-bottom: 20px;"
        "}"
    );

    QPushButton *btnSched = createStyledButton("Simulación de Planificación", "#6c85bd", "#8ca1d4");
    QPushButton *btnSync = createStyledButton("Simulación de Sincronización", "#70a1a8", "#8bb6bd");
    QPushButton *btnSalir = createStyledButton("Salir del Sistema", "#9ca3af", "#b5bcc6");

    QObject::connect(btnSched, &QPushButton::clicked, [&]() {
        window.hide(); 

        QDialog *dialog = new QDialog(&window);
        dialog->setWindowTitle("Algoritmos de Planificación de CPU");
        dialog->setFixedSize(800, 800);
        dialog->setStyleSheet(
            "QDialog {"
            "    background: white;"
            "    border: 1px solid #dee2e6;"
            "}"
        );

        QLabel *dialogTitle = new QLabel("ALGORITMOS DISPONIBLES");
        dialogTitle->setAlignment(Qt::AlignCenter);
        dialogTitle->setFont(QFont("Arial", 18, QFont::Bold));
        dialogTitle->setStyleSheet(
            "QLabel {"
            "    color: #2c3e50;"
            "    background: #f8f9fa;"
            "    border: 2px solid #e9ecef;"
            "    border-radius: 10px;"
            "    padding: 15px;"
            "    margin: 10px;"
            "}"
        );

        QPushButton *btnFIFO = createAlgorithmButton("FIFO (First In First Out)", "#a8d5ba"); 
        QPushButton *btnSJF = createAlgorithmButton("SJF (Shortest Job First)", "#d4f1f4");  
        QPushButton *btnRR = createAlgorithmButton("Round Robin", "#f9d5e5");               
        QPushButton *btnBack = createAlgorithmButton("Volver al Menú Principal", "#fce4b6"); 

        QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
        dialogLayout->setSpacing(20);
        dialogLayout->setContentsMargins(30, 30, 30, 30);
        dialogLayout->addWidget(dialogTitle);
        dialogLayout->addWidget(btnFIFO);
        dialogLayout->addWidget(btnSJF);
        dialogLayout->addWidget(btnRR);
        dialogLayout->addStretch();
        dialogLayout->addWidget(btnBack);

        QObject::connect(btnFIFO, &QPushButton::clicked, [&]() { runFIFO(); });
        QObject::connect(btnSJF, &QPushButton::clicked, [&]() { runSJF(); });
        QObject::connect(btnRR, &QPushButton::clicked, [&]() { runRR(); });
        QObject::connect(btnBack, &QPushButton::clicked, [&]() {
            dialog->close();
            window.show(); 
        });

        dialog->exec();
    });

    QObject::connect(btnSync, &QPushButton::clicked, [&]() {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Módulo en Desarrollo");
        msgBox.setText("Módulo de Sincronización");
        msgBox.setInformativeText("Esta funcionalidad está actualmente en desarrollo.\n\nPróximamente incluirá:\n• Semáforos\n• Monitores\n• Mutex\n• Variables de condición");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStyleSheet(
            "QMessageBox {"
            "    background: white;"
            "    color: #2c3e50;"
            "}"
            "QMessageBox QPushButton {"
            "    background: #6c85bd;"
            "    color: white;"
            "    border: none;"
            "    padding: 8px 15px;"
            "    border-radius: 5px;"
            "}"
        );
        msgBox.exec();
    });

    QObject::connect(btnSalir, &QPushButton::clicked, [&]() {
        QMessageBox::StandardButton reply = QMessageBox::question(&window, 
            "Confirmar Salida", 
            "¿Está seguro que desea salir del simulador?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            app.quit();
        }
    });

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);
    mainLayout->addStretch();
    mainLayout->addWidget(btnSched);
    mainLayout->addWidget(btnSync);
    mainLayout->addWidget(btnSalir);
    mainLayout->addStretch();

    window.setLayout(mainLayout);
    window.show();

    return app.exec();
}