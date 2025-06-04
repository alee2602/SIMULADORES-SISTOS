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
#include <QFileDialog>
#include <QTextEdit>
#include <QScrollArea>
#include <QPainter>
#include <QTimer>
#include <QProgressBar>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStackedWidget>
#include <QButtonGroup>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <map>
#include "synchronizer.h"
#include "scheduler.h"
#include "processsimulator.h"
#include "ganttchartwidget.h"

using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application style
    app.setStyleSheet(
        "QWidget { font-family: 'Segoe UI', Arial, sans-serif; }"
        "QTableWidget { gridline-color: #e0e0e0; }"
        "QTableWidget::item { padding: 8px; }"
        "QTableWidget::item:selected { background-color: #3498db; color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; "
        "padding: 10px; border: none; font-weight: bold; }"
        "QSpinBox { padding: 5px; border: 1px solid #ddd; border-radius: 4px; }"
        "QLabel { color: #2c3e50; }"
    );

    ProcessSimulator simulator;
    simulator.show();

    return app.exec();
}
