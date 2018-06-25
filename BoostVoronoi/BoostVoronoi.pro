#include(../rasterwindow/rasterwindow.pri)

INCLUDEPATH += C:/libs/boost_1_67_0

# work-around for QTBUG-13496
#CONFIG += no_batch

SOURCES += \
    voronoi_visualizer.cpp

#target.path = $$[QT_INSTALL_EXAMPLES]/gui/analogclock
#INSTALLS += target

LIBS += -lOpenGL32

QT += widgets opengl
