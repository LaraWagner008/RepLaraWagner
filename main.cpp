#include "mainwindow.h"
#include <QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QString>
#include <Flood.h>
#include <Relief.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QVector>
#include <QRandomGenerator>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <functional>
#include <algorithm> // Для функции std::min
#include <limits>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QHeaderView>
#include <QDialog>
#include <QValueAxis>
#include <QFont>
#include <QBrush>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QTableWidget>

#include <QFile>
#include <QTextStream>
#include <QStringList>

QT_CHARTS_USE_NAMESPACE

class TerrainMap : public QGraphicsView {
public:
    TerrainMap(int rows, int cols, QWidget *parent = nullptr)
        : QGraphicsView(parent), rows(rows), cols(cols) {
        scene = new QGraphicsScene(this);
        this->setScene(scene);
        generateTerrain1();
        generateTerrain2();
        generateTerrain3();
        generateTerrain4();
    }

  //private:
        QGraphicsScene *scene;
        int rows;
        int cols;
        QVector<QVector<int>> heights;
        QVector<QVector<int>> waterLevel;


// 1 - ПОЛОВОДЬЕ
//ОСНОВНОЙ КОД ДЛЯ РЕЛЬЕФА ПО ТИПУ НАВОДНЕНИЯ
   void generateTerrain1() {

        //Открыть базу данных "Data"
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
               db.setDatabaseName("D://Data.db");
               if (!db.open()) {
                       qDebug() << "Connection error";
                     //  return 1;
                   }
                   else {
                       qDebug() << "Connection opened";
                   }
        heights.resize(rows);
        waterLevel.resize(rows);
        for (int i = 0; i < rows; ++i) {
            heights[i].resize(cols);
            waterLevel[i].resize(cols);
        }

//Генерация рельефа (заполнение высот)
                QRandomGenerator *gen = QRandomGenerator::global();
                int firstRow = 0;
                int firstCol = 0;

                for (int i = 0; i < rows; ++i) {
                    for (int j = 0; j < cols; ++j) {
                        // Диапазон рельефа
                        int minHeight = 0;
                        int maxHeight = 200;

                        // Если начальная ячейка, то устанавливаем заданный диапазон
                        if (i == firstRow && j == firstCol) {
                            minHeight = 20;
                            maxHeight = 40;
                        } else {
                            if (i > 0) {
                                minHeight = std::max(minHeight, heights[i - 1][j] - 5);
                                maxHeight = std::min(maxHeight, heights[i - 1][j] + 5);
                            }
                            if (j > 0) {
                                minHeight = std::max(minHeight, heights[i][j - 1] - 5);
                                maxHeight = std::min(maxHeight, heights[i][j - 1] + 5);
                            }
                        }

                        // Убеждаемся, что minHeight меньше или равен maxHeight
                        minHeight = std::min(minHeight, maxHeight);
                        heights[i][j] = gen->bounded(minHeight, maxHeight + 1);


         //Заполнение ячейки водой
                            QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, heightToColor(heights[i][j]));
                            rect->setToolTip(QString("Высота: %1").arg(heights[i][j]));
                waterLevel[i][j] = 0;
            }
        }
                //УРОВЕНЬ ВОДЫ ПРИСВАИВАЕТСЯ ЗДЕСЬ
                QSqlQuery query;
                double levelWater1 = 0;
                 if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 1")) {
                     int i =  0;
                         while (query.next() && i < 1) {
                             levelWater1 = query.value(3).toDouble();
                         ++i;
                         }
                 }
                // qDebug() << levelWater1;
                waterLevel[rows / 2][cols / 2] = levelWater1;
                db.close();
    }

    void spreadWater1() {

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
           db.setDatabaseName("D://Data.db");
           if (!db.open()) {
                   qDebug() << "Connection error";
                 //  return 1;
               }
               else {
                   qDebug() << "Connection opened";
               }

        bool changed = false;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (waterLevel[i][j] > 0) {
                    int min_height = INT_MAX;
                    int min_i = -1;
                    int min_j = -1;

                    // Проверяем соседние ячейки только в том случае, если они не заполнены водой
                    if (i > 0 && waterLevel[i - 1][j] == 0 && heights[i - 1][j] < min_height) {
                        min_height = heights[i - 1][j];
                        min_i = i - 1;
                        min_j = j;
                    }
                    if (j > 0 && waterLevel[i][j - 1] == 0 && heights[i][j - 1] < min_height) {
                        min_height = heights[i][j - 1];
                        min_i = i;
                        min_j = j - 1;
                    }
                    if (i < rows - 1 && waterLevel[i + 1][j] == 0 && heights[i + 1][j] < min_height) {
                        min_height = heights[i + 1][j];
                        min_i = i + 1;
                        min_j = j;
                    }
                    if (j < cols - 1 && waterLevel[i][j + 1] == 0 && heights[i][j + 1] < min_height) {
                        min_height = heights[i][j + 1];
                        min_i = i;
                        min_j = j + 1;
                    }

                    // Заполняем соседнюю ячейку с минимальной высотой водой
                    if (min_height != INT_MAX) {
                        waterLevel[min_i][min_j] = waterLevel[i][j] + 1;
                        changed = true;
                        // Отрисовка новой синей ячейки
                        QGraphicsRectItem *rect = scene->addRect(min_j * 10, min_i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                        rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[min_i][min_j]));
                        // Отображение обновленного рисунка с синими ячейками
                        scene->update();
                    } else {
                        // Если соседних ячеек с меньшей высотой нет, проверяем, окружена ли текущая ячейка ячейками с большей высотой
                        bool surrounded = true;
                        if (i > 0 && heights[i - 1][j] >= heights[i][j]) surrounded = false;
                        if (j > 0 && heights[i][j - 1] >= heights[i][j]) surrounded = false;
                        if (i < rows - 1 && heights[i + 1][j] >= heights[i][j]) surrounded = false;
                        if (j < cols - 1 && heights[i][j + 1] >= heights[i][j]) surrounded = false;

                        // Если текущая ячейка окружена ячейками с большей высотой, заполняем ее водой
                        if (surrounded) {
                            waterLevel[i][j] = waterLevel[i][j] + 1;
                            changed = true;
                            // Отрисовка новой синей ячейки
                            QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                            rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[i][j]));
                            // Отображение обновленного рисунка с синими ячейками
                            scene->update();
                        }
                    }// Добавление задержки для более медленного распространения воды
                    QTime dieTime= QTime::currentTime().addMSecs(50);
                    while (QTime::currentTime() < dieTime)
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }
            }
        }
        int water_count = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (waterLevel[i][j] > 0) {
                    water_count++;
                }
            }
        }
        //ЗДЕСЬ ПРИСВАИВАЕТСЯ ПЛОЩАДЬ НАВОДНЕНИЯ (В ЯЧЕЙКАХ - КОЛИЧЕСТВО ТОЧЕК ЭВАКУАЦИИ)

        QSqlQuery query;
        int squareWater1 = 0;
         if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 1")) {
             int i =  0;
                 while (query.next() && i < 1) {
                     squareWater1 = query.value(4).toDouble();
                 ++i;
                 }
         }
       //  qDebug() << squareWater1; //должно выйти 300
        db.close();

        //Либо оставить большую карту 4900км2, где 1 ячейка это 10км2 и при наводнении в 300км2 будет 30 точек эвакуации, либо уменьшить карту
        if (water_count >= squareWater1) {
            return; // Вода перестает распространяться
        }
        spreadWater1();// Если вода еще не достигла 30 ячеек, вызываем spreadWater() снова
    }



    //2 - ПАВОДОК
    //ОСНОВНОЙ КОД ДЛЯ РЕЛЬЕФА ПО ТИПУ НАВОДНЕНИЯ
       void generateTerrain2() {

            //Открыть базу данных "Data"
                QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                   db.setDatabaseName("D://Data.db");
                   if (!db.open()) {
                           qDebug() << "Connection error";
                         //  return 1;
                       }
                       else {
                           qDebug() << "Connection opened";
                       }
            heights.resize(rows);
            waterLevel.resize(rows);
            for (int i = 0; i < rows; ++i) {
                heights[i].resize(cols);
                waterLevel[i].resize(cols);
            }

    //Генерация рельефа (заполнение высот)
                    QRandomGenerator *gen = QRandomGenerator::global();
                    int firstRow = 0;
                    int firstCol = 0;

                    for (int i = 0; i < rows; ++i) {
                        for (int j = 0; j < cols; ++j) {
                            // Диапазон рельефа
                            int minHeight = 0;
                            int maxHeight = 200;

                            // Если начальная ячейка, то устанавливаем заданный диапазон
                            if (i == firstRow && j == firstCol) {
                                minHeight = 20;
                                maxHeight = 40;
                            } else {
                                if (i > 0) {
                                    minHeight = std::max(minHeight, heights[i - 1][j] - 5);
                                    maxHeight = std::min(maxHeight, heights[i - 1][j] + 5);
                                }
                                if (j > 0) {
                                    minHeight = std::max(minHeight, heights[i][j - 1] - 5);
                                    maxHeight = std::min(maxHeight, heights[i][j - 1] + 5);
                                }
                            }

                            // Убеждаемся, что minHeight меньше или равен maxHeight
                            minHeight = std::min(minHeight, maxHeight);
                            heights[i][j] = gen->bounded(minHeight, maxHeight + 1);


             //Заполнение ячейки водой
                                QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, heightToColor(heights[i][j]));
                                rect->setToolTip(QString("Высота: %1").arg(heights[i][j]));
                    waterLevel[i][j] = 0;
                }
            }
                    //УРОВЕНЬ ВОДЫ ПРИСВАИВАЕТСЯ ЗДЕСЬ
                    QSqlQuery query;
                    double levelWater2 = 0;
                     if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 2")) {
                         int i =  0;
                             while (query.next() && i < 1) {
                                 levelWater2 = query.value(3).toDouble();
                             ++i;
                             }
                     }
                     qDebug() << levelWater2;
                    waterLevel[rows / 2][cols / 2] = levelWater2;
                    db.close();

        }

        void spreadWater2() {

            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
               db.setDatabaseName("D://Data.db");
               if (!db.open()) {
                       qDebug() << "Connection error";
                     //  return 1;
                   }
                   else {
                       qDebug() << "Connection opened";
                   }

            bool changed = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (waterLevel[i][j] > 0) {
                        int min_height = INT_MAX;
                        int min_i = -1;
                        int min_j = -1;

                        // Проверяем соседние ячейки только в том случае, если они не заполнены водой
                        if (i > 0 && waterLevel[i - 1][j] == 0 && heights[i - 1][j] < min_height) {
                            min_height = heights[i - 1][j];
                            min_i = i - 1;
                            min_j = j;
                        }
                        if (j > 0 && waterLevel[i][j - 1] == 0 && heights[i][j - 1] < min_height) {
                            min_height = heights[i][j - 1];
                            min_i = i;
                            min_j = j - 1;
                        }
                        if (i < rows - 1 && waterLevel[i + 1][j] == 0 && heights[i + 1][j] < min_height) {
                            min_height = heights[i + 1][j];
                            min_i = i + 1;
                            min_j = j;
                        }
                        if (j < cols - 1 && waterLevel[i][j + 1] == 0 && heights[i][j + 1] < min_height) {
                            min_height = heights[i][j + 1];
                            min_i = i;
                            min_j = j + 1;
                        }

                        // Заполняем соседнюю ячейку с минимальной высотой водой
                        if (min_height != INT_MAX) {
                            waterLevel[min_i][min_j] = waterLevel[i][j] + 1;
                            changed = true;
                            // Отрисовка новой синей ячейки
                            QGraphicsRectItem *rect = scene->addRect(min_j * 10, min_i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                            rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[min_i][min_j]));
                            // Отображение обновленного рисунка с синими ячейками
                            scene->update();
                        } else {
                            // Если соседних ячеек с меньшей высотой нет, проверяем, окружена ли текущая ячейка ячейками с большей высотой
                            bool surrounded = true;
                            if (i > 0 && heights[i - 1][j] >= heights[i][j]) surrounded = false;
                            if (j > 0 && heights[i][j - 1] >= heights[i][j]) surrounded = false;
                            if (i < rows - 1 && heights[i + 1][j] >= heights[i][j]) surrounded = false;
                            if (j < cols - 1 && heights[i][j + 1] >= heights[i][j]) surrounded = false;

                            // Если текущая ячейка окружена ячейками с большей высотой, заполняем ее водой
                            if (surrounded) {
                                waterLevel[i][j] = waterLevel[i][j] + 1;
                                changed = true;
                                // Отрисовка новой синей ячейки
                                QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                                rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[i][j]));
                                // Отображение обновленного рисунка с синими ячейками
                                scene->update();
                            }
                        }// Добавление задержки для более медленного распространения воды
                        QTime dieTime= QTime::currentTime().addMSecs(50);
                        while (QTime::currentTime() < dieTime)
                            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                    }
                }
            }
            int water_count = 0;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (waterLevel[i][j] > 0) {
                        water_count++;
                    }
                }
            }
            //ЗДЕСЬ ПРИСВАИВАЕТСЯ ПЛОЩАДЬ НАВОДНЕНИЯ (В ЯЧЕЙКАХ - КОЛИЧЕСТВО ТОЧЕК ЭВАКУАЦИИ)

            QSqlQuery query;
            int squareWater2 = 0;
             if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 2")) {
                 int i =  0;
                     while (query.next() && i < 1) {
                         squareWater2 = query.value(4).toDouble();
                     ++i;
                     }
             }
             qDebug() << squareWater2; //должно выйти 300
            db.close();

            //Либо оставить большую карту 4900км2, где 1 ячейка это 10км2 и при наводнении в 300км2 будет 30 точек эвакуации, либо уменьшить карту
            if (water_count >= squareWater2) {
                return; // Вода перестает распространяться
            }
            spreadWater2();// Если вода еще не достигла 30 ячеек, вызываем spreadWater() снова
        }


        //3 - ПРОРЫВ ПЛОТИНЫ
        //ОСНОВНОЙ КОД ДЛЯ РЕЛЬЕФА ПО ТИПУ НАВОДНЕНИЯ
           void generateTerrain3() {

                //Открыть базу данных "Data"
                    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                       db.setDatabaseName("D://Data.db");
                       if (!db.open()) {
                               qDebug() << "Connection error";
                             //  return 1;
                           }
                           else {
                               qDebug() << "Connection opened";
                           }
                heights.resize(rows);
                waterLevel.resize(rows);
                for (int i = 0; i < rows; ++i) {
                    heights[i].resize(cols);
                    waterLevel[i].resize(cols);
                }

        //Генерация рельефа (заполнение высот)
                        QRandomGenerator *gen = QRandomGenerator::global();
                        int firstRow = 0;
                        int firstCol = 0;

                        for (int i = 0; i < rows; ++i) {
                            for (int j = 0; j < cols; ++j) {
                                // Диапазон рельефа
                                int minHeight = 0;
                                int maxHeight = 200;

                                // Если начальная ячейка, то устанавливаем заданный диапазон
                                if (i == firstRow && j == firstCol) {
                                    minHeight = 20;
                                    maxHeight = 40;
                                } else {
                                    if (i > 0) {
                                        minHeight = std::max(minHeight, heights[i - 1][j] - 5);
                                        maxHeight = std::min(maxHeight, heights[i - 1][j] + 5);
                                    }
                                    if (j > 0) {
                                        minHeight = std::max(minHeight, heights[i][j - 1] - 5);
                                        maxHeight = std::min(maxHeight, heights[i][j - 1] + 5);
                                    }
                                }

                                // Убеждаемся, что minHeight меньше или равен maxHeight
                                minHeight = std::min(minHeight, maxHeight);
                                heights[i][j] = gen->bounded(minHeight, maxHeight + 1);


                 //Заполнение ячейки водой
                                    QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, heightToColor(heights[i][j]));
                                    rect->setToolTip(QString("Высота: %1").arg(heights[i][j]));
                        waterLevel[i][j] = 0;
                    }
                }
                        //УРОВЕНЬ ВОДЫ ПРИСВАИВАЕТСЯ ЗДЕСЬ
                        QSqlQuery query;
                        double levelWater3 = 0;
                         if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 3")) {
                             int i =  0;
                                 while (query.next() && i < 1) {
                                     levelWater3 = query.value(3).toDouble();
                                 ++i;
                                 }
                         }
                         qDebug() << levelWater3;
                        waterLevel[rows / 2][cols / 2] = levelWater3;
                        db.close();

            }

            void spreadWater3() {

                QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                   db.setDatabaseName("D://Data.db");
                   if (!db.open()) {
                           qDebug() << "Connection error";
                         //  return 1;
                       }
                       else {
                           qDebug() << "Connection opened";
                       }

                bool changed = false;
                for (int i = 0; i < rows; ++i) {
                    for (int j = 0; j < cols; ++j) {
                        if (waterLevel[i][j] > 0) {
                            int min_height = INT_MAX;
                            int min_i = -1;
                            int min_j = -1;

                            // Проверяем соседние ячейки только в том случае, если они не заполнены водой
                            if (i > 0 && waterLevel[i - 1][j] == 0 && heights[i - 1][j] < min_height) {
                                min_height = heights[i - 1][j];
                                min_i = i - 1;
                                min_j = j;
                            }
                            if (j > 0 && waterLevel[i][j - 1] == 0 && heights[i][j - 1] < min_height) {
                                min_height = heights[i][j - 1];
                                min_i = i;
                                min_j = j - 1;
                            }
                            if (i < rows - 1 && waterLevel[i + 1][j] == 0 && heights[i + 1][j] < min_height) {
                                min_height = heights[i + 1][j];
                                min_i = i + 1;
                                min_j = j;
                            }
                            if (j < cols - 1 && waterLevel[i][j + 1] == 0 && heights[i][j + 1] < min_height) {
                                min_height = heights[i][j + 1];
                                min_i = i;
                                min_j = j + 1;
                            }

                            // Заполняем соседнюю ячейку с минимальной высотой водой
                            if (min_height != INT_MAX) {
                                waterLevel[min_i][min_j] = waterLevel[i][j] + 1;
                                changed = true;
                                // Отрисовка новой синей ячейки
                                QGraphicsRectItem *rect = scene->addRect(min_j * 10, min_i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                                rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[min_i][min_j]));
                                // Отображение обновленного рисунка с синими ячейками
                                scene->update();
                            } else {
                                // Если соседних ячеек с меньшей высотой нет, проверяем, окружена ли текущая ячейка ячейками с большей высотой
                                bool surrounded = true;
                                if (i > 0 && heights[i - 1][j] >= heights[i][j]) surrounded = false;
                                if (j > 0 && heights[i][j - 1] >= heights[i][j]) surrounded = false;
                                if (i < rows - 1 && heights[i + 1][j] >= heights[i][j]) surrounded = false;
                                if (j < cols - 1 && heights[i][j + 1] >= heights[i][j]) surrounded = false;

                                // Если текущая ячейка окружена ячейками с большей высотой, заполняем ее водой
                                if (surrounded) {
                                    waterLevel[i][j] = waterLevel[i][j] + 1;
                                    changed = true;
                                    // Отрисовка новой синей ячейки
                                    QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                                    rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[i][j]));
                                    // Отображение обновленного рисунка с синими ячейками
                                    scene->update();
                                }
                            }// Добавление задержки для более медленного распространения воды
                            QTime dieTime= QTime::currentTime().addMSecs(50);
                            while (QTime::currentTime() < dieTime)
                                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                        }
                    }
                }
                int water_count = 0;
                for (int i = 0; i < rows; ++i) {
                    for (int j = 0; j < cols; ++j) {
                        if (waterLevel[i][j] > 0) {
                            water_count++;
                        }
                    }
                }
                //ЗДЕСЬ ПРИСВАИВАЕТСЯ ПЛОЩАДЬ НАВОДНЕНИЯ (В ЯЧЕЙКАХ - КОЛИЧЕСТВО ТОЧЕК ЭВАКУАЦИИ)

                QSqlQuery query;
                int squareWater3 = 0;
                 if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 3")) {
                     int i =  0;
                         while (query.next() && i < 1) {
                             squareWater3 = query.value(4).toDouble();
                         ++i;
                         }
                 }
                 qDebug() << squareWater3; //должно выйти 300
                db.close();

                //Либо оставить большую карту 4900км2, где 1 ячейка это 10км2 и при наводнении в 300км2 будет 30 точек эвакуации, либо уменьшить карту
                if (water_count >= squareWater3) {
                    return; // Вода перестает распространяться
                }
                spreadWater3();// Если вода еще не достигла 30 ячеек, вызываем spreadWater() снова
            }

            //4 - ЦУНАМИ
            //ОСНОВНОЙ КОД ДЛЯ РЕЛЬЕФА ПО ТИПУ НАВОДНЕНИЯ
               void generateTerrain4() {

                    //Открыть базу данных "Data"
                        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                           db.setDatabaseName("D://Data.db");
                           if (!db.open()) {
                                   qDebug() << "Connection error";
                                 //  return 1;
                               }
                               else {
                                   qDebug() << "Connection opened";
                               }
                    heights.resize(rows);
                    waterLevel.resize(rows);
                    for (int i = 0; i < rows; ++i) {
                        heights[i].resize(cols);
                        waterLevel[i].resize(cols);
                    }

            //Генерация рельефа (заполнение высот)
                            QRandomGenerator *gen = QRandomGenerator::global();
                            int firstRow = 0;
                            int firstCol = 0;

                            for (int i = 0; i < rows; ++i) {
                                for (int j = 0; j < cols; ++j) {
                                    // Диапазон рельефа
                                    int minHeight = 0;
                                    int maxHeight = 200;

                                    // Если начальная ячейка, то устанавливаем заданный диапазон
                                    if (i == firstRow && j == firstCol) {
                                        minHeight = 20;
                                        maxHeight = 40;
                                    } else {
                                        if (i > 0) {
                                            minHeight = std::max(minHeight, heights[i - 1][j] - 5);
                                            maxHeight = std::min(maxHeight, heights[i - 1][j] + 5);
                                        }
                                        if (j > 0) {
                                            minHeight = std::max(minHeight, heights[i][j - 1] - 5);
                                            maxHeight = std::min(maxHeight, heights[i][j - 1] + 5);
                                        }
                                    }

                                    // Убеждаемся, что minHeight меньше или равен maxHeight
                                    minHeight = std::min(minHeight, maxHeight);
                                    heights[i][j] = gen->bounded(minHeight, maxHeight + 1);


                     //Заполнение ячейки водой
                                        QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, heightToColor(heights[i][j]));
                                        rect->setToolTip(QString("Высота: %1").arg(heights[i][j]));
                            waterLevel[i][j] = 0;
                        }
                    }
                            //УРОВЕНЬ ВОДЫ ПРИСВАИВАЕТСЯ ЗДЕСЬ
                            QSqlQuery query;
                            double levelWater4 = 0;
                             if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 4")) {
                                 int i =  0;
                                     while (query.next() && i < 1) {
                                         levelWater4 = query.value(3).toDouble();
                                     ++i;
                                     }
                             }
                             qDebug() << levelWater4;
                            waterLevel[rows / 2][cols / 2] = levelWater4;
                            db.close();

                }

                void spreadWater4() {

                    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                       db.setDatabaseName("D://Data.db");
                       if (!db.open()) {
                               qDebug() << "Connection error";
                             //  return 1;
                           }
                           else {
                               qDebug() << "Connection opened";
                           }

                    bool changed = false;
                    for (int i = 0; i < rows; ++i) {
                        for (int j = 0; j < cols; ++j) {
                            if (waterLevel[i][j] > 0) {
                                int min_height = INT_MAX;
                                int min_i = -1;
                                int min_j = -1;

                                // Проверяем соседние ячейки только в том случае, если они не заполнены водой
                                if (i > 0 && waterLevel[i - 1][j] == 0 && heights[i - 1][j] < min_height) {
                                    min_height = heights[i - 1][j];
                                    min_i = i - 1;
                                    min_j = j;
                                }
                                if (j > 0 && waterLevel[i][j - 1] == 0 && heights[i][j - 1] < min_height) {
                                    min_height = heights[i][j - 1];
                                    min_i = i;
                                    min_j = j - 1;
                                }
                                if (i < rows - 1 && waterLevel[i + 1][j] == 0 && heights[i + 1][j] < min_height) {
                                    min_height = heights[i + 1][j];
                                    min_i = i + 1;
                                    min_j = j;
                                }
                                if (j < cols - 1 && waterLevel[i][j + 1] == 0 && heights[i][j + 1] < min_height) {
                                    min_height = heights[i][j + 1];
                                    min_i = i;
                                    min_j = j + 1;
                                }

                                // Заполняем соседнюю ячейку с минимальной высотой водой
                                if (min_height != INT_MAX) {
                                    waterLevel[min_i][min_j] = waterLevel[i][j] + 1;
                                    changed = true;
                                    // Отрисовка новой синей ячейки
                                    QGraphicsRectItem *rect = scene->addRect(min_j * 10, min_i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                                    rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[min_i][min_j]));
                                    // Отображение обновленного рисунка с синими ячейками
                                    scene->update();
                                } else {
                                    // Если соседних ячеек с меньшей высотой нет, проверяем, окружена ли текущая ячейка ячейками с большей высотой
                                    bool surrounded = true;
                                    if (i > 0 && heights[i - 1][j] >= heights[i][j]) surrounded = false;
                                    if (j > 0 && heights[i][j - 1] >= heights[i][j]) surrounded = false;
                                    if (i < rows - 1 && heights[i + 1][j] >= heights[i][j]) surrounded = false;
                                    if (j < cols - 1 && heights[i][j + 1] >= heights[i][j]) surrounded = false;

                                    // Если текущая ячейка окружена ячейками с большей высотой, заполняем ее водой
                                    if (surrounded) {
                                        waterLevel[i][j] = waterLevel[i][j] + 1;
                                        changed = true;
                                        // Отрисовка новой синей ячейки
                                        QGraphicsRectItem *rect = scene->addRect(j * 10, i * 10, 10, 10, Qt::NoPen, QColor(47, 90, 153)); //Qt::blue
                                        rect->setToolTip(QString("Высота воды: %1").arg(waterLevel[i][j]));
                                        // Отображение обновленного рисунка с синими ячейками
                                        scene->update();
                                    }
                                }// Добавление задержки для более медленного распространения воды
                                QTime dieTime= QTime::currentTime().addMSecs(50);
                                while (QTime::currentTime() < dieTime)
                                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                            }
                        }
                    }
                    int water_count = 0;
                    for (int i = 0; i < rows; ++i) {
                        for (int j = 0; j < cols; ++j) {
                            if (waterLevel[i][j] > 0) {
                                water_count++;
                            }
                        }
                    }
                    //ЗДЕСЬ ПРИСВАИВАЕТСЯ ПЛОЩАДЬ НАВОДНЕНИЯ (В ЯЧЕЙКАХ - КОЛИЧЕСТВО ТОЧЕК ЭВАКУАЦИИ)

                    QSqlQuery query;
                    int squareWater4 = 0;
                     if (query.exec("SELECT * FROM Flood WHERE ID_Flood = 4")) {
                         int i =  0;
                             while (query.next() && i < 1) {
                                 squareWater4 = query.value(4).toDouble();
                             ++i;
                             }
                     }
                     qDebug() << squareWater4; //должно выйти 300
                    db.close();

                    if (water_count >= squareWater4) {
                        return; // Вода перестает распространяться
                    }
                    spreadWater4();// Если вода еще не достигла 30 ячеек, вызываем spreadWater() снова
                }

    //Цвет рельефа
    QColor heightToColor(float height) {
        // Определение цвета в зависимости от высоты
        const float max_height = 100.0;
        int red = static_cast<int>(255 * (height / max_height));
        int green = static_cast<int>(190 * (1 - (height / max_height)));
        int blue = 60; // фиксированное значение для синего цвета

        return QColor(red, green, blue);
    }

};

