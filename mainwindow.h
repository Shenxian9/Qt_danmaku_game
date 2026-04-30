#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <QVector>

class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void touchEvent(QTouchEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct Bullet {
        QPointF pos;
        qreal speed;
    };

    struct Enemy {
        QPointF pos;
        qreal speed;
        qreal radius;
    };

    void updateGame();
    void spawnEnemy();
    void handleInputAt(const QPointF &screenPos);
    void resetPlayer();

private:
    QTimer *m_updateTimer;
    QTimer *m_spawnTimer;

    QPointF m_playerPos;
    qreal m_playerRadius;
    qreal m_playerSpeed;
    QPointF m_lastInputPos;
    bool m_hasInput;

    QVector<Bullet> m_bullets;
    QVector<Enemy> m_enemies;

    int m_score;
    bool m_gameOver;
};

#endif // MAINWINDOW_H
