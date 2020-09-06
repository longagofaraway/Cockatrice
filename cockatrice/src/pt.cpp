#include "pt.h"

QVariantList parsePT(const QString &pt)
{
    QVariantList ptList = QVariantList();
    if (!pt.isEmpty()) {
        int sep = pt.indexOf('/');
        if (sep == 0) {
            ptList.append(QVariant(pt.mid(1))); // cut off starting '/' and take full string
        } else {
            int start = 0;
            for (;;) {
                QString item = pt.mid(start, sep - start);
                if (item.isEmpty()) {
                    ptList.append(QVariant(QString()));
                } else if (item[0] == '+') {
                    ptList.append(QVariant(item.mid(1).toInt())); // add as int
                } else if (item[0] == '-') {
                    ptList.append(QVariant(item.toInt())); // add as int
                } else {
                    ptList.append(QVariant(item)); // add as qstring
                }
                if (sep == -1) {
                    break;
                }
                start = sep + 1;
                sep = pt.indexOf('/', start);
            }
        }
    }
    return ptList;
}

QString addPowerToughness(const QVariantList &ptList, int deltaP, int deltaT)
{
    QString newpt;
    if (ptList.isEmpty()) {
        newpt = QString::number(deltaP) + (deltaT ? "/" + QString::number(deltaT) : "");
    } else if (ptList.size() == 1) {
        newpt = QString::number(ptList.at(0).toInt() + deltaP) + (deltaT ? "/" + QString::number(deltaT) : "");
    } else {
        newpt = QString::number(ptList.at(0).toInt() + deltaP) + "/" + QString::number(ptList.at(1).toInt() + deltaT);
    }
    return newpt;
}