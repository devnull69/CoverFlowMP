/****************************************************************************
** Meta object code from reading C++ file 'PlayerController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/player/PlayerController.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlayerController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_PlayerController_t {
    uint offsetsAndSizes[34];
    char stringdata0[17];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[16];
    char stringdata5[9];
    char stringdata6[9];
    char stringdata7[14];
    char stringdata8[12];
    char stringdata9[13];
    char stringdata10[8];
    char stringdata11[5];
    char stringdata12[15];
    char stringdata13[13];
    char stringdata14[7];
    char stringdata15[9];
    char stringdata16[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PlayerController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_PlayerController_t qt_meta_stringdata_PlayerController = {
    {
        QT_MOC_LITERAL(0, 16),  // "PlayerController"
        QT_MOC_LITERAL(17, 13),  // "pausedChanged"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 15),  // "positionChanged"
        QT_MOC_LITERAL(48, 15),  // "durationChanged"
        QT_MOC_LITERAL(64, 8),  // "playFile"
        QT_MOC_LITERAL(73, 8),  // "filePath"
        QT_MOC_LITERAL(82, 13),  // "startPosition"
        QT_MOC_LITERAL(96, 11),  // "togglePause"
        QT_MOC_LITERAL(108, 12),  // "seekRelative"
        QT_MOC_LITERAL(121, 7),  // "seconds"
        QT_MOC_LITERAL(129, 4),  // "stop"
        QT_MOC_LITERAL(134, 14),  // "attachToWindow"
        QT_MOC_LITERAL(149, 12),  // "windowObject"
        QT_MOC_LITERAL(162, 6),  // "paused"
        QT_MOC_LITERAL(169, 8),  // "position"
        QT_MOC_LITERAL(178, 8)   // "duration"
    },
    "PlayerController",
    "pausedChanged",
    "",
    "positionChanged",
    "durationChanged",
    "playFile",
    "filePath",
    "startPosition",
    "togglePause",
    "seekRelative",
    "seconds",
    "stop",
    "attachToWindow",
    "windowObject",
    "paused",
    "position",
    "duration"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PlayerController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       3,   87, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    4 /* Public */,
       3,    0,   69,    2, 0x06,    5 /* Public */,
       4,    0,   70,    2, 0x06,    6 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       5,    2,   71,    2, 0x02,    7 /* Public */,
       5,    1,   76,    2, 0x22,   10 /* Public | MethodCloned */,
       8,    0,   79,    2, 0x02,   12 /* Public */,
       9,    1,   80,    2, 0x02,   13 /* Public */,
      11,    0,   83,    2, 0x02,   15 /* Public */,
      12,    1,   84,    2, 0x02,   16 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Double,    6,    7,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QObjectStar,   13,

 // properties: name, type, flags
      14, QMetaType::Bool, 0x00015001, uint(0), 0,
      15, QMetaType::Double, 0x00015001, uint(1), 0,
      16, QMetaType::Double, 0x00015001, uint(2), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject PlayerController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PlayerController.offsetsAndSizes,
    qt_meta_data_PlayerController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PlayerController_t,
        // property 'paused'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'position'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'duration'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PlayerController, std::true_type>,
        // method 'pausedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'positionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'durationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'playFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'playFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'togglePause'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'seekRelative'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'stop'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'attachToWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QObject *, std::false_type>
    >,
    nullptr
} };

void PlayerController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlayerController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->pausedChanged(); break;
        case 1: _t->positionChanged(); break;
        case 2: _t->durationChanged(); break;
        case 3: _t->playFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 4: _t->playFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->togglePause(); break;
        case 6: _t->seekRelative((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->stop(); break;
        case 8: _t->attachToWindow((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::pausedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::positionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::durationChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PlayerController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->paused(); break;
        case 1: *reinterpret_cast< double*>(_v) = _t->position(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->duration(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *PlayerController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlayerController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlayerController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlayerController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void PlayerController::pausedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PlayerController::positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PlayerController::durationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
