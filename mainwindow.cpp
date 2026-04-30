#include "mainwindow.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QTouchEvent>

#include <algorithm>
#include <cmath>

namespace {
constexpr int kTickMs = 16;
constexpr qint64 kEnemySpawnIntervalMs = 650;
constexpr qint64 kFireIntervalMs = 140;
constexpr float kBulletSpeed = 14.0f;
constexpr float kEnemySpeedMin = 3.5f;
constexpr float kEnemySpeedMax = 8.0f;
constexpr float kEnemyRadiusMin = 16.0f;
constexpr float kEnemyRadiusMax = 34.0f;
constexpr float kPlayerPadding = 8.0f;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("弹幕躲避游戏"));
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setMouseTracking(true);

    m_gameTimer.setInterval(kTickMs);
    connect(&m_gameTimer, &QTimer::timeout, this, &MainWindow::onGameTick);

    resetGame();
    m_gameTimer.start();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(8, 10, 18));

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 200, 255));
    p.drawEllipse(m_playerPos, m_playerRadius, m_playerRadius);

    p.setBrush(QColor(255, 240, 120));
    for (const Bullet &b : m_bullets) {
        p.drawEllipse(b.pos, b.radius, b.radius);
    }

    p.setBrush(QColor(255, 90, 90));
    for (const Enemy &e : m_enemies) {
        p.drawEllipse(e.pos, e.radius, e.radius);
    }

    p.setPen(QColor(220, 230, 255));
    p.setFont(QFont(QStringLiteral("Sans"), 16, QFont::Bold));
    p.drawText(20, 40, QStringLiteral("分数: %1").arg(m_score));

    if (m_gameOver) {
        p.setPen(QColor(255, 120, 120));
        p.setFont(QFont(QStringLiteral("Sans"), 28, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter,
                   QStringLiteral("游戏结束\n轻触屏幕重新开始"));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!m_gameOver && m_playerPos.isNull()) {
        m_playerPos = QPointF(width() * 0.15, height() * 0.5);
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd) {
        auto *touchEvent = static_cast<QTouchEvent *>(event);
        const QList<QEventPoint> points = touchEvent->points();

        if (!points.isEmpty()) {
            const QPointF current = points.first().position();
            if (m_gameOver && event->type() == QEvent::TouchBegin) {
                resetGame();
                return true;
            }

            if (event->type() == QEvent::TouchBegin) {
                m_dragging = true;
                m_lastInputPos = current;
            } else if (event->type() == QEvent::TouchUpdate && m_dragging) {
                movePlayerByDelta(current - m_lastInputPos);
                m_lastInputPos = current;
            } else if (event->type() == QEvent::TouchEnd) {
                m_dragging = false;
            }
        }
        return true;
    }

    return QMainWindow::event(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_gameOver) {
        resetGame();
        return;
    }
    m_dragging = true;
    m_lastInputPos = event->position();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || m_gameOver) {
        return;
    }
    const QPointF current = event->position();
    movePlayerByDelta(current - m_lastInputPos);
    m_lastInputPos = current;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_dragging = false;
}

void MainWindow::onGameTick()
{
    if (m_gameOver) {
        update();
        return;
    }

    if (m_spawnClock.elapsed() > kEnemySpawnIntervalMs) {
        spawnEnemy();
        m_spawnClock.restart();
    }

    if (m_fireClock.elapsed() > kFireIntervalMs) {
        fireBullet();
        m_fireClock.restart();
    }

    for (Bullet &b : m_bullets) {
        b.pos.rx() += b.speed;
    }
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
                                   [this](const Bullet &b) { return b.pos.x() - b.radius > width(); }),
                    m_bullets.end());

    for (Enemy &e : m_enemies) {
        e.pos.rx() -= e.speed;
    }
    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
                                   [](const Enemy &e) { return e.pos.x() + e.radius < 0.0f; }),
                    m_enemies.end());

    handleCollisions();
    update();
}

void MainWindow::resetGame()
{
    m_bullets.clear();
    m_enemies.clear();
    m_score = 0;
    m_gameOver = false;
    m_dragging = false;
    m_playerPos = QPointF(width() * 0.15, height() * 0.5);
    m_spawnClock.restart();
    m_fireClock.restart();
    update();
}

void MainWindow::spawnEnemy()
{
    Enemy e;
    e.radius = QRandomGenerator::global()->bounded(kEnemyRadiusMin, kEnemyRadiusMax);
    e.speed = QRandomGenerator::global()->bounded(kEnemySpeedMin, kEnemySpeedMax);
    e.pos = QPointF(width() + e.radius,
                    QRandomGenerator::global()->bounded(e.radius, height() - e.radius));
    m_enemies.push_back(e);
}

void MainWindow::fireBullet()
{
    Bullet b;
    b.radius = 6.0f;
    b.speed = kBulletSpeed;
    b.pos = QPointF(m_playerPos.x() + m_playerRadius + 6.0f, m_playerPos.y());
    m_bullets.push_back(b);
}

void MainWindow::movePlayerByDelta(const QPointF &delta)
{
    m_playerPos += delta * m_playerSpeedScale;

    m_playerPos.rx() = std::clamp(m_playerPos.x(), m_playerRadius + kPlayerPadding,
                                  width() - m_playerRadius - kPlayerPadding);
    m_playerPos.ry() = std::clamp(m_playerPos.y(), m_playerRadius + kPlayerPadding,
                                  height() - m_playerRadius - kPlayerPadding);
}

void MainWindow::handleCollisions()
{
    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        bool removed = false;
        for (int j = m_bullets.size() - 1; j >= 0; --j) {
            const QPointF diff = m_enemies[i].pos - m_bullets[j].pos;
            const float dist2 = diff.x() * diff.x() + diff.y() * diff.y();
            const float r = m_enemies[i].radius + m_bullets[j].radius;
            if (dist2 <= r * r) {
                m_enemies.removeAt(i);
                m_bullets.removeAt(j);
                ++m_score;
                removed = true;
                break;
            }
        }
        if (removed) {
            continue;
        }
    }

    for (const Enemy &e : m_enemies) {
        const QPointF diff = e.pos - m_playerPos;
        const float dist2 = diff.x() * diff.x() + diff.y() * diff.y();
        const float r = e.radius + m_playerRadius;
        if (dist2 <= r * r) {
            m_gameOver = true;
            break;
        }
    }
}
