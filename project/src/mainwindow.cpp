#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    // Put items on lists
    connect(ui->mygl, SIGNAL(sig_addVertsToList(QListWidgetItem *)), this, SLOT(slot_addVertsToList(QListWidgetItem *)));
    connect(ui->mygl, SIGNAL(sig_addFacesToList(QListWidgetItem *)), this, SLOT(slot_addFacesToList(QListWidgetItem *)));
    connect(ui->mygl, SIGNAL(sig_addHalfEdgesToList(QListWidgetItem *)), this, SLOT(slot_addHalfEdgesToList(QListWidgetItem *)));

    // Put items to tree
    connect(ui->mygl, SIGNAL(sig_addJointsToTree(QTreeWidgetItem *)), this, SLOT(slot_addJointsToTree(QTreeWidgetItem *)));

    // Clear lists
    connect(ui->mygl, SIGNAL(sig_clearVertsList()), ui->vertsListWidget, SLOT(clear()));
    connect(ui->mygl, SIGNAL(sig_clearFacesList()), ui->facesListWidget, SLOT(clear()));
    connect(ui->mygl, SIGNAL(sig_clearHalfEdgesList()), ui->halfEdgesListWidget, SLOT(clear()));

    // Put data to Spinboxes
    connect(ui->mygl, SIGNAL(sig_refreshVertPosX(double)), ui->vertPosXSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshVertPosY(double)), ui->vertPosYSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshVertPosZ(double)), ui->vertPosZSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshFaceRed(double)), ui->faceRedSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshFaceGreen(double)), ui->faceGreenSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshFaceBlue(double)), ui->faceBlueSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshJointPosX(double)), ui->jointXSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshJointPosY(double)), ui->jointYSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_refreshJointPosZ(double)), ui->jointZSpinBox, SLOT(setValue(double)));

    // Lists click event
    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem *)), ui->mygl, SLOT(slot_onVertexSelected(QListWidgetItem *)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem *)), ui->mygl, SLOT(slot_onFaceSelected(QListWidgetItem *)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem *)), ui->mygl, SLOT(slot_onHalfEdgeSelected(QListWidgetItem *)));

    // Tree click event
    connect(ui->jointsTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), ui->mygl, SLOT(slot_onJointSelected(QTreeWidgetItem *, int)));

    // Button click event
    connect(ui->splitEdgeButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_SplitEdge(bool)));
    connect(ui->triangulateButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_Triangulate(bool)));
    connect(ui->subdivisionButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_Subdivision(bool)));
    connect(ui->loadObjButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_LoadObj(bool)));
    connect(ui->loadJsonButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_LoadJson(bool)));
    connect(ui->skinMeshButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_SkinMesh(bool)));
    connect(ui->jointRotX, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_jointRotX(bool)));
    connect(ui->jointRotMinusX, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_jointRotMinusX(bool)));
    connect(ui->jointRotY, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_jointRotY(bool)));
    connect(ui->jointRotMinusY, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_jointRotMinusY(bool)));
    connect(ui->jointRotZ, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_jointRotZ(bool)));
    connect(ui->jointRotMinusZ, SIGNAL(clicked(bool)), ui->mygl, SLOT(on_jointRotMinusZ(bool)));

    // On Spinbox changed
    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_vertPosXChanged(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_vertPosYChanged(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_vertPosZChanged(double)));
    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_faceRedChanged(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_faceGreenChanged(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_faceBlueChanged(double)));
    connect(ui->jointXSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_jointPosXChanged(double)));
    connect(ui->jointYSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_jointPosYChanged(double)));
    connect(ui->jointZSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(on_jointPosZChanged(double)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

void MainWindow::slot_addVertsToList(QListWidgetItem *add)
{
    ui->vertsListWidget->addItem(add);
}
void MainWindow::slot_addFacesToList(QListWidgetItem *add)
{
    ui->facesListWidget->addItem(add);
}
void MainWindow::slot_addHalfEdgesToList(QListWidgetItem *add)
{
    ui->halfEdgesListWidget->addItem(add);
}

void MainWindow::slot_addJointsToTree(QTreeWidgetItem *add)
{
    ui->jointsTreeWidget->clear();
    ui->jointsTreeWidget->addTopLevelItem(add);
}
