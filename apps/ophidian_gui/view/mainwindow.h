#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "dialogiccad2015.h"
#include "dialogiccad2017.h"
#include "control/maincontroller.h"

#include <iostream>

namespace Ui {
class MainWindow;
} // namespace Ui

class MainWindow : public QMainWindow
{
    Q_OBJECT

    MainController mMainController;

public:
    //! MainWindow Constructor
    /*!
       \brief Constructs a MainWindow.
     */
    explicit MainWindow(QWidget *parent = 0);

    //! MainWindow Destructor
    /*!
       \brief Destroys the MainWindow, including its properties.
     */
    ~MainWindow();

private slots:
    //! Trigger ICCAD 2017 event
    /*!
       \brief Opens a dialog window to inform the iccad 2017 files.
     */
    void on_actionICCAD_2017_triggered();

    //! Trigger ICCAD 2017 event
    /*!
       \brief Opens a dialog window to inform the iccad 2015 files.
     */
    void on_actionICCAD_2015_triggered();

    //! Quit button event
    /*!
       \brief Closes the application.
     */
    void on_actionQuit_triggered();

    //! Receiving circuit information
    /*!
       \brief Receiving the circuit information to update the displayed information.
       \param name Circuit name.
       \param die Circuit size.
       \param cells Amount of cells in the circuit.
       \param pins Amount of pins in the circuit.
       \param nets Amount of nets in the circuit.
     */
    void on_receive_circuitChanged(QString name, QString die, size_t cells, size_t pins, size_t nets);

    //! Receiving cell information
    /*!
       \brief Receiving information from a cell to update the displayed information.
       \param name Cell name.
       \param type Cell type.
       \param x Position of the origin on the abscissa axis.
       \param y Position of the origin on the ordinate axis.
       \param worstSlack not implemented
     */
    void on_receive_cellChanged(QString name, QString type, double width, double height, double x, double y);

    //! Changes the position of a cell
    /*!
       \brief Sends a cell's position change to the main control.
     */
    void on_selectedCellButton_clicked();

    //! Deployment helper function
    /*!
       \brief Auxiliary function to implement behavior different from design.
     */
    void on_actionSlot_1_triggered();

    //! Deployment helper function
    /*!
       \brief Auxiliary function to implement behavior different from design.
     */
    void on_actionSlot_2_triggered();

    //! Deployment helper function
    /*!
       \brief Auxiliary function to implement behavior different from design.
     */
    void on_actionSlot_3_triggered();

    //! Find a cell by name
    /*!
       \brief Search for a cell by its name and display it on the canvas.
     */
    void on_findNameButton_clicked();

    //! Clear fields of action: find cell by name
    /*!
       \brief Cleans the fields for ease of performing a new search.
     */
    void on_findNameButtonClear_clicked();

    //! Save PNG of the circuit
    /*!
       \brief Draw the current view of the circuit into a PNG image.
     */
    void on_actionPNG_triggered();

    //! Save SVG of the circuit
    /*!
       \brief Draw the current view of the circuit into a SVG image.
     */
    void on_actionSVG_triggered();

signals:
    //! Deployment helper function
    /*!
       \brief Auxiliary function to implement behavior different from design.
     */
    void update();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
