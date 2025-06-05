/****************************************************************************
** Meta object code from reading C++ file 'processsimulator.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../processsimulator.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'processsimulator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ProcessSimulator_t {
    const uint offsetsAndSize[18];
    char stringdata0[147];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_ProcessSimulator_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_ProcessSimulator_t qt_meta_stringdata_ProcessSimulator = {
    {
QT_MOC_LITERAL(0, 16), // "ProcessSimulator"
QT_MOC_LITERAL(17, 26), // "runNextAlgorithmInSequence"
QT_MOC_LITERAL(44, 0), // ""
QT_MOC_LITERAL(45, 14), // "cleanProcesses"
QT_MOC_LITERAL(60, 26), // "displayComparisonTableOnly"
QT_MOC_LITERAL(87, 10), // "algorithms"
QT_MOC_LITERAL(98, 19), // "std::vector<double>"
QT_MOC_LITERAL(118, 12), // "waitingTimes"
QT_MOC_LITERAL(131, 15) // "turnaroundTimes"

    },
    "ProcessSimulator\0runNextAlgorithmInSequence\0"
    "\0cleanProcesses\0displayComparisonTableOnly\0"
    "algorithms\0std::vector<double>\0"
    "waitingTimes\0turnaroundTimes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProcessSimulator[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x08,    1 /* Private */,
       3,    0,   33,    2, 0x08,    2 /* Private */,
       4,    3,   34,    2, 0x08,    3 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList, 0x80000000 | 6, 0x80000000 | 6,    5,    7,    8,

       0        // eod
};

void ProcessSimulator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ProcessSimulator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->runNextAlgorithmInSequence(); break;
        case 1: _t->cleanProcesses(); break;
        case 2: _t->displayComparisonTableOnly((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObject ProcessSimulator::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ProcessSimulator.offsetsAndSize,
    qt_meta_data_ProcessSimulator,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_ProcessSimulator_t
, QtPrivate::TypeAndForceComplete<ProcessSimulator, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>, QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>, QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>


>,
    nullptr
} };


const QMetaObject *ProcessSimulator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProcessSimulator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ProcessSimulator.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ProcessSimulator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
