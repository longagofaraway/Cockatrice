#pragma once

#include <QString>
#include <QVariantList>

QVariantList parsePT(const QString &pt);

QString addPowerToughness(const QVariantList &ptList, int deltaP, int deltaT);