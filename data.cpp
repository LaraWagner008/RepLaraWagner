#include "data.h"
#include "ui_people.h"

People::People(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::People)
{
    ui->setupUi(this);
}

People::~People()
{
    delete ui;
}
/*
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
   // MainWindow w;
    //w.show();

    QWidget *window = new QWidget();

    // Создание виджетов
    QLabel *label1 = new QLabel("Выберите тип наводнения");
    QComboBox *comboBox = new QComboBox();
    QLabel *label2 = new QLabel("Введите количество пострадавших");
    QLineEdit *lineEdit1 = new QLineEdit();
    QLabel *label3 = new QLabel("Введите расстояние от вертодрома до первой точки");
    QLineEdit *lineEdit2 = new QLineEdit();
    QLabel *label4 = new QLabel("Введите ограничение на время выполнения операции");
    QLineEdit *lineEdit3 = new QLineEdit();
    QPushButton *pushButton1 = new QPushButton ("Результат");
    QPushButton *pushButton2 = new QPushButton("Завершить");


  //  lineEdit2->setParent(window);
    //pushButton1->setParent(window);

    // Задаём фиксированный размер для comboBox и lineEdits
    comboBox->setFixedWidth(200);
        comboBox->addItem(" ");
        comboBox->addItem("Половодье");
        comboBox->addItem("Паводок");
        comboBox->addItem("Прорыв плотины");
        comboBox->addItem("Цунами");
    lineEdit1->setFixedWidth(200);
    lineEdit2->setFixedWidth(200);

    // График
    QGraphicsView *graphWidget = new QGraphicsView();
    graphWidget->setFixedSize(700, 700);

    // TerrainMap
    QGraphicsView *terrainMapWidget = new QGraphicsView();
    terrainMapWidget->setFixedSize(700, 700);

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
    lineEdit3->setFixedWidth(200); //потому что оно почему-то строится длиннее, чем остальные lineEdit
    leftColumnLayout->addStretch(1); // Добавляет пространство внизу
    leftColumnLayout->addWidget(pushButton1);
    leftColumnLayout->addWidget(pushButton2);

    // Горизонтальный компоновщик для основной области
    QHBoxLayout *mainLayout = new QHBoxLayout(window);
    mainLayout->addLayout(leftColumnLayout); // Добавляем левую колонку
    mainLayout->addWidget(graphWidget); // Сначала график
    //mainLayout->addWidget(terrainMapWidget); // Затем карту

    window->setLayout(mainLayout);
    window->show();


    return a.exec();
}
*/
