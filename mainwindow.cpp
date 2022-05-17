#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <windows.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , processFlag(false)
    , pausedFlag(false)
    , idCounter(1)
    , counterTime(0)
    , processTableThread(&MainWindow::updateProcessTable, this)
    , actualProcessThread(&MainWindow::updateActualProcess, this)
    , executeProcessThread(&MainWindow::executeActualProcess, this)
    , executeGlobalCounterThread(&MainWindow::executeGlobalCounter, this)
{
    ui->setupUi(this);
    setWindowTitle("FCFS");

    // Configuración inicial del frame Proceso Actual
    ui->pb_continue->setEnabled(false);
    ui->pb_pause->setEnabled(false);
    ui->pb_end->setEnabled(false);

    ui->pb_process->setEnabled((false));
    ui->sb_processBurstTime->setMinimum(1);
    ui->sb_processBurstTime->clear();

    // Editar el encabezado de la tabla de la cola de procesos
    QStringList processesTitles;
    processesTitles << "ID" << "Nombre" << "Duración" << "Llegada" << "Estado";
    ui->processTable->setColumnCount(5);
    ui->processTable->setHorizontalHeaderLabels(processesTitles);

    // Editar el encabezado de la tabla de proceso finalizados
    QStringList endedProcessesTitles;
    endedProcessesTitles << "ID" << "Nombre" << "Burst Time" << "Arrival Time" << "Completion Time" << "Turn Around Time" << "Waiting Time";
    ui->endedProcessTable->setColumnCount(7);
    ui->endedProcessTable->setHorizontalHeaderLabels(endedProcessesTitles);

    qDebug() << "Se ejecutó el constructor";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pb_process_clicked() {
    Process newProcess(
                this->idCounter++, // ID
                ui->le_processName->text().toStdString(), // Nombre del proceso
                ui->sb_processBurstTime->text().toInt(), // Duración del proceso, burstTime
                this->counterTime, // Llegada del proceso, arrivalTime
                created // Estado del proceso (No iniciado)
    );

    // Agregar el proceso a la lista
    this->processes.push_back(newProcess);

    // Agregamos el proceso a la tabla
    QString newName = QString::fromStdString(newProcess.getName());
    QString newDuration = QString::number(newProcess.getBurstTime());
    QString newState = QString::fromStdString(newProcess.getStateStr());

    ui->processTable->insertRow(ui->processTable->rowCount());
    ui->processTable->setItem(ui->processTable->rowCount()-1, name, new QTableWidgetItem(newName));
    ui->processTable->setItem(ui->processTable->rowCount()-1, duration, new QTableWidgetItem(newDuration));
    ui->processTable->setItem(ui->processTable->rowCount()-1, state, new QTableWidgetItem(newState));

    // Limpiamos los campos del nuevo proceso
    ui->le_processName->clear();
    ui->sb_processBurstTime->clear();
}

void MainWindow::updateProcessTable() {
    while(1){

    }
}

void MainWindow::executeActualProcess() {
    while(1) {
        if(this->processFlag == true) {
            if(this->actualProcess.getBurstTime() > 0) {
                if(!pausedFlag) {
                    this->actualProcess.setBurstTime(this->actualProcess.getBurstTime()-1);
                    ui->lb_actualDurationData->setText(QString::number(this->actualProcess.getBurstTime()));
                    Sleep(1000);
                }
            }
            else {
                ui->lb_actualNameData->clear();
                ui->lb_actualDurationData->clear();
                ui->lb_actualStateData->clear();
                this->processes.erase(this->processes.begin());
                this->processFlag = false;
                this->actualProcess.setState(ended);
                this->endedProcesses.push_back(this->actualProcess);
                updateEndedProcessTable();                                
            }
        }        
    }
}

void MainWindow::updateActualProcess() {
    while(1) {
        if(this->processFlag == false && this->processes.size() > 0) {
            this->actualProcess = this->processes.front();
            this->actualProcess.setState(execution);
            ui->lb_actualNameData->setText(QString::fromStdString(this->actualProcess.getName()));
            ui->lb_actualStateData->setText(QString::fromStdString(this->actualProcess.getStateStr()));
            ui->lb_actualDurationData->setText(QString::number(this->actualProcess.getBurstTime()));
            ui->pb_pause->setEnabled(true);
            ui->pb_end->setEnabled(true);
            this->processFlag = true;
        }
    }
}

/**
 * @brief MainWindow::executeGlobalCounter
 *
 * Función que se ejecuta en un hilo
 * para tener un contador global que inicia en 0
 * y aumenta en 1 cada segundo.
 */
void MainWindow::executeGlobalCounter() {
    while(1) {
        Sleep(1000);
        ui->lcd_counterTime->display(QString::number(this->counterTime++));
    }
}

void MainWindow::updateEndedProcessTable() {
    Process lastProcess = this->endedProcesses.at(this->endedProcesses.size()-1);

    // Agregamos el proceso a la tabla
    QString newName = QString::fromStdString(lastProcess.getName());
    QString newDuration = QString::number(lastProcess.getBurstTime());
    QString newState = QString::fromStdString(lastProcess.getStateStr());

    ui->endedProcessTable->insertRow(ui->endedProcessTable->rowCount());
    ui->endedProcessTable->setItem(ui->endedProcessTable->rowCount()-1, name, new QTableWidgetItem(newName));
    ui->endedProcessTable->setItem(ui->endedProcessTable->rowCount()-1, duration, new QTableWidgetItem(newDuration));
    ui->endedProcessTable->setItem(ui->endedProcessTable->rowCount()-1, state, new QTableWidgetItem(newState));

    ui->pb_continue->setEnabled(false);
    ui->pb_pause->setEnabled(false);
    ui->pb_end->setEnabled(false);
}


void MainWindow::on_le_processName_textChanged(const QString &arg1) {
    if(arg1.length() > 0 && ui->sb_processBurstTime->text().toInt() > 0) {
        ui->pb_process->setEnabled(true);
    }
    else {
        ui->pb_process->setEnabled(false);
    }
}


void MainWindow::on_sb_processBurstTime_valueChanged(int arg1) {
    if(ui->le_processName->text().length() > 0 && arg1 > 0) {
        ui->pb_process->setEnabled(true);
    }
    else {
        ui->pb_process->setEnabled(false);
    }
}


void MainWindow::on_pb_pause_clicked() {
    this->pausedFlag = !this->pausedFlag;
    ui->pb_pause->setEnabled(false);
    ui->pb_continue->setEnabled(true);
    this->actualProcess.setState(paused);
    ui->lb_actualStateData->setText(QString::fromStdString(this->actualProcess.getStateStr()));
}


void MainWindow::on_pb_continue_clicked() {
    this->pausedFlag = !this->pausedFlag;
    ui->pb_pause->setEnabled(true);
    ui->pb_continue->setEnabled(false);
    this->actualProcess.setState(execution);
    ui->lb_actualStateData->setText(QString::fromStdString(this->actualProcess.getStateStr()));
}


void MainWindow::on_pb_end_clicked() {
    ui->lb_actualNameData->clear();
    ui->lb_actualDurationData->clear();
    this->processes.erase(this->processes.begin());
    this->processFlag = false;
    this->actualProcess.setState(ended);
    this->endedProcesses.push_back(this->actualProcess);
    updateEndedProcessTable();
}

