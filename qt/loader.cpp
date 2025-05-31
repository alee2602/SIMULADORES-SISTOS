#include "loader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QColor>

std::vector<Process> loadProcesses(const QString& filename) {
    std::vector<Process> processes;
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << filename;
        return processes;
    }
    
    QTextStream in(&file);
    QStringList colors = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7", 
                         "#DDA0DD", "#F0E68C", "#FFB6C1", "#87CEEB", "#98FB98"};
    int colorIndex = 0;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue; // Skip empty lines and comments
        
        QStringList parts = line.split(",");
        if (parts.size() >= 4) {
            Process p;
            p.pid = parts[0].trimmed();
            p.burst_time = parts[1].trimmed().toInt();
            p.arrival_time = parts[2].trimmed().toInt();
            p.priority = parts[3].trimmed().toInt();
            p.remaining_time = p.burst_time;
            p.color = QColor(colors[colorIndex % colors.size()]);
            colorIndex++;
            
            processes.push_back(p);
        }
    }
    
    file.close();
    return processes;
}

std::vector<Resource> loadResources(const QString& filename) {
    std::vector<Resource> resources;
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << filename;
        return resources;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        QStringList parts = line.split(",");
        if (parts.size() >= 2) {
            Resource r;
            r.name = parts[0].trimmed();
            r.count = parts[1].trimmed().toInt();
            r.available = r.count;
            
            resources.push_back(r);
        }
    }
    
    file.close();
    return resources;
}

std::vector<Action> loadActions(const QString& filename) {
    std::vector<Action> actions;
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << filename;
        return actions;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        QStringList parts = line.split(",");
        if (parts.size() >= 4) {
            Action a;
            a.pid = parts[0].trimmed();
            a.type = parts[1].trimmed().toUpper();
            a.resource = parts[2].trimmed();
            a.cycle = parts[3].trimmed().toInt();
            
            actions.push_back(a);
        }
    }
    
    file.close();
    return actions;
}