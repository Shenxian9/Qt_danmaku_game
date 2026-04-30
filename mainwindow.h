#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <QTimer>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct Bullet
{
    QPointF pos;
    float radius;
    float speed;
};

struct Enemy
{
    QPointF pos;
    float radius;
    float speed;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updateGame();

private:
    void clampPlayer();
    void spawnEnemy();
    void shootBullet();

    Ui::MainWindow *ui;
    QTimer gameTimer;
    QTimer enemySpawnTimer;

    QPointF playerPos;
    float playerRadius = 24.0f;

    bool dragging = false;
    QPointF lastDragPos;

    QVector<Bullet> bullets;
    QVector<Enemy> enemies;

    int hp = 5;
    int score = 0;
    int frameCounter = 0;
};

#endif // MAINWINDOW_H
