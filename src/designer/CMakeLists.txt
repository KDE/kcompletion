include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KComboBox
    TOOLTIP "Combo Box (KF5)"
    GROUP "Input (KF5)"
)
ecm_qtdesignerplugin_widget(KHistoryComboBox
    TOOLTIP "A combobox for offering a history and completion (KF5)"
    WHATSTHIS "A combobox which implements a history like a unix shell"
    GROUP "Input (KF5)"
)
ecm_qtdesignerplugin_widget(KLineEdit
    TOOLTIP "Line Edit (KF5)"
    GROUP "Input (KF5)"
)

ecm_add_qtdesignerplugin(kcompletionwidgets
    NAME KCompletionWidgets
    OUTPUT_NAME kcompletion5widgets
    WIDGETS
        KComboBox
        KHistoryComboBox
        KLineEdit
    LINK_LIBRARIES
        KF5::Completion
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
