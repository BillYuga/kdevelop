project( okteta )
add_definitions(-DTRANSLATION_DOMAIN=\"kdevokteta\")

include_directories( ${okteta_SOURCE_DIR} ${okteta_BINARY_DIR} )

SET( kdevokteta_PART_SRCS
    kastentoolviewwidget.cpp
    oktetatoolviewfactory.cpp
    oktetaplugin.cpp
    oktetadocument.cpp
    oktetaview.cpp
    oktetawidget.cpp
)

qt5_add_resources(kdevokteta_PART_SRCS kdevokteta.qrc)
kdevplatform_add_plugin(kdevokteta JSON kdevokteta.json SOURCES
	${kdevokteta_PART_SRCS})

TARGET_LINK_LIBRARIES( kdevokteta
    KDev::Interfaces
    KDev::Project
    KDev::Sublime
    KDev::Shell
    KF5::WidgetsAddons
    OktetaKastenControllers
    KastenControllers
)

install(TARGETS kdevokteta DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
