#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPointF>
#include <QTimer>
#include <QVector>

struct Bullet {
    QPointF pos;
    float speed;
    float radius;
};

struct Enemy {
    QPointF pos;
    float speed;
    float radius;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onGameTick();

private:
    void resetGame();
    void spawnEnemy();
    void fireBullet();
    void movePlayerByDelta(const QPointF &delta);
    void handleCollisions();

    QTimer m_gameTimer;
    QElapsedTimer m_spawnClock;
    QElapsedTimer m_fireClock;

    QVector<Bullet> m_bullets;
    QVector<Enemy> m_enemies;

    QPointF m_playerPos;
    float m_playerRadius = 24.0f;
    float m_playerSpeedScale = 1.0f;

    bool m_dragging = false;
    QPointF m_lastInputPos;

    bool m_gameOver = false;
    int m_score = 0;
};

#endif // MAINWINDOW_H
