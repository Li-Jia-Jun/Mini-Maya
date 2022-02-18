#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTreeWidgetItem>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slot_addVertsToList(QListWidgetItem *);
    void slot_addFacesToList(QListWidgetItem *);
    void slot_addHalfEdgesToList(QListWidgetItem *);

    void slot_addJointsToTree(QTreeWidgetItem *);

    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
