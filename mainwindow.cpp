#include "mainwindow.h"

#include <QMouseEvent>
#include <algorithm>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <QTouchEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_updateTimer(new QTimer(this))
    , m_spawnTimer(new QTimer(this))
    , m_playerRadius(24.0)
    , m_playerSpeed(10.0)
    , m_hasInput(false)
    , m_score(0)
    , m_gameOver(false)
{
    setWindowTitle(QStringLiteral("弹幕躲避"));
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setMouseTracking(true);
    setStyleSheet(QStringLiteral("background-color: black;"));

    resetPlayer();

    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateGame);
    connect(m_spawnTimer, &QTimer::timeout, this, &MainWindow::spawnEnemy);

    m_updateTimer->start(16);
    m_spawnTimer->start(450);
}

void MainWindow::resetPlayer()
{
    m_playerPos = QPointF(width() * 0.2, height() * 0.5);
    m_lastInputPos = m_playerPos;
    m_bullets.clear();
    m_enemies.clear();
    m_score = 0;
    m_gameOver = false;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!m_hasInput) {
        m_playerPos = QPointF(width() * 0.2, height() * 0.5);
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(10, 10, 16));

    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(80, 220, 255));
    painter.drawEllipse(m_playerPos, m_playerRadius, m_playerRadius);

    painter.setBrush(QColor(255, 240, 80));
    for (const Bullet &bullet : m_bullets) {
        painter.drawEllipse(bullet.pos, 6.0, 3.0);
    }

    painter.setBrush(QColor(255, 80, 80));
    for (const Enemy &enemy : m_enemies) {
        painter.drawEllipse(enemy.pos, enemy.radius, enemy.radius);
    }

    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    QFont font = painter.font();
    font.setPointSize(18);
    painter.setFont(font);
    painter.drawText(24, 40, QStringLiteral("分数: %1").arg(m_score));

    if (m_gameOver) {
        QFont overFont = painter.font();
        overFont.setPointSize(28);
        overFont.setBold(true);
        painter.setFont(overFont);
        painter.drawText(rect(), Qt::AlignCenter,
                         QStringLiteral("游戏结束\n任意位置滑动重新开始"));
    }
}

void MainWindow::handleInputAt(const QPointF &screenPos)
{
    if (m_gameOver) {
        resetPlayer();
    }

    if (!m_hasInput) {
        m_lastInputPos = screenPos;
        m_hasInput = true;
        return;
    }

    const QPointF delta = screenPos - m_lastInputPos;
    m_lastInputPos = screenPos;

    m_playerPos += QPointF(delta.x(), delta.y()) * 0.75;
    m_playerPos.setX(qBound(m_playerRadius, m_playerPos.x(), width() - m_playerRadius));
    m_playerPos.setY(qBound(m_playerRadius, m_playerPos.y(), height() - m_playerRadius));
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    handleInputAt(event->position());
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        handleInputAt(event->position());
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::touchEvent(QTouchEvent *event)
{
    const QList<QEventPoint> points = event->points();
    if (!points.isEmpty()) {
        handleInputAt(points.first().position());
    }
    event->accept();
}

void MainWindow::spawnEnemy()
{
    if (m_gameOver) {
        return;
    }

    Enemy enemy;
    enemy.radius = QRandomGenerator::global()->bounded(14.0, 34.0);
    enemy.pos = QPointF(width() + enemy.radius,
                        QRandomGenerator::global()->bounded(enemy.radius, height() - enemy.radius));
    enemy.speed = QRandomGenerator::global()->bounded(2.0, 6.0);
    m_enemies.push_back(enemy);
}

void MainWindow::updateGame()
{
    if (!m_gameOver) {
        if (QRandomGenerator::global()->bounded(4) == 0) {
            Bullet bullet;
            bullet.pos = QPointF(m_playerPos.x() + m_playerRadius, m_playerPos.y());
            bullet.speed = 12.0;
            m_bullets.push_back(bullet);
        }

        for (Bullet &bullet : m_bullets) {
            bullet.pos.rx() += bullet.speed;
        }
        m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(), [this](const Bullet &b) {
            return b.pos.x() > width() + 20;
        }), m_bullets.end());

        for (Enemy &enemy : m_enemies) {
            enemy.pos.rx() -= enemy.speed;
        }

        for (int i = m_enemies.size() - 1; i >= 0; --i) {
            bool removed = false;
            for (int j = m_bullets.size() - 1; j >= 0; --j) {
                const qreal dx = m_enemies[i].pos.x() - m_bullets[j].pos.x();
                const qreal dy = m_enemies[i].pos.y() - m_bullets[j].pos.y();
                const qreal dist2 = dx * dx + dy * dy;
                const qreal hitR = m_enemies[i].radius + 6.0;
                if (dist2 <= hitR * hitR) {
                    m_enemies.remove(i);
                    m_bullets.remove(j);
                    m_score += 10;
                    removed = true;
                    break;
                }
            }
            if (removed) {
                continue;
            }

            const qreal pdx = m_enemies[i].pos.x() - m_playerPos.x();
            const qreal pdy = m_enemies[i].pos.y() - m_playerPos.y();
            const qreal playerHit = m_enemies[i].radius + m_playerRadius;
            if (pdx * pdx + pdy * pdy <= playerHit * playerHit) {
                m_gameOver = true;
                break;
            }
        }

        m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(), [this](const Enemy &e) {
            return e.pos.x() < -e.radius;
        }), m_enemies.end());
    }

    update();
}
