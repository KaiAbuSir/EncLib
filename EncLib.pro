# -------------------------------------------------
# Project created by QtCreator 2009-12-10T18:19:05
# -------------------------------------------------
TARGET = EncLib
TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release
CONFIG += build_all

INCLUDEPATH += incl
INCLUDEPATH += ../geographiclib-1.1/include
INCLUDEPATH += ../iso8211lib-1_4

CONFIG(release, debug|release){
DESTDIR = ../$${TARGET}-build/$${QMAKE_CXX}/release
}
CONFIG(debug, debug|release){
DESTDIR = ../$${TARGET}-build/$${QMAKE_CXX}/debug
}

win32 {
DEFINES += _CRT_SECURE_NO_WARNINGS
}

message("Building with qmake version: " $$QMAKE_QMAKE " - Using compiler: " $$QMAKE_CXX)

SOURCES += src/exsetview.cpp \
    src/s57updater.cpp \
    src/iso8211_simple.cpp \
    src/helper.cpp \
    src/geo_projections.cpp \
    src/exsetview.cpp \
    src/exSetFilterWidget.cpp \
    src/exSetCheckS57.cpp \
    src/checksum.cpp \
    src/cellheader_widget.cpp \
    src/cellheader_tabmodel.cpp \
    src/cell_writer_iso8211dirty.cpp \
    src/cell_writer.cpp \
    src/cell_s57_update.cpp \
    src/cell_s57_iterators.cpp \
    src/cell_s57_base.cpp \
    src/cell_s57.cpp \
    src/cell_reocord_fields.cpp \
    src/cell_records_fields_header.cpp \
    src/cell_records.cpp \
    src/cell_parser_iso8211Gdal.cpp \
    src/cell_parser_iso8211dirty4updt.cpp \
    src/cell_parser_iso8211dirty4header.cpp \
    src/cell_parser_iso8211dirty4base.cpp \
    src/cell_parser_iso8211dirty.cpp \
    src/cell_parser_iso8211.cpp \
    src/cell_check_s57.cpp \
    src/cell_check_result.cpp \
    src/catalog031reader.cpp \
    src/boundingbox_edit.cpp \
    src/boundingbox_degrees.cpp \
    src/naviView.cpp \
    src/naviScene.cpp \
    src/naviWidget.cpp \
    src/naviSceneItems.cpp \
    src/dictionaryS52.cpp

HEADERS += incl/exsetview.h \
    incl/boundingbox_degrees.h \
    incl/version.h \
    incl/s57updater.h \
    incl/iso8211_simple.h \
    incl/helper.h \
    incl/geo_spheroids.h \
    incl/geo_projections.h \
    incl/exsetview.h \
    incl/exSetFilterWidget.h \
    incl/exSetCheckS57.h \
    incl/checksum.h \
    incl/cellheader_widget.h \
    incl/cellheader_tabmodel.h \
    incl/cell_writer_iso8211dirty.h \
    incl/cell_writer.h \
    incl/cell_s57_update.h \
    incl/cell_s57_iterators.h \
    incl/cell_s57_base.h \
    incl/cell_s57.h \
    incl/cell_records.h \
    incl/cell_record_fields.h \
    incl/cell_parser_iso8211Gdal.h \
    incl/cell_parser_iso8211dirty4updt.h \
    incl/cell_parser_iso8211dirty4header.h \
    incl/cell_parser_iso8211dirty4base.h \
    incl/cell_parser_iso8211dirty.h \
    incl/cell_parser_iso8211.h \
    incl/cell_check_s57.h \
    incl/cell_check_result.h \
    incl/catalog031reader.h \
    incl/boundingbox_edit.h \
    incl/boundingbox_degrees.h \
    incl/naviView.h \
    incl/naviScene.h \
    incl/naviSceneItems.h \
    incl/dictionaryS52.h \
    incl/naviWidget.h

OTHER_FILES += \
    incl/GNU_V2.inc \
    incl/cell_writer_iso8211dirtyDDR.incl

RESOURCES += \
    EncLibQt.qrc
