// Copyright (c) 2022 The DECENOMY Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVATELABEL_H
#define PRIVATELABEL_H

#include "util.h"

#include <QEvent>
#include <QLabel>
#include <QObject>
#include <QString>
#include <QWidget>

class PrivateQLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(bool isPrivate READ isPrivate WRITE setIsPrivate)

private:
    bool _isPrivate = fPrivacyMode;
    bool _isHovered = false;
    QString _text;
    QString _masked = "*********";

    void refresh();

protected:
    void enterEvent(QEvent* ev) override;
    void leaveEvent(QEvent* ev) override;

public:
    explicit PrivateQLabel(QWidget* parent = Q_NULLPTR);
    explicit PrivateQLabel(const QString& text, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~PrivateQLabel();

    QString text() const;
    void setText(const QString& text);

    inline bool isPrivate() const { return _isPrivate; }
    void setIsPrivate(bool value);

    inline bool isHovered() const { return _isHovered; }
    void setIsHovered(bool value);
};

#endif // PRIVATELABEL_H
