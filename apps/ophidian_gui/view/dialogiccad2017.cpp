#include "dialogiccad2017.h"
#include "ui_dialogiccad2017.h"

DialogICCAD2017::DialogICCAD2017(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogICCAD2017)
{
    ui->setupUi(this);
}

DialogICCAD2017::~DialogICCAD2017()
{
    delete ui;
}

void DialogICCAD2017::on_cells_choose_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                    tr("LEF Files (*.lef)"));

    if (!fileName.isEmpty()) {
        ui->cells_lef_2->setText(fileName);
        ui->cells_lef->setStyleSheet("color: rgb(0, 0, 0);");
    }
}

void DialogICCAD2017::on_tech_choose_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                    tr("LEF Files (*.lef)"));

    if (!fileName.isEmpty()) {
        ui->tech_lef_2->setText(fileName);
        ui->tech_lef->setStyleSheet("color: rgb(0, 0, 0);");
    }
}

void DialogICCAD2017::on_placed_choose_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                    tr("DEF Files (*.def)"));

    if (!fileName.isEmpty()) {
        ui->placed_def_2->setText(fileName);
        ui->placed_def->setStyleSheet("color: rgb(0, 0, 0);");
    }
}

void DialogICCAD2017::on_okButton_clicked()
{
    bool cells_lef = ui->cells_lef_2->text().isEmpty();
    bool tech_lef = ui->tech_lef_2->text().isEmpty();
    bool placed_def = ui->placed_def_2->text().isEmpty();
    QString color;

    if (cells_lef || tech_lef || placed_def)
    {
        ui->erro->setText("Informe o local dos arquivos em vermelho!");
        if (cells_lef)
        {
            color = "color: rgb(255, 0, 0);";
        } else {
            color = "color: rgb(0, 0, 0);";
        }

        ui->cells_lef->setStyleSheet(color);

        if (tech_lef)
        {
            color = "color: rgb(255, 0, 0);";
        } else {
            color = "color: rgb(0, 0, 0);";
        }

        ui->tech_lef->setStyleSheet(color);

        if (placed_def)
        {
            color = "color: rgb(255, 0, 0);";
        } else {
            color = "color: rgb(0, 0, 0);";
        }

        ui->placed_def->setStyleSheet(color);

    } else {
        emit buildICCAD2017(ui->cells_lef_2->text().toStdString(), ui->tech_lef_2->text().toStdString(), ui->placed_def_2->text().toStdString());
        this->close();
    }
}

void DialogICCAD2017::on_cancelButton_clicked()
{
    this->close();
}
