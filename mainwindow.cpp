#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint, true);
    setCursor(Qt::BlankCursor);
    setMouseTracking(true);

    playerPos = QPointF(width() * 0.15, height() * 0.5);

    gameTimer.setInterval(16);
    connect(&gameTimer, &QTimer::timeout, this, &MainWindow::updateGame);
    gameTimer.start();

    enemySpawnTimer.setInterval(600);
    connect(&enemySpawnTimer, &QTimer::timeout, this, &MainWindow::spawnEnemy);
    enemySpawnTimer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rect(), QColor(10, 10, 20));

    p.setPen(QPen(QColor(40, 40, 65), 1));
    for (int x = 0; x < width(); x += 50) {
        p.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += 50) {
        p.drawLine(0, y, width(), y);
    }

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(40, 220, 120));
    p.drawEllipse(playerPos, playerRadius, playerRadius);

    p.setBrush(QColor(255, 210, 80));
    for (const Bullet &bullet : bullets) {
        p.drawEllipse(bullet.pos, bullet.radius, bullet.radius);
    }

    p.setBrush(QColor(255, 90, 90));
    for (const Enemy &enemy : enemies) {
        p.drawEllipse(enemy.pos, enemy.radius, enemy.radius);
    }

    p.setPen(QColor(230, 230, 255));
    QFont font = p.font();
    font.setPointSize(16);
    font.setBold(true);
    p.setFont(font);
    p.drawText(20, 35, QString("HP: %1   Score: %2").arg(hp).arg(score));

    if (hp <= 0) {
        p.setPen(QColor(255, 140, 140));
        QFont overFont = p.font();
        overFont.setPointSize(40);
        overFont.setBold(true);
        p.setFont(overFont);
        p.drawText(rect(), Qt::AlignCenter, "GAME OVER");
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    clampPlayer();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    dragging = true;
    lastDragPos = event->position();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging || hp <= 0) {
        return;
    }

    const QPointF currentPos = event->position();
    const QPointF delta = currentPos - lastDragPos;
    lastDragPos = currentPos;

    playerPos += delta;
    clampPlayer();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    dragging = false;
}

void MainWindow::updateGame()
{
    if (hp <= 0) {
        update();
        return;
    }

    frameCounter++;
    if (frameCounter % 8 == 0) {
        shootBullet();
    }

    for (int i = bullets.size() - 1; i >= 0; --i) {
        bullets[i].pos.rx() += bullets[i].speed;
        if (bullets[i].pos.x() - bullets[i].radius > width()) {
            bullets.remove(i);
        }
    }

    for (int i = enemies.size() - 1; i >= 0; --i) {
        enemies[i].pos.rx() -= enemies[i].speed;

        if (enemies[i].pos.x() + enemies[i].radius < 0) {
            enemies.remove(i);
            continue;
        }

        const qreal distToPlayer = QLineF(enemies[i].pos, playerPos).length();
        if (distToPlayer < enemies[i].radius + playerRadius) {
            enemies.remove(i);
            hp--;
        }
    }

    for (int ei = enemies.size() - 1; ei >= 0; --ei) {
        bool destroyed = false;
        for (int bi = bullets.size() - 1; bi >= 0; --bi) {
            const qreal d = QLineF(enemies[ei].pos, bullets[bi].pos).length();
            if (d < enemies[ei].radius + bullets[bi].radius) {
                enemies.remove(ei);
                bullets.remove(bi);
                score += 10;
                destroyed = true;
                break;
            }
        }
        if (destroyed) {
            continue;
        }
    }

    update();
}

void MainWindow::clampPlayer()
{
    playerPos.setX(qBound(playerRadius, playerPos.x(), width() - playerRadius));
    playerPos.setY(qBound(playerRadius, playerPos.y(), height() - playerRadius));
}

void MainWindow::spawnEnemy()
{
    if (hp <= 0) {
        return;
    }

    Enemy e;
    e.radius = QRandomGenerator::global()->bounded(14.0, 28.0);
    e.speed = QRandomGenerator::global()->bounded(3.0, 7.5);
    e.pos = QPointF(width() + e.radius,
                    QRandomGenerator::global()->bounded(e.radius, height() - e.radius));
    enemies.push_back(e);
}

void MainWindow::shootBullet()
{
    Bullet b;
    b.radius = 6.0f;
    b.speed = 12.0f;
    b.pos = QPointF(playerPos.x() + playerRadius + b.radius + 2.0, playerPos.y());
    bullets.push_back(b);
}
