/****************************************************************************
** Meta object code from reading C++ file 'AppController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/app/AppController.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_AppController_t {
    uint offsetsAndSizes[46];
    char stringdata0[14];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[20];
    char stringdata4[24];
    char stringdata5[27];
    char stringdata6[29];
    char stringdata7[11];
    char stringdata8[12];
    char stringdata9[13];
    char stringdata10[6];
    char stringdata11[21];
    char stringdata12[26];
    char stringdata13[22];
    char stringdata14[19];
    char stringdata15[20];
    char stringdata16[14];
    char stringdata17[16];
    char stringdata18[14];
    char stringdata19[13];
    char stringdata20[17];
    char stringdata21[20];
    char stringdata22[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_AppController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_AppController_t qt_meta_stringdata_AppController = {
    {
        QT_MOC_LITERAL(0, 13),  // "AppController"
        QT_MOC_LITERAL(14, 20),  // "playerVisibleChanged"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 19),  // "currentIndexChanged"
        QT_MOC_LITERAL(56, 23),  // "currentVideoNameChanged"
        QT_MOC_LITERAL(80, 26),  // "resumePromptVisibleChanged"
        QT_MOC_LITERAL(107, 28),  // "pendingResumePositionChanged"
        QT_MOC_LITERAL(136, 10),  // "initialize"
        QT_MOC_LITERAL(147, 11),  // "videoFolder"
        QT_MOC_LITERAL(159, 12),  // "playSelected"
        QT_MOC_LITERAL(172, 5),  // "index"
        QT_MOC_LITERAL(178, 20),  // "decideResumePlayback"
        QT_MOC_LITERAL(199, 25),  // "continueFromSavedPosition"
        QT_MOC_LITERAL(225, 21),  // "canDeleteCurrentVideo"
        QT_MOC_LITERAL(247, 18),  // "deleteCurrentVideo"
        QT_MOC_LITERAL(266, 19),  // "resetResumeDatabase"
        QT_MOC_LITERAL(286, 13),  // "backToBrowser"
        QT_MOC_LITERAL(300, 15),  // "setCurrentIndex"
        QT_MOC_LITERAL(316, 13),  // "playerVisible"
        QT_MOC_LITERAL(330, 12),  // "currentIndex"
        QT_MOC_LITERAL(343, 16),  // "currentVideoName"
        QT_MOC_LITERAL(360, 19),  // "resumePromptVisible"
        QT_MOC_LITERAL(380, 21)   // "pendingResumePosition"
    },
    "AppController",
    "playerVisibleChanged",
    "",
    "currentIndexChanged",
    "currentVideoNameChanged",
    "resumePromptVisibleChanged",
    "pendingResumePositionChanged",
    "initialize",
    "videoFolder",
    "playSelected",
    "index",
    "decideResumePlayback",
    "continueFromSavedPosition",
    "canDeleteCurrentVideo",
    "deleteCurrentVideo",
    "resetResumeDatabase",
    "backToBrowser",
    "setCurrentIndex",
    "playerVisible",
    "currentIndex",
    "currentVideoName",
    "resumePromptVisible",
    "pendingResumePosition"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_AppController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       5,  113, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x06,    6 /* Public */,
       3,    0,   93,    2, 0x06,    7 /* Public */,
       4,    0,   94,    2, 0x06,    8 /* Public */,
       5,    0,   95,    2, 0x06,    9 /* Public */,
       6,    0,   96,    2, 0x06,   10 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       7,    1,   97,    2, 0x02,   11 /* Public */,
       9,    1,  100,    2, 0x02,   13 /* Public */,
      11,    1,  103,    2, 0x02,   15 /* Public */,
      13,    0,  106,    2, 0x102,   17 /* Public | MethodIsConst  */,
      14,    0,  107,    2, 0x02,   18 /* Public */,
      15,    0,  108,    2, 0x02,   19 /* Public */,
      16,    0,  109,    2, 0x02,   20 /* Public */,
      17,    1,  110,    2, 0x02,   21 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   10,

 // properties: name, type, flags
      18, QMetaType::Bool, 0x00015001, uint(0), 0,
      19, QMetaType::Int, 0x00015001, uint(1), 0,
      20, QMetaType::QString, 0x00015001, uint(2), 0,
      21, QMetaType::Bool, 0x00015001, uint(3), 0,
      22, QMetaType::Double, 0x00015001, uint(4), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject AppController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AppController.offsetsAndSizes,
    qt_meta_data_AppController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_AppController_t,
        // property 'playerVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'currentIndex'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'currentVideoName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'resumePromptVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'pendingResumePosition'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AppController, std::true_type>,
        // method 'playerVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'currentIndexChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'currentVideoNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'resumePromptVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'pendingResumePositionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'initialize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'playSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'decideResumePlayback'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'canDeleteCurrentVideo'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'deleteCurrentVideo'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'resetResumeDatabase'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'backToBrowser'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setCurrentIndex'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void AppController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AppController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->playerVisibleChanged(); break;
        case 1: _t->currentIndexChanged(); break;
        case 2: _t->currentVideoNameChanged(); break;
        case 3: _t->resumePromptVisibleChanged(); break;
        case 4: _t->pendingResumePositionChanged(); break;
        case 5: _t->initialize((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->playSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->decideResumePlayback((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: { bool _r = _t->canDeleteCurrentVideo();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->deleteCurrentVideo();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->resetResumeDatabase();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->backToBrowser(); break;
        case 12: _t->setCurrentIndex((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AppController::*)();
            if (_t _q_method = &AppController::playerVisibleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AppController::*)();
            if (_t _q_method = &AppController::currentIndexChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AppController::*)();
            if (_t _q_method = &AppController::currentVideoNameChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (AppController::*)();
            if (_t _q_method = &AppController::resumePromptVisibleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (AppController::*)();
            if (_t _q_method = &AppController::pendingResumePositionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<AppController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->playerVisible(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->currentIndex(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->currentVideoName(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->resumePromptVisible(); break;
        case 4: *reinterpret_cast< double*>(_v) = _t->pendingResumePosition(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *AppController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AppController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AppController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void AppController::playerVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AppController::currentIndexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AppController::currentVideoNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void AppController::resumePromptVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void AppController::pendingResumePositionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