//ДЛЯ ВЫБОРА 1 ИЗ 4 ТИПОВ НАВОДНЕНИЙ
void FloodSelection(const QString &floodType) {}

// БД
//запись результата (обновляется каждый раз)
void saveResultsToDb(const QString &floodType, int peopleCount, double startValue,
                     const QString &minCostSpringFlood, int countHelicoptersSpringFloodMin,
                     double minCostSpringFloodValue) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("D://Data.db");
    if (!db.open()) {
        qDebug() << "Connection error WRITE: " << db.lastError().text();
        return;
    }
    qDebug() << "Connection opened WRITE";

    // Создаем таблицу только если она не существует
    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS Result ("
                    "Flood TEXT, " //ID_Flood
                    "People INTEGER, "
                    "Distance REAL, "
                    "RationalHC TEXT, " //ID_Type
                    "CountHC INTEGER, "
                    "Cost REAL)")) {
        qDebug() << "Ошибка при создании таблицы: " << query.lastError().text();
    }

    // Удаляем существующие данные из таблицы
    if (!query.exec("DELETE FROM Result")) {
        qDebug() << "Ошибка при очистке таблицы: " << query.lastError().text();
    }

    // Теперь можно вставить данные
    QSqlQuery queryInsert(db);
    queryInsert.prepare("INSERT INTO Result (Flood, People, Distance, RationalHC, CountHC, Cost) "
                        "VALUES (:Flood, :People, :Distance, :RationalHC, :CountHC, :Cost)");

    queryInsert.bindValue(":Flood", floodType);
    queryInsert.bindValue(":People", peopleCount);
    queryInsert.bindValue(":Distance", startValue);
    queryInsert.bindValue(":RationalHC", minCostSpringFlood);
    queryInsert.bindValue(":CountHC", countHelicoptersSpringFloodMin);
    queryInsert.bindValue(":Cost", minCostSpringFloodValue);

    if (!queryInsert.exec()) {
        qDebug() << "Ошибка при выполнении запроса на запись: " << queryInsert.lastError().text();
    } else {
        qDebug() << "Record inserted successfully!";
    }

    db.close();
}




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget *window = new QWidget();

    // Создание виджетов
    QLabel *label1 = new QLabel("Выберите тип наводнения");
    QComboBox *comboBox = new QComboBox();
    QLabel *label2 = new QLabel("Введите количество пострадавших");
    QLineEdit *lineEdit1 = new QLineEdit();
    QLabel *label3 = new QLabel("Введите расстояние от вертодрома до первой точки (км)");
    QLineEdit *lineEdit2 = new QLineEdit();
    QLabel *label4 = new QLabel("Введите ограничение на время (ч)");
    QLineEdit *lineEdit3 = new QLineEdit();

  //  QPushButton *pushButton0 = new QPushButton ("Открыть базу данных"); // для бд в программе

    QPushButton *pushButton1 = new QPushButton ("Результат");
    QPushButton *pushButton3 = new QPushButton("Очистить");
    QPushButton *pushButton2 = new QPushButton("Завершить");
    QWidget *dataWindow = new QWidget; // Создаем новое окно
    QWidget *windowGraph = new QWidget; // Создаем новое окно





    //для бд в программе.........................................................................................................................
    /*
       QDialog *dbDialog = new QDialog(nullptr);
    dbDialog->setWindowTitle("База данных");
        dbDialog->resize(900, 600);
        QTabWidget *tabWidget = new QTabWidget(dbDialog);
        QVBoxLayout *dialogLayout = new QVBoxLayout(dbDialog);
        dialogLayout->addWidget(tabWidget);

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("Data.db");
            if (!db.open()) {
                qDebug() << "Не удалось открыть базу данных: " << db.lastError().text();
                return -1;
            }
            */

