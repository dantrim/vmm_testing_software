# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'mainwindow.ui',
# licensing of 'mainwindow.ui' applies.
#
# Created: Mon Nov 25 11:13:55 2019
#      by: pyside2-uic  running on PySide2 5.13.1
#
# WARNING! All changes made in this file will be lost!

from PySide2 import QtCore, QtGui, QtWidgets

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(883, 463)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.tabWidget = QtWidgets.QTabWidget(self.centralwidget)
        self.tabWidget.setGeometry(QtCore.QRect(10, 10, 691, 421))
        self.tabWidget.setObjectName("tabWidget")
        self.tab0 = QtWidgets.QWidget()
        self.tab0.setObjectName("tab0")
        self.layoutWidget = QtWidgets.QWidget(self.tab0)
        self.layoutWidget.setGeometry(QtCore.QRect(3, 10, 681, 371))
        self.layoutWidget.setObjectName("layoutWidget")
        self.verticalLayout_5 = QtWidgets.QVBoxLayout(self.layoutWidget)
        self.verticalLayout_5.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_5.setObjectName("verticalLayout_5")
        self.groupBox = QtWidgets.QGroupBox(self.layoutWidget)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.groupBox.setFont(font)
        self.groupBox.setFlat(False)
        self.groupBox.setCheckable(False)
        self.groupBox.setObjectName("groupBox")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.groupBox)
        self.gridLayout_5.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.verticalLayout_4 = QtWidgets.QVBoxLayout()
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setSpacing(1)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label_11 = QtWidgets.QLabel(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.label_11.setFont(font)
        self.label_11.setObjectName("label_11")
        self.horizontalLayout.addWidget(self.label_11)
        self.lineEdit_vts_config_file = QtWidgets.QLineEdit(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.lineEdit_vts_config_file.setFont(font)
        self.lineEdit_vts_config_file.setObjectName("lineEdit_vts_config_file")
        self.horizontalLayout.addWidget(self.lineEdit_vts_config_file)
        self.pushButton = QtWidgets.QPushButton(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.pushButton.setFont(font)
        self.pushButton.setObjectName("pushButton")
        self.horizontalLayout.addWidget(self.pushButton)
        self.button_vts_config_check = QtWidgets.QPushButton(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.button_vts_config_check.setFont(font)
        self.button_vts_config_check.setIconSize(QtCore.QSize(16, 16))
        self.button_vts_config_check.setObjectName("button_vts_config_check")
        self.horizontalLayout.addWidget(self.button_vts_config_check)
        self.verticalLayout_4.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setSpacing(4)
        self.horizontalLayout_2.setSizeConstraint(QtWidgets.QLayout.SetFixedSize)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label_13 = QtWidgets.QLabel(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.label_13.setFont(font)
        self.label_13.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.label_13.setObjectName("label_13")
        self.horizontalLayout_2.addWidget(self.label_13)
        self.button_start_vts_server = QtWidgets.QPushButton(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.button_start_vts_server.setFont(font)
        self.button_start_vts_server.setObjectName("button_start_vts_server")
        self.horizontalLayout_2.addWidget(self.button_start_vts_server)
        self.button_shutdown_vts_server = QtWidgets.QPushButton(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.button_shutdown_vts_server.setFont(font)
        self.button_shutdown_vts_server.setObjectName("button_shutdown_vts_server")
        self.horizontalLayout_2.addWidget(self.button_shutdown_vts_server)
        self.button_ping_vts_server = QtWidgets.QPushButton(self.groupBox)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.button_ping_vts_server.setFont(font)
        self.button_ping_vts_server.setObjectName("button_ping_vts_server")
        self.horizontalLayout_2.addWidget(self.button_ping_vts_server)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem)
        self.frame_3 = QtWidgets.QFrame(self.groupBox)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.frame_3.sizePolicy().hasHeightForWidth())
        self.frame_3.setSizePolicy(sizePolicy)
        self.frame_3.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_3.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_3.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_3.setLineWidth(1)
        self.frame_3.setObjectName("frame_3")
        self.gridLayout_6 = QtWidgets.QGridLayout(self.frame_3)
        self.gridLayout_6.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.label_vts_server_status = QtWidgets.QLabel(self.frame_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_vts_server_status.sizePolicy().hasHeightForWidth())
        self.label_vts_server_status.setSizePolicy(sizePolicy)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_vts_server_status.setFont(font)
        self.label_vts_server_status.setAlignment(QtCore.Qt.AlignCenter)
        self.label_vts_server_status.setObjectName("label_vts_server_status")
        self.gridLayout_6.addWidget(self.label_vts_server_status, 0, 0, 1, 1)
        self.horizontalLayout_2.addWidget(self.frame_3)
        self.verticalLayout_4.addLayout(self.horizontalLayout_2)
        self.gridLayout_5.addLayout(self.verticalLayout_4, 0, 0, 1, 1)
        self.verticalLayout_5.addWidget(self.groupBox)
        self.groupBox_3 = QtWidgets.QGroupBox(self.layoutWidget)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.groupBox_3.setFont(font)
        self.groupBox_3.setObjectName("groupBox_3")
        self.gridLayout_8 = QtWidgets.QGridLayout(self.groupBox_3)
        self.gridLayout_8.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_8.setObjectName("gridLayout_8")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_3.setSpacing(4)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.label_18 = QtWidgets.QLabel(self.groupBox_3)
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setWeight(50)
        font.setBold(False)
        self.label_18.setFont(font)
        self.label_18.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.label_18.setObjectName("label_18")
        self.horizontalLayout_3.addWidget(self.label_18)
        self.button_acquire_vmm_serial = QtWidgets.QPushButton(self.groupBox_3)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_acquire_vmm_serial.setFont(font)
        self.button_acquire_vmm_serial.setObjectName("button_acquire_vmm_serial")
        self.horizontalLayout_3.addWidget(self.button_acquire_vmm_serial)
        self.button_ping_camera = QtWidgets.QPushButton(self.groupBox_3)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_ping_camera.setFont(font)
        self.button_ping_camera.setObjectName("button_ping_camera")
        self.horizontalLayout_3.addWidget(self.button_ping_camera)
        spacerItem1 = QtWidgets.QSpacerItem(37, 17, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem1)
        self.frame_4 = QtWidgets.QFrame(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.frame_4.sizePolicy().hasHeightForWidth())
        self.frame_4.setSizePolicy(sizePolicy)
        self.frame_4.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_4.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_4.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_4.setObjectName("frame_4")
        self.gridLayout_7 = QtWidgets.QGridLayout(self.frame_4)
        self.gridLayout_7.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_7.setObjectName("gridLayout_7")
        self.label_vts_vmm_sn = QtWidgets.QLabel(self.frame_4)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding, QtWidgets.QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_vts_vmm_sn.sizePolicy().hasHeightForWidth())
        self.label_vts_vmm_sn.setSizePolicy(sizePolicy)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_vts_vmm_sn.setFont(font)
        self.label_vts_vmm_sn.setAlignment(QtCore.Qt.AlignCenter)
        self.label_vts_vmm_sn.setTextInteractionFlags(QtCore.Qt.LinksAccessibleByMouse|QtCore.Qt.TextEditable)
        self.label_vts_vmm_sn.setObjectName("label_vts_vmm_sn")
        self.gridLayout_7.addWidget(self.label_vts_vmm_sn, 0, 0, 1, 1)
        self.horizontalLayout_3.addWidget(self.frame_4)
        self.gridLayout_8.addLayout(self.horizontalLayout_3, 0, 0, 1, 1)
        self.verticalLayout_5.addWidget(self.groupBox_3)
        self.groupBox_2 = QtWidgets.QGroupBox(self.layoutWidget)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.groupBox_2.setFont(font)
        self.groupBox_2.setObjectName("groupBox_2")
        self.gridLayout_14 = QtWidgets.QGridLayout(self.groupBox_2)
        self.gridLayout_14.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_14.setObjectName("gridLayout_14")
        self.verticalLayout_10 = QtWidgets.QVBoxLayout()
        self.verticalLayout_10.setSpacing(4)
        self.verticalLayout_10.setObjectName("verticalLayout_10")
        self.horizontalLayout_9 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_9.setObjectName("horizontalLayout_9")
        self.label_14 = QtWidgets.QLabel(self.groupBox_2)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_14.setFont(font)
        self.label_14.setObjectName("label_14")
        self.horizontalLayout_9.addWidget(self.label_14)
        self.lineEdit_test_dir = QtWidgets.QLineEdit(self.groupBox_2)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.lineEdit_test_dir.setFont(font)
        self.lineEdit_test_dir.setObjectName("lineEdit_test_dir")
        self.horizontalLayout_9.addWidget(self.lineEdit_test_dir)
        self.button_tests_load = QtWidgets.QPushButton(self.groupBox_2)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_tests_load.setFont(font)
        self.button_tests_load.setObjectName("button_tests_load")
        self.horizontalLayout_9.addWidget(self.button_tests_load)
        self.verticalLayout_10.addLayout(self.horizontalLayout_9)
        self.horizontalLayout_10 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_10.setObjectName("horizontalLayout_10")
        self.listWidget_loaded_tests = QtWidgets.QListWidget(self.groupBox_2)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.listWidget_loaded_tests.setFont(font)
        self.listWidget_loaded_tests.setDragEnabled(True)
        self.listWidget_loaded_tests.setDefaultDropAction(QtCore.Qt.MoveAction)
        self.listWidget_loaded_tests.setAlternatingRowColors(True)
        self.listWidget_loaded_tests.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)
        self.listWidget_loaded_tests.setMovement(QtWidgets.QListView.Snap)
        self.listWidget_loaded_tests.setObjectName("listWidget_loaded_tests")
        self.horizontalLayout_10.addWidget(self.listWidget_loaded_tests)
        self.verticalLayout_8 = QtWidgets.QVBoxLayout()
        self.verticalLayout_8.setSpacing(6)
        self.verticalLayout_8.setContentsMargins(1, 1, 1, 1)
        self.verticalLayout_8.setObjectName("verticalLayout_8")
        self.button_tests_start = QtWidgets.QPushButton(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_tests_start.sizePolicy().hasHeightForWidth())
        self.button_tests_start.setSizePolicy(sizePolicy)
        self.button_tests_start.setObjectName("button_tests_start")
        self.verticalLayout_8.addWidget(self.button_tests_start)
        self.button_tests_stop = QtWidgets.QPushButton(self.groupBox_2)
        self.button_tests_stop.setEnabled(False)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_tests_stop.sizePolicy().hasHeightForWidth())
        self.button_tests_stop.setSizePolicy(sizePolicy)
        self.button_tests_stop.setObjectName("button_tests_stop")
        self.verticalLayout_8.addWidget(self.button_tests_stop)
        self.horizontalLayout_10.addLayout(self.verticalLayout_8)
        self.verticalLayout_10.addLayout(self.horizontalLayout_10)
        self.gridLayout_14.addLayout(self.verticalLayout_10, 0, 0, 1, 1)
        self.verticalLayout_5.addWidget(self.groupBox_2)
        self.horizontalLayout_11 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_11.setSpacing(4)
        self.horizontalLayout_11.setObjectName("horizontalLayout_11")
        self.label_25 = QtWidgets.QLabel(self.layoutWidget)
        self.label_25.setAlignment(QtCore.Qt.AlignCenter)
        self.label_25.setObjectName("label_25")
        self.horizontalLayout_11.addWidget(self.label_25)
        self.progressBar = QtWidgets.QProgressBar(self.layoutWidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.progressBar.sizePolicy().hasHeightForWidth())
        self.progressBar.setSizePolicy(sizePolicy)
        self.progressBar.setSizeIncrement(QtCore.QSize(0, 0))
        self.progressBar.setBaseSize(QtCore.QSize(0, 4))
        self.progressBar.setProperty("value", 24)
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setInvertedAppearance(False)
        self.progressBar.setObjectName("progressBar")
        self.horizontalLayout_11.addWidget(self.progressBar)
        self.verticalLayout_5.addLayout(self.horizontalLayout_11)
        self.tabWidget.addTab(self.tab0, "")
        self.tab1 = QtWidgets.QWidget()
        self.tab1.setObjectName("tab1")
        self.layoutWidget1 = QtWidgets.QWidget(self.tab1)
        self.layoutWidget1.setGeometry(QtCore.QRect(3, 0, 681, 261))
        self.layoutWidget1.setObjectName("layoutWidget1")
        self.verticalLayout_7 = QtWidgets.QVBoxLayout(self.layoutWidget1)
        self.verticalLayout_7.setSpacing(1)
        self.verticalLayout_7.setContentsMargins(2, 2, 2, 2)
        self.verticalLayout_7.setObjectName("verticalLayout_7")
        self.groupBox_4 = QtWidgets.QGroupBox(self.layoutWidget1)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.groupBox_4.setFont(font)
        self.groupBox_4.setObjectName("groupBox_4")
        self.gridLayout_12 = QtWidgets.QGridLayout(self.groupBox_4)
        self.gridLayout_12.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_12.setVerticalSpacing(0)
        self.gridLayout_12.setObjectName("gridLayout_12")
        self.verticalLayout_6 = QtWidgets.QVBoxLayout()
        self.verticalLayout_6.setSpacing(4)
        self.verticalLayout_6.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_6.setObjectName("verticalLayout_6")
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_4.setSpacing(4)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.label_16 = QtWidgets.QLabel(self.groupBox_4)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_16.setFont(font)
        self.label_16.setObjectName("label_16")
        self.horizontalLayout_4.addWidget(self.label_16)
        self.lineEdit = QtWidgets.QLineEdit(self.groupBox_4)
        self.lineEdit.setObjectName("lineEdit")
        self.horizontalLayout_4.addWidget(self.lineEdit)
        self.pushButton_6 = QtWidgets.QPushButton(self.groupBox_4)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.pushButton_6.setFont(font)
        self.pushButton_6.setObjectName("pushButton_6")
        self.horizontalLayout_4.addWidget(self.pushButton_6)
        self.pushButton_15 = QtWidgets.QPushButton(self.groupBox_4)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.pushButton_15.setFont(font)
        self.pushButton_15.setObjectName("pushButton_15")
        self.horizontalLayout_4.addWidget(self.pushButton_15)
        self.verticalLayout_6.addLayout(self.horizontalLayout_4)
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_5.setSpacing(4)
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.button_fpga_configure = QtWidgets.QPushButton(self.groupBox_4)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_fpga_configure.setFont(font)
        self.button_fpga_configure.setObjectName("button_fpga_configure")
        self.horizontalLayout_5.addWidget(self.button_fpga_configure)
        self.button_fpga_ping = QtWidgets.QPushButton(self.groupBox_4)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_fpga_ping.setFont(font)
        self.button_fpga_ping.setObjectName("button_fpga_ping")
        self.horizontalLayout_5.addWidget(self.button_fpga_ping)
        spacerItem2 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_5.addItem(spacerItem2)
        self.frame_5 = QtWidgets.QFrame(self.groupBox_4)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.frame_5.sizePolicy().hasHeightForWidth())
        self.frame_5.setSizePolicy(sizePolicy)
        self.frame_5.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_5.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_5.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_5.setLineWidth(1)
        self.frame_5.setObjectName("frame_5")
        self.gridLayout_9 = QtWidgets.QGridLayout(self.frame_5)
        self.gridLayout_9.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_9.setObjectName("gridLayout_9")
        self.label_17 = QtWidgets.QLabel(self.frame_5)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_17.setFont(font)
        self.label_17.setAlignment(QtCore.Qt.AlignCenter)
        self.label_17.setObjectName("label_17")
        self.gridLayout_9.addWidget(self.label_17, 0, 0, 1, 1)
        self.horizontalLayout_5.addWidget(self.frame_5)
        self.verticalLayout_6.addLayout(self.horizontalLayout_5)
        self.gridLayout_12.addLayout(self.verticalLayout_6, 0, 0, 1, 1)
        self.verticalLayout_7.addWidget(self.groupBox_4)
        self.groupBox_5 = QtWidgets.QGroupBox(self.layoutWidget1)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.groupBox_5.setFont(font)
        self.groupBox_5.setObjectName("groupBox_5")
        self.gridLayout_13 = QtWidgets.QGridLayout(self.groupBox_5)
        self.gridLayout_13.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_13.setVerticalSpacing(0)
        self.gridLayout_13.setObjectName("gridLayout_13")
        self.horizontalLayout_6 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_6.setObjectName("horizontalLayout_6")
        self.label_19 = QtWidgets.QLabel(self.groupBox_5)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_19.setFont(font)
        self.label_19.setObjectName("label_19")
        self.horizontalLayout_6.addWidget(self.label_19)
        self.lineEdit_2 = QtWidgets.QLineEdit(self.groupBox_5)
        self.lineEdit_2.setObjectName("lineEdit_2")
        self.horizontalLayout_6.addWidget(self.lineEdit_2)
        self.pushButton_12 = QtWidgets.QPushButton(self.groupBox_5)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.pushButton_12.setFont(font)
        self.pushButton_12.setObjectName("pushButton_12")
        self.horizontalLayout_6.addWidget(self.pushButton_12)
        self.pushButton_16 = QtWidgets.QPushButton(self.groupBox_5)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.pushButton_16.setFont(font)
        self.pushButton_16.setObjectName("pushButton_16")
        self.horizontalLayout_6.addWidget(self.pushButton_16)
        self.gridLayout_13.addLayout(self.horizontalLayout_6, 0, 0, 1, 1)
        self.horizontalLayout_7 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_7.setSpacing(4)
        self.horizontalLayout_7.setObjectName("horizontalLayout_7")
        self.button_vmm_configure = QtWidgets.QPushButton(self.groupBox_5)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_vmm_configure.setFont(font)
        self.button_vmm_configure.setObjectName("button_vmm_configure")
        self.horizontalLayout_7.addWidget(self.button_vmm_configure)
        self.button_vmm_reset = QtWidgets.QPushButton(self.groupBox_5)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_vmm_reset.setFont(font)
        self.button_vmm_reset.setObjectName("button_vmm_reset")
        self.horizontalLayout_7.addWidget(self.button_vmm_reset)
        spacerItem3 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_7.addItem(spacerItem3)
        self.frame_6 = QtWidgets.QFrame(self.groupBox_5)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.frame_6.sizePolicy().hasHeightForWidth())
        self.frame_6.setSizePolicy(sizePolicy)
        self.frame_6.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_6.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_6.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_6.setLineWidth(1)
        self.frame_6.setObjectName("frame_6")
        self.gridLayout_11 = QtWidgets.QGridLayout(self.frame_6)
        self.gridLayout_11.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_11.setObjectName("gridLayout_11")
        self.label_20 = QtWidgets.QLabel(self.frame_6)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.label_20.setFont(font)
        self.label_20.setAlignment(QtCore.Qt.AlignCenter)
        self.label_20.setObjectName("label_20")
        self.gridLayout_11.addWidget(self.label_20, 0, 0, 1, 1)
        self.horizontalLayout_7.addWidget(self.frame_6)
        self.gridLayout_13.addLayout(self.horizontalLayout_7, 1, 0, 1, 1)
        self.verticalLayout_7.addWidget(self.groupBox_5)
        self.groupBox_6 = QtWidgets.QGroupBox(self.layoutWidget1)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.groupBox_6.setFont(font)
        self.groupBox_6.setObjectName("groupBox_6")
        self.gridLayout_10 = QtWidgets.QGridLayout(self.groupBox_6)
        self.gridLayout_10.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_10.setVerticalSpacing(0)
        self.gridLayout_10.setObjectName("gridLayout_10")
        self.horizontalLayout_8 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_8.setSpacing(4)
        self.horizontalLayout_8.setObjectName("horizontalLayout_8")
        self.button_acq_on = QtWidgets.QPushButton(self.groupBox_6)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_acq_on.setFont(font)
        self.button_acq_on.setObjectName("button_acq_on")
        self.horizontalLayout_8.addWidget(self.button_acq_on)
        self.button_acq_off = QtWidgets.QPushButton(self.groupBox_6)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.button_acq_off.setFont(font)
        self.button_acq_off.setObjectName("button_acq_off")
        self.horizontalLayout_8.addWidget(self.button_acq_off)
        spacerItem4 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_8.addItem(spacerItem4)
        self.gridLayout_10.addLayout(self.horizontalLayout_8, 0, 0, 1, 1)
        self.verticalLayout_7.addWidget(self.groupBox_6)
        self.tabWidget.addTab(self.tab1, "")
        self.tab3 = QtWidgets.QWidget()
        self.tab3.setObjectName("tab3")
        self.tabWidget.addTab(self.tab3, "")
        self.frame = QtWidgets.QFrame(self.centralwidget)
        self.frame.setGeometry(QtCore.QRect(710, 80, 162, 181))
        self.frame.setFrameShape(QtWidgets.QFrame.Box)
        self.frame.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame.setLineWidth(2)
        self.frame.setObjectName("frame")
        self.gridLayout = QtWidgets.QGridLayout(self.frame)
        self.gridLayout.setContentsMargins(1, 1, 1, 1)
        self.gridLayout.setObjectName("gridLayout")
        self.verticalLayout = QtWidgets.QVBoxLayout()
        self.verticalLayout.setSpacing(1)
        self.verticalLayout.setObjectName("verticalLayout")
        self.label = QtWidgets.QLabel(self.frame)
        self.label.setFrameShape(QtWidgets.QFrame.Box)
        self.label.setText("")
        self.label.setPixmap(QtGui.QPixmap("interface/resources/uci.png"))
        self.label.setScaledContents(True)
        self.label.setObjectName("label")
        self.verticalLayout.addWidget(self.label)
        self.label_2 = QtWidgets.QLabel(self.frame)
        self.label_2.setFrameShape(QtWidgets.QFrame.Box)
        self.label_2.setText("")
        self.label_2.setPixmap(QtGui.QPixmap("interface/resources/ntua.png"))
        self.label_2.setScaledContents(True)
        self.label_2.setObjectName("label_2")
        self.verticalLayout.addWidget(self.label_2)
        self.label_3 = QtWidgets.QLabel(self.frame)
        self.label_3.setFrameShape(QtWidgets.QFrame.Box)
        self.label_3.setText("")
        self.label_3.setPixmap(QtGui.QPixmap("interface/resources/brookhaven.png"))
        self.label_3.setScaledContents(True)
        self.label_3.setObjectName("label_3")
        self.verticalLayout.addWidget(self.label_3)
        self.gridLayout.addLayout(self.verticalLayout, 0, 0, 1, 1)
        self.frame_2 = QtWidgets.QFrame(self.centralwidget)
        self.frame_2.setGeometry(QtCore.QRect(710, 10, 162, 61))
        self.frame_2.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_2.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_2.setLineWidth(2)
        self.frame_2.setObjectName("frame_2")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.frame_2)
        self.gridLayout_4.setContentsMargins(1, 1, 1, 1)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.label_10 = QtWidgets.QLabel(self.frame_2)
        self.label_10.setText("")
        self.label_10.setPixmap(QtGui.QPixmap("interface/resources/vts_logo.png"))
        self.label_10.setScaledContents(True)
        self.label_10.setObjectName("label_10")
        self.gridLayout_4.addWidget(self.label_10, 0, 0, 1, 1)
        self.frame_8 = QtWidgets.QFrame(self.centralwidget)
        self.frame_8.setGeometry(QtCore.QRect(710, 270, 162, 110))
        self.frame_8.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_8.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_8.setLineWidth(2)
        self.frame_8.setObjectName("frame_8")
        self.frame_7 = QtWidgets.QFrame(self.frame_8)
        self.frame_7.setGeometry(QtCore.QRect(5, 5, 152, 21))
        self.frame_7.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_7.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_7.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_7.setObjectName("frame_7")
        self.frame_9 = QtWidgets.QFrame(self.frame_8)
        self.frame_9.setGeometry(QtCore.QRect(5, 55, 152, 21))
        self.frame_9.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_9.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_9.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_9.setObjectName("frame_9")
        self.frame_10 = QtWidgets.QFrame(self.frame_8)
        self.frame_10.setGeometry(QtCore.QRect(5, 80, 152, 21))
        self.frame_10.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_10.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_10.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_10.setObjectName("frame_10")
        self.frame_11 = QtWidgets.QFrame(self.frame_8)
        self.frame_11.setGeometry(QtCore.QRect(5, 30, 75, 21))
        self.frame_11.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_11.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_11.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_11.setObjectName("frame_11")
        self.frame_12 = QtWidgets.QFrame(self.frame_8)
        self.frame_12.setGeometry(QtCore.QRect(82, 30, 75, 21))
        self.frame_12.setStyleSheet("background-color: rgb(189, 189, 189);")
        self.frame_12.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_12.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_12.setObjectName("frame_12")
        self.frame_13 = QtWidgets.QFrame(self.centralwidget)
        self.frame_13.setGeometry(QtCore.QRect(710, 390, 161, 41))
        self.frame_13.setStyleSheet("")
        self.frame_13.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_13.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_13.setLineWidth(2)
        self.frame_13.setObjectName("frame_13")
        self.frame_14 = QtWidgets.QFrame(self.frame_13)
        self.frame_14.setGeometry(QtCore.QRect(4, 4, 153, 33))
        self.frame_14.setStyleSheet("background-color: rgb(244, 255, 122)")
        self.frame_14.setFrameShape(QtWidgets.QFrame.Box)
        self.frame_14.setFrameShadow(QtWidgets.QFrame.Plain)
        self.frame_14.setObjectName("frame_14")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar()
        self.menubar.setGeometry(QtCore.QRect(0, 0, 883, 22))
        self.menubar.setObjectName("menubar")
        self.menuVMM_Testing_Software = QtWidgets.QMenu(self.menubar)
        self.menuVMM_Testing_Software.setObjectName("menuVMM_Testing_Software")
        MainWindow.setMenuBar(self.menubar)
        self.menubar.addAction(self.menuVMM_Testing_Software.menuAction())

        self.retranslateUi(MainWindow)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QtWidgets.QApplication.translate("MainWindow", "MainWindow", None, -1))
        self.groupBox.setTitle(QtWidgets.QApplication.translate("MainWindow", "VTS Control", None, -1))
        self.label_11.setText(QtWidgets.QApplication.translate("MainWindow", "VTS Config  ", None, -1))
        self.pushButton.setText(QtWidgets.QApplication.translate("MainWindow", "Load", None, -1))
        self.button_vts_config_check.setText(QtWidgets.QApplication.translate("MainWindow", "OK", None, -1))
        self.label_13.setText(QtWidgets.QApplication.translate("MainWindow", "VTS Server", None, -1))
        self.button_start_vts_server.setText(QtWidgets.QApplication.translate("MainWindow", "Start", None, -1))
        self.button_shutdown_vts_server.setText(QtWidgets.QApplication.translate("MainWindow", "Shutdown", None, -1))
        self.button_ping_vts_server.setText(QtWidgets.QApplication.translate("MainWindow", "Server Alive?", None, -1))
        self.label_vts_server_status.setText(QtWidgets.QApplication.translate("MainWindow", "Unknown", None, -1))
        self.groupBox_3.setTitle(QtWidgets.QApplication.translate("MainWindow", "Device Capture", None, -1))
        self.label_18.setText(QtWidgets.QApplication.translate("MainWindow", "Camera  ", None, -1))
        self.button_acquire_vmm_serial.setText(QtWidgets.QApplication.translate("MainWindow", "Acquire VMM Serial", None, -1))
        self.button_ping_camera.setText(QtWidgets.QApplication.translate("MainWindow", "Camera Alive?", None, -1))
        self.label_vts_vmm_sn.setText(QtWidgets.QApplication.translate("MainWindow", "No VMM Acknowledged", None, -1))
        self.groupBox_2.setTitle(QtWidgets.QApplication.translate("MainWindow", "Testing", None, -1))
        self.label_14.setText(QtWidgets.QApplication.translate("MainWindow", "Test Dir  ", None, -1))
        self.button_tests_load.setText(QtWidgets.QApplication.translate("MainWindow", "Load", None, -1))
        self.button_tests_start.setText(QtWidgets.QApplication.translate("MainWindow", "START", None, -1))
        self.button_tests_stop.setText(QtWidgets.QApplication.translate("MainWindow", "STOP", None, -1))
        self.label_25.setText(QtWidgets.QApplication.translate("MainWindow", "N/M     ", None, -1))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab0), QtWidgets.QApplication.translate("MainWindow", "VTS Control", None, -1))
        self.groupBox_4.setTitle(QtWidgets.QApplication.translate("MainWindow", "FPGA", None, -1))
        self.label_16.setText(QtWidgets.QApplication.translate("MainWindow", "Config   ", None, -1))
        self.pushButton_6.setText(QtWidgets.QApplication.translate("MainWindow", "Load", None, -1))
        self.pushButton_15.setText(QtWidgets.QApplication.translate("MainWindow", "Show", None, -1))
        self.button_fpga_configure.setText(QtWidgets.QApplication.translate("MainWindow", "Configure", None, -1))
        self.button_fpga_ping.setText(QtWidgets.QApplication.translate("MainWindow", "FPGA Alive?", None, -1))
        self.label_17.setText(QtWidgets.QApplication.translate("MainWindow", "none", None, -1))
        self.groupBox_5.setTitle(QtWidgets.QApplication.translate("MainWindow", "VMM", None, -1))
        self.label_19.setText(QtWidgets.QApplication.translate("MainWindow", "Config   ", None, -1))
        self.pushButton_12.setText(QtWidgets.QApplication.translate("MainWindow", "Load", None, -1))
        self.pushButton_16.setText(QtWidgets.QApplication.translate("MainWindow", "Show", None, -1))
        self.button_vmm_configure.setText(QtWidgets.QApplication.translate("MainWindow", "Configure", None, -1))
        self.button_vmm_reset.setText(QtWidgets.QApplication.translate("MainWindow", "Reset", None, -1))
        self.label_20.setText(QtWidgets.QApplication.translate("MainWindow", "none", None, -1))
        self.groupBox_6.setTitle(QtWidgets.QApplication.translate("MainWindow", "DAQ", None, -1))
        self.button_acq_on.setText(QtWidgets.QApplication.translate("MainWindow", "ACQ on", None, -1))
        self.button_acq_off.setText(QtWidgets.QApplication.translate("MainWindow", "ACQ off", None, -1))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab1), QtWidgets.QApplication.translate("MainWindow", "Frontend Control", None, -1))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab3), QtWidgets.QApplication.translate("MainWindow", "Misc", None, -1))
        self.menuVMM_Testing_Software.setTitle(QtWidgets.QApplication.translate("MainWindow", "VMM Testing Software", None, -1))

