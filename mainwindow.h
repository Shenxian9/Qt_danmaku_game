#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <QRectF>
#include <QTimer>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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

private slots:
    void gameTick();

private:
    struct Bullet
    {
        QPointF pos;
        float speed;
    };

    struct Enemy
    {
        QPointF pos;
        float speed;
        float radius;
    };

    void resetPlayer();
    void updateBullets(float dt);
    void updateEnemies(float dt);
    void spawnEnemy();
    void shootBullet();
    void resolveCollisions();

    Ui::MainWindow *ui;

    QTimer m_gameTimer;
    int m_tickMs = 16;

    QRectF m_playerRect;
    float m_playerSpeedFactor = 1.0f;

    QVector<Bullet> m_bullets;
    QVector<Enemy> m_enemies;

    QPointF m_lastDragPos;
    bool m_dragging = false;

    float m_bulletSpawnAccumulator = 0.0f;
    float m_enemySpawnAccumulator = 0.0f;

    int m_score = 0;
    bool m_gameOver = false;
};

#endif // MAINWINDOW_H