/*
    QDialog dbDialog;
    dbDialog.setWindowTitle("Просмотр базы данных");
    dbDialog.resize(850, 400);

    // Tab widget для переключения между таблицами
    QVBoxLayout dialogLayout(&dbDialog);
    QTabWidget tabWidget;
    dialogLayout.addWidget(&tabWidget);

    // Подключение базы данных
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Data.db");
    if (!db.open()) {
        qDebug() << "Не удалось открыть базу данных: " << db.lastError();
    }

    // Функция для добавления вкладки с таблицей
    auto setupTable = [&db, &tabWidget](const QString &tableName) {
        QTableView *tableView = new QTableView();
        QSqlTableModel *tableModel = new QSqlTableModel(tableView, db);
        tableModel->setTable(tableName);
        tableModel->setEditStrategy(QSqlTableModel::OnFieldChange); // Изменения сохраняются сразу
        tableModel->select(); // Загрузить данные из таблицы
        tableView->setModel(tableModel);
        tabWidget.addTab(tableView, tableName);
        // Вызов select() для таблицы "Result"
                if (tableName == "Result") {
                    tableModel->select();
                }
    };
*/


    // Задаём фиксированный размер для comboBox и lineEdits
    comboBox->setFixedWidth(200);
        comboBox->addItem("");
        comboBox->addItem("Половодье");
        comboBox->addItem("Паводок");
        comboBox->addItem("Прорыв плотины");
        comboBox->addItem("Цунами");
    lineEdit1->setFixedWidth(200);
    lineEdit2->setFixedWidth(200);
    lineEdit3->setFixedWidth(200);


    // График
    QGraphicsView *graphWidget = new QGraphicsView();
    graphWidget->setFixedSize(700, 500);

    // TerrainMap
    QGraphicsView *terrainMapWidget = new QGraphicsView();
    terrainMapWidget->setFixedSize(700, 500);

    // Вертикальная компоновка для левой колонки
    QVBoxLayout *leftColumnLayout = new QVBoxLayout();
    leftColumnLayout->addWidget(label1);
    leftColumnLayout->addWidget(comboBox);
    leftColumnLayout->addWidget(label2);
    leftColumnLayout->addWidget(lineEdit1);
    leftColumnLayout->addWidget(label3);
    leftColumnLayout->addWidget(lineEdit2);
    leftColumnLayout->addWidget(label4);
    leftColumnLayout->addWidget(lineEdit3);
    leftColumnLayout->addStretch(1); // Добавляет пространство внизу

    //leftColumnLayout->addWidget(pushButton0);// для бд в программе

    leftColumnLayout->addWidget(pushButton1);
   leftColumnLayout->addWidget(pushButton3);
    leftColumnLayout->addWidget(pushButton2);

    // можно вводить только числа, причём первым не может быть 0
    QRegExp regExp1("[1-9][0-9]*");
        QRegExpValidator *validator1 = new QRegExpValidator(regExp1, lineEdit1);
        lineEdit1->setValidator(validator1);
    QRegExp regExp2("[1-9][0-9]*");
        QRegExpValidator *validator2 = new QRegExpValidator(regExp2, lineEdit2);
        lineEdit2->setValidator(validator2);

        QRegExp regExp3("^(?:0|[1-9]\\d*)(?:\\.\\d+)?$");
        QRegExpValidator *validator3 = new QRegExpValidator(regExp3, lineEdit3);
        lineEdit3->setValidator(validator3);


    // Горизонтальный компоновщик для основной области
    QHBoxLayout *mainLayout = new QHBoxLayout(window);
    mainLayout->addLayout(leftColumnLayout); // Добавляем левую колонку
    mainLayout->addWidget(graphWidget); // Сначала график

    QObject::connect(comboBox, &QComboBox::currentTextChanged, FloodSelection);
  //  TerrainMap *map = new TerrainMap(70, 70);

        QPointer<TerrainMap> currentMapWindow = nullptr;  // Используем QPointer для безопасных указателей

        // Слот для создания и отображения дополнительного окна
        QObject::connect(pushButton1, &QPushButton::clicked, [comboBox, &currentMapWindow, lineEdit1, lineEdit2, lineEdit3, dataWindow,windowGraph,graphWidget]() {

            // Получаем выбранный индекс
            int currentIndex = comboBox->currentIndex();

            // Если уже открыто дополнительное окно, закрываем его
            if (currentMapWindow) {
                currentMapWindow->close();  // Окно удалится, так как у него установлен флаг WA_DeleteOnClose
            }

            // Создаём новое дополнительное окно
            TerrainMap *map = new TerrainMap(70, 70);
            map->setAttribute(Qt::WA_DeleteOnClose); // Указываем, чтобы объект удалился при закрытии окна
            currentMapWindow = map; // Сохраняем указатель на новое окно

            //Открыть базу данных "Data"
                QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                   db.setDatabaseName("D://Data.db");
                   if (!db.open()) {
                           qDebug() << "Connection error";
                         //  return 1;
                       }
                       else {
                           qDebug() << "Connection opened";
                       }


                   QSqlQuery query;

                   //КОНСТАНТЫ из БД
                   int fuelCost = 0;
                   double timeHelp = 0;
                   double evacuationTimeForOnePerson = 0;
                   int resqueRaft = 0;
                   if (query.exec("SELECT * FROM ConstData")) {
                       if (query.next()) {
                           fuelCost = query.value(0).toInt(); // Из первого столбца
                           timeHelp = query.value(1).toDouble(); // Из второго столбца
                           evacuationTimeForOnePerson = query.value(2).toDouble(); // Из третьего столбца
                           resqueRaft = query.value(3).toInt(); // Из четвертого столбца
                       }
                   }

                   // Извлечение из БД скоростей наводнения
                   double speedWaterSpringFlood = 0;
                   double speedWaterFlashFlood = 0;
                   double speedWaterDamFlood = 0;
                   double speedWaterTsunamiFlood = 0;
                   if (query.exec("SELECT * FROM Flood")) {
                        int i =  0;
                        while (query.next() && i < 4) { //4 строки (от 0 до 3)
                            if (i == 0) speedWaterSpringFlood = query.value(2).toDouble()*3.6; // м/с -> км/ч
                                else if (i == 1) speedWaterFlashFlood = query.value(2).toDouble()*3.6;
                                else if (i == 2) speedWaterDamFlood = query.value(2).toDouble()*3.6;
                                else if (i == 3) speedWaterTsunamiFlood = query.value(2).toDouble()*3.6;
                            ++i;
                            }
                        }




















                   // Извлечение из БД данных о Ми-8
                   double capacityMi8 = 0;
                   double speedMi8 = 0;
                   double rangeMi8 = 0;
                   double fuelConsumptionMi8 = 0;
                   double fuelWeightMi8 = 0;
                   double rentMi8 = 0;
                   if (query.exec("SELECT * FROM Type")) {
                        int i =  0;
                        while (query.next() && i < 6) {
                            if (i == 0) capacityMi8 = query.value(2).toInt() + resqueRaft;
                                else if (i == 1) speedMi8 = query.value(2).toInt();
                                else if (i == 2) rangeMi8 = query.value(2).toInt();
                                else if (i == 3) fuelConsumptionMi8 = query.value(2).toInt();
                            else if (i == 4) fuelWeightMi8 = query.value(2).toInt();
                            else if (i == 5) rentMi8 = query.value(2).toInt();
                            ++i;
                            }
                        }

                   // Извлечение из БД данных о Ми-26
                   double capacityMi26 = 0;
                   double speedMi26 = 0;
                   double rangeMi26 = 0;
                   double fuelConsumptionMi26 = 0;
                   double fuelWeightMi26 = 0;
                   double rentMi26 = 0;
                   if (query.exec("SELECT * FROM Type")) {
                        int i =  0;
                        while (query.next() && i < 6) {
                            if (i == 0) capacityMi26 = query.value(3).toInt() + resqueRaft;
                                else if (i == 1) speedMi26 = query.value(3).toInt();
                                else if (i == 2) rangeMi26 = query.value(3).toInt();
                                else if (i == 3) fuelConsumptionMi26 = query.value(3).toInt();
                            else if (i == 4) fuelWeightMi26 = query.value(3).toInt();
                            else if (i == 5) rentMi26 = query.value(3).toInt();
                            ++i;
                            }
                        }

                   // Извлечение из БД данных о Sikorsky S-61
                   double capacitySikorskyS61 = 0;
                   double speedSikorskyS61 = 0;
                   double rangeSikorskyS61 = 0;
                   double fuelConsumptionSikorskyS61 = 0;
                   double fuelWeightSikorskyS61 = 0;
                   double rentSikorskyS61 = 0;
                   if (query.exec("SELECT * FROM Type")) {
                        int i =  0;
                        while (query.next() && i < 6) {
                            if (i == 0) capacitySikorskyS61 = query.value(4).toInt(); //сюда и в Ми-14 пока спасательный плот не добавляю
                                else if (i == 1) speedSikorskyS61 = query.value(4).toInt();
                                else if (i == 2) rangeSikorskyS61 = query.value(4).toInt();
                                else if (i == 3) fuelConsumptionSikorskyS61 = query.value(4).toInt();
                            else if (i == 4) fuelWeightSikorskyS61 = query.value(4).toInt();
                            else if (i == 5) rentSikorskyS61 = query.value(4).toInt();
                            ++i;
                            }
                        }

                   // Извлечение из БД данных о Ми-14
                   double capacityMi14 = 0;
                   double speedMi14 = 0;
                   double rangeMi14 = 0;
                   double fuelConsumptionMi14 = 0;
                   double fuelWeightMi14 = 0;
                   double rentMi14 = 0;
                   if (query.exec("SELECT * FROM Type")) {
                        int i =  0;
                        while (query.next() && i < 6) {
                            if (i == 0) capacityMi14 = query.value(5).toInt();
                                else if (i == 1) speedMi14 = query.value(5).toInt();
                                else if (i == 2) rangeMi14 = query.value(5).toInt();
                                else if (i == 3) fuelConsumptionMi14 = query.value(5).toInt();
                            else if (i == 4) fuelWeightMi14 = query.value(5).toInt();
                            else if (i == 5) rentMi14 = query.value(5).toInt();
                            ++i;
                            }
                        }













                   // Извлечение из БД данных о площади наводнения для опредедления количества точек с пострадавшими
                   int squareWater1 = 0;
                   int squareWater2 = 0;
                   int squareWater3 = 0;
                   int squareWater4 = 0;
                   if (query.exec("SELECT * FROM Flood")) {
                        int i =  0;
                        while (query.next() && i < 4) {
                            if (i == 0) squareWater1 = query.value(4).toInt();
                                else if (i == 1) squareWater2 = query.value(4).toInt();
                                else if (i == 2) squareWater3 = query.value(4).toInt();
                                else if (i == 3) squareWater4 = query.value(4).toInt();
                            ++i;
                            }
                        }

                              // Вывод рельефа и распространение воды в зависимости от выбранного типа наводнения
                              switch (currentIndex) {
// ПОЛОВОДЬЕ***********************************************************************************************************************************************
                                  {case 1:
                                      map->generateTerrain1();
                                      int peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                      //Расстояние между точками
                                      int startValue = lineEdit2->text().toInt(); // Первое значение из lineEdit
                                      double timeLimit = lineEdit3->text().toDouble(); // Первое значение из lineEdit
                                      QVector<double> time(squareWater1/10); // Создаем массив на 30 элементов - количество точек
                                      double step = timeLimit / (squareWater1/10); //9 / 30 = 0,3ч
                                      time[0] = step;
                                      for(int i = 1; i < squareWater1 / 10; ++i) {
                                      time[i] = time [i-1] + step;
                                      }
                                      for (int i = 0; i < squareWater1 / 10; ++i) {
                                          qDebug() << "time" << i << "=" << time[i];
                                      }

                                          double sSum = 0; // Для хранения суммы всех расстояний
                                          QVector<int> s(squareWater1/10); // Создаем массив на 30 элементов - количество точек
                                          //заполняем массив с расстояниями
                                            s[0] = startValue;
                                            for(int i = 1; i < squareWater1 / 10; ++i) {
                                                    s[i] = s[i - 1] + 10;
                                            }
                                            //сумма всех расстояний
                                            for (int i = 0; i < squareWater1 / 10; ++i) {
                                                    sSum += s[i]; // Суммируем все расстояния
                                            }
                                          // среднее арифметическое расстояний
                                          double sAverage = sSum / (squareWater1 / 10);
                                          // проверка ср ариф
                                          qDebug() << "sAverage" << sAverage;
                                          // проверка массива
                                          for (int i = 0; i < squareWater1/10; ++i) {
                                              qDebug() << "s" << i << "=" << s[i];
                                          }

                                         //массив времени воды до точек
                                          QVector<double> timeWaterFlood(squareWater1/10);
                                          for(int i = 0; i < squareWater1/10; ++i) {
                                                    timeWaterFlood[i] = s[i] / speedWaterSpringFlood; // время в часах
                                                }
                                          for (int i = 0; i < squareWater1/10; ++i) {
                                              qDebug() << "timeWaterFlood" << i << "=" << timeWaterFlood[i];
                                          }

                                          // Время между точками для всех ВК
                                          // Время для облета точек для Ми-8
                                            double tMi8Sum = 0; // Для хранения суммы всех расстояний
                                            QVector<double> tMi8(squareWater1/10);
                                              for(int i = 0; i < squareWater1/10; ++i) {
                                                      tMi8[i] = s[i] / speedMi8; // время в часах
                                                  }
                                              //сумма всех расстояний
                                              for (int i = 0; i < squareWater1 / 10; ++i) {
                                                      tMi8Sum += tMi8[i]; // Суммируем всё время
                                              }
                                            // среднее арифметическое расстояний
                                            double tMi8Average = tMi8Sum / (squareWater1 / 10);
                                            // проверка ср ариф
                                            qDebug() << "tMi8Average" << tMi8Average;
                                            // проверка массива
                                            for (int i = 0; i < squareWater1/10; ++i) {
                                                qDebug() << "tMi8" << i << "=" << tMi8[i];
                                            }

                                            // Время для облета точек для Ми-26
                                              double tMi26Sum = 0; // Для хранения суммы всех расстояний
                                              QVector<double> tMi26(squareWater1/10);
                                                for(int i = 0; i < squareWater1/10; ++i) {
                                                        tMi26[i] = s[i] / speedMi26; // время в часах
                                                    }
                                                //сумма всех расстояний
                                                for (int i = 0; i < squareWater1 / 10; ++i) {
                                                        tMi26Sum += tMi26[i]; // Суммируем всё время
                                                }
                                              // среднее арифметическое расстояний
                                              double tMi26Average = tMi26Sum / (squareWater1 / 10);
                                              // проверка ср ариф
                                              qDebug() << "tMi26Average" << tMi26Average;
                                              // проверка массива
                                              for (int i = 0; i < squareWater1/10; ++i) {
                                                  qDebug() << "tMi26" << i << "=" << tMi26[i];
                                              }

                                              // Время для облета точек для Sikorsky S-61
                                                double tSikorskyS61Sum = 0; // Для хранения суммы всех расстояний
                                                QVector<double> tSikorskyS61(squareWater1/10);
                                                  for(int i = 0; i < squareWater1/10; ++i) {
                                                          tSikorskyS61[i] = s[i] / speedSikorskyS61; // время в часах
                                                      }
                                                  //сумма всех расстояний
                                                  for (int i = 0; i < squareWater1 / 10; ++i) {
                                                          tSikorskyS61Sum += tSikorskyS61[i]; // Суммируем всё время
                                                  }
                                                // среднее арифметическое расстояний
                                                double tSikorskyS61Average = tSikorskyS61Sum / (squareWater1 / 10);
                                                // проверка ср ариф
                                                qDebug() << "tSikorskyS61Average" << tSikorskyS61Average;
                                                // проверка массива
                                                for (int i = 0; i < squareWater1/10; ++i) {
                                                    qDebug() << "tSikorskyS61" << i << "=" << tSikorskyS61[i];
                                                }

                                                // Время для облета точек для Ми-14
                                                  double tMi14Sum = 0; // Для хранения суммы всех расстояний
                                                  QVector<double> tMi14(squareWater1/10);
                                                    for(int i = 0; i < squareWater1/10; ++i) {
                                                            tMi14[i] = s[i] / speedMi14; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater1 / 10; ++i) {
                                                            tMi14Sum += tMi14[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double tMi14Average = tMi14Sum / (squareWater1 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "tMi14Average" << tMi14Average;
                                                  // проверка массива
                                                  for (int i = 0; i < squareWater1/10; ++i) {
                                                      qDebug() << "tMi14" << i << "=" << tMi14[i];
                                                  }

                                                  // Расход топлива * время Ми-8
                                                  double fuelMi8Sum = 0; // Для хранения суммы всего топлива
                                                  QVector<double> fuelMi8(squareWater1/10);
                                                    for(int i = 0; i < squareWater1/10; ++i) {
                                                            fuelMi8[i] = fuelConsumptionMi8 * tMi8[i]; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater1 / 10; ++i) {
                                                            fuelMi8Sum += fuelMi8[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double fuelMi8Average = fuelMi8Sum / (squareWater1 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "fuelMi8Average" << fuelMi8Average;

                                                    //проверка массива
                                                    for (int i = 0; i < squareWater1/10; ++i) {
                                                        qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                    }

                                                    // Расход топлива * время Ми-8
                                                    double fuelMi26Sum = 0; // Для хранения суммы всего топлива
                                                    QVector<double> fuelMi26(squareWater1/10);
                                                      for(int i = 0; i < squareWater1/10; ++i) {
                                                              fuelMi26[i] = fuelConsumptionMi26 * tMi26[i]; // время в часах
                                                          }
                                                      //сумма всех расстояний
                                                      for (int i = 0; i < squareWater1 / 10; ++i) {
                                                              fuelMi26Sum += fuelMi26[i]; // Суммируем всё время
                                                      }
                                                    // среднее арифметическое расстояний
                                                    double fuelMi26Average = fuelMi26Sum / (squareWater1 / 10);
                                                    // проверка ср ариф
                                                    qDebug() << "fuelMi26Average" << fuelMi26Average;
                                                      //проверка массива
                                                      for (int i = 0; i < squareWater1/10; ++i) {
                                                          qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                      }

                                                    // Расход топлива * время Sikorsky S-61
                                                 double fuelSikorskyS61Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelSikorskyS61(squareWater1/10);
                                                   for(int i = 0; i < squareWater1/10; ++i) {
                                                           fuelSikorskyS61[i] = fuelConsumptionSikorskyS61 * tSikorskyS61[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater1 / 10; ++i) {
                                                           fuelSikorskyS61Sum += fuelSikorskyS61[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelSikorskyS61Average = fuelSikorskyS61Sum / (squareWater1 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelSikorskyS61Average" << fuelSikorskyS61Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater1/10; ++i) {
                                                       qDebug() << "fuelSikorskyS61" << i << "=" << fuelSikorskyS61[i];
                                                   }

                                                    // Расход топлива * время Ми-14
                                                 double fuelMi14Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelMi14(squareWater1/10);
                                                   for(int i = 0; i < squareWater1/10; ++i) {
                                                           fuelMi14[i] = fuelConsumptionMi14 * tMi14[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater1 / 10; ++i) {
                                                           fuelMi14Sum += fuelMi14[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelMi14Average = fuelMi14Sum / (squareWater1 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelMi14Average" << fuelMi14Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater1/10; ++i) {
                                                       qDebug() << "fuelMi14" << i << "=" << fuelMi14[i];
                                                   }

                                        // Распределение количества пострадавших по всем точкам (в каждой точке минимум 3 человека)
                                        QVector<int> people(squareWater1/10, 3);
                                        peopleCount -= 3 * squareWater1/10;
                                        // среднее арифметическое расстояний
                                        double peopleAverage = peopleCount / (squareWater1 / 10);
                                        // проверка ср ариф
                                        qDebug() << "peopleAverage" << peopleAverage;
                                        QRandomGenerator *peopleGenerator = QRandomGenerator::system(); // Получаем системный генератор случайных чисел
                                        while (peopleCount > 0) {
                                            int randomIndex = peopleGenerator->bounded(0, squareWater1/10);
                                            int randomValue = peopleGenerator->bounded(1, qMin(peopleCount, 100) + 1);
                                            people[randomIndex] += randomValue;
                                            peopleCount -= randomValue; // из общего кол-ва пострадавших убирать тех, кто уже зачислен в точку
                                        }
                                        // проверка массива
                                        for (int i = 0; i < squareWater1/10; ++i) {
                                            qDebug() << "people" << i << "=" << people[i];
                                        }

                                //АЛГОРИТМ ЭВАКУАЦИИ
                                // Ми-8
                                double costOneMi8; // затраты на операцию для 1 вертолёта
                                  int flightCountMi8 = 0; // количество вылетов для одного вертолёта
                                  double rangeMi8Change = 0; // расходуемая дальность
                                  QVector<int> fuelWeightMi8Change(squareWater1/10, 0);
                                  fuelWeightMi8Change[0] = 0;
                                  double tOneMi8 = 0; // количество часов на выполнение операции для одного вертолёта
                                  double capacityEvacMi8 = capacityMi8 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                 if (startValue <= rangeMi8/2){ //сможет ли вертолёт вообще полететь и вернуться
                                  for(int i = 0; i < squareWater1/10; ++i) { //летим по точкам эвакуации
                                  if (tMi8[i] < timeWaterFlood[i] ){ //если Ми-8 прибудет раньше, чем вода затопит точку
                                              while ((rangeMi8Change < rangeMi8) && (fuelWeightMi8Change[i] < fuelWeightMi8) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                  while (capacityEvacMi8 > 0){ // пока есть место в вертолёте
                                                      capacityEvacMi8 = capacityEvacMi8 - peopleAverage; // забираем людей
                                                      // куда летим теперь (проверяем следующий вылет)
                                                      if (capacityEvacMi8 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                  }
                                                  flightCountMi8++; // считаем вылет
                                                  for(int i = 1; i < squareWater1/10; ++i) {
                                                  fuelWeightMi8Change[i] = fuelWeightMi8Change[i-1] + fuelMi8Average; // затраченное топливо
                                                  }
                                                  costOneMi8 = costOneMi8 + fuelWeightMi8Change[i]*fuelCost;
                                                  rangeMi8Change = rangeMi8Change + sAverage; // затраченная дальность
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. расстояние от вертодрома слишком большое");
                                         QVector<double> costMi8(squareWater1/10);
                                           for(int i = 0; i < squareWater1/10; ++i) {
                                                   costMi8[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi8Change.begin(), fuelWeightMi8Change.end());
                                           qDebug() << "fuelWeightMi8Change" << fuelWeightMi8Change; //проверка
                                         costOneMi8 = costOneMi8 + rentMi8; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi8" << flightCountMi8;
                                         qDebug() << "costOneMi8" << costOneMi8;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi8 =  peopleCount/capacityMi8 * tMi8Average / flightCountMi8 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi8" << tOneMi8;
                                         QVector<int> helicoptersMi8Count(squareWater1/10);
                                           for(int i = 0; i < squareWater1/10; ++i) {
                                                   helicoptersMi8Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                             helicoptersMi8Count[i] = tOneMi8 / time[i];
                                             if (helicoptersMi8Count[i] <= 1) { // Условие 1: если helicoptersMi8Count меньше или равно 1
                                                     helicoptersMi8Count[i] = 1;
                                                     costMi8[i] = (costOneMi8* helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi8Count больше 1
                                                 helicoptersMi8Count[i]= helicoptersMi8Count[i] +1;
                                                 costMi8[i] = (costMi8[i] + costOneMi8 * helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi8Count.begin(), helicoptersMi8Count.end());
                                         std::reverse(costMi8.begin(), costMi8.end());
                                         qDebug() << "helicoptersMi8Count" << helicoptersMi8Count;
                                         qDebug() << "costMi8" << costMi8;

                                         // Ми-26
                                         double costOneMi26; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         int flightCountMi26 = 0; // количество вылетов для одного вертолёта
                                         double rangeMi26Change = 0; // расходуемая дальность
                                         QVector<int> fuelWeightMi26Change(squareWater1/10, 0);
                                         fuelWeightMi26Change[0] = 0;
                                         double tOneMi26 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacMi26 = capacityMi26 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                        if (startValue <= rangeMi26/2){ //сможет ли вертолёт вообще полететь и вернуться
                                         for(int i = 0; i < squareWater1/10; ++i) { //летим по точкам эвакуации
                                         if (tMi26[i] < timeWaterFlood[i] ){ //если Ми-26 прибудет раньше, чем вода затопит точку
                                                     while ((rangeMi26Change < rangeMi26) && (fuelWeightMi26Change[i] < fuelWeightMi26) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                         while (capacityEvacMi26 > 0){ // пока есть место в вертолёте
                                                             capacityEvacMi26 = capacityEvacMi26 - peopleAverage; // забираем людей
                                                             // куда летим теперь (проверяем следующий вылет)
                                                             if (capacityEvacMi26 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                         }
                                                         flightCountMi26++; // считаем вылет
                                                         for(int i = 1; i < squareWater1/10; ++i) {
                                                         fuelWeightMi26Change[i] = fuelWeightMi26Change[i-1] + fuelMi26Average; // затраченное топливо
                                                         }
                                                         costOneMi26 = costOneMi26 + fuelWeightMi26Change[i]*fuelCost;
                                                         rangeMi26Change = rangeMi26Change + sAverage; // затраченная дальность
                                                     }
                                         }else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                        }
                               }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. расстояние от вертодрома слишком большое");
                                        QVector<double> costMi26(squareWater1/10);
                                          for(int i = 0; i < squareWater1/10; ++i) {
                                                  costMi26[i] = 0;
                                              }
                                          std::reverse(fuelWeightMi26Change.begin(), fuelWeightMi26Change.end());
                                          qDebug() << "fuelWeightMi26Change" << fuelWeightMi26Change; //проверка
                                        costOneMi26 = costOneMi26 + rentMi26; // затраты с 1 шт
                                        // проверка
                                        qDebug() << "flightCountMi26" << flightCountMi26;
                                        qDebug() << "costOneMi26" << costOneMi26;
                                        peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                        // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                        tOneMi26 =  peopleCount/capacityMi26 * tMi26Average / flightCountMi26 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                        qDebug() << "tOneMi26" << tOneMi26;
                                        QVector<int> helicoptersMi26Count(squareWater1/10);
                                          for(int i = 0; i < squareWater1/10; ++i) {
                                                  helicoptersMi26Count[i] = 0;
                                              }
                                        for(int i = 0; i < squareWater1/10; ++i) {
                                            helicoptersMi26Count[i] = tOneMi26 / time[i];
                                            if (helicoptersMi26Count[i] <= 1) { // Условие 1: если helicoptersMi26Count меньше или равно 1
                                                    helicoptersMi26Count[i] = 1;
                                                    costMi26[i] = (costOneMi26* helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000;
                                            }
                                            else { // Условие 2: если helicoptersMi26Count больше 1
                                                helicoptersMi26Count[i]= helicoptersMi26Count[i] +1;
                                                costMi26[i] = (costMi26[i] + costOneMi26 * helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                             }
                                        }
                                        std::reverse(helicoptersMi26Count.begin(), helicoptersMi26Count.end());
                                        std::reverse(costMi26.begin(), costMi26.end());
                                        qDebug() << "helicoptersMi26Count" << helicoptersMi26Count;
                                        qDebug() << "costMi26" << costMi26;

                                       // Sikorsky S-61
                                         double costOneSikorskyS61; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                        // int helicoptersSikorskyS61Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                         int flightCountSikorskyS61=0;
                                         QVector<int> fuelWeightSikorskyS61Change(squareWater1/10, 0);
                                         fuelWeightSikorskyS61Change[0] = 0;
                                         double rangeSikorskyS61Change = 0; // расходуемая дальность
                                         double costFirstSikorskyS61 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                         double tOneSikorskyS61 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacSikorskyS61 = capacitySikorskyS61; // не добавляю спасательный плот для одного вертолёта
                                         //вертолёт сбрасывает ровно столько спасательных плотов, чтоб хватило всем
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                                 while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61)) {
                                                     rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                     for(int i = 1; i < squareWater1/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                             }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                         costFirstSikorskyS61 = costFirstSikorskyS61 + rentSikorskyS61; // затраты на первый вертолёт
                                         capacityEvacSikorskyS61 = capacitySikorskyS61+resqueRaft;
                                         // обновляем дальность и топливо, чтоб считать второй вертолёт
                                         rangeSikorskyS61Change = 0; // расходуемая дальность
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                         fuelWeightSikorskyS61Change[i] = 0;
                                         }
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeSikorskyS61/2){ //сможет ли вертолёт вообще полететь и вернуться
                                        for(int i = 0; i < squareWater1/10; ++i) { //летим по точкам эвакуации
                                       // if (tSikorskyS61[i] < timeWaterFlood[i] ){ //если Sikorsky S-61 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                    while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                        while (capacityEvacSikorskyS61 > 0){ // пока есть место в вертолёте
                                                            capacityEvacSikorskyS61 = capacityEvacSikorskyS61 - peopleAverage; // забираем людей
                                                            // куда летим теперь (проверяем следующий вылет)
                                                            if (capacityEvacSikorskyS61 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                        }
                                                        flightCountSikorskyS61++; // считаем вылет
                                                        for(int i = 1; i < squareWater1/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                                        rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                    }
                                        }
                              }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. расстояние от вертодрома слишком большое");
                                       }}
                              QVector<double> costSikorskyS61(squareWater1/10);
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                                 costSikorskyS61[i] = 0;
                                             }
                                         std::reverse(fuelWeightSikorskyS61Change.begin(), fuelWeightSikorskyS61Change.end());
                                         qDebug() << "fuelWeightSikorskyS61Change" << fuelWeightSikorskyS61Change; //проверка
                                       costOneSikorskyS61 = costOneSikorskyS61 + rentSikorskyS61; // затраты с 1 шт
                                       // проверка
                                       qDebug() << "flightCountSikorskyS61" << flightCountSikorskyS61;
                                       qDebug() << "costOneSikorskyS61" << costOneSikorskyS61;
                                       peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                       // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                       tOneSikorskyS61 =  peopleCount/capacitySikorskyS61 * tSikorskyS61Average / flightCountSikorskyS61 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                       qDebug() << "tOneSikorskyS61" << tOneSikorskyS61;
                                       QVector<int> helicoptersSikorskyS61Count(squareWater1/10);
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                                 helicoptersSikorskyS61Count[i] = 0;
                                             }
                                       for(int i = 0; i < squareWater1/10; ++i) {
                                           helicoptersSikorskyS61Count[i] = tOneSikorskyS61 / time[i];
                                           if (helicoptersSikorskyS61Count[i] <= 1) { // Условие 1: если helicoptersSikorskyS61Count меньше или равно 1
                                                   helicoptersSikorskyS61Count[i] = 1;
                                                   costSikorskyS61[i] = (costOneSikorskyS61* helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000;
                                           }
                                           else { // Условие 2: если helicoptersSikorskyS61Count больше 1
                                               helicoptersSikorskyS61Count[i]= helicoptersSikorskyS61Count[i] +1;
                                               costSikorskyS61[i] = (costSikorskyS61[i] + costOneSikorskyS61 * helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                            }
                                       }
                                       std::reverse(helicoptersSikorskyS61Count.begin(), helicoptersSikorskyS61Count.end());
                                       std::reverse(costSikorskyS61.begin(), costSikorskyS61.end());
                                       qDebug() << "helicoptersSikorskyS61Count" << helicoptersSikorskyS61Count;
                                       qDebug() << "costSikorskyS61" << costSikorskyS61;

                                        // Ми-14
                                          double costOneMi14; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         // int helicoptersMi14Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                          int flightCountMi14=0;
                                          QVector<int> fuelWeightMi14Change(squareWater1/10, 0);
                                          fuelWeightMi14Change[0] = 0;
                                          double rangeMi14Change = 0; // расходуемая дальность
                                          double costFirstMi14 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                          double tOneMi14 = 0; // количество часов на выполнение операции для одного вертолёта
                                          double capacityEvacMi14 = capacityMi14; // не добавляю спасательный плот для одного вертолёта
                                          for(int i = 0; i < squareWater1/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                                  while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14)) {
                                                      rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      for(int i = 1; i < squareWater1/10; ++i) {
                                                         fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                         }
                                                         costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                          }
                                          costFirstMi14 = costFirstMi14 + rentMi14; // затраты на первый вертолёт
                                          capacityEvacMi14 = capacityMi14+resqueRaft;
                                          // обновляем дальность и топливо, чтоб считать второй вертолёт
                                          rangeMi14Change = 0; // расходуемая дальность
                                          for(int i = 0; i < squareWater1/10; ++i) {
                                          fuelWeightMi14Change[i] = 0;
                                          }
                                          for(int i = 0; i < squareWater1/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                          if (startValue <= rangeMi14/2){ //сможет ли вертолёт вообще полететь и вернуться
                                          for(int i = 0; i < squareWater1/10; ++i) { //летим по точкам эвакуации
                                         // if (tMi14[i] < timeWaterFlood[i] ){ //если Ми-14 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                      while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                          while (capacityEvacMi14 > 0){ // пока есть место в вертолёте
                                                              capacityEvacMi14 = capacityEvacMi14 - peopleAverage; // забираем людей
                                                              // куда летим теперь (проверяем следующий вылет)
                                                              if (capacityEvacMi14 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                          }
                                                          flightCountMi14++; // считаем вылет
                                                          for(int i = 1; i < squareWater1/10; ++i) {
                                                          fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                          }
                                                          costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                                          rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      }
                                          }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. расстояние от вертодрома слишком большое");
                                        }}
                              QVector<double> costMi14(squareWater1/10);
                                           for(int i = 0; i < squareWater1/10; ++i) {
                                                   costMi14[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi14Change.begin(), fuelWeightMi14Change.end());
                                           qDebug() << "fuelWeightMi14Change" << fuelWeightMi14Change; //проверка
                                         costOneMi14 = costOneMi14 + rentMi14; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi14" << flightCountMi14;
                                         qDebug() << "costOneMi14" << costOneMi14;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi14 =  peopleCount/capacityMi14 * tMi14Average / flightCountMi14 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi14" << tOneMi14;
                                         QVector<int> helicoptersMi14Count(squareWater1/10);
                                           for(int i = 0; i < squareWater1/10; ++i) {
                                                   helicoptersMi14Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater1/10; ++i) {
                                             helicoptersMi14Count[i] = tOneMi14 / time[i];
                                             if (helicoptersMi14Count[i] <= 1) { // Условие 1: если helicoptersMi14Count меньше или равно 1
                                                     helicoptersMi14Count[i] = 1;
                                                     costMi14[i] = (costOneMi14* helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi14Count больше 1
                                                 helicoptersMi14Count[i]= helicoptersMi14Count[i] +1;
                                                 costMi14[i] = (costMi14[i] + costOneMi14 * helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi14Count.begin(), helicoptersMi14Count.end());
                                         std::reverse(costMi14.begin(), costMi14.end());
                                         qDebug() << "helicoptersMi14Count" << helicoptersMi14Count;
                                         qDebug() << "costMi14" << costMi14;


                                         // ОПРЕДЕЛЕНИЕ РАЦИОНАЛЬНОГО ВК ПО ЗАТРАТАМ
                                         // Настройки карты стоимостей
                                        QMap<QString, double>
                                                costAllFlood = {
                                            {"Mi8", costMi8[squareWater1/10-1]},
                                            {"Mi26", costMi26[squareWater1/10-1]},
                                            {"SikorskyS61", costSikorskyS61[squareWater1/10-1]},
                                            {"Mi14", costMi14[squareWater1/10-1]}
                                        };

                                        // Карта счетчиков вертолетов
                                        QMap<QString, int> helicoptersCountMap = {
                                            {"Mi8", helicoptersMi8Count[squareWater1/10-1]},
                                            {"Mi26", helicoptersMi26Count[squareWater1/10-1]},
                                            {"SikorskyS61", helicoptersSikorskyS61Count[squareWater1/10-1]},
                                            {"Mi14", helicoptersMi14Count[squareWater1/10-1]}
                                        };

                                        // Карта счетчиков полётов вертолетов - чтобы если вертолёт не подошёл, его затраты не учитывались, т.к. они равны 0
                                        QMap<QString, int> helicoptersFlightCountMap = {
                                            {"Mi8", flightCountMi8},
                                            {"Mi26", flightCountMi26},
                                            {"SikorskyS61", flightCountSikorskyS61},
                                            {"Mi14", flightCountMi14}
                                        };

                                        QString minCostFlood;
                                        double minCostFloodValue = std::numeric_limits<double>::max();

                                        for (auto it = costAllFlood.constBegin(); it != costAllFlood.constEnd(); ++it) {
                                            // Проверяем, что количество полетов для этого типа вертолетов больше нуля
                                            if (it.value() < minCostFloodValue && helicoptersFlightCountMap[it.key()] > 0) {
                                                minCostFloodValue = it.value();
                                                minCostFlood = it.key();
                                            }
                                        }
                                        // Проверяем, было ли найдено соответствующее минимальное значение
                                        if (minCostFloodValue == std::numeric_limits<double>::max()) {
                                          QMessageBox::critical(nullptr, "Ошибка", "Ни один вертолёт не подошёл");
                                          minCostFloodValue = 0;
                                        }

                                        // Запрос количества вертолетов для найденной минимальной стоимости
                                        int countHelicoptersFloodMin = helicoptersCountMap[minCostFlood];

                                        qDebug() << "Рациональный ВК: " << minCostFlood //ПОЛОВОДЬЕ
                                                 << "; Количество ВК: " << countHelicoptersFloodMin
                                                 << "; Затраты: " << minCostFloodValue;

                                          QString floodType="Половодье"; // для вывода в результаты

                                          // Сохранить в таблице БД результаты
                                          saveResultsToDb(floodType, peopleCount, startValue, minCostFlood, countHelicoptersFloodMin, minCostFloodValue);

                                        // ГРАФИКИ SPRING FLOOD
                                          //если пострадаших нет, расстояние 0 км, + расстояние до вертодрома меньше дальности вертолёта то ничего не выводим
                                          if ((peopleCount != 0) && (startValue != 0) && (timeLimit != 0)){
                                          QChart *chart = new QChart();
                                          chart->update();
                                          chart->legend()->hide();

                                          QLineSeries *seriesMi8 = new QLineSeries();
                                          if (flightCountMi8>0){
                                        seriesMi8->setName("Ми-8");
                                        for (int i = 0; i < time.size(); ++i) {
                                                seriesMi8->append(time[i], costMi8[i]);
                                            }
                                        chart->addSeries(seriesMi8);
                                        }
                                            QLineSeries *seriesMi26 = new QLineSeries();

                                            if (flightCountMi26>0){
                                                seriesMi26->setName("Ми-26");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi26->append(time[i], costMi26[i]);
                                            }
                                            chart->addSeries(seriesMi26);
                                             }

                                            QLineSeries *seriesSikorskyS61 = new QLineSeries();
                                            if (flightCountSikorskyS61>0){
                                            seriesSikorskyS61->setName("Sikorsky S-61");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesSikorskyS61->append(time[i], costSikorskyS61[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesSikorskyS61);
                                            }

                                            QLineSeries *seriesMi14 = new QLineSeries();
                                            if (flightCountMi14>0){
                                            seriesMi14->setName("Ми-14");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi14->append(time[i], costMi14[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesMi14);
                                            }

                                            chart->createDefaultAxes();
                                            chart->setTitle("Затраты (млн.руб.) всех ВК в зависимости от времени (ч)");
                                            chart->legend()->setVisible(true);






                                            QValueAxis *auxAxisX = new QValueAxis;
                                            auxAxisX->setRange(1, int(squareWater1/10)); // Установите диапазон значений оси
                                            auxAxisX->setTickCount(int(squareWater1/10)); // Количество делений оси
                                            auxAxisX->setLinePenColor(Qt::gray); // Цвет линии оси
                                            QFont labelsFont;
                                            labelsFont.setPointSize(4); // Устанавливаем меньший размер шрифта
                                            QBrush labelsBrush(Qt::black);
                                            labelsFont.setBold(true);
                                            auxAxisX->setLabelsFont(labelsFont); // Применяем шрифт к меткам
                                            auxAxisX->setLabelFormat("%d");
                                            chart->addAxis(auxAxisX, Qt::AlignTop);





                                            QChartView *chartView = new QChartView(chart);
                                            chartView->setRenderHint(QPainter::Antialiasing);
                                            // Устанавливаем QChartView как центральный виджет в windowGraph
                                            QVBoxLayout *layout = new QVBoxLayout;
                                            layout->addWidget(chartView);

                                       //ВЫВОД РЕЗУЛЬТАТА В ОТДЕЛЬНОМ ОКНЕ
                                       // dataWindow->setAttribute(Qt::WA_DeleteOnClose); // Окно будет удаляться при закрытии
                                        dataWindow->setWindowTitle("Результат");
                                        dataWindow->resize(200, 400);

                                        QString variableName0 = "Тип наводнения";
                                        QString variableName1 = "Количество пострадавших";
                                        QString variableName2 = "Рациональный ВК";
                                        QString variableName3 = "Расстояние от вертодрома";
                                        QString variableName6 = "Ограничение на время";
                                        QString variableName4 = "Количество ВК";
                                        QString variableName5 = "Затраты";

                                        // Создаем таблицу для отображения данных
                                        QTableWidget *tableWidget = new QTableWidget(dataWindow); // Создан с 0 строк и 0 столбцов
                                        tableWidget->clearContents();
                                        tableWidget->setRowCount(0);
                                        tableWidget->setRowCount(7); // Устанавливаем 6 строк
                                        tableWidget->setColumnCount(2);
                                        // tableWidget->setHorizontalHeaderLabels(QStringList() << "Параметр" << "Значение");
                                        tableWidget->verticalHeader()->setVisible(false);   // Скрывает нумерацию строк
                                        tableWidget->horizontalHeader()->setVisible(false); // Скрывает названия столбцов
                                        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
                                        tableWidget->setSelectionMode(QAbstractItemView::NoSelection); // Запрет выделения

                                        tableWidget->setStyleSheet("QTableView { border: 1px solid black; }"
                                                                   "QTableView::item { border: 1px solid black; }"
                                                                   "QHeaderView::section { border: 1px solid black; }");
                                        // Заполняем строки
                                        tableWidget->setItem(0, 0, new QTableWidgetItem(variableName0));
                                        tableWidget->setItem(0, 1, new QTableWidgetItem(floodType));

                                        tableWidget->setItem(1, 0, new QTableWidgetItem(variableName1));
                                        tableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(peopleCount) + " человек"));

                                        tableWidget->setItem(2, 0, new QTableWidgetItem(variableName2));
                                        tableWidget->setItem(2, 1, new QTableWidgetItem(minCostFlood));

                                        tableWidget->setItem(3, 0, new QTableWidgetItem(variableName3));
                                        tableWidget->setItem(3, 1, new QTableWidgetItem(QString::number(startValue) + " км"));

                                        tableWidget->setItem(4, 0, new QTableWidgetItem(variableName6));
                                        tableWidget->setItem(4, 1, new QTableWidgetItem(QString::number(timeLimit) + " ч"));

                                        tableWidget->setItem(5, 0, new QTableWidgetItem(variableName4));
                                        tableWidget->setItem(5, 1, new QTableWidgetItem(QString::number(countHelicoptersFloodMin)));

                                        tableWidget->setItem(6, 0, new QTableWidgetItem(variableName5));
                                        tableWidget->setItem(6, 1, new QTableWidgetItem(QString::number(minCostFloodValue) + " млн.руб."));

                                        tableWidget->resizeColumnsToContents(); // Автоматическая настройка ширины колонок
                                        tableWidget->horizontalHeader()->setStretchLastSection(true); // Последний столбец растягивается

                                        // Устанавливаем layout для отображения только что созданной таблицы
                                        dataWindow->setLayout(new QVBoxLayout);
                                        dataWindow->layout()->addWidget(tableWidget);

                                        dataWindow->setMinimumSize(450, 287);
                                        dataWindow->setMaximumSize(450, 287);
                                        dataWindow->show();

                                        graphWidget->setLayout(layout);
                                        graphWidget->show(); // показать график
                                        map->show(); // Вывод рельефа в дополнительном окне
                                        map->spreadWater1(); // распространение воды по рельефу

                                        } else QMessageBox::critical(nullptr, "Ошибка", "Поле для ввода пустое");
                                        break;}


// ПАВОДОК*********************************************************************************************************************************
                              {case 2: map->generateTerrain2();
                                      int peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                      //Расстояние между точками
                                      int startValue = lineEdit2->text().toInt(); // Первое значение из lineEdit
                                      double timeLimit = lineEdit3->text().toDouble(); // Первое значение из lineEdit

                                      QVector<double> time(squareWater2/10); // Создаем массив на 30 элементов - количество точек
                                      double step = timeLimit / (squareWater2/10); //9 / 30 = 0,3ч
                                      time[0] = step;
                                      for(int i = 1; i < squareWater2 / 10; ++i) {
                                      time[i] = time [i-1] + step;
                                      }
                                      for (int i = 0; i < squareWater2 / 10; ++i) {
                                          qDebug() << "time" << i << "=" << time[i];
                                      }

                                          double sSum = 0; // Для хранения суммы всех расстояний
                                          QVector<int> s(squareWater2/10); // Создаем массив на 30 элементов - количество точек
                                          //заполняем массив с расстояниями
                                            s[0] = startValue;
                                            for(int i = 1; i < squareWater2 / 10; ++i) {
                                                    s[i] = s[i - 1] + 10;
                                            }
                                            //сумма всех расстояний
                                            for (int i = 0; i < squareWater2 / 10; ++i) {
                                                    sSum += s[i]; // Суммируем все расстояния
                                            }
                                          // среднее арифметическое расстояний
                                          double sAverage = sSum / (squareWater2 / 10);
                                          // проверка ср ариф
                                          qDebug() << "sAverage" << sAverage;
                                          // проверка массива
                                          for (int i = 0; i < squareWater2/10; ++i) {
                                              qDebug() << "s" << i << "=" << s[i];
                                          }

                                         //массив времени воды до точек
                                          QVector<double> timeWaterFlood(squareWater2/10);
                                          for(int i = 0; i < squareWater2/10; ++i) {
                                                    timeWaterFlood[i] = s[i] / speedWaterFlashFlood; // время в часах
                                                }
                                          for (int i = 0; i < squareWater2/10; ++i) {
                                              qDebug() << "timeWaterFlood" << i << "=" << timeWaterFlood[i];
                                          }

                                          // Время между точками для всех ВК
                                          // Время для облета точек для Ми-8
                                            double tMi8Sum = 0; // Для хранения суммы всех расстояний
                                            QVector<double> tMi8(squareWater2/10);
                                              for(int i = 0; i < squareWater2/10; ++i) {
                                                      tMi8[i] = s[i] / speedMi8; // время в часах
                                                  }
                                              //сумма всех расстояний
                                              for (int i = 0; i < squareWater2 / 10; ++i) {
                                                      tMi8Sum += tMi8[i]; // Суммируем всё время
                                              }
                                            // среднее арифметическое расстояний
                                            double tMi8Average = tMi8Sum / (squareWater2 / 10);
                                            // проверка ср ариф
                                            qDebug() << "tMi8Average" << tMi8Average;
                                            // проверка массива
                                            for (int i = 0; i < squareWater2/10; ++i) {
                                                qDebug() << "tMi8" << i << "=" << tMi8[i];
                                            }

                                            // Время для облета точек для Ми-26
                                              double tMi26Sum = 0; // Для хранения суммы всех расстояний
                                              QVector<double> tMi26(squareWater2/10);
                                                for(int i = 0; i < squareWater2/10; ++i) {
                                                        tMi26[i] = s[i] / speedMi26; // время в часах
                                                    }
                                                //сумма всех расстояний
                                                for (int i = 0; i < squareWater2 / 10; ++i) {
                                                        tMi26Sum += tMi26[i]; // Суммируем всё время
                                                }
                                              // среднее арифметическое расстояний
                                              double tMi26Average = tMi26Sum / (squareWater2 / 10);
                                              // проверка ср ариф
                                              qDebug() << "tMi26Average" << tMi26Average;
                                              // проверка массива
                                              for (int i = 0; i < squareWater2/10; ++i) {
                                                  qDebug() << "tMi26" << i << "=" << tMi26[i];
                                              }

                                              // Время для облета точек для Sikorsky S-61
                                                double tSikorskyS61Sum = 0; // Для хранения суммы всех расстояний
                                                QVector<double> tSikorskyS61(squareWater2/10);
                                                  for(int i = 0; i < squareWater2/10; ++i) {
                                                          tSikorskyS61[i] = s[i] / speedSikorskyS61; // время в часах
                                                      }
                                                  //сумма всех расстояний
                                                  for (int i = 0; i < squareWater2 / 10; ++i) {
                                                          tSikorskyS61Sum += tSikorskyS61[i]; // Суммируем всё время
                                                  }
                                                // среднее арифметическое расстояний
                                                double tSikorskyS61Average = tSikorskyS61Sum / (squareWater2 / 10);
                                                // проверка ср ариф
                                                qDebug() << "tSikorskyS61Average" << tSikorskyS61Average;
                                                // проверка массива
                                                for (int i = 0; i < squareWater2/10; ++i) {
                                                    qDebug() << "tSikorskyS61" << i << "=" << tSikorskyS61[i];
                                                }

                                                // Время для облета точек для Ми-14
                                                  double tMi14Sum = 0; // Для хранения суммы всех расстояний
                                                  QVector<double> tMi14(squareWater2/10);
                                                    for(int i = 0; i < squareWater2/10; ++i) {
                                                            tMi14[i] = s[i] / speedMi14; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater2 / 10; ++i) {
                                                            tMi14Sum += tMi14[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double tMi14Average = tMi14Sum / (squareWater2 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "tMi14Average" << tMi14Average;
                                                  // проверка массива
                                                  for (int i = 0; i < squareWater2/10; ++i) {
                                                      qDebug() << "tMi14" << i << "=" << tMi14[i];
                                                  }

                                                  // Расход топлива * время Ми-8
                                                  double fuelMi8Sum = 0; // Для хранения суммы всего топлива
                                                  QVector<double> fuelMi8(squareWater2/10);
                                                    for(int i = 0; i < squareWater2/10; ++i) {
                                                            fuelMi8[i] = fuelConsumptionMi8 * tMi8[i]; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater2 / 10; ++i) {
                                                            fuelMi8Sum += fuelMi8[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double fuelMi8Average = fuelMi8Sum / (squareWater2 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "fuelMi8Average" << fuelMi8Average;

                                                    //проверка массива
                                                    for (int i = 0; i < squareWater2/10; ++i) {
                                                        qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                    }

                                                    // Расход топлива * время Ми-8
                                                    double fuelMi26Sum = 0; // Для хранения суммы всего топлива
                                                    QVector<double> fuelMi26(squareWater2/10);
                                                      for(int i = 0; i < squareWater2/10; ++i) {
                                                              fuelMi26[i] = fuelConsumptionMi26 * tMi26[i]; // время в часах
                                                          }
                                                      //сумма всех расстояний
                                                      for (int i = 0; i < squareWater2 / 10; ++i) {
                                                              fuelMi26Sum += fuelMi26[i]; // Суммируем всё время
                                                      }
                                                    // среднее арифметическое расстояний
                                                    double fuelMi26Average = fuelMi26Sum / (squareWater2 / 10);
                                                    // проверка ср ариф
                                                    qDebug() << "fuelMi26Average" << fuelMi26Average;
                                                      //проверка массива
                                                      for (int i = 0; i < squareWater2/10; ++i) {
                                                          qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                      }

                                                    // Расход топлива * время Sikorsky S-61
                                                 double fuelSikorskyS61Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelSikorskyS61(squareWater2/10);
                                                   for(int i = 0; i < squareWater2/10; ++i) {
                                                           fuelSikorskyS61[i] = fuelConsumptionSikorskyS61 * tSikorskyS61[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater2 / 10; ++i) {
                                                           fuelSikorskyS61Sum += fuelSikorskyS61[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelSikorskyS61Average = fuelSikorskyS61Sum / (squareWater2 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelSikorskyS61Average" << fuelSikorskyS61Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater2/10; ++i) {
                                                       qDebug() << "fuelSikorskyS61" << i << "=" << fuelSikorskyS61[i];
                                                   }

                                                    // Расход топлива * время Ми-14
                                                 double fuelMi14Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelMi14(squareWater2/10);
                                                   for(int i = 0; i < squareWater2/10; ++i) {
                                                           fuelMi14[i] = fuelConsumptionMi14 * tMi14[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater2 / 10; ++i) {
                                                           fuelMi14Sum += fuelMi14[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelMi14Average = fuelMi14Sum / (squareWater2 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelMi14Average" << fuelMi14Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater2/10; ++i) {
                                                       qDebug() << "fuelMi14" << i << "=" << fuelMi14[i];
                                                   }

                                        // Распределение количества пострадавших по всем точкам (в каждой точке минимум 3 человека)
                                        QVector<int> people(squareWater2/10, 3);
                                        peopleCount -= 3 * squareWater2/10;
                                        // среднее арифметическое расстояний
                                        double peopleAverage = peopleCount / (squareWater2 / 10);
                                        // проверка ср ариф
                                        qDebug() << "peopleAverage" << peopleAverage;
                                        QRandomGenerator *peopleGenerator = QRandomGenerator::system(); // Получаем системный генератор случайных чисел
                                        while (peopleCount > 0) {
                                            int randomIndex = peopleGenerator->bounded(0, squareWater2/10);
                                            int randomValue = peopleGenerator->bounded(1, qMin(peopleCount, 100) + 1);
                                            people[randomIndex] += randomValue;
                                            peopleCount -= randomValue; // из общего кол-ва пострадавших убирать тех, кто уже зачислен в точку
                                        }
                                        // проверка массива
                                        for (int i = 0; i < squareWater2/10; ++i) {
                                            qDebug() << "people" << i << "=" << people[i];
                                        }

                                //АЛГОРИТМ ЭВАКУАЦИИ
                                // Ми-8
                                double costOneMi8; // затраты на операцию для 1 вертолёта
                                  int flightCountMi8 = 0; // количество вылетов для одного вертолёта
                                  double rangeMi8Change = 0; // расходуемая дальность
                                  QVector<int> fuelWeightMi8Change(squareWater2/10, 0);
                                  fuelWeightMi8Change[0] = 0;
                                  double tOneMi8 = 0; // количество часов на выполнение операции для одного вертолёта
                                  double capacityEvacMi8 = capacityMi8 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                 if (startValue <= rangeMi8/2){ //сможет ли вертолёт вообще полететь и вернуться
                                  for(int i = 0; i < squareWater2/10; ++i) { //летим по точкам эвакуации
                                  if (tMi8[i] < timeWaterFlood[i] ){ //если Ми-8 прибудет раньше, чем вода затопит точку
                                              while ((rangeMi8Change < rangeMi8) && (fuelWeightMi8Change[i] < fuelWeightMi8) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                  while (capacityEvacMi8 > 0){ // пока есть место в вертолёте
                                                      capacityEvacMi8 = capacityEvacMi8 - peopleAverage; // забираем людей
                                                      // куда летим теперь (проверяем следующий вылет)
                                                      if (capacityEvacMi8 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                  }
                                                  flightCountMi8++; // считаем вылет
                                                  for(int i = 1; i < squareWater2/10; ++i) {
                                                  fuelWeightMi8Change[i] = fuelWeightMi8Change[i-1] + fuelMi8Average; // затраченное топливо
                                                  }
                                                  costOneMi8 = costOneMi8 + fuelWeightMi8Change[i]*fuelCost;
                                                  rangeMi8Change = rangeMi8Change + sAverage; // затраченная дальность
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. расстояние от вертодрома слишком большое");
                                         QVector<double> costMi8(squareWater2/10);
                                           for(int i = 0; i < squareWater2/10; ++i) {
                                                   costMi8[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi8Change.begin(), fuelWeightMi8Change.end());
                                           qDebug() << "fuelWeightMi8Change" << fuelWeightMi8Change; //проверка
                                         costOneMi8 = costOneMi8 + rentMi8; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi8" << flightCountMi8;
                                         qDebug() << "costOneMi8" << costOneMi8;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi8 =  peopleCount/capacityMi8 * tMi8Average / flightCountMi8 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi8" << tOneMi8;
                                         QVector<int> helicoptersMi8Count(squareWater2/10);
                                           for(int i = 0; i < squareWater2/10; ++i) {
                                                   helicoptersMi8Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                             helicoptersMi8Count[i] = tOneMi8 / time[i];
                                             if (helicoptersMi8Count[i] <= 1) { // Условие 1: если helicoptersMi8Count меньше или равно 1
                                                     helicoptersMi8Count[i] = 1;
                                                     costMi8[i] = (costOneMi8* helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi8Count больше 1
                                                 helicoptersMi8Count[i]= helicoptersMi8Count[i] +1;
                                                 costMi8[i] = (costMi8[i] + costOneMi8 * helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi8Count.begin(), helicoptersMi8Count.end());
                                         std::reverse(costMi8.begin(), costMi8.end());
                                         qDebug() << "helicoptersMi8Count" << helicoptersMi8Count;
                                         qDebug() << "costMi8" << costMi8;

                                         // Ми-26
                                         double costOneMi26; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         int flightCountMi26 = 0; // количество вылетов для одного вертолёта
                                         double rangeMi26Change = 0; // расходуемая дальность
                                         QVector<int> fuelWeightMi26Change(squareWater2/10, 0);
                                         fuelWeightMi26Change[0] = 0;
                                         double tOneMi26 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacMi26 = capacityMi26 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                        if (startValue <= rangeMi26/2){ //сможет ли вертолёт вообще полететь и вернуться
                                         for(int i = 0; i < squareWater2/10; ++i) { //летим по точкам эвакуации
                                         if (tMi26[i] < timeWaterFlood[i] ){ //если Ми-26 прибудет раньше, чем вода затопит точку
                                                     while ((rangeMi26Change < rangeMi26) && (fuelWeightMi26Change[i] < fuelWeightMi26) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                         while (capacityEvacMi26 > 0){ // пока есть место в вертолёте
                                                             capacityEvacMi26 = capacityEvacMi26 - peopleAverage; // забираем людей
                                                             // куда летим теперь (проверяем следующий вылет)
                                                             if (capacityEvacMi26 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                         }
                                                         flightCountMi26++; // считаем вылет
                                                         for(int i = 1; i < squareWater2/10; ++i) {
                                                         fuelWeightMi26Change[i] = fuelWeightMi26Change[i-1] + fuelMi26Average; // затраченное топливо
                                                         }
                                                         costOneMi26 = costOneMi26 + fuelWeightMi26Change[i]*fuelCost;
                                                         rangeMi26Change = rangeMi26Change + sAverage; // затраченная дальность
                                                     }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                        }
                               }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. расстояние от вертодрома слишком большое");
                                        QVector<double> costMi26(squareWater2/10);
                                          for(int i = 0; i < squareWater2/10; ++i) {
                                                  costMi26[i] = 0;
                                              }
                                          std::reverse(fuelWeightMi26Change.begin(), fuelWeightMi26Change.end());
                                          qDebug() << "fuelWeightMi26Change" << fuelWeightMi26Change; //проверка
                                        costOneMi26 = costOneMi26 + rentMi26; // затраты с 1 шт
                                        // проверка
                                        qDebug() << "flightCountMi26" << flightCountMi26;
                                        qDebug() << "costOneMi26" << costOneMi26;
                                        peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                        // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                        tOneMi26 =  peopleCount/capacityMi26 * tMi26Average / flightCountMi26 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                        qDebug() << "tOneMi26" << tOneMi26;
                                        QVector<int> helicoptersMi26Count(squareWater2/10);
                                          for(int i = 0; i < squareWater2/10; ++i) {
                                                  helicoptersMi26Count[i] = 0;
                                              }
                                        for(int i = 0; i < squareWater2/10; ++i) {
                                            helicoptersMi26Count[i] = tOneMi26 / time[i];
                                            if (helicoptersMi26Count[i] <= 1) { // Условие 1: если helicoptersMi26Count меньше или равно 1
                                                    helicoptersMi26Count[i] = 1;
                                                    costMi26[i] = (costOneMi26* helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000;
                                            }
                                            else { // Условие 2: если helicoptersMi26Count больше 1
                                                helicoptersMi26Count[i]= helicoptersMi26Count[i] +1;
                                                costMi26[i] = (costMi26[i] + costOneMi26 * helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                             }
                                        }
                                        std::reverse(helicoptersMi26Count.begin(), helicoptersMi26Count.end());
                                        std::reverse(costMi26.begin(), costMi26.end());
                                        qDebug() << "helicoptersMi26Count" << helicoptersMi26Count;
                                        qDebug() << "costMi26" << costMi26;

                                       // Sikorsky S-61
                                         double costOneSikorskyS61; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                        // int helicoptersSikorskyS61Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                         int flightCountSikorskyS61=0;
                                         QVector<int> fuelWeightSikorskyS61Change(squareWater2/10, 0);
                                         fuelWeightSikorskyS61Change[0] = 0;
                                         double rangeSikorskyS61Change = 0; // расходуемая дальность
                                         double costFirstSikorskyS61 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                         double tOneSikorskyS61 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacSikorskyS61 = capacitySikorskyS61; // не добавляю спасательный плот для одного вертолёта
                                         //вертолёт сбрасывает ровно столько спасательных плотов, чтоб хватило всем
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                                 while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61)) {
                                                     rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                     for(int i = 1; i < squareWater2/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                             }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                         costFirstSikorskyS61 = costFirstSikorskyS61 + rentSikorskyS61; // затраты на первый вертолёт
                                         capacityEvacSikorskyS61 = capacitySikorskyS61+resqueRaft;
                                         // обновляем дальность и топливо, чтоб считать второй вертолёт
                                         rangeSikorskyS61Change = 0; // расходуемая дальность
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                         fuelWeightSikorskyS61Change[i] = 0;
                                         }
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeSikorskyS61/2){ //сможет ли вертолёт вообще полететь и вернуться
                                        for(int i = 0; i < squareWater2/10; ++i) { //летим по точкам эвакуации
                                       // if (tSikorskyS61[i] < timeWaterFlood[i] ){ //если Sikorsky S-61 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                    while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                        while (capacityEvacSikorskyS61 > 0){ // пока есть место в вертолёте
                                                            capacityEvacSikorskyS61 = capacityEvacSikorskyS61 - peopleAverage; // забираем людей
                                                            // куда летим теперь (проверяем следующий вылет)
                                                            if (capacityEvacSikorskyS61 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                        }
                                                        flightCountSikorskyS61++; // считаем вылет
                                                        for(int i = 1; i < squareWater2/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                                        rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                    }
                                        }
                              }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. расстояние от вертодрома слишком большое");
                                       }}
                              QVector<double> costSikorskyS61(squareWater2/10);
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                                 costSikorskyS61[i] = 0;
                                             }
                                         std::reverse(fuelWeightSikorskyS61Change.begin(), fuelWeightSikorskyS61Change.end());
                                         qDebug() << "fuelWeightSikorskyS61Change" << fuelWeightSikorskyS61Change; //проверка
                                       costOneSikorskyS61 = costOneSikorskyS61 + rentSikorskyS61; // затраты с 1 шт
                                       // проверка
                                       qDebug() << "flightCountSikorskyS61" << flightCountSikorskyS61;
                                       qDebug() << "costOneSikorskyS61" << costOneSikorskyS61;
                                       peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                       // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                       tOneSikorskyS61 =  peopleCount/capacitySikorskyS61 * tSikorskyS61Average / flightCountSikorskyS61 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                       qDebug() << "tOneSikorskyS61" << tOneSikorskyS61;
                                       QVector<int> helicoptersSikorskyS61Count(squareWater2/10);
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                                 helicoptersSikorskyS61Count[i] = 0;
                                             }
                                       for(int i = 0; i < squareWater2/10; ++i) {
                                           helicoptersSikorskyS61Count[i] = tOneSikorskyS61 / time[i];
                                           if (helicoptersSikorskyS61Count[i] <= 1) { // Условие 1: если helicoptersSikorskyS61Count меньше или равно 1
                                                   helicoptersSikorskyS61Count[i] = 1;
                                                   costSikorskyS61[i] = (costOneSikorskyS61* helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000;
                                           }
                                           else { // Условие 2: если helicoptersSikorskyS61Count больше 1
                                               helicoptersSikorskyS61Count[i]= helicoptersSikorskyS61Count[i] +1;
                                               costSikorskyS61[i] = (costSikorskyS61[i] + costOneSikorskyS61 * helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                            }
                                       }
                                       std::reverse(helicoptersSikorskyS61Count.begin(), helicoptersSikorskyS61Count.end());
                                       std::reverse(costSikorskyS61.begin(), costSikorskyS61.end());
                                       qDebug() << "helicoptersSikorskyS61Count" << helicoptersSikorskyS61Count;
                                       qDebug() << "costSikorskyS61" << costSikorskyS61;

                                        // Ми-14
                                          double costOneMi14; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         // int helicoptersMi14Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                          int flightCountMi14=0;
                                          QVector<int> fuelWeightMi14Change(squareWater2/10, 0);
                                          fuelWeightMi14Change[0] = 0;
                                          double rangeMi14Change = 0; // расходуемая дальность
                                          double costFirstMi14 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                          double tOneMi14 = 0; // количество часов на выполнение операции для одного вертолёта
                                          double capacityEvacMi14 = capacityMi14; // не добавляю спасательный плот для одного вертолёта
                                          for(int i = 0; i < squareWater2/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                                  while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14)) {
                                                      rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      for(int i = 1; i < squareWater2/10; ++i) {
                                                         fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                         }
                                                         costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                          }
                                          costFirstMi14 = costFirstMi14 + rentMi14; // затраты на первый вертолёт
                                          capacityEvacMi14 = capacityMi14+resqueRaft;
                                          // обновляем дальность и топливо, чтоб считать второй вертолёт
                                          rangeMi14Change = 0; // расходуемая дальность
                                          for(int i = 0; i < squareWater2/10; ++i) {
                                          fuelWeightMi14Change[i] = 0;
                                          }
                                          for(int i = 0; i < squareWater2/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeMi14/2){ //сможет ли вертолёт вообще полететь и вернуться
                                          for(int i = 0; i < squareWater2/10; ++i) { //летим по точкам эвакуации
                                         // if (tMi14[i] < timeWaterFlood[i] ){ //если Ми-14 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                      while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                          while (capacityEvacMi14 > 0){ // пока есть место в вертолёте
                                                              capacityEvacMi14 = capacityEvacMi14 - peopleAverage; // забираем людей
                                                              // куда летим теперь (проверяем следующий вылет)
                                                              if (capacityEvacMi14 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                          }
                                                          flightCountMi14++; // считаем вылет
                                                          for(int i = 1; i < squareWater2/10; ++i) {
                                                          fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                          }
                                                          costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                                          rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      }
                                          }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. расстояние от вертодрома слишком большое");
                                        }}
                              QVector<double> costMi14(squareWater2/10);
                                           for(int i = 0; i < squareWater2/10; ++i) {
                                                   costMi14[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi14Change.begin(), fuelWeightMi14Change.end());
                                           qDebug() << "fuelWeightMi14Change" << fuelWeightMi14Change; //проверка
                                         costOneMi14 = costOneMi14 + rentMi14; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi14" << flightCountMi14;
                                         qDebug() << "costOneMi14" << costOneMi14;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi14 =  peopleCount/capacityMi14 * tMi14Average / flightCountMi14 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi14" << tOneMi14;
                                         QVector<int> helicoptersMi14Count(squareWater2/10);
                                           for(int i = 0; i < squareWater2/10; ++i) {
                                                   helicoptersMi14Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater2/10; ++i) {
                                             helicoptersMi14Count[i] = tOneMi14 / time[i];
                                             if (helicoptersMi14Count[i] <= 1) { // Условие 1: если helicoptersMi14Count меньше или равно 1
                                                     helicoptersMi14Count[i] = 1;
                                                     costMi14[i] = (costOneMi14* helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi14Count больше 1
                                                 helicoptersMi14Count[i]= helicoptersMi14Count[i] +1;
                                                 costMi14[i] = (costMi14[i] + costOneMi14 * helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi14Count.begin(), helicoptersMi14Count.end());
                                         std::reverse(costMi14.begin(), costMi14.end());
                                         qDebug() << "helicoptersMi14Count" << helicoptersMi14Count;
                                         qDebug() << "costMi14" << costMi14;


                                         // ОПРЕДЕЛЕНИЕ РАЦИОНАЛЬНОГО ВК ПО ЗАТРАТАМ
                                         // Настройки карты стоимостей
                                        QMap<QString, double>
                                                costAllFlood = {
                                            {"Mi8", costMi8[squareWater2/10-1]},
                                            {"Mi26", costMi26[squareWater2/10-1]},
                                            {"SikorskyS61", costSikorskyS61[squareWater2/10-1]},
                                            {"Mi14", costMi14[squareWater2/10-1]}
                                        };

                                        // Карта счетчиков вертолетов
                                        QMap<QString, int> helicoptersCountMap = {
                                            {"Mi8", helicoptersMi8Count[squareWater2/10-1]},
                                            {"Mi26", helicoptersMi26Count[squareWater2/10-1]},
                                            {"SikorskyS61", helicoptersSikorskyS61Count[squareWater2/10-1]},
                                            {"Mi14", helicoptersMi14Count[squareWater2/10-1]}
                                        };

                                        // Карта счетчиков полётов вертолетов - чтобы если вертолёт не подошёл, его затраты не учитывались, т.к. они равны 0
                                        QMap<QString, int> helicoptersFlightCountMap = {
                                            {"Mi8", flightCountMi8},
                                            {"Mi26", flightCountMi26},
                                            {"SikorskyS61", flightCountSikorskyS61},
                                            {"Mi14", flightCountMi14}
                                        };

                                        QString minCostFlood;
                                        double minCostFloodValue = std::numeric_limits<double>::max();

                                        for (auto it = costAllFlood.constBegin(); it != costAllFlood.constEnd(); ++it) {
                                            // Проверяем, что количество полетов для этого типа вертолетов больше нуля
                                            if (it.value() < minCostFloodValue && helicoptersFlightCountMap[it.key()] > 0) {
                                                minCostFloodValue = it.value();
                                                minCostFlood = it.key();
                                            }
                                        }
                                        // Проверяем, было ли найдено соответствующее минимальное значение
                                        if (minCostFloodValue == std::numeric_limits<double>::max()) {
                                          QMessageBox::critical(nullptr, "Ошибка", "Ни один вертолёт не подошёл");
                                          minCostFloodValue = 0;
                                        }

                                        // Запрос количества вертолетов для найденной минимальной стоимости
                                        int countHelicoptersFloodMin = helicoptersCountMap[minCostFlood];

                                        qDebug() << "Рациональный ВК: " << minCostFlood //ПАВОДОК
                                                 << "; Количество ВК: " << countHelicoptersFloodMin
                                                 << "; Затраты: " << minCostFloodValue;

                                          QString floodType="Паводок"; // для вывода в результаты

                                          // Сохранить в таблице БД результаты
                                          saveResultsToDb(floodType, peopleCount, startValue, minCostFlood, countHelicoptersFloodMin, minCostFloodValue);

                                        // ГРАФИКИ FLASH FLOOD
                                          //если пострадаших нет, расстояние 0 км, + расстояние до вертодрома меньше дальности вертолёта то ничего не выводим
                                          if ((peopleCount != 0) && (startValue != 0) && (timeLimit != 0)){
                                          QChart *chart = new QChart();
                                          chart->update();
                                          chart->legend()->hide();

                                          QLineSeries *seriesMi8 = new QLineSeries();
                                          if (flightCountMi8>0){
                                        seriesMi8->setName("Ми-8");
                                        for (int i = 0; i < time.size(); ++i) {
                                                seriesMi8->append(time[i], costMi8[i]);
                                            }
                                        chart->addSeries(seriesMi8);
                                        }
                                            QLineSeries *seriesMi26 = new QLineSeries();

                                            if (flightCountMi26>0){
                                                seriesMi26->setName("Ми-26");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi26->append(time[i], costMi26[i]);
                                            }
                                            chart->addSeries(seriesMi26);
                                             }

                                            QLineSeries *seriesSikorskyS61 = new QLineSeries();
                                            if (flightCountSikorskyS61>0){
                                            seriesSikorskyS61->setName("Sikorsky S-61");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesSikorskyS61->append(time[i], costSikorskyS61[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesSikorskyS61);
                                            }

                                            QLineSeries *seriesMi14 = new QLineSeries();
                                            if (flightCountMi14>0){
                                            seriesMi14->setName("Ми-14");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi14->append(time[i], costMi14[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesMi14);
                                            }

                                            chart->createDefaultAxes();
                                            chart->setTitle("Затраты (млн.руб.) всех ВК в зависимости от времени (ч)");
                                            chart->legend()->setVisible(true);


                                            QValueAxis *auxAxisX = new QValueAxis;
                                            auxAxisX->setRange(1, int(squareWater2/10)); // Установите диапазон значений оси
                                            auxAxisX->setTickCount(int(squareWater2/10)); // Количество делений оси
                                            auxAxisX->setLinePenColor(Qt::gray); // Цвет линии оси
                                            QFont labelsFont;
                                            labelsFont.setPointSize(4); // Устанавливаем меньший размер шрифта
                                            QBrush labelsBrush(Qt::black);
                                            labelsFont.setBold(true);
                                            auxAxisX->setLabelsFont(labelsFont); // Применяем шрифт к меткам
                                            auxAxisX->setLabelFormat("%d");
                                            chart->addAxis(auxAxisX, Qt::AlignTop);

                                            QChartView *chartView = new QChartView(chart);
                                            chartView->setRenderHint(QPainter::Antialiasing);
                                            // Устанавливаем QChartView как центральный виджет в windowGraph
                                            QVBoxLayout *layout = new QVBoxLayout;
                                            layout->addWidget(chartView);

                                       //ВЫВОД РЕЗУЛЬТАТА В ОТДЕЛЬНОМ ОКНЕ
                                       // dataWindow->setAttribute(Qt::WA_DeleteOnClose); // Окно будет удаляться при закрытии
                                        dataWindow->setWindowTitle("Результат");
                                        dataWindow->resize(200, 400);

                                        QString variableName0 = "Тип наводнения";
                                        QString variableName1 = "Количество пострадавших";
                                        QString variableName2 = "Рациональный ВК";
                                        QString variableName3 = "Расстояние от вертодрома";
                                        QString variableName6 = "Ограничение на время";
                                        QString variableName4 = "Количество ВК";
                                        QString variableName5 = "Затраты";

                                        // Создаем таблицу для отображения данных
                                        QTableWidget *tableWidget = new QTableWidget(dataWindow); // Создан с 0 строк и 0 столбцов
                                        tableWidget->clearContents();
                                        tableWidget->setRowCount(0);
                                        tableWidget->setRowCount(7); // Устанавливаем 6 строк
                                        tableWidget->setColumnCount(2);
                                        // tableWidget->setHorizontalHeaderLabels(QStringList() << "Параметр" << "Значение");
                                        tableWidget->verticalHeader()->setVisible(false);   // Скрывает нумерацию строк
                                        tableWidget->horizontalHeader()->setVisible(false); // Скрывает названия столбцов
                                        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
                                        tableWidget->setSelectionMode(QAbstractItemView::NoSelection); // Запрет выделения

                                        tableWidget->setStyleSheet("QTableView { border: 1px solid black; }"
                                                                   "QTableView::item { border: 1px solid black; }"
                                                                   "QHeaderView::section { border: 1px solid black; }");
                                        // Заполняем строки
                                        tableWidget->setItem(0, 0, new QTableWidgetItem(variableName0));
                                        tableWidget->setItem(0, 1, new QTableWidgetItem(floodType));

                                        tableWidget->setItem(1, 0, new QTableWidgetItem(variableName1));
                                        tableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(peopleCount) + " человек"));

                                        tableWidget->setItem(2, 0, new QTableWidgetItem(variableName2));
                                        tableWidget->setItem(2, 1, new QTableWidgetItem(minCostFlood));

                                        tableWidget->setItem(3, 0, new QTableWidgetItem(variableName3));
                                        tableWidget->setItem(3, 1, new QTableWidgetItem(QString::number(startValue) + " км"));

                                        tableWidget->setItem(4, 0, new QTableWidgetItem(variableName6));
                                        tableWidget->setItem(4, 1, new QTableWidgetItem(QString::number(timeLimit) + " ч"));

                                        tableWidget->setItem(5, 0, new QTableWidgetItem(variableName4));
                                        tableWidget->setItem(5, 1, new QTableWidgetItem(QString::number(countHelicoptersFloodMin)));

                                        tableWidget->setItem(6, 0, new QTableWidgetItem(variableName5));
                                        tableWidget->setItem(6, 1, new QTableWidgetItem(QString::number(minCostFloodValue) + " млн.руб."));

                                        tableWidget->resizeColumnsToContents(); // Автоматическая настройка ширины колонок
                                        tableWidget->horizontalHeader()->setStretchLastSection(true); // Последний столбец растягивается

                                        // Устанавливаем layout для отображения только что созданной таблицы
                                        dataWindow->setLayout(new QVBoxLayout);
                                        dataWindow->layout()->addWidget(tableWidget);

                                        dataWindow->setMinimumSize(450, 287);
                                        dataWindow->setMaximumSize(450, 287);
                                        dataWindow->show();

                                        graphWidget->setLayout(layout);
                                        graphWidget->show(); // показать график
                                        map->show(); // Вывод рельефа в дополнительном окне
                                        map->spreadWater2(); // распространение воды по рельефу
                                          } else QMessageBox::critical(nullptr, "Ошибка", "Поле для ввода пустое");
                                        break;}

// ПРОРЫВ ПЛОТИНЫ**************************************************************************************************************************
                              {case 3: map->generateTerrain3();
                                      int peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                      //Расстояние между точками
                                      int startValue = lineEdit2->text().toInt(); // Первое значение из lineEdit
                                      double timeLimit = lineEdit3->text().toDouble(); // Первое значение из lineEdit

                                      QVector<double> time(squareWater3/10); // Создаем массив на 30 элементов - количество точек
                                      double step = timeLimit / (squareWater3/10); //9 / 30 = 0,3ч
                                      time[0] = step;
                                      for(int i = 1; i < squareWater3 / 10; ++i) {
                                      time[i] = time [i-1] + step;
                                      }
                                      for (int i = 0; i < squareWater3 / 10; ++i) {
                                          qDebug() << "time" << i << "=" << time[i];
                                      }

                                          double sSum = 0; // Для хранения суммы всех расстояний
                                          QVector<int> s(squareWater3/10); // Создаем массив на 30 элементов - количество точек
                                          //заполняем массив с расстояниями
                                            s[0] = startValue;
                                            for(int i = 1; i < squareWater3 / 10; ++i) {
                                                    s[i] = s[i - 1] + 10;
                                            }
                                            //сумма всех расстояний
                                            for (int i = 0; i < squareWater3 / 10; ++i) {
                                                    sSum += s[i]; // Суммируем все расстояния
                                            }
                                          // среднее арифметическое расстояний
                                          double sAverage = sSum / (squareWater3 / 10);
                                          // проверка ср ариф
                                          qDebug() << "sAverage" << sAverage;
                                          // проверка массива
                                          for (int i = 0; i < squareWater3/10; ++i) {
                                              qDebug() << "s" << i << "=" << s[i];
                                          }

                                         //массив времени воды до точек
                                          QVector<double> timeWaterFlood(squareWater3/10);
                                          for(int i = 0; i < squareWater3/10; ++i) {
                                                    timeWaterFlood[i] = s[i] / speedWaterDamFlood; // время в часах
                                                }
                                          for (int i = 0; i < squareWater3/10; ++i) {
                                              qDebug() << "timeWaterFlood" << i << "=" << timeWaterFlood[i];
                                          }

                                          // Время между точками для всех ВК
                                          // Время для облета точек для Ми-8
                                            double tMi8Sum = 0; // Для хранения суммы всех расстояний
                                            QVector<double> tMi8(squareWater3/10);
                                              for(int i = 0; i < squareWater3/10; ++i) {
                                                      tMi8[i] = s[i] / speedMi8; // время в часах
                                                  }
                                              //сумма всех расстояний
                                              for (int i = 0; i < squareWater3 / 10; ++i) {
                                                      tMi8Sum += tMi8[i]; // Суммируем всё время
                                              }
                                            // среднее арифметическое расстояний
                                            double tMi8Average = tMi8Sum / (squareWater3 / 10);
                                            // проверка ср ариф
                                            qDebug() << "tMi8Average" << tMi8Average;
                                            // проверка массива
                                            for (int i = 0; i < squareWater3/10; ++i) {
                                                qDebug() << "tMi8" << i << "=" << tMi8[i];
                                            }

                                            // Время для облета точек для Ми-26
                                              double tMi26Sum = 0; // Для хранения суммы всех расстояний
                                              QVector<double> tMi26(squareWater3/10);
                                                for(int i = 0; i < squareWater3/10; ++i) {
                                                        tMi26[i] = s[i] / speedMi26; // время в часах
                                                    }
                                                //сумма всех расстояний
                                                for (int i = 0; i < squareWater3 / 10; ++i) {
                                                        tMi26Sum += tMi26[i]; // Суммируем всё время
                                                }
                                              // среднее арифметическое расстояний
                                              double tMi26Average = tMi26Sum / (squareWater3 / 10);
                                              // проверка ср ариф
                                              qDebug() << "tMi26Average" << tMi26Average;
                                              // проверка массива
                                              for (int i = 0; i < squareWater3/10; ++i) {
                                                  qDebug() << "tMi26" << i << "=" << tMi26[i];
                                              }

                                              // Время для облета точек для Sikorsky S-61
                                                double tSikorskyS61Sum = 0; // Для хранения суммы всех расстояний
                                                QVector<double> tSikorskyS61(squareWater3/10);
                                                  for(int i = 0; i < squareWater3/10; ++i) {
                                                          tSikorskyS61[i] = s[i] / speedSikorskyS61; // время в часах
                                                      }
                                                  //сумма всех расстояний
                                                  for (int i = 0; i < squareWater3 / 10; ++i) {
                                                          tSikorskyS61Sum += tSikorskyS61[i]; // Суммируем всё время
                                                  }
                                                // среднее арифметическое расстояний
                                                double tSikorskyS61Average = tSikorskyS61Sum / (squareWater3 / 10);
                                                // проверка ср ариф
                                                qDebug() << "tSikorskyS61Average" << tSikorskyS61Average;
                                                // проверка массива
                                                for (int i = 0; i < squareWater3/10; ++i) {
                                                    qDebug() << "tSikorskyS61" << i << "=" << tSikorskyS61[i];
                                                }

                                                // Время для облета точек для Ми-14
                                                  double tMi14Sum = 0; // Для хранения суммы всех расстояний
                                                  QVector<double> tMi14(squareWater3/10);
                                                    for(int i = 0; i < squareWater3/10; ++i) {
                                                            tMi14[i] = s[i] / speedMi14; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater3 / 10; ++i) {
                                                            tMi14Sum += tMi14[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double tMi14Average = tMi14Sum / (squareWater3 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "tMi14Average" << tMi14Average;
                                                  // проверка массива
                                                  for (int i = 0; i < squareWater3/10; ++i) {
                                                      qDebug() << "tMi14" << i << "=" << tMi14[i];
                                                  }

                                                  // Расход топлива * время Ми-8
                                                  double fuelMi8Sum = 0; // Для хранения суммы всего топлива
                                                  QVector<double> fuelMi8(squareWater3/10);
                                                    for(int i = 0; i < squareWater3/10; ++i) {
                                                            fuelMi8[i] = fuelConsumptionMi8 * tMi8[i]; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater3 / 10; ++i) {
                                                            fuelMi8Sum += fuelMi8[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double fuelMi8Average = fuelMi8Sum / (squareWater3 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "fuelMi8Average" << fuelMi8Average;

                                                    //проверка массива
                                                    for (int i = 0; i < squareWater3/10; ++i) {
                                                        qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                    }

                                                    // Расход топлива * время Ми-8
                                                    double fuelMi26Sum = 0; // Для хранения суммы всего топлива
                                                    QVector<double> fuelMi26(squareWater3/10);
                                                      for(int i = 0; i < squareWater3/10; ++i) {
                                                              fuelMi26[i] = fuelConsumptionMi26 * tMi26[i]; // время в часах
                                                          }
                                                      //сумма всех расстояний
                                                      for (int i = 0; i < squareWater3 / 10; ++i) {
                                                              fuelMi26Sum += fuelMi26[i]; // Суммируем всё время
                                                      }
                                                    // среднее арифметическое расстояний
                                                    double fuelMi26Average = fuelMi26Sum / (squareWater3 / 10);
                                                    // проверка ср ариф
                                                    qDebug() << "fuelMi26Average" << fuelMi26Average;
                                                      //проверка массива
                                                      for (int i = 0; i < squareWater3/10; ++i) {
                                                          qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                      }

                                                    // Расход топлива * время Sikorsky S-61
                                                 double fuelSikorskyS61Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelSikorskyS61(squareWater3/10);
                                                   for(int i = 0; i < squareWater3/10; ++i) {
                                                           fuelSikorskyS61[i] = fuelConsumptionSikorskyS61 * tSikorskyS61[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater3 / 10; ++i) {
                                                           fuelSikorskyS61Sum += fuelSikorskyS61[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelSikorskyS61Average = fuelSikorskyS61Sum / (squareWater3 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelSikorskyS61Average" << fuelSikorskyS61Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater3/10; ++i) {
                                                       qDebug() << "fuelSikorskyS61" << i << "=" << fuelSikorskyS61[i];
                                                   }

                                                    // Расход топлива * время Ми-14
                                                 double fuelMi14Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelMi14(squareWater3/10);
                                                   for(int i = 0; i < squareWater3/10; ++i) {
                                                           fuelMi14[i] = fuelConsumptionMi14 * tMi14[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater3 / 10; ++i) {
                                                           fuelMi14Sum += fuelMi14[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelMi14Average = fuelMi14Sum / (squareWater3 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelMi14Average" << fuelMi14Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater3/10; ++i) {
                                                       qDebug() << "fuelMi14" << i << "=" << fuelMi14[i];
                                                   }

                                        // Распределение количества пострадавших по всем точкам (в каждой точке минимум 3 человека)
                                        QVector<int> people(squareWater3/10, 3);
                                        peopleCount -= 3 * squareWater3/10;
                                        // среднее арифметическое расстояний
                                        double peopleAverage = peopleCount / (squareWater3 / 10);
                                        // проверка ср ариф
                                        qDebug() << "peopleAverage" << peopleAverage;
                                        QRandomGenerator *peopleGenerator = QRandomGenerator::system(); // Получаем системный генератор случайных чисел
                                        while (peopleCount > 0) {
                                            int randomIndex = peopleGenerator->bounded(0, squareWater3/10);
                                            int randomValue = peopleGenerator->bounded(1, qMin(peopleCount, 100) + 1);
                                            people[randomIndex] += randomValue;
                                            peopleCount -= randomValue; // из общего кол-ва пострадавших убирать тех, кто уже зачислен в точку
                                        }
                                        // проверка массива
                                        for (int i = 0; i < squareWater3/10; ++i) {
                                            qDebug() << "people" << i << "=" << people[i];
                                        }

                                //АЛГОРИТМ ЭВАКУАЦИИ
                                // Ми-8
                                double costOneMi8; // затраты на операцию для 1 вертолёта
                                  int flightCountMi8 = 0; // количество вылетов для одного вертолёта
                                  double rangeMi8Change = 0; // расходуемая дальность
                                  QVector<int> fuelWeightMi8Change(squareWater3/10, 0);
                                  fuelWeightMi8Change[0] = 0;
                                  double tOneMi8 = 0; // количество часов на выполнение операции для одного вертолёта
                                  double capacityEvacMi8 = capacityMi8 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                 if (startValue <= rangeMi8/2){ //сможет ли вертолёт вообще полететь и вернуться
                                  for(int i = 0; i < squareWater3/10; ++i) { //летим по точкам эвакуации
                                  if (tMi8[i] < timeWaterFlood[i] ){ //если Ми-8 прибудет раньше, чем вода затопит точку
                                              while ((rangeMi8Change < rangeMi8) && (fuelWeightMi8Change[i] < fuelWeightMi8) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                  while (capacityEvacMi8 > 0){ // пока есть место в вертолёте
                                                      capacityEvacMi8 = capacityEvacMi8 - peopleAverage; // забираем людей
                                                      // куда летим теперь (проверяем следующий вылет)
                                                      if (capacityEvacMi8 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                  }
                                                  flightCountMi8++; // считаем вылет
                                                  for(int i = 1; i < squareWater3/10; ++i) {
                                                  fuelWeightMi8Change[i] = fuelWeightMi8Change[i-1] + fuelMi8Average; // затраченное топливо
                                                  }
                                                  costOneMi8 = costOneMi8 + fuelWeightMi8Change[i]*fuelCost;
                                                  rangeMi8Change = rangeMi8Change + sAverage; // затраченная дальность
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. расстояние от вертодрома слишком большое");
                                         QVector<double> costMi8(squareWater3/10);
                                           for(int i = 0; i < squareWater3/10; ++i) {
                                                   costMi8[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi8Change.begin(), fuelWeightMi8Change.end());
                                           qDebug() << "fuelWeightMi8Change" << fuelWeightMi8Change; //проверка
                                         costOneMi8 = costOneMi8 + rentMi8; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi8" << flightCountMi8;
                                         qDebug() << "costOneMi8" << costOneMi8;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi8 =  peopleCount/capacityMi8 * tMi8Average / flightCountMi8 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi8" << tOneMi8;
                                         QVector<int> helicoptersMi8Count(squareWater3/10);
                                           for(int i = 0; i < squareWater3/10; ++i) {
                                                   helicoptersMi8Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                             helicoptersMi8Count[i] = tOneMi8 / time[i];
                                             if (helicoptersMi8Count[i] <= 1) { // Условие 1: если helicoptersMi8Count меньше или равно 1
                                                     helicoptersMi8Count[i] = 1;
                                                     costMi8[i] = (costOneMi8* helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi8Count больше 1
                                                 helicoptersMi8Count[i]= helicoptersMi8Count[i] +1;
                                                 costMi8[i] = (costMi8[i] + costOneMi8 * helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi8Count.begin(), helicoptersMi8Count.end());
                                         std::reverse(costMi8.begin(), costMi8.end());
                                         qDebug() << "helicoptersMi8Count" << helicoptersMi8Count;
                                         qDebug() << "costMi8" << costMi8;

                                         // Ми-26
                                         double costOneMi26; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         int flightCountMi26 = 0; // количество вылетов для одного вертолёта
                                         double rangeMi26Change = 0; // расходуемая дальность
                                         QVector<int> fuelWeightMi26Change(squareWater3/10, 0);
                                         fuelWeightMi26Change[0] = 0;
                                         double tOneMi26 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacMi26 = capacityMi26 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                        if (startValue <= rangeMi26/2){ //сможет ли вертолёт вообще полететь и вернуться
                                         for(int i = 0; i < squareWater3/10; ++i) { //летим по точкам эвакуации
                                         if (tMi26[i] < timeWaterFlood[i] ){ //если Ми-26 прибудет раньше, чем вода затопит точку
                                                     while ((rangeMi26Change < rangeMi26) && (fuelWeightMi26Change[i] < fuelWeightMi26) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                         while (capacityEvacMi26 > 0){ // пока есть место в вертолёте
                                                             capacityEvacMi26 = capacityEvacMi26 - peopleAverage; // забираем людей
                                                             // куда летим теперь (проверяем следующий вылет)
                                                             if (capacityEvacMi26 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                         }
                                                         flightCountMi26++; // считаем вылет
                                                         for(int i = 1; i < squareWater3/10; ++i) {
                                                         fuelWeightMi26Change[i] = fuelWeightMi26Change[i-1] + fuelMi26Average; // затраченное топливо
                                                         }
                                                         costOneMi26 = costOneMi26 + fuelWeightMi26Change[i]*fuelCost;
                                                         rangeMi26Change = rangeMi26Change + sAverage; // затраченная дальность
                                                     }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                        }
                               }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. расстояние от вертодрома слишком большое");
                                        QVector<double> costMi26(squareWater3/10);
                                          for(int i = 0; i < squareWater3/10; ++i) {
                                                  costMi26[i] = 0;
                                              }
                                          std::reverse(fuelWeightMi26Change.begin(), fuelWeightMi26Change.end());
                                          qDebug() << "fuelWeightMi26Change" << fuelWeightMi26Change; //проверка
                                        costOneMi26 = costOneMi26 + rentMi26; // затраты с 1 шт
                                        // проверка
                                        qDebug() << "flightCountMi26" << flightCountMi26;
                                        qDebug() << "costOneMi26" << costOneMi26;
                                        peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                        // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                        tOneMi26 =  peopleCount/capacityMi26 * tMi26Average / flightCountMi26 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                        qDebug() << "tOneMi26" << tOneMi26;
                                        QVector<int> helicoptersMi26Count(squareWater3/10);
                                          for(int i = 0; i < squareWater3/10; ++i) {
                                                  helicoptersMi26Count[i] = 0;
                                              }
                                        for(int i = 0; i < squareWater3/10; ++i) {
                                            helicoptersMi26Count[i] = tOneMi26 / time[i];
                                            if (helicoptersMi26Count[i] <= 1) { // Условие 1: если helicoptersMi26Count меньше или равно 1
                                                    helicoptersMi26Count[i] = 1;
                                                    costMi26[i] = (costOneMi26* helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000;
                                            }
                                            else { // Условие 2: если helicoptersMi26Count больше 1
                                                helicoptersMi26Count[i]= helicoptersMi26Count[i] +1;
                                                costMi26[i] = (costMi26[i] + costOneMi26 * helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                             }
                                        }
                                        std::reverse(helicoptersMi26Count.begin(), helicoptersMi26Count.end());
                                        std::reverse(costMi26.begin(), costMi26.end());
                                        qDebug() << "helicoptersMi26Count" << helicoptersMi26Count;
                                        qDebug() << "costMi26" << costMi26;

                                       // Sikorsky S-61
                                         double costOneSikorskyS61; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                        // int helicoptersSikorskyS61Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                         int flightCountSikorskyS61=0;
                                         QVector<int> fuelWeightSikorskyS61Change(squareWater3/10, 0);
                                         fuelWeightSikorskyS61Change[0] = 0;
                                         double rangeSikorskyS61Change = 0; // расходуемая дальность
                                         double costFirstSikorskyS61 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                         double tOneSikorskyS61 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacSikorskyS61 = capacitySikorskyS61; // не добавляю спасательный плот для одного вертолёта
                                         //вертолёт сбрасывает ровно столько спасательных плотов, чтоб хватило всем
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                                 while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61)) {
                                                     rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                     for(int i = 1; i < squareWater3/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                             }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                         costFirstSikorskyS61 = costFirstSikorskyS61 + rentSikorskyS61; // затраты на первый вертолёт
                                         capacityEvacSikorskyS61 = capacitySikorskyS61+resqueRaft;
                                         // обновляем дальность и топливо, чтоб считать второй вертолёт
                                         rangeSikorskyS61Change = 0; // расходуемая дальность
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                         fuelWeightSikorskyS61Change[i] = 0;
                                         }
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeSikorskyS61/2){ //сможет ли вертолёт вообще полететь и вернуться
                                        for(int i = 0; i < squareWater3/10; ++i) { //летим по точкам эвакуации
                                       // if (tSikorskyS61[i] < timeWaterFlood[i] ){ //если Sikorsky S-61 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                    while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                        while (capacityEvacSikorskyS61 > 0){ // пока есть место в вертолёте
                                                            capacityEvacSikorskyS61 = capacityEvacSikorskyS61 - peopleAverage; // забираем людей
                                                            // куда летим теперь (проверяем следующий вылет)
                                                            if (capacityEvacSikorskyS61 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                        }
                                                        flightCountSikorskyS61++; // считаем вылет
                                                        for(int i = 1; i < squareWater3/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                                        rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                    }
                                        }
                              }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. расстояние от вертодрома слишком большое");
                              }}
                                       QVector<double> costSikorskyS61(squareWater3/10);
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                                 costSikorskyS61[i] = 0;
                                             }
                                         std::reverse(fuelWeightSikorskyS61Change.begin(), fuelWeightSikorskyS61Change.end());
                                         qDebug() << "fuelWeightSikorskyS61Change" << fuelWeightSikorskyS61Change; //проверка
                                       costOneSikorskyS61 = costOneSikorskyS61 + rentSikorskyS61; // затраты с 1 шт
                                       // проверка
                                       qDebug() << "flightCountSikorskyS61" << flightCountSikorskyS61;
                                       qDebug() << "costOneSikorskyS61" << costOneSikorskyS61;
                                       peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                       // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                       tOneSikorskyS61 =  peopleCount/capacitySikorskyS61 * tSikorskyS61Average / flightCountSikorskyS61 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                       qDebug() << "tOneSikorskyS61" << tOneSikorskyS61;
                                       QVector<int> helicoptersSikorskyS61Count(squareWater3/10);
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                                 helicoptersSikorskyS61Count[i] = 0;
                                             }
                                       for(int i = 0; i < squareWater3/10; ++i) {
                                           helicoptersSikorskyS61Count[i] = tOneSikorskyS61 / time[i];
                                           if (helicoptersSikorskyS61Count[i] <= 1) { // Условие 1: если helicoptersSikorskyS61Count меньше или равно 1
                                                   helicoptersSikorskyS61Count[i] = 1;
                                                   costSikorskyS61[i] = (costOneSikorskyS61* helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000;
                                           }
                                           else { // Условие 2: если helicoptersSikorskyS61Count больше 1
                                               helicoptersSikorskyS61Count[i]= helicoptersSikorskyS61Count[i] +1;
                                               costSikorskyS61[i] = (costSikorskyS61[i] + costOneSikorskyS61 * helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                            }
                                       }
                                       std::reverse(helicoptersSikorskyS61Count.begin(), helicoptersSikorskyS61Count.end());
                                       std::reverse(costSikorskyS61.begin(), costSikorskyS61.end());
                                       qDebug() << "helicoptersSikorskyS61Count" << helicoptersSikorskyS61Count;
                                       qDebug() << "costSikorskyS61" << costSikorskyS61;

                                        // Ми-14
                                          double costOneMi14; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         // int helicoptersMi14Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                          int flightCountMi14=0;
                                          QVector<int> fuelWeightMi14Change(squareWater3/10, 0);
                                          fuelWeightMi14Change[0] = 0;
                                          double rangeMi14Change = 0; // расходуемая дальность
                                          double costFirstMi14 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                          double tOneMi14 = 0; // количество часов на выполнение операции для одного вертолёта
                                          double capacityEvacMi14 = capacityMi14; // не добавляю спасательный плот для одного вертолёта
                                          for(int i = 0; i < squareWater3/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                                  while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14)) {
                                                      rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      for(int i = 1; i < squareWater3/10; ++i) {
                                                         fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                         }
                                                         costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                          }
                                          costFirstMi14 = costFirstMi14 + rentMi14; // затраты на первый вертолёт
                                          capacityEvacMi14 = capacityMi14+resqueRaft;
                                          // обновляем дальность и топливо, чтоб считать второй вертолёт
                                          rangeMi14Change = 0; // расходуемая дальность
                                          for(int i = 0; i < squareWater3/10; ++i) {
                                          fuelWeightMi14Change[i] = 0;
                                          }
                                          for(int i = 0; i < squareWater3/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeMi14/2){ //сможет ли вертолёт вообще полететь и вернуться
                                          for(int i = 0; i < squareWater3/10; ++i) { //летим по точкам эвакуации
                                         // if (tMi14[i] < timeWaterFlood[i] ){ //если Ми-14 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                      while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                          while (capacityEvacMi14 > 0){ // пока есть место в вертолёте
                                                              capacityEvacMi14 = capacityEvacMi14 - peopleAverage; // забираем людей
                                                              // куда летим теперь (проверяем следующий вылет)
                                                              if (capacityEvacMi14 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                          }
                                                          flightCountMi14++; // считаем вылет
                                                          for(int i = 1; i < squareWater3/10; ++i) {
                                                          fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                          }
                                                          costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                                          rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      }
                                          }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. расстояние от вертодрома слишком большое");
                                              }}
                                          QVector<double> costMi14(squareWater3/10);
                                           for(int i = 0; i < squareWater3/10; ++i) {
                                                   costMi14[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi14Change.begin(), fuelWeightMi14Change.end());
                                           qDebug() << "fuelWeightMi14Change" << fuelWeightMi14Change; //проверка
                                         costOneMi14 = costOneMi14 + rentMi14; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi14" << flightCountMi14;
                                         qDebug() << "costOneMi14" << costOneMi14;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi14 =  peopleCount/capacityMi14 * tMi14Average / flightCountMi14 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi14" << tOneMi14;
                                         QVector<int> helicoptersMi14Count(squareWater3/10);
                                           for(int i = 0; i < squareWater3/10; ++i) {
                                                   helicoptersMi14Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater3/10; ++i) {
                                             helicoptersMi14Count[i] = tOneMi14 / time[i];
                                             if (helicoptersMi14Count[i] <= 1) { // Условие 1: если helicoptersMi14Count меньше или равно 1
                                                     helicoptersMi14Count[i] = 1;
                                                     costMi14[i] = (costOneMi14* helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi14Count больше 1
                                                 helicoptersMi14Count[i]= helicoptersMi14Count[i] +1;
                                                 costMi14[i] = (costMi14[i] + costOneMi14 * helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi14Count.begin(), helicoptersMi14Count.end());
                                         std::reverse(costMi14.begin(), costMi14.end());
                                         qDebug() << "helicoptersMi14Count" << helicoptersMi14Count;
                                         qDebug() << "costMi14" << costMi14;


                                         // ОПРЕДЕЛЕНИЕ РАЦИОНАЛЬНОГО ВК ПО ЗАТРАТАМ
                                         // Настройки карты стоимостей
                                        QMap<QString, double>
                                                costAllFlood = {
                                            {"Mi8", costMi8[squareWater3/10-1]},
                                            {"Mi26", costMi26[squareWater3/10-1]},
                                            {"SikorskyS61", costSikorskyS61[squareWater3/10-1]},
                                            {"Mi14", costMi14[squareWater3/10-1]}
                                        };

                                        // Карта счетчиков вертолетов
                                        QMap<QString, int> helicoptersCountMap = {
                                            {"Mi8", helicoptersMi8Count[squareWater3/10-1]},
                                            {"Mi26", helicoptersMi26Count[squareWater3/10-1]},
                                            {"SikorskyS61", helicoptersSikorskyS61Count[squareWater3/10-1]},
                                            {"Mi14", helicoptersMi14Count[squareWater3/10-1]}
                                        };

                                        // Карта счетчиков полётов вертолетов - чтобы если вертолёт не подошёл, его затраты не учитывались, т.к. они равны 0
                                        QMap<QString, int> helicoptersFlightCountMap = {
                                            {"Mi8", flightCountMi8},
                                            {"Mi26", flightCountMi26},
                                            {"SikorskyS61", flightCountSikorskyS61},
                                            {"Mi14", flightCountMi14}
                                        };

                                        QString minCostFlood;
                                        double minCostFloodValue = std::numeric_limits<double>::max();

                                        for (auto it = costAllFlood.constBegin(); it != costAllFlood.constEnd(); ++it) {
                                            // Проверяем, что количество полетов для этого типа вертолетов больше нуля
                                            if (it.value() < minCostFloodValue && helicoptersFlightCountMap[it.key()] > 0) {
                                                minCostFloodValue = it.value();
                                                minCostFlood = it.key();
                                            }
                                        }
                                        // Проверяем, было ли найдено соответствующее минимальное значение
                                        if (minCostFloodValue == std::numeric_limits<double>::max()) {
                                          QMessageBox::critical(nullptr, "Ошибка", "Ни один вертолёт не подошёл");
                                          minCostFloodValue = 0;
                                        }

                                        // Запрос количества вертолетов для найденной минимальной стоимости
                                        int countHelicoptersFloodMin = helicoptersCountMap[minCostFlood];

                                        qDebug() << "Рациональный ВК: " << minCostFlood //ПРОРЫВ ПЛОТИНЫ
                                                 << "; Количество ВК: " << countHelicoptersFloodMin
                                                 << "; Затраты: " << minCostFloodValue;

                                          QString floodType="Прорыв плотины"; // для вывода в результаты

                                          // Сохранить в таблице БД результаты
                                          saveResultsToDb(floodType, peopleCount, startValue, minCostFlood, countHelicoptersFloodMin, minCostFloodValue);

                                        // ГРАФИКИ DAM FLOOD
                                          //если пострадаших нет, расстояние 0 км, + расстояние до вертодрома меньше дальности вертолёта то ничего не выводим
                                          if ((peopleCount != 0) && (startValue != 0) && (timeLimit != 0)){
                                          QChart *chart = new QChart();
                                          chart->update();
                                          chart->legend()->hide();

                                          QLineSeries *seriesMi8 = new QLineSeries();
                                          if (flightCountMi8>0){
                                        seriesMi8->setName("Ми-8");
                                        for (int i = 0; i < time.size(); ++i) {
                                                seriesMi8->append(time[i], costMi8[i]);
                                            }
                                        chart->addSeries(seriesMi8);
                                        }
                                            QLineSeries *seriesMi26 = new QLineSeries();

                                            if (flightCountMi26>0){
                                                seriesMi26->setName("Ми-26");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi26->append(time[i], costMi26[i]);
                                            }
                                            chart->addSeries(seriesMi26);
                                             }

                                            QLineSeries *seriesSikorskyS61 = new QLineSeries();
                                            if (flightCountSikorskyS61>0){
                                            seriesSikorskyS61->setName("Sikorsky S-61");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesSikorskyS61->append(time[i], costSikorskyS61[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesSikorskyS61);
                                            }

                                            QLineSeries *seriesMi14 = new QLineSeries();
                                            if (flightCountMi14>0){
                                            seriesMi14->setName("Ми-14");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi14->append(time[i], costMi14[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesMi14);
                                            }

                                            chart->createDefaultAxes();
                                            chart->setTitle("Затраты (млн.руб.) всех ВК в зависимости от времени (ч)");
                                            chart->legend()->setVisible(true);


                                            QValueAxis *auxAxisX = new QValueAxis;
                                            auxAxisX->setRange(1, int(squareWater3/10)); // Установите диапазон значений оси
                                            auxAxisX->setTickCount(int(squareWater3/10)); // Количество делений оси
                                            auxAxisX->setLinePenColor(Qt::gray); // Цвет линии оси
                                            QFont labelsFont;
                                            labelsFont.setPointSize(3.5); // Устанавливаем меньший размер шрифта
                                            QBrush labelsBrush(Qt::black);
                                            labelsFont.setBold(true);
                                            auxAxisX->setLabelsFont(labelsFont); // Применяем шрифт к меткам
                                            auxAxisX->setLabelFormat("%d");
                                            chart->addAxis(auxAxisX, Qt::AlignTop);

                                            QChartView *chartView = new QChartView(chart);
                                            chartView->setRenderHint(QPainter::Antialiasing);
                                            // Устанавливаем QChartView как центральный виджет в windowGraph
                                            QVBoxLayout *layout = new QVBoxLayout;
                                            layout->addWidget(chartView);

                                       //ВЫВОД РЕЗУЛЬТАТА В ОТДЕЛЬНОМ ОКНЕ
                                       // dataWindow->setAttribute(Qt::WA_DeleteOnClose); // Окно будет удаляться при закрытии
                                        dataWindow->setWindowTitle("Результат");
                                        dataWindow->resize(200, 400);

                                        QString variableName0 = "Тип наводнения";
                                        QString variableName1 = "Количество пострадавших";
                                        QString variableName2 = "Рациональный ВК";
                                        QString variableName3 = "Расстояние от вертодрома";
                                        QString variableName6 = "Ограничение на время";
                                        QString variableName4 = "Количество ВК";
                                        QString variableName5 = "Затраты";

                                        // Создаем таблицу для отображения данных
                                        QTableWidget *tableWidget = new QTableWidget(dataWindow); // Создан с 0 строк и 0 столбцов
                                        tableWidget->clearContents();
                                        tableWidget->setRowCount(0);
                                        tableWidget->setRowCount(7); // Устанавливаем 6 строк
                                        tableWidget->setColumnCount(2);
                                        // tableWidget->setHorizontalHeaderLabels(QStringList() << "Параметр" << "Значение");
                                        tableWidget->verticalHeader()->setVisible(false);   // Скрывает нумерацию строк
                                        tableWidget->horizontalHeader()->setVisible(false); // Скрывает названия столбцов
                                        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
                                        tableWidget->setSelectionMode(QAbstractItemView::NoSelection); // Запрет выделения

                                        tableWidget->setStyleSheet("QTableView { border: 1px solid black; }"
                                                                   "QTableView::item { border: 1px solid black; }"
                                                                   "QHeaderView::section { border: 1px solid black; }");
                                        // Заполняем строки
                                        tableWidget->setItem(0, 0, new QTableWidgetItem(variableName0));
                                        tableWidget->setItem(0, 1, new QTableWidgetItem(floodType));

                                        tableWidget->setItem(1, 0, new QTableWidgetItem(variableName1));
                                        tableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(peopleCount) + " человек"));

                                        tableWidget->setItem(2, 0, new QTableWidgetItem(variableName2));
                                        tableWidget->setItem(2, 1, new QTableWidgetItem(minCostFlood));

                                        tableWidget->setItem(3, 0, new QTableWidgetItem(variableName3));
                                        tableWidget->setItem(3, 1, new QTableWidgetItem(QString::number(startValue) + " км"));

                                        tableWidget->setItem(4, 0, new QTableWidgetItem(variableName6));
                                        tableWidget->setItem(4, 1, new QTableWidgetItem(QString::number(timeLimit) + " ч"));

                                        tableWidget->setItem(5, 0, new QTableWidgetItem(variableName4));
                                        tableWidget->setItem(5, 1, new QTableWidgetItem(QString::number(countHelicoptersFloodMin)));

                                        tableWidget->setItem(6, 0, new QTableWidgetItem(variableName5));
                                        tableWidget->setItem(6, 1, new QTableWidgetItem(QString::number(minCostFloodValue) + " млн.руб."));

                                        tableWidget->resizeColumnsToContents(); // Автоматическая настройка ширины колонок
                                        tableWidget->horizontalHeader()->setStretchLastSection(true); // Последний столбец растягивается

                                        // Устанавливаем layout для отображения только что созданной таблицы
                                        dataWindow->setLayout(new QVBoxLayout);
                                        dataWindow->layout()->addWidget(tableWidget);

                                        dataWindow->setMinimumSize(450, 287);
                                        dataWindow->setMaximumSize(450, 287);
                                        dataWindow->show();

                                        graphWidget->setLayout(layout);
                                        graphWidget->show(); // показать график
                                        map->show(); // Вывод рельефа в дополнительном окне
                                        map->spreadWater3(); // распространение воды по рельефу
                                          } else QMessageBox::critical(nullptr, "Ошибка", "Поле для ввода пустое");
                                        break;}

//ЦУНАМИ***********************************************************************************************************************************
                              {case 4: map->generateTerrain4();
                                      int peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                      //Расстояние между точками
                                      int startValue = lineEdit2->text().toInt(); // Первое значение из lineEdit
                                      double timeLimit = lineEdit3->text().toDouble(); // Первое значение из lineEdit

                                      QVector<double> time(squareWater4/10); // Создаем массив на 30 элементов - количество точек
                                      double step = timeLimit / (squareWater4/10); //9 / 30 = 0,3ч
                                      time[0] = step;
                                      for(int i = 1; i < squareWater4 / 10; ++i) {
                                      time[i] = time [i-1] + step;
                                      }
                                      for (int i = 0; i < squareWater4 / 10; ++i) {
                                          qDebug() << "time" << i << "=" << time[i];
                                      }

                                          double sSum = 0; // Для хранения суммы всех расстояний
                                          QVector<int> s(squareWater4/10); // Создаем массив на 30 элементов - количество точек
                                          //заполняем массив с расстояниями
                                            s[0] = startValue;
                                            for(int i = 1; i < squareWater4 / 10; ++i) {
                                                    s[i] = s[i - 1] + 10;
                                            }
                                            //сумма всех расстояний
                                            for (int i = 0; i < squareWater4 / 10; ++i) {
                                                    sSum += s[i]; // Суммируем все расстояния
                                            }
                                          // среднее арифметическое расстояний
                                          double sAverage = sSum / (squareWater4 / 10);
                                          // проверка ср ариф
                                          qDebug() << "sAverage" << sAverage;
                                          // проверка массива
                                          for (int i = 0; i < squareWater4/10; ++i) {
                                              qDebug() << "s" << i << "=" << s[i];
                                          }

                                         //массив времени воды до точек
                                          QVector<double> timeWaterFlood(squareWater4/10);
                                          for(int i = 0; i < squareWater4/10; ++i) {
                                                    timeWaterFlood[i] = s[i] / speedWaterTsunamiFlood; // время в часах
                                                }
                                          for (int i = 0; i < squareWater4/10; ++i) {
                                              qDebug() << "timeWaterFlood" << i << "=" << timeWaterFlood[i];
                                          }

                                          // Время между точками для всех ВК
                                          // Время для облета точек для Ми-8
                                            double tMi8Sum = 0; // Для хранения суммы всех расстояний
                                            QVector<double> tMi8(squareWater4/10);
                                              for(int i = 0; i < squareWater4/10; ++i) {
                                                      tMi8[i] = s[i] / speedMi8; // время в часах
                                                  }
                                              //сумма всех расстояний
                                              for (int i = 0; i < squareWater4 / 10; ++i) {
                                                      tMi8Sum += tMi8[i]; // Суммируем всё время
                                              }
                                            // среднее арифметическое расстояний
                                            double tMi8Average = tMi8Sum / (squareWater4 / 10);
                                            // проверка ср ариф
                                            qDebug() << "tMi8Average" << tMi8Average;
                                            // проверка массива
                                            for (int i = 0; i < squareWater4/10; ++i) {
                                                qDebug() << "tMi8" << i << "=" << tMi8[i];
                                            }

                                            // Время для облета точек для Ми-26
                                              double tMi26Sum = 0; // Для хранения суммы всех расстояний
                                              QVector<double> tMi26(squareWater4/10);
                                                for(int i = 0; i < squareWater4/10; ++i) {
                                                        tMi26[i] = s[i] / speedMi26; // время в часах
                                                    }
                                                //сумма всех расстояний
                                                for (int i = 0; i < squareWater4 / 10; ++i) {
                                                        tMi26Sum += tMi26[i]; // Суммируем всё время
                                                }
                                              // среднее арифметическое расстояний
                                              double tMi26Average = tMi26Sum / (squareWater4 / 10);
                                              // проверка ср ариф
                                              qDebug() << "tMi26Average" << tMi26Average;
                                              // проверка массива
                                              for (int i = 0; i < squareWater4/10; ++i) {
                                                  qDebug() << "tMi26" << i << "=" << tMi26[i];
                                              }

                                              // Время для облета точек для Sikorsky S-61
                                                double tSikorskyS61Sum = 0; // Для хранения суммы всех расстояний
                                                QVector<double> tSikorskyS61(squareWater4/10);
                                                  for(int i = 0; i < squareWater4/10; ++i) {
                                                          tSikorskyS61[i] = s[i] / speedSikorskyS61; // время в часах
                                                      }
                                                  //сумма всех расстояний
                                                  for (int i = 0; i < squareWater4 / 10; ++i) {
                                                          tSikorskyS61Sum += tSikorskyS61[i]; // Суммируем всё время
                                                  }
                                                // среднее арифметическое расстояний
                                                double tSikorskyS61Average = tSikorskyS61Sum / (squareWater4 / 10);
                                                // проверка ср ариф
                                                qDebug() << "tSikorskyS61Average" << tSikorskyS61Average;
                                                // проверка массива
                                                for (int i = 0; i < squareWater4/10; ++i) {
                                                    qDebug() << "tSikorskyS61" << i << "=" << tSikorskyS61[i];
                                                }

                                                // Время для облета точек для Ми-14
                                                  double tMi14Sum = 0; // Для хранения суммы всех расстояний
                                                  QVector<double> tMi14(squareWater4/10);
                                                    for(int i = 0; i < squareWater4/10; ++i) {
                                                            tMi14[i] = s[i] / speedMi14; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater4 / 10; ++i) {
                                                            tMi14Sum += tMi14[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double tMi14Average = tMi14Sum / (squareWater4 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "tMi14Average" << tMi14Average;
                                                  // проверка массива
                                                  for (int i = 0; i < squareWater4/10; ++i) {
                                                      qDebug() << "tMi14" << i << "=" << tMi14[i];
                                                  }

                                                  // Расход топлива * время Ми-8
                                                  double fuelMi8Sum = 0; // Для хранения суммы всего топлива
                                                  QVector<double> fuelMi8(squareWater4/10);
                                                    for(int i = 0; i < squareWater4/10; ++i) {
                                                            fuelMi8[i] = fuelConsumptionMi8 * tMi8[i]; // время в часах
                                                        }
                                                    //сумма всех расстояний
                                                    for (int i = 0; i < squareWater4 / 10; ++i) {
                                                            fuelMi8Sum += fuelMi8[i]; // Суммируем всё время
                                                    }
                                                  // среднее арифметическое расстояний
                                                  double fuelMi8Average = fuelMi8Sum / (squareWater4 / 10);
                                                  // проверка ср ариф
                                                  qDebug() << "fuelMi8Average" << fuelMi8Average;

                                                    //проверка массива
                                                    for (int i = 0; i < squareWater4/10; ++i) {
                                                        qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                    }

                                                    // Расход топлива * время Ми-8
                                                    double fuelMi26Sum = 0; // Для хранения суммы всего топлива
                                                    QVector<double> fuelMi26(squareWater4/10);
                                                      for(int i = 0; i < squareWater4/10; ++i) {
                                                              fuelMi26[i] = fuelConsumptionMi26 * tMi26[i]; // время в часах
                                                          }
                                                      //сумма всех расстояний
                                                      for (int i = 0; i < squareWater4 / 10; ++i) {
                                                              fuelMi26Sum += fuelMi26[i]; // Суммируем всё время
                                                      }
                                                    // среднее арифметическое расстояний
                                                    double fuelMi26Average = fuelMi26Sum / (squareWater4 / 10);
                                                    // проверка ср ариф
                                                    qDebug() << "fuelMi26Average" << fuelMi26Average;
                                                      //проверка массива
                                                      for (int i = 0; i < squareWater4/10; ++i) {
                                                          qDebug() << "fuelMi8" << i << "=" << fuelMi8[i];
                                                      }

                                                    // Расход топлива * время Sikorsky S-61
                                                 double fuelSikorskyS61Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelSikorskyS61(squareWater4/10);
                                                   for(int i = 0; i < squareWater4/10; ++i) {
                                                           fuelSikorskyS61[i] = fuelConsumptionSikorskyS61 * tSikorskyS61[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater4 / 10; ++i) {
                                                           fuelSikorskyS61Sum += fuelSikorskyS61[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelSikorskyS61Average = fuelSikorskyS61Sum / (squareWater4 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelSikorskyS61Average" << fuelSikorskyS61Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater4/10; ++i) {
                                                       qDebug() << "fuelSikorskyS61" << i << "=" << fuelSikorskyS61[i];
                                                   }

                                                    // Расход топлива * время Ми-14
                                                 double fuelMi14Sum = 0; // Для хранения суммы всего топлива
                                                 QVector<double> fuelMi14(squareWater4/10);
                                                   for(int i = 0; i < squareWater4/10; ++i) {
                                                           fuelMi14[i] = fuelConsumptionMi14 * tMi14[i]; // время в часах
                                                       }

                                                   //сумма всех расстояний
                                                   for (int i = 0; i < squareWater4 / 10; ++i) {
                                                           fuelMi14Sum += fuelMi14[i]; // Суммируем всё время
                                                   }
                                                 // среднее арифметическое расстояний
                                                 double fuelMi14Average = fuelMi14Sum / (squareWater4 / 10);
                                                 // проверка ср ариф
                                                 qDebug() << "fuelMi14Average" << fuelMi14Average;

                                                   //проверка массива
                                                   for (int i = 0; i < squareWater4/10; ++i) {
                                                       qDebug() << "fuelMi14" << i << "=" << fuelMi14[i];
                                                   }

                                        // Распределение количества пострадавших по всем точкам (в каждой точке минимум 3 человека)
                                        QVector<int> people(squareWater4/10, 3);
                                        peopleCount -= 3 * squareWater4/10;
                                        // среднее арифметическое расстояний
                                        double peopleAverage = peopleCount / (squareWater4 / 10);
                                        // проверка ср ариф
                                        qDebug() << "peopleAverage" << peopleAverage;
                                        QRandomGenerator *peopleGenerator = QRandomGenerator::system(); // Получаем системный генератор случайных чисел
                                        while (peopleCount > 0) {
                                            int randomIndex = peopleGenerator->bounded(0, squareWater4/10);
                                            int randomValue = peopleGenerator->bounded(1, qMin(peopleCount, 100) + 1);
                                            people[randomIndex] += randomValue;
                                            peopleCount -= randomValue; // из общего кол-ва пострадавших убирать тех, кто уже зачислен в точку
                                        }
                                        // проверка массива
                                        for (int i = 0; i < squareWater4/10; ++i) {
                                            qDebug() << "people" << i << "=" << people[i];
                                        }

                                //АЛГОРИТМ ЭВАКУАЦИИ
                                // Ми-8
                                double costOneMi8; // затраты на операцию для 1 вертолёта
                                  int flightCountMi8 = 0; // количество вылетов для одного вертолёта
                                  double rangeMi8Change = 0; // расходуемая дальность
                                  QVector<int> fuelWeightMi8Change(squareWater4/10, 0);
                                  fuelWeightMi8Change[0] = 0;
                                  double tOneMi8 = 0; // количество часов на выполнение операции для одного вертолёта
                                  double capacityEvacMi8 = capacityMi8 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                 if (startValue <= rangeMi8/2){ //сможет ли вертолёт вообще полететь и вернуться
                                  for(int i = 0; i < squareWater4/10; ++i) { //летим по точкам эвакуации
                                  if (tMi8[i] < timeWaterFlood[i] ){ //если Ми-8 прибудет раньше, чем вода затопит точку
                                              while ((rangeMi8Change < rangeMi8) && (fuelWeightMi8Change[i] < fuelWeightMi8) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                  while (capacityEvacMi8 > 0){ // пока есть место в вертолёте
                                                      capacityEvacMi8 = capacityEvacMi8 - peopleAverage; // забираем людей
                                                      // куда летим теперь (проверяем следующий вылет)
                                                      if (capacityEvacMi8 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                      else if (capacityEvacMi8 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                          capacityEvacMi8 = capacityMi8; // высадили на вертодроме, места освободились
                                                          break;
                                                      }
                                                  }
                                                  flightCountMi8++; // считаем вылет
                                                  for(int i = 1; i < squareWater4/10; ++i) {
                                                  fuelWeightMi8Change[i] = fuelWeightMi8Change[i-1] + fuelMi8Average; // затраченное топливо
                                                  }
                                                  costOneMi8 = costOneMi8 + fuelWeightMi8Change[i]*fuelCost;
                                                  rangeMi8Change = rangeMi8Change + sAverage; // затраченная дальность
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-8 не подходит, т.к. расстояние от вертодрома слишком большое");
                                         QVector<double> costMi8(squareWater4/10);
                                           for(int i = 0; i < squareWater4/10; ++i) {
                                                   costMi8[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi8Change.begin(), fuelWeightMi8Change.end());
                                           qDebug() << "fuelWeightMi8Change" << fuelWeightMi8Change; //проверка
                                         costOneMi8 = costOneMi8 + rentMi8; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi8" << flightCountMi8;
                                         qDebug() << "costOneMi8" << costOneMi8;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi8 =  peopleCount/capacityMi8 * tMi8Average / flightCountMi8 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi8" << tOneMi8;
                                         QVector<int> helicoptersMi8Count(squareWater4/10);
                                           for(int i = 0; i < squareWater4/10; ++i) {
                                                   helicoptersMi8Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                             helicoptersMi8Count[i] = tOneMi8 / time[i];
                                             if (helicoptersMi8Count[i] <= 1) { // Условие 1: если helicoptersMi8Count меньше или равно 1
                                                     helicoptersMi8Count[i] = 1;
                                                     costMi8[i] = (costOneMi8* helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi8Count больше 1
                                                 helicoptersMi8Count[i]= helicoptersMi8Count[i] +1;
                                                 costMi8[i] = (costMi8[i] + costOneMi8 * helicoptersMi8Count[i]+fuelWeightMi8Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi8Count.begin(), helicoptersMi8Count.end());
                                         std::reverse(costMi8.begin(), costMi8.end());
                                         qDebug() << "helicoptersMi8Count" << helicoptersMi8Count;
                                         qDebug() << "costMi8" << costMi8;

                                         // Ми-26
                                         double costOneMi26; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         int flightCountMi26 = 0; // количество вылетов для одного вертолёта
                                         double rangeMi26Change = 0; // расходуемая дальность
                                         QVector<int> fuelWeightMi26Change(squareWater4/10, 0);
                                         fuelWeightMi26Change[0] = 0;
                                         double tOneMi26 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacMi26 = capacityMi26 + resqueRaft; // добавили спасательный плот для одного вертолёта
                                        if (startValue <= rangeMi26/2){ //сможет ли вертолёт вообще полететь и вернуться
                                         for(int i = 0; i < squareWater4/10; ++i) { //летим по точкам эвакуации
                                         if (tMi26[i] < timeWaterFlood[i] ){ //если Ми-26 прибудет раньше, чем вода затопит точку
                                                     while ((rangeMi26Change < rangeMi26) && (fuelWeightMi26Change[i] < fuelWeightMi26) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                         while (capacityEvacMi26 > 0){ // пока есть место в вертолёте
                                                             capacityEvacMi26 = capacityEvacMi26 - peopleAverage; // забираем людей
                                                             // куда летим теперь (проверяем следующий вылет)
                                                             if (capacityEvacMi26 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                             else if (capacityEvacMi26 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                 capacityEvacMi26 = capacityMi26; // высадили на вертодроме, места освободились
                                                                 break;
                                                             }
                                                         }
                                                         flightCountMi26++; // считаем вылет
                                                         for(int i = 1; i < squareWater4/10; ++i) {
                                                         fuelWeightMi26Change[i] = fuelWeightMi26Change[i-1] + fuelMi26Average; // затраченное топливо
                                                         }
                                                         costOneMi26 = costOneMi26 + fuelWeightMi26Change[i]*fuelCost;
                                                         rangeMi26Change = rangeMi26Change + sAverage; // затраченная дальность
                                                     }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                        }
                               }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-26 не подходит, т.к. расстояние от вертодрома слишком большое");
                                        QVector<double> costMi26(squareWater4/10);
                                          for(int i = 0; i < squareWater4/10; ++i) {
                                                  costMi26[i] = 0;
                                              }
                                          std::reverse(fuelWeightMi26Change.begin(), fuelWeightMi26Change.end());
                                          qDebug() << "fuelWeightMi26Change" << fuelWeightMi26Change; //проверка
                                        costOneMi26 = costOneMi26 + rentMi26; // затраты с 1 шт
                                        // проверка
                                        qDebug() << "flightCountMi26" << flightCountMi26;
                                        qDebug() << "costOneMi26" << costOneMi26;
                                        peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                        // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                        tOneMi26 =  peopleCount/capacityMi26 * tMi26Average / flightCountMi26 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                        qDebug() << "tOneMi26" << tOneMi26;
                                        QVector<int> helicoptersMi26Count(squareWater4/10);
                                          for(int i = 0; i < squareWater4/10; ++i) {
                                                  helicoptersMi26Count[i] = 0;
                                              }
                                        for(int i = 0; i < squareWater4/10; ++i) {
                                            helicoptersMi26Count[i] = tOneMi26 / time[i];
                                            if (helicoptersMi26Count[i] <= 1) { // Условие 1: если helicoptersMi26Count меньше или равно 1
                                                    helicoptersMi26Count[i] = 1;
                                                    costMi26[i] = (costOneMi26* helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000;
                                            }
                                            else { // Условие 2: если helicoptersMi26Count больше 1
                                                helicoptersMi26Count[i]= helicoptersMi26Count[i] +1;
                                                costMi26[i] = (costMi26[i] + costOneMi26 * helicoptersMi26Count[i]+fuelWeightMi26Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                             }
                                        }
                                        std::reverse(helicoptersMi26Count.begin(), helicoptersMi26Count.end());
                                        std::reverse(costMi26.begin(), costMi26.end());
                                        qDebug() << "helicoptersMi26Count" << helicoptersMi26Count;
                                        qDebug() << "costMi26" << costMi26;

                                       // Sikorsky S-61
                                         double costOneSikorskyS61; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                        // int helicoptersSikorskyS61Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                         int flightCountSikorskyS61=0;
                                         QVector<int> fuelWeightSikorskyS61Change(squareWater4/10, 0);
                                         fuelWeightSikorskyS61Change[0] = 0;
                                         double rangeSikorskyS61Change = 0; // расходуемая дальность
                                         double costFirstSikorskyS61 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                         double tOneSikorskyS61 = 0; // количество часов на выполнение операции для одного вертолёта
                                         double capacityEvacSikorskyS61 = capacitySikorskyS61; // не добавляю спасательный плот для одного вертолёта
                                         //вертолёт сбрасывает ровно столько спасательных плотов, чтоб хватило всем
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                                 while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61)) {
                                                     rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                     for(int i = 1; i < squareWater4/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                             }
                                         } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                         }
                                         costFirstSikorskyS61 = costFirstSikorskyS61 + rentSikorskyS61; // затраты на первый вертолёт
                                         capacityEvacSikorskyS61 = capacitySikorskyS61+resqueRaft;
                                         // обновляем дальность и топливо, чтоб считать второй вертолёт
                                         rangeSikorskyS61Change = 0; // расходуемая дальность
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                         fuelWeightSikorskyS61Change[i] = 0;
                                         }
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                             if (tSikorskyS61[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeSikorskyS61/2){ //сможет ли вертолёт вообще полететь и вернуться
                                        for(int i = 0; i < squareWater4/10; ++i) { //летим по точкам эвакуации
                                       // if (tSikorskyS61[i] < timeWaterFlood[i] ){ //если Sikorsky S-61 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                    while ((rangeSikorskyS61Change < rangeSikorskyS61) && (fuelWeightSikorskyS61Change[i] < fuelWeightSikorskyS61) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                        while (capacityEvacSikorskyS61 > 0){ // пока есть место в вертолёте
                                                            capacityEvacSikorskyS61 = capacityEvacSikorskyS61 - peopleAverage; // забираем людей
                                                            // куда летим теперь (проверяем следующий вылет)
                                                            if (capacityEvacSikorskyS61 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                            else if (capacityEvacSikorskyS61 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                capacityEvacSikorskyS61 = capacitySikorskyS61; // высадили на вертодроме, места освободились
                                                                break;
                                                            }
                                                        }
                                                        flightCountSikorskyS61++; // считаем вылет
                                                        for(int i = 1; i < squareWater4/10; ++i) {
                                                        fuelWeightSikorskyS61Change[i] = fuelWeightSikorskyS61Change[i-1] + fuelSikorskyS61Average; // затраченное топливо
                                                        }
                                                        costOneSikorskyS61 = costOneSikorskyS61 + fuelWeightSikorskyS61Change[i]*fuelCost;
                                                        rangeSikorskyS61Change = rangeSikorskyS61Change + sAverage; // затраченная дальность
                                                    }
                                        }
                              }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Sikorsky S-61 не подходит, т.к. расстояние от вертодрома слишком большое");
                                      }}
                                         QVector<double> costSikorskyS61(squareWater4/10);
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                                 costSikorskyS61[i] = 0;
                                             }
                                         std::reverse(fuelWeightSikorskyS61Change.begin(), fuelWeightSikorskyS61Change.end());
                                         qDebug() << "fuelWeightSikorskyS61Change" << fuelWeightSikorskyS61Change; //проверка
                                       costOneSikorskyS61 = costOneSikorskyS61 + rentSikorskyS61; // затраты с 1 шт
                                       // проверка
                                       qDebug() << "flightCountSikorskyS61" << flightCountSikorskyS61;
                                       qDebug() << "costOneSikorskyS61" << costOneSikorskyS61;
                                       peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                       // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                       tOneSikorskyS61 =  peopleCount/capacitySikorskyS61 * tSikorskyS61Average / flightCountSikorskyS61 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                       qDebug() << "tOneSikorskyS61" << tOneSikorskyS61;
                                       QVector<int> helicoptersSikorskyS61Count(squareWater4/10);
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                                 helicoptersSikorskyS61Count[i] = 0;
                                             }
                                       for(int i = 0; i < squareWater4/10; ++i) {
                                           helicoptersSikorskyS61Count[i] = tOneSikorskyS61 / time[i];
                                           if (helicoptersSikorskyS61Count[i] <= 1) { // Условие 1: если helicoptersSikorskyS61Count меньше или равно 1
                                                   helicoptersSikorskyS61Count[i] = 1;
                                                   costSikorskyS61[i] = (costOneSikorskyS61* helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000;
                                           }
                                           else { // Условие 2: если helicoptersSikorskyS61Count больше 1
                                               helicoptersSikorskyS61Count[i]= helicoptersSikorskyS61Count[i] +1;
                                               costSikorskyS61[i] = (costSikorskyS61[i] + costOneSikorskyS61 * helicoptersSikorskyS61Count[i]+fuelWeightSikorskyS61Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                            }
                                       }
                                       std::reverse(helicoptersSikorskyS61Count.begin(), helicoptersSikorskyS61Count.end());
                                       std::reverse(costSikorskyS61.begin(), costSikorskyS61.end());
                                       qDebug() << "helicoptersSikorskyS61Count" << helicoptersSikorskyS61Count;
                                       qDebug() << "costSikorskyS61" << costSikorskyS61;

                                        // Ми-14
                                          double costOneMi14; // затраты на операцию для 1 вертолёта (потом добавить стоимость аренды 1 вертолёта, потом умножить на кол-во вертолётов)
                                         // int helicoptersMi14Count = 1; // количество вертолётов для выполнения операции (уже 1 т.к. первый вертолёт развозит спассредства)
                                          int flightCountMi14=0;
                                          QVector<int> fuelWeightMi14Change(squareWater4/10, 0);
                                          fuelWeightMi14Change[0] = 0;
                                          double rangeMi14Change = 0; // расходуемая дальность
                                          double costFirstMi14 = 0; // стоимость первого вертолёта, который сбрасывает плоты (допущение: плотов столько, чтоб хватило на всех людей)
                                          double tOneMi14 = 0; // количество часов на выполнение операции для одного вертолёта
                                          double capacityEvacMi14 = capacityMi14; // не добавляю спасательный плот для одного вертолёта
                                          for(int i = 0; i < squareWater4/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                                  while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14)) {
                                                      rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      for(int i = 1; i < squareWater4/10; ++i) {
                                                         fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                         }
                                                         costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                              }
                                          } else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. скорость ВК меньше скорости прибывания воды"); break;
                                          }
                                          costFirstMi14 = costFirstMi14 + rentMi14; // затраты на первый вертолёт
                                          capacityEvacMi14 = capacityMi14+resqueRaft;
                                          // обновляем дальность и топливо, чтоб считать второй вертолёт
                                          rangeMi14Change = 0; // расходуемая дальность
                                          for(int i = 0; i < squareWater4/10; ++i) {
                                          fuelWeightMi14Change[i] = 0;
                                          }
                                          for(int i = 0; i < squareWater4/10; ++i) {
                                              if (tMi14[i]<timeWaterFlood[i]){
                                         if (startValue <= rangeMi14/2){ //сможет ли вертолёт вообще полететь и вернуться
                                          for(int i = 0; i < squareWater4/10; ++i) { //летим по точкам эвакуации
                                         // if (tMi14[i] < timeWaterFlood[i] ){ //если Ми-14 прибудет раньше, чем вода затопит точку - не учитываем, т.к. может прибыть и после затопления точки
                                                      while ((rangeMi14Change < rangeMi14) && (fuelWeightMi14Change[i] < fuelWeightMi14) ) { // пока остались дальность и топливо, +расстояние от вертодрома меньше дальности
                                                          while (capacityEvacMi14 > 0){ // пока есть место в вертолёте
                                                              capacityEvacMi14 = capacityEvacMi14 - peopleAverage; // забираем людей
                                                              // куда летим теперь (проверяем следующий вылет)
                                                              if (capacityEvacMi14 > 0) { //продолжаем рассчитывать вылет и летим к след точке
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 < 0) { // продолжаем рассчитывать вылет до вертодрома базирования и вернёмся к этой же точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                              else if (capacityEvacMi14 == 0) { //  продолжаем рассчитывать вылет до вертодрома базирования и полетим к след точке
                                                                  capacityEvacMi14 = capacityMi14; // высадили на вертодроме, места освободились
                                                                  break;
                                                              }
                                                          }
                                                          flightCountMi14++; // считаем вылет
                                                          for(int i = 1; i < squareWater4/10; ++i) {
                                                          fuelWeightMi14Change[i] = fuelWeightMi14Change[i-1] + fuelMi14Average; // затраченное топливо
                                                          }
                                                          costOneMi14 = costOneMi14 + fuelWeightMi14Change[i]*fuelCost;
                                                          rangeMi14Change = rangeMi14Change + sAverage; // затраченная дальность
                                                      }
                                          }
                                }  else QMessageBox::information(nullptr, "Предупреждение", "Вертолёт Ми-14 не подходит, т.к. расстояние от вертодрома слишком большое");
                                              }}
                                         QVector<double> costMi14(squareWater4/10);
                                           for(int i = 0; i < squareWater4/10; ++i) {
                                                   costMi14[i] = 0;
                                               }
                                           std::reverse(fuelWeightMi14Change.begin(), fuelWeightMi14Change.end());
                                           qDebug() << "fuelWeightMi14Change" << fuelWeightMi14Change; //проверка
                                         costOneMi14 = costOneMi14 + rentMi14; // затраты с 1 шт
                                         // проверка
                                         qDebug() << "flightCountMi14" << flightCountMi14;
                                         qDebug() << "costOneMi14" << costOneMi14;
                                         peopleCount = lineEdit1->text().toInt(); // Количество пострадавших
                                         // если б 1 вертолёт облетал все точки без ограничений, количество часов:
                                         tOneMi14 =  peopleCount/capacityMi14 * tMi14Average / flightCountMi14 + peopleCount*evacuationTimeForOnePerson + timeHelp;
                                         qDebug() << "tOneMi14" << tOneMi14;
                                         QVector<int> helicoptersMi14Count(squareWater4/10);
                                           for(int i = 0; i < squareWater4/10; ++i) {
                                                   helicoptersMi14Count[i] = 0;
                                               }
                                         for(int i = 0; i < squareWater4/10; ++i) {
                                             helicoptersMi14Count[i] = tOneMi14 / time[i];
                                             if (helicoptersMi14Count[i] <= 1) { // Условие 1: если helicoptersMi14Count меньше или равно 1
                                                     helicoptersMi14Count[i] = 1;
                                                     costMi14[i] = (costOneMi14* helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000;
                                             }
                                             else { // Условие 2: если helicoptersMi14Count больше 1
                                                 helicoptersMi14Count[i]= helicoptersMi14Count[i] +1;
                                                 costMi14[i] = (costMi14[i] + costOneMi14 * helicoptersMi14Count[i]+fuelWeightMi14Change[i]*fuelCost)/1000000; // в зависимости от i (т.е. точки, по нему же и время) затраты увеличиваются
                                              }
                                         }
                                         std::reverse(helicoptersMi14Count.begin(), helicoptersMi14Count.end());
                                         std::reverse(costMi14.begin(), costMi14.end());
                                         qDebug() << "helicoptersMi14Count" << helicoptersMi14Count;
                                         qDebug() << "costMi14" << costMi14;


                                         // ОПРЕДЕЛЕНИЕ РАЦИОНАЛЬНОГО ВК ПО ЗАТРАТАМ
                                         // Настройки карты стоимостей
                                        QMap<QString, double>
                                                costAllFlood = {
                                            {"Mi8", costMi8[squareWater4/10-1]},
                                            {"Mi26", costMi26[squareWater4/10-1]},
                                            {"SikorskyS61", costSikorskyS61[squareWater4/10-1]},
                                            {"Mi14", costMi14[squareWater4/10-1]}
                                        };

                                        // Карта счетчиков вертолетов
                                        QMap<QString, int> helicoptersCountMap = {
                                            {"Mi8", helicoptersMi8Count[squareWater4/10-1]},
                                            {"Mi26", helicoptersMi26Count[squareWater4/10-1]},
                                            {"SikorskyS61", helicoptersSikorskyS61Count[squareWater4/10-1]},
                                            {"Mi14", helicoptersMi14Count[squareWater4/10-1]}
                                        };

                                        // Карта счетчиков полётов вертолетов - чтобы если вертолёт не подошёл, его затраты не учитывались, т.к. они равны 0
                                        QMap<QString, int> helicoptersFlightCountMap = {
                                            {"Mi8", flightCountMi8},
                                            {"Mi26", flightCountMi26},
                                            {"SikorskyS61", flightCountSikorskyS61},
                                            {"Mi14", flightCountMi14}
                                        };

                                        QString minCostFlood;
                                        double minCostFloodValue = std::numeric_limits<double>::max();

                                        for (auto it = costAllFlood.constBegin(); it != costAllFlood.constEnd(); ++it) {
                                            // Проверяем, что количество полетов для этого типа вертолетов больше нуля
                                            if (it.value() < minCostFloodValue && helicoptersFlightCountMap[it.key()] > 0) {
                                                minCostFloodValue = it.value();
                                                minCostFlood = it.key();
                                            }
                                        }
                                        // Проверяем, было ли найдено соответствующее минимальное значение
                                        if (minCostFloodValue == std::numeric_limits<double>::max()) {
                                          QMessageBox::critical(nullptr, "Ошибка", "Ни один вертолёт не подошёл");
                                          minCostFloodValue = 0;
                                        }

                                        // Запрос количества вертолетов для найденной минимальной стоимости
                                        int countHelicoptersFloodMin = helicoptersCountMap[minCostFlood];

                                        qDebug() << "Рациональный ВК: " << minCostFlood //ЦУНАМИ
                                                 << "; Количество ВК: " << countHelicoptersFloodMin
                                                 << "; Затраты: " << minCostFloodValue;

                                          QString floodType="Цунами"; // для вывода в результаты

                                          // Сохранить в таблице БД результаты
                                          saveResultsToDb(floodType, peopleCount, startValue, minCostFlood, countHelicoptersFloodMin, minCostFloodValue);

                                        // ГРАФИКИ TSUNAMI FLOOD
                                          //если пострадаших нет, расстояние 0 км, + расстояние до вертодрома меньше дальности вертолёта то ничего не выводим
                                          if ((peopleCount != 0) && (startValue != 0) && (timeLimit != 0)){
                                          QChart *chart = new QChart();
                                          chart->update();
                                          chart->legend()->hide();

                                          QLineSeries *seriesMi8 = new QLineSeries();
                                          if (flightCountMi8>0){
                                        seriesMi8->setName("Ми-8");
                                        for (int i = 0; i < time.size(); ++i) {
                                                seriesMi8->append(time[i], costMi8[i]);
                                            }
                                        chart->addSeries(seriesMi8);
                                        }
                                            QLineSeries *seriesMi26 = new QLineSeries();

                                            if (flightCountMi26>0){
                                                seriesMi26->setName("Ми-26");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi26->append(time[i], costMi26[i]);
                                            }
                                            chart->addSeries(seriesMi26);
                                             }

                                            QLineSeries *seriesSikorskyS61 = new QLineSeries();
                                            if (flightCountSikorskyS61>0){
                                            seriesSikorskyS61->setName("Sikorsky S-61");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesSikorskyS61->append(time[i], costSikorskyS61[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesSikorskyS61);
                                            }

                                            QLineSeries *seriesMi14 = new QLineSeries();
                                            if (flightCountMi14>0){
                                            seriesMi14->setName("Ми-14");
                                            for (int i = 0; i < time.size(); ++i) {
                                                seriesMi14->append(time[i], costMi14[i]); // бонус времени
                                            }
                                            chart->addSeries(seriesMi14);
                                            }

                                            chart->createDefaultAxes();
                                            chart->setTitle("Затраты (млн.руб.) всех ВК в зависимости от времени (ч)");
                                            chart->legend()->setVisible(true);

/*
                                            QPen penMi8;
                                            penMi8.setColor(Qt::cyan); // Голубой цвет для Ми-8
                                            seriesMi8->setPen(penMi8);
                                            QPen penMi26;
                                            penMi26.setColor(QColor(50, 205, 50)); // Зеленый цвет для Ми-26
                                            seriesMi26->setPen(penMi26);
                                            QPen penSikorskyS61;
                                            penSikorskyS61.setColor(QColor(255, 165, 0)); // Оранжевый цвет для Sikorsky S-61
                                            seriesSikorskyS61->setPen(penSikorskyS61);
                                            QPen penMi14;
                                            penMi14.setColor(QColor(128, 0, 128)); // Фиолетовый цвет для Ми-14 (RGB код фиолетового)
                                            seriesMi14->setPen(penMi14);
                                            */

                                            QValueAxis *auxAxisX = new QValueAxis;
                                            auxAxisX->setRange(1, int(squareWater4/10)); // Установите диапазон значений оси
                                            auxAxisX->setTickCount(int(squareWater4/10)); // Количество делений оси
                                            auxAxisX->setLinePenColor(Qt::gray); // Цвет линии оси
                                            QFont labelsFont;
                                            labelsFont.setPointSize(4); // Устанавливаем меньший размер шрифта
                                            QBrush labelsBrush(Qt::black);
                                            labelsFont.setBold(true);
                                            auxAxisX->setLabelsFont(labelsFont); // Применяем шрифт к меткам
                                            auxAxisX->setLabelFormat("%d");
                                            chart->addAxis(auxAxisX, Qt::AlignTop);

                                            QChartView *chartView = new QChartView(chart);
                                            chartView->setRenderHint(QPainter::Antialiasing);
                                            // Устанавливаем QChartView как центральный виджет в windowGraph
                                            QVBoxLayout *layout = new QVBoxLayout;
                                            layout->addWidget(chartView);

                                       //ВЫВОД РЕЗУЛЬТАТА В ОТДЕЛЬНОМ ОКНЕ
                                       // dataWindow->setAttribute(Qt::WA_DeleteOnClose); // Окно будет удаляться при закрытии
                                        dataWindow->setWindowTitle("Результат");
                                        dataWindow->resize(200, 400);

                                        QString variableName0 = "Тип наводнения";
                                        QString variableName1 = "Количество пострадавших";
                                        QString variableName2 = "Рациональный ВК";
                                        QString variableName3 = "Расстояние от вертодрома";
                                        QString variableName6 = "Ограничение на время";
                                        QString variableName4 = "Количество ВК";
                                        QString variableName5 = "Затраты";

                                        // Создаем таблицу для отображения данных
                                        QTableWidget *tableWidget = new QTableWidget(dataWindow); // Создан с 0 строк и 0 столбцов
                                        tableWidget->clearContents();
                                        tableWidget->setRowCount(0);
                                        tableWidget->setRowCount(7); // Устанавливаем 6 строк
                                        tableWidget->setColumnCount(2);
                                        // tableWidget->setHorizontalHeaderLabels(QStringList() << "Параметр" << "Значение");
                                        tableWidget->verticalHeader()->setVisible(false);   // Скрывает нумерацию строк
                                        tableWidget->horizontalHeader()->setVisible(false); // Скрывает названия столбцов
                                        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
                                        tableWidget->setSelectionMode(QAbstractItemView::NoSelection); // Запрет выделения

                                        tableWidget->setStyleSheet("QTableView { border: 1px solid black; }"
                                                                   "QTableView::item { border: 1px solid black; }"
                                                                   "QHeaderView::section { border: 1px solid black; }");
                                        // Заполняем строки
                                        tableWidget->setItem(0, 0, new QTableWidgetItem(variableName0));
                                        tableWidget->setItem(0, 1, new QTableWidgetItem(floodType));

                                        tableWidget->setItem(1, 0, new QTableWidgetItem(variableName1));
                                        tableWidget->setItem(1, 1, new QTableWidgetItem(QString::number(peopleCount) + " человек"));

                                        tableWidget->setItem(2, 0, new QTableWidgetItem(variableName2));
                                        tableWidget->setItem(2, 1, new QTableWidgetItem(minCostFlood));

                                        tableWidget->setItem(3, 0, new QTableWidgetItem(variableName3));
                                        tableWidget->setItem(3, 1, new QTableWidgetItem(QString::number(startValue) + " км"));

                                        tableWidget->setItem(4, 0, new QTableWidgetItem(variableName6));
                                        tableWidget->setItem(4, 1, new QTableWidgetItem(QString::number(timeLimit) + " ч"));

                                        tableWidget->setItem(5, 0, new QTableWidgetItem(variableName4));
                                        tableWidget->setItem(5, 1, new QTableWidgetItem(QString::number(countHelicoptersFloodMin)));

                                        tableWidget->setItem(6, 0, new QTableWidgetItem(variableName5));
                                        tableWidget->setItem(6, 1, new QTableWidgetItem(QString::number(minCostFloodValue) + " млн.руб."));

                                        tableWidget->resizeColumnsToContents(); // Автоматическая настройка ширины колонок
                                        tableWidget->horizontalHeader()->setStretchLastSection(true); // Последний столбец растягивается

                                        // Устанавливаем layout для отображения только что созданной таблицы
                                        dataWindow->setLayout(new QVBoxLayout);
                                        dataWindow->layout()->addWidget(tableWidget);

                                        dataWindow->setMinimumSize(450, 287);
                                        dataWindow->setMaximumSize(450, 287);
                                        dataWindow->show();

                                        graphWidget->setLayout(layout);
                                        graphWidget->show(); // показать график
                                        map->show(); // Вывод рельефа в дополнительном окне
                                        map->spreadWater4(); // распространение воды по рельефу
                                          } else QMessageBox::critical(nullptr, "Ошибка", "Поле для ввода пустое");
                                        break;}

                                  //default:  // Если индекс неизвестен, ничего не делаем и очищаем указатель
                              if ((comboBox->currentText() == "")){
                                QMessageBox::critical(nullptr, "Ошибка", "Поле для ввода пустое");}

                                      currentMapWindow.clear();
                                      return;
                              }
            db.close();

        });

        //кнопка очистить
        QObject::connect(pushButton3, &QPushButton::clicked, [window, &currentMapWindow, dataWindow, windowGraph, graphWidget]() {
            if (currentMapWindow) {
                currentMapWindow->close();
            }
            dataWindow->close();
            windowGraph->close();
            window->close();
            // Выходим из текущего приложения (автоматический перезапуск программы)
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
                   qApp->exit(0);
        });


        // для бд в программе
        /*
        QObject::connect(openDbButton, &QPushButton::clicked, [&]() {
                // Type
                QSqlTableModel *modelType = new QSqlTableModel(tableViewType, db);
                modelType->setTable("Type");
                modelType->select();
                tableViewType->setModel(modelType);
                tableViewType->setVisible(true);
                modelType->setEditStrategy(QSqlTableModel::OnFieldChange);

                // Flood
                QSqlTableModel *modelFlood = new QSqlTableModel(tableViewFlood, db);
                modelFlood->setTable("Flood");
                modelFlood->select();
                tableViewFlood->setModel(modelFlood);
                tableViewFlood->setVisible(true);
                modelFlood->setEditStrategy(QSqlTableModel::OnFieldChange);

                // Result
                QSqlQuery query(db);
                query.exec("CREATE TABLE IF NOT EXISTS Result(id INTEGER PRIMARY KEY, data TEXT)");
                QSqlTableModel *modelResult = new QSqlTableModel(tableViewResult, db);
                modelResult->setTable("Result");
                modelResult->select();
                tableViewResult->setModel(modelResult);
                tableViewResult->setVisible(true);
                modelResult->setEditStrategy(QSqlTableModel::OnFieldChange);
            });
        */

        /*
        QObject::connect(pushButton0, &QPushButton::clicked, [db, tabWidget, dbDialog] {

            while(tabWidget->count() > 0) {
                   tabWidget->removeTab(0);
               }
            // Загрузка данных и создание вкладок для таблиц

                // Type
                QTableView *tableViewType = new QTableView();
                QSqlTableModel *modelType = new QSqlTableModel(tableViewType, db);
                modelType->setTable("Type");
                modelType->select();
                modelType->setEditStrategy(QSqlTableModel::OnFieldChange);
                tableViewType->setModel(modelType);
                tabWidget->addTab(tableViewType, "Type");

                // Flood
                QTableView *tableViewFlood = new QTableView();
                QSqlTableModel *modelFlood = new QSqlTableModel(tableViewFlood, db);
                modelFlood->setTable("Flood");
                modelFlood->select();
                tableViewFlood->setModel(modelFlood);
                tabWidget->addTab(tableViewFlood, "Flood");
                tableViewFlood->setVisible(true);

                // Result
                QSqlQuery query(db);
                query.exec("CREATE TABLE IF NOT EXISTS Result ("
                           "Flood TEXT, "
                           "People INTEGER, "
                           "Distance REAL, "
                           "RationalHC TEXT, "
                           "CountHC INTEGER, "
                           "Cost REAL)");
                QTableView *tableViewResult = new QTableView();
                QSqlTableModel *modelResult = new QSqlTableModel(tableViewResult, db);
                modelResult->setTable("Result");
                modelResult->select();
                tableViewResult->setModel(modelResult);
                tabWidget->addTab(tableViewResult, "Result");

                dbDialog->show(); // Показываем диалог с таблицами
            });
*/

/*
        QObject::connect(pushButton0, &QPushButton::clicked, [&]() {
            while(tabWidget.count() > 0) {
                    tabWidget.removeTab(0);
                }
            setupTable("Type");
            setupTable("Flood");
            setupTable("Result"); // Таблица создаваемая в коде
            dbDialog.show();
        });
*/


        // Слот для закрытия всех окон и выхода из приложения
        QObject::connect(pushButton2, &QPushButton::clicked, [window, &currentMapWindow, dataWindow, windowGraph, graphWidget]() {
            // Закрываем дополнительное окно, если открыто
            if (currentMapWindow) {
                currentMapWindow->close(); // Окно удалится, так как у него установлен флаг WA_DeleteOnClose
            }
            // Закрываем главное окно и выходим из приложения
            dataWindow->close();
            windowGraph->close();
            window->close();
        });
        window->setMinimumSize(1080, 520);
        window->setMaximumSize(1080, 520);
    window->setLayout(mainLayout);
    window->show();

    return a.exec();
}
